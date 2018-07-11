// jokisch.h
#ifndef JOKISCH_H_INCLUDED
#define JOKISCH_H_INCLUDED
namespace Jokisch
{
double integral_flux (double p, double cs);
double differential_flux (double p, double cs);
double trx2pcut (double trx, double cs, double pcut0, double pmaxr);
}
#endif	// JOKISCH_H_INCLUDED
