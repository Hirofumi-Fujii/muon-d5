// simacc.cpp
//
// g++ -Wall simacc.cpp mersenne.cpp hist2d.cpp ncpng.cpp -o simacc
//

#include <iostream>
#include <ctime>
#include "hist2d.h"
#include "randomc.h"

static const double PI = 3.1415926536;

int main (int argc, char* argv[])
{
	static const int numhist2d = 1;
	mylibrary::Hist2D hist2d [numhist2d] =
	{
		mylibrary::Hist2D ("phi-cos(theta)", -PI, PI, 100, -1.0, 1.0, 100),
	};

	int maxevents = 1000;

	CRandomMersenne unirand(time(0));
	for (int i = 0; i < maxevents; i++)
	{
		double phi = unirand.Random () * PI * 2.0 - PI;
		double cst = unirand.Random () * 2.0 - 1.0;
		hist2d[0].cumulate (phi, cst);
	}
	
	for (int ihist = 0; ihist < numhist2d; ihist++)
	{
		hist2d[ihist].CSVdump (std::cout);
	}
	return 0;	
}
