// genflux.cpp
//
// g++ -Wall genflux.cpp genfluxopt.cpp csvarray.cpp hist2d.cpp ncpng.cpp mytimer.cpp -o genflux
//
// Usage: genflux datafile.csv accfile.csv
//
// 2018-06-07 Command line E/O correction is added.
// 2018-06-04 Detector5 is added (VERSION = 102)
// 2017-07-17 output controls for png image file and gnuplot data file (VERSION = 101)
// 2017-06-30 VERSION added (VESION = 100)
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include "genfluxopt.h"
#include "csvarray.h"
#include "hist2d.h"
#include "mytimer.h"

static const int VERSION = 102;

enum
{
	ACCFORMAT_UNKNOWN = 0,
	ACCFORMAT_JAPC,
	ACCFORMAT_1F1,
	ACCFORMAT_GENACC0,
};

int main (int argc, char* argv[])
{
	bool eoout = true;
	bool self_eocorr = false;
	bool yreverse = false;
	int accformat = ACCFORMAT_UNKNOWN;

	double S0unit = 10000.0;	// cm^2

	mylibrary::GenFluxOpt opt;

	if (!opt.setargs (argc, argv))
	{
		opt.usage (std::cout, argv[0]);
		return (-1);
	}

	std::ifstream ifdat (opt.m_datfilename.c_str());
	if (!ifdat)
	{
		std::cerr << "ERROR: data file " << opt.m_datfilename << " cannot be opened." << std::endl;
		return (-2);
	}

	std::ifstream ifacc (opt.m_accfilename.c_str());
	if (!ifacc)
	{
		std::cerr << "ERROR: acc file " << opt.m_accfilename << " cannot be opened." << std::endl;
		return (-2);
	}

	mylibrary::MyTimer mytimer;
	mytimer.start ();

	mylibrary::CSVArray datcsv;
	datcsv.Read (ifdat);
	mylibrary::CSVArray acccsv;
	acccsv.Read (ifacc);

	if (acccsv.CellString (0, 8) == "Version(major,minor)")
	{
		// genacc0 for 1F2/1F3 and D5
		accformat = ACCFORMAT_GENACC0;
		if (acccsv.CellString (0, 11) == "Detector5")
		{
			std::cerr << "accfile is genacc0-D5" << std::endl;
			self_eocorr = false;
			S0unit = 1.0;
		}
		else
		{
			std::cerr << "accfile is genacc0" << std::endl;
			self_eocorr = true;
		}
	}
	else if (acccsv.CellString (1, 0) == "Datasetname")
	{
		// anaacc1f1 for 1F1
		std::cerr << "accfile is anaacc1f1" << std::endl;
		accformat = ACCFORMAT_1F1;
	}
	else
	{
		std::cerr << "ERROR: unknown accfile format" << std::endl;
		return (-1);
	}

	double daqtime;
	if (datcsv.CellString (1, 0) == "detector")
	{
		// Format for JAPC
		self_eocorr = false;
		daqtime = datcsv.CellDoubleValue (1, 6);
	}
	else if (datcsv.CellString (1, 4) == "duration(sec)")
	{
		// Format for 1F1
		yreverse = true;
		daqtime = datcsv.CellDoubleValue (1, 5);
	}
	else if (datcsv.CellString (1, 0) == "Duration(sec)")
	{
		daqtime = datcsv.CellDoubleValue (1, 1);
	}
	else
	{
		std::cerr << "ERROR: cannot read DAQTime from data file." << std::endl;
		return (-1);
	}

	// E/O correction
	if (opt.m_eocorr_given)
	{
		if (opt.m_eocorr == 0)
			self_eocorr = false;
		else
			self_eocorr = true;
	}

	std::cout << "DAQTime = " << daqtime << " sec., "
		<< (daqtime / 3600.0 / 24.0) << " days."
		<< std::endl;

	int ixoffs = 3;
	int iyoffs = 9;

	double xmin = acccsv.CellDoubleValue (4, 1);
	double xmax = acccsv.CellDoubleValue (4, 2);
	int nxbins = (int)(acccsv.CellLongValue (4, 3));
//	double xstep = (xmax - xmin) / (double)(nxbins);

	double ymin = acccsv.CellDoubleValue (5, 1);
	double ymax = acccsv.CellDoubleValue (5, 2);
	int nybins = (int)(acccsv.CellLongValue (5, 3));
//	double ystep = (ymax - ymin) / (double)(nybins);

	double xcorr [nxbins];
	double xcerr [nxbins];
	for (int ix = 0; ix < nxbins; ix++)
	{
		xcorr [ix] = 1.0;
		xcerr [ix] = 1.0;
	}

	double ycorr [nybins];
	double ycerr [nybins];
	for (int iy = 0; iy < nybins; iy++)
	{
		ycorr [iy] = 1.0;
		ycerr [iy] = 1.0;
	}

	if (self_eocorr)
	{
		for (int ix = 0; ix < nxbins; ix++)
		{
			double d2 = datcsv.CellDoubleValue (iyoffs - 2, ix + ixoffs);
			double d1 = d2;
			double d3 = d2;
			double ds = d2;
			if (ix > 0)
			{
				d1 = datcsv.CellDoubleValue (iyoffs - 2, ix - 1 + ixoffs);
				ds = ds + d1;
			}
			if (ix < (nxbins - 1))
			{
				d3 = datcsv.CellDoubleValue (iyoffs - 2, ix + 1 + ixoffs);
				ds = ds + d3;
			}
			double dt = d1 + d2 + d2 + d3;
			if (d2 != 0.0)
			{
				xcorr [ix] = d2 * 4.0 / dt;
				xcerr [ix] = xcorr [ix] / sqrt (ds);
			}
		}
		
		for (int iy = 0; iy < nybins; iy++)
		{
			double d2 = datcsv.CellDoubleValue (iy + iyoffs, 1);
			double d1 = d2;
			double d3 = d2;
			double ds = d2;
			if (iy > 0)
			{
				d1 = datcsv.CellDoubleValue (iy - 1 + iyoffs, 1);
				ds = ds + d1;
			}
			if (iy < (nybins - 1))
			{
				d3 = datcsv.CellDoubleValue (iy + 1 + iyoffs, 1);
				ds = ds + d3;
			}
			double dt = d1 + d2 + d2 + d3;
			if (d2 != 0.0)
			{
				ycorr [iy] = d2 * 4.0 / dt;
				ycerr [iy] = ycorr [iy] / sqrt (ds);
			}
		}
	}

	// parameters to covert tan()
	double xdiff;
	double ydiff;
	double xdist;
	double ydist;

	if (accformat == ACCFORMAT_GENACC0)
	{
		xdiff = acccsv.CellDoubleValue (1, 8);
		ydiff = acccsv.CellDoubleValue (1, 9);
		xdist = -acccsv.CellDoubleValue (1, 5);
		ydist = -acccsv.CellDoubleValue (1, 6);
	}
	else if (accformat == ACCFORMAT_1F1)
	{
		xdiff = acccsv.CellDoubleValue (1, 11);
		ydiff = acccsv.CellDoubleValue (1, 13);
		xdist = acccsv.CellDoubleValue (1, 15);
		ydist = acccsv.CellDoubleValue (1, 17);
	}

	double tnxmin = (xmin + xdiff) / xdist;
	double tnxmax = (xmax + xdiff) / xdist;
	double tnxstep = (tnxmax - tnxmin) / double (nxbins);
	double tnymin = (ymin + ydiff) / ydist;
	double tnymax = (ymax + ydiff) / ydist;
	double tnystep = (tnymax - tnymin) / double (nybins);

	static const int NUMHIST2D = 2;
	mylibrary::Hist2D hist2d [NUMHIST2D] =
	{
		mylibrary::Hist2D ("flux", tnxmin, tnxmax, nxbins, tnymin, tnymax, nybins),
		mylibrary::Hist2D ("flux-err", tnxmin, tnxmax, nxbins, tnymin, tnymax, nybins),
	};

	double f0 = S0unit / daqtime;

	for (int iy = 0; iy < nybins; iy++)
	{
		int jy = iy;
		if (yreverse)
			jy = nybins - 1 - iy;
		double yv = ((double)(jy) + 0.5) * tnystep + tnymin;
		for (int ix = 0; ix < nxbins; ix++)
		{
			double xv = ((double)(ix) + 0.5) * tnxstep + tnxmin;
			double datv = datcsv.CellDoubleValue (iy + iyoffs, ix + ixoffs);
			double accv = acccsv.CellDoubleValue (iy + iyoffs, ix + ixoffs);
			double flx = 0.0;
			double fle = 0.0;
			double c0 = xcorr [ix] * ycorr [iy] * accv;
			if (c0 > 0.0 )
			{
				if ((accv > opt.m_acccut) &&
					(datv > opt.m_datcut))
				{
					flx = datv / c0 * f0;
					fle = sqrt (datv) / c0 * f0;
				}
			}
			hist2d[0].cumulate (xv, yv, flx);
			hist2d[1].cumulate (xv, yv, fle);
		}
	}
	mytimer.stop ();

/// OUTPUT section

	std::string outname;
	if (opt.m_outname_given)
		outname = opt.m_outname;
	else
		outname = "flux";
	for (int i = 0; i < NUMHIST2D; i++)
	{
		std::stringstream ss;
		ss << outname << "-" << i;
		std::string ofnam;
		ss >> ofnam;
		std::string ofncsv;
		ofncsv = ofnam + ".csv";
		std::ofstream ofcsv (ofncsv.c_str());
		if (!ofcsv)
			std::cerr << "ERROR: " << ofncsv << " cannot be created." << std::endl;
		else
		{
			ofcsv.precision(16);
			ofcsv << "\"";
			for (int ia = 0; ia < argc; ia++)
			{
				if (ia)
					ofcsv << ' ';
				ofcsv << argv[ia];
			}
			ofcsv << "\""
				<< ",genflux,version"
				<< ',' << VERSION << ',';
				mytimer.csvout (ofcsv);
			ofcsv
				<< ",\"Compiled: "
				<< __DATE__ << ' ' << __TIME__ << "\""
				<< ",DataFilename"
				<< ',' << opt.m_datfilename
				<< ',' << opt.m_datcut
				<< ",AccFilename"
				<< ',' << opt.m_accfilename
				<< ',' << opt.m_acccut;
			if (self_eocorr)
				ofcsv << ",e/o corr,self";
			else
				ofcsv << ",e/o corr,none";
			ofcsv << std::endl;
			ofcsv
				<< "DAQTime(sec)"
				<< ',' << daqtime
				<< std::endl;
			hist2d[i].CSVdump (ofcsv);
		}
	}
	if (eoout)
	{
		std::string eoname = outname + "-eo.csv";
		std::ofstream ofeo (eoname.c_str());
		if (!ofeo)
		{
			std::cerr
				<< "ERROR: E/O correction file ("
				<< eoname
				<< ") cannot be created."
				<< std::endl;
		}
		else
		{
			ofeo.precision (16);
			ofeo << "E/O correction" << std::endl;

			ofeo << "X-corr,,";
			for (int ix = 0; ix < nxbins; ix++)
			{
				ofeo << ',' << xcorr[ix];
			}
			ofeo << std::endl;
			ofeo << "X-cerr,,";
			for (int ix = 0; ix < nxbins; ix++)
			{
				ofeo << ',' << xcerr[ix];
			}
			ofeo << std::endl;

			ofeo << '\n' << std::endl;

			ofeo << "Y-corr,,";
			for (int iy = 0; iy < nybins; iy++)
			{
				ofeo << ',' << ycorr[iy];
			}
			ofeo << std::endl;
			ofeo << "Y-cerr,,";
			for (int iy = 0; iy < nybins; iy++)
			{
				ofeo << ',' << ycerr[iy];
			}
			ofeo << std::endl;
		}
	}
	if (opt.m_png_out)
	{
		std::string pngname = outname + ".png";
		std::ofstream ofpng (pngname.c_str());
		if (!ofpng)
			std::cerr << "ERROR: pngfile (" << pngname << ") cannot be opened." << std::endl;
		else
			hist2d[0].PNGdump (ofpng);
	}
	if (opt.m_gpl_out)
	{
		std::string gplname = outname + ".gpl";
		std::ofstream ofgpl (gplname.c_str());
		if (!ofgpl)
			std::cerr << "ERROR: gplfile (" << gplname << ") cannot be opened." << std::endl;
		else
			hist2d[0].GPLdump (ofgpl);
	}
	return 0;
}
