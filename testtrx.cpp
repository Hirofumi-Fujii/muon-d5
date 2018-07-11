// testtrx.cpp
// g++ -Wall testtrx.cpp csvarray.cpp -o testtrx

#include <iostream>
#include <fstream>
#include "csvarray.h"

int main (int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " filename" << std::endl;
		return (-1);
	}

	std::ifstream ifs (argv[1]);
	if (!ifs)
	{
		std::cerr << "ERROR: input file " << argv[1] << " cannot be opened." << std::endl;
		return (-2);
	}

	mylibrary::CSVArray csvarray;
	csvarray.Read (ifs);
	int nlines = (int)(csvarray.m_array.size ());
	std::cout << nlines << std::endl;
	int histstart = -1;
	for (int i = 0; i < nlines; i++)
	{
			std::cout << i << ' ' << csvarray.CellString(i, 0) << std::endl;
			if (csvarray.CellString(i, 0) == "Hist2D::")
				histstart = i;
	}
	if (histstart >= 0)
	{
		std::cout << "Hist2D starts at " << histstart << std::endl;
		double xmin = csvarray.CellDoubleValue (histstart + 2, 1);
		double xmax = csvarray.CellDoubleValue (histstart + 2, 2);
		int nxbins = (int)(csvarray.CellLongValue (histstart + 2, 3));
		std::cout << "xmin=" << xmin << " xmax=" << xmax << " nxbins=" << nxbins << std::endl;
		double ymin = csvarray.CellDoubleValue (histstart + 3, 1);
		double ymax = csvarray.CellDoubleValue (histstart + 3, 2);
		int nybins = (int)(csvarray.CellLongValue (histstart + 3, 3));
		std::cout << "ymin=" << ymin << " ymax=" << ymax << " nybins=" << nybins << std::endl;
	}
	return 0;
}
