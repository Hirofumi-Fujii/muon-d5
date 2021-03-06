// jokisch.cpp
//
// 2017-08-04 Bug fixed in the equations
//
// Cosmic muon flux at sea level
//
// Physical Review D Volume 19 Number 5, March 1979, pp.1368-1372
// Cosmic-ray muon spectrum up to 1 TeV at 75-deg zenith angle
// H. Jokisch, K. Carstensen, W. D. Dau, H. J. Meyer, and O. C. Allkofer
//
#include <cmath>
#include "jokisch.h"

namespace Jokisch
{
static const double xx = 2.57;

static const double xa = 451.0;
static const double xb =  77.2;
static const double xc =   9.2;
static const double xd =  19.8;

static const double xr = 35.0;
static const double xs = 57.3;
static const double xt = 11.4;
static const double xu =  8.9;

double
integral_flux (double p, double cs)
{
	// returns momentum integrated muon flux in (1/m^2/sec/str)
	// input:
	//	p	lower limit momentum in GeV/c (upper limit is infinity)
	//	cs	cos(zenith-angle)

	double g = p * cs;
	double f = xr / (g + xs)
			* pow ((cs / (5.0 * g + xt)), (xx - 1.0))
			* (g + (xu * 3.0 * cs)) / (g + xu * (2.0 * cs + 1.0)); 
	return (f * 10000.0);	// cm^2 -> m^2
}

double
differential_flux (double p, double cs)
{
	// returns muon flux in (1/m^2/sec/str/(GeV/c)
	// input
	//	p	momentum in GeV/c
	//	cs	cos(zenith-angle)

	double g = p * cs;
	double f = xa / (g + xb)
			 * pow (cs / (5.0 * g + xc), xx)
			 * (g + xd * cs) / (g + xd);
	return (f * 10000.0);	// cm^2 -> m^2
}

double
trx2pcut (double trx, double cs, double pcut0, double pmaxr)
{

	// get the integrated-flux(pcut0, cs)

	if (trx >= 1.0)
		return pcut0;
	if (trx <= 0.0)
		return pmaxr;

	double flux0 = Jokisch::integral_flux (pcut0, cs);
	double flux = flux0 * trx;
	double dflux = flux * 0.001;

	// search for p2 which satisfies the condition, flux < flux(p2).
	// assuming that the integral_flux() is monotonically increasing

	double p1 = pcut0;
	double p2 = 100.0;
	double f2 = Jokisch::integral_flux (p2, cs);

	while (f2 > flux)
	{
		if (p2 > pmaxr)
			return pmaxr;
		p1 = p2;
		p2 = p2 * 2.0;
		f2 = Jokisch::integral_flux (p2, cs);
	}

	// narrow the range keeping the condition (flux(p1) > flux > flux(p2)),
	// and assuming that the integral_flux() is monotonically increasing

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

}
