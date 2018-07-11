// fit2dline.cpp

#include "fit2dline.h"
#include <cfloat>

namespace mylibrary
{

Fit2DLine::Fit2DLine (int n, const double *x, const double *y)
{
	m_a = DBL_MAX;
	m_b = DBL_MAX;
	if ((n < 2) || (x == 0) || (y == 0))
		return;

	double A11 = 0.0;
	double A12 = 0.0;
	double A22 = (double)(n);

	double B1 = 0.0;
	double B2 = 0.0;

	for (int i = 0; i < n; i++)
	{
		double xi = *x++;
		double yi = *y++;
		A11 = (xi * xi) + A11;
		A12 = xi + A12;
		B1 = (xi * yi) + B1;
		B2 = yi + B2;
	}
	double det = (A11 * A22) - (A12 * A12);
	if (det == 0.0)
		return;
	m_a = ((A22 * B1) - (A12 * B2)) / det;
	m_b = ((A11 * B2) - (A12 * B1)) / det; 
}

}	// namespace mylibrary
