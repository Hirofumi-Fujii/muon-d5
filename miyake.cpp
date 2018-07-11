// miyake.cpp

#include <cmath>
#include "miyake.h"

namespace Miyake
{
static const double mumass = 0.10566;	// mass of muon in GeV/(c^2)

double
differential_flux (double pmu, double cs)
{
	double energy = sqrt ((pmu * pmu) + (mumass * mumass));
	double te = energy - mumass;	// Kinetic energy
	double a0 = cs / (5.0 * te * cs + 10.0);
	double a1 = pow (a0, 2.57);
	double a2 = (372.0 / (5.0 * te * cs + 80.0)) * a1;	
	double a3 = ((te + 15.0) * cs) / ((te * cs) + 15.0);
	return (a2 * a3 * 10000.0);
}

double
integral_flux (double pmu, double cs)
{
	double energy = sqrt ((pmu * pmu) + (mumass * mumass));
	double te = energy - mumass;	// Kinetic energy
	double a0 = cs / (5.0 * te * cs + 10.0);
	double a1 = pow (a0, 1.57);
	double a2 = (174.0 / (5.0 * te * cs + 400.0)) * a1;	
	double a3 = ((te + 15.0) * cs) / ((te + 10.0) * cs + 5.0);
	return (a2 * a3 * 10000.0) * (pmu / energy);
}

}	// namespace Miyake
