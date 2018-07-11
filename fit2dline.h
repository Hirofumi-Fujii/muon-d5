// fit2dline.h
//
#ifndef FIT2DLINE_H_INCLUDED
#define FIT2DLINE_H_INCLUDED

#include <iostream>

namespace mylibrary
{

class Fit2DLine
{
public:
	Fit2DLine(int n, const double* x, const double* y);
	double a () const { return m_a; }
	double b () const { return m_b; }
	double y (double x) const { return ((m_a * x) + m_b); }

protected:
	double m_a;
	double m_b;
};
	
}	// namespace mylibrary

#endif	// FIT2DLINE_H_INCLUDED
