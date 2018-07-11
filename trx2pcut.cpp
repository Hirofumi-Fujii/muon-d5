// trx2pcut.cpp
// g++ -Wall trx2pcut.cpp jokisch.cpp -o trx2pcut
//
// Get cut-off momentum from transmittance

#include <iostream>
#include <sstream>
#include <cmath>
#include "jokisch.h"

int main(int argc, char* argv[])
{
	double trx;
	double cs;
	double pcut0;

	pcut0 = 0.1;

	std::stringstream ss1(argv[1]);
	std::stringstream ss2(argv[2]);

	if (!(ss1 >> trx))
		return (-1);

	if (!(ss2 >> cs))
		return (-1);

	
	// get the integrated-flux(pcut0, cs)
	double flux0 = Jokisch::integral_flux (pcut0, cs);
	double flux = flux0 * trx;
	double dflux = flux * 0.01;
	std::cout << "trx=" << trx << ", cs=" << cs << ", flux0=" << flux0
		<< ", flux=" << flux << std::endl;

	// start p
	double p1 = pcut0;
	double p2 = 100.0;

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
	std::cout << "pcut=" << p << ", f=" << f << ", ntry=" << ntry << std::endl;
	return 0;
}
	