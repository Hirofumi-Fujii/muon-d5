// gentrx.cpp
//
// g++ -Wall gentrx.cpp gentrxopt.cpp csvarray.cpp hist2d.cpp ncpng.cpp mytimer.cpp -o gentrx
//
// Usage: gentrx -p0 value -p1 value flux.csv flux-error.csv
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include "gentrxopt.h"
#include "csvarray.h"
#include "hist2d.h"
#include "mytimer.h"

static const int VERSION = 100;

int main (int argc, char* argv[])
{
	mylibrary::GenTrxOpt opt;

	if (!opt.setargs (argc, argv))
	{
		opt.usage (std::cout, argv[0]);
		return (-1);
	}

	std::ifstream ifflx (opt.m_flxfilename.c_str());
	if (!ifflx)
	{
		std::cerr << "ERROR: flux file " << opt.m_flxfilename << " cannot be opened." << std::endl;
		return (-2);
	}

	std::ifstream iferr (opt.m_errfilename.c_str());
	if (!iferr)
	{
		std::cerr << "ERROR: flux-error file " << opt.m_errfilename << " cannot be opened." << std::endl;
		return (-2);
	}

	mylibrary::MyTimer mytimer;
	mytimer.start ();

	mylibrary::CSVArray flxcsv;
	flxcsv.Read (ifflx);
	mylibrary::CSVArray errcsv;
	errcsv.Read (iferr);

	int ixoffs = 3;
	int iyoffs = 9;

	double xmin = flxcsv.CellDoubleValue (4, 1);
	double xmax = flxcsv.CellDoubleValue (4, 2);
	int nxbins = (int)(flxcsv.CellLongValue (4, 3));
//	double xstep = (xmax - xmin) / (double)(nxbins);

	double ymin = flxcsv.CellDoubleValue (5, 1);
	double ymax = flxcsv.CellDoubleValue (5, 2);
	int nybins = (int)(flxcsv.CellLongValue (5, 3));
//	double ystep = (ymax - ymin) / (double)(nybins);

	static const int NUMHIST2D = 2;
	mylibrary::Hist2D hist2d [NUMHIST2D] =
	{
		mylibrary::Hist2D ("trx", xmin, xmax, nxbins, ymin, ymax, nybins),
		mylibrary::Hist2D ("trx-err", xmin, xmax, nxbins, ymin, ymax, nybins),
	};

	for (int iy = 0; iy < nybins; iy++)
	{
		double yv = flxcsv.CellDoubleValue (iy + iyoffs, 0);
		for (int ix = 0; ix < nxbins; ix++)
		{
			double xv = flxcsv.CellDoubleValue (iyoffs - 3, ix + ixoffs);
			double flxv = flxcsv.CellDoubleValue (iy + iyoffs, ix + ixoffs);
			double errv = errcsv.CellDoubleValue (iy + iyoffs, ix + ixoffs);
			double trx = 0.0;
			double tre = 0.0;
			double cs2 = (yv * yv) / (xv * xv + yv * yv + 1.0);
			double flux0 = opt.m_p0 * cs2 + opt.m_p1;
			if (flux0 > 0.0 )
			{
				trx = flxv / flux0;
				tre = errv / flux0;
			}
			hist2d[0].cumulate (xv, yv, trx);
			hist2d[1].cumulate (xv, yv, tre);
		}
	}
	mytimer.stop ();

/// OUTPUT section

	std::string outname;
	if (opt.m_outname_given)
		outname = opt.m_outname;
	else
		outname = "trx";
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
		{
			std::cerr
				<< "ERROR: output file ("
				<< ofncsv
				<< ") cannot be created."
				<< std::endl;
		}
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
			ofcsv
				<< "\""
				<< ",gentrx,version"
				<< ',' << VERSION << ',';
				mytimer.csvout (ofcsv);
			ofcsv
				<< ",\"Compiled: "
				<< __DATE__ << ' ' << __TIME__ << "\""
				<< ",FluxFilename"
				<< ',' << opt.m_flxfilename
				<< ",FluxErrorFilename"
				<< ',' << opt.m_errfilename
				<< std::endl;
			ofcsv
				<< "Method"
				<< ',' << opt.m_method
				<< ",p0"
				<< ',' << opt.m_p0
				<< ",p1"
				<< ',' << opt.m_p1
				<< std::endl;
			hist2d[i].CSVdump (ofcsv);
		}
	}
	std::string gplname = outname + ".gpl";
	std::ofstream ofgpl (gplname.c_str());
	if (!ofgpl)
	{
		std::cerr
			<< "ERROR: gplfile ("
			<< gplname
			<< ") cannot be created."
			<< std::endl;
	}
	else
		hist2d[0].GPLdump (ofgpl);
	return 0;
}
