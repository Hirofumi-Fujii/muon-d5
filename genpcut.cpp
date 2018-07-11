// genpcut.cpp
// g++ -Wall genpcut.cpp csvarray.cpp jokisch.cpp -o genpcut

#include <iostream>
#include <fstream>
#include <cmath>
#include "csvarray.h"
#include "jokisch.h"

double
trx2pcut (double trx, double cs, double pcut0)
{
	// get the integrated-flux(pcut0, cs)
	double flux0 = Jokisch::jokisch_integral (pcut0, cs);
	double flux = flux0 * trx;
	double dflux = flux * 0.01;
//@	std::cout << "trx=" << trx << ", cs=" << cs << ", flux0=" << flux0
//@		<< ", flux=" << flux << std::endl;

	// start p
	double p1 = pcut0;
	double p2 = 100.0;
	double f2 = Jokisch::integral_flux (p2, cs);

	while (f2 > flux)
	{
		p1 = p2;
		p2 = p2 * 2.0;
		f2 = Jokisch::integral_flux (p2, cs);
	}

	double p = (p1 + p2) / 2.0;
	double f = Jokisch::integral_flux (p, cs);

	int ntry = 0;
	while ( fabs(f - flux) > dflux )
	{
		ntry += 1;
		if (f > flux)
			p1 = p;
		else
			p2 = p;
		p = (p1 + p2) / 2.0;
		f = Jokisch::integral_flux (p, cs);
	}
	return p;
}

int main (int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " trxfilename" << std::endl;
		return (-1);
	}

	std::ifstream ifs (argv[1]);
	if (!ifs)
	{
		std::cerr << "ERROR: input file " << argv[1] << " cannot be opened." << std::endl;
		return (-2);
	}

	mylibrary::CSVArray trxcsv;
	trxcsv.Read (ifs);
	int nlines = (int)(trxcsv.m_array.size ());
	std::cout << "Total lines=" << nlines << std::endl;
	if ((nlines < 1) || trxcsv.CellString(0, 0) != "transmittance")
	{
		std::cerr << "ERROR: this is not a transmittance file, lines "
			<< nlines << ", " << trxcsv.CellString(0, 0)
			<< std::endl;
		return (-2);
	}

	int histstart = -1;
	for (int i = 0; i < nlines; i++)
	{
//			std::cout << i << ' ' << trxcsv.CellString(i, 0) << std::endl;
			if (trxcsv.CellString(i, 0) == "Hist2D::")
				histstart = i;
	}
	if (histstart < 0)
	{
		std::cerr << "ERROR: Hist2D is not found."
			<< std::endl;
		return (-2);
	}

	std::cout << "Hist2D starts at " << histstart << std::endl;
	double xmin = trxcsv.CellDoubleValue (histstart + 2, 1);
	double xmax = trxcsv.CellDoubleValue (histstart + 2, 2);
	int nxbins = (int)(trxcsv.CellLongValue (histstart + 2, 3));
	std::cout << "xmin=" << xmin << " xmax=" << xmax << " nxbins=" << nxbins << std::endl;

	double ymin = trxcsv.CellDoubleValue (histstart + 3, 1);
	double ymax = trxcsv.CellDoubleValue (histstart + 3, 2);
	int nybins = (int)(trxcsv.CellLongValue (histstart + 3, 3));
	std::cout << "ymin=" << ymin << " ymax=" << ymax << " nybins=" << nybins << std::endl;


	int ixoffs = 3;
	int iyoffs = histstart + 7;
	
	double trxmin = 2.0;
	double trxmax = 0.0;

	double pcut0 = 0.0;
	double pcut = pcut0;

	double udistx = 474.0;
	double udisty = 516.0;
	double ydiff = 139.0;
	double xdiff = 0.0;

	double xstep = (xmax - xmin) / double (nxbins);
	double ystep = (ymax - ymin) / double (nybins);
	double dx0 = xmin + (xstep * 0.5);
	double dy0 = ymin + (ystep * 0.5);

	for (int ny = 0; ny < nybins; ny++)
	{
		std::cout << "Line" << ny;
		double dy = ystep * double (ny) + dy0 + ydiff;
		for (int nx = 0; nx < nxbins; nx++)
		{
			double trx = trxcsv.CellDoubleValue (ny + iyoffs, nx + ixoffs);
			if (trx < trxmin)
				trxmin = trx;
			if (trx > trxmax)
				trxmax = trx;
			if ((trx > 0.005) && (trx < 0.995))
			{
				double dx = xstep * double (nx) + dx0 + xdiff;
				double rdy = dy / udisty;
				double rdx = dx / udistx;
				double cs2f = 1.0 / (rdx * rdx + 1.0);
				double cs2t = (rdy * rdy * cs2f) / (rdy * rdy * cs2f + 1.0);
				double cs = sqrt (cs2t);
				pcut = trx2pcut (trx, cs, pcut0);
			}
			else
				pcut = pcut0;
			std::cout << ',' << pcut;
		}
		std::cout << std::endl;
	}
	std::cout << "Transmittance min=" << trxmin << " max=" << trxmax
		<< std::endl;
	return 0;
}
