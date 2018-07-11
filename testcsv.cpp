// testcsv.cpp
// g++ -Wall testcsv.cpp csvarray.cpp -o testcsv

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

	std::cout << "Lines=" << nlines << std::endl;

	for (int ny = 0; ny < nlines; ny++)
	{
		int ncols = (int)(csvarray.m_array[ny].size());
		std::cout << "Line=" << ny << " Cols=" << ncols;
		for (int nx = 0; nx < ncols; nx++)
			std::cout << " [" << nx << "]" << csvarray.m_array[ny][nx];
		std::cout << std::endl;
	}
	return 0;
}
