
// hist1d.cpp
#include "hist1d.h"
#include <cmath>

namespace mylibrary
{
Hist1D::Hist1D () :
	m_title ("")
{
	setup (0.0, 1.0, 1);
	clear ();
}

Hist1D::Hist1D (const std::string& title,
		double xmin, double xmax, unsigned int xbins ) :
	m_title (title)
{
	setup (xmin, xmax, xbins);
	clear ();
}

Hist1D::Hist1D (const Hist1D& src) :
	m_title (src.m_title)
{
	setup (src.m_xmin, src.m_xmax, src.m_xbins);
	// copy the contents
	for (int i = 0; i < 2; i++)
	{
		m_wsum[i] = src.m_wsum[i];
		m_xsum[i] = src.m_xsum[i];
		m_xsqsum[i] = src.m_xsqsum[i];
	}
	unsigned long l = 0;
	for (unsigned int x = 0; x < (m_xbins + 3); x++)
	{
		m_bin[l] = src.m_bin[l];
		++l;
	}
}

Hist1D::~Hist1D()
{
	delete [] m_bin;
}

Hist1D&
Hist1D::operator= (const Hist1D& src)
{
	if( &src == this )
		return *this;

	// destruct previos object
	delete [] m_bin;
	m_bin = 0;

	// setup for new object
	setup (src.m_xmin, src.m_xmax, src.m_xbins);

	// copy the contents
	m_title = src.m_title;
	for (int i = 0; i < 2; i++)
	{
		m_wsum[i] = src.m_wsum[i];
		m_xsum[i] = src.m_xsum[i];
		m_xsqsum[i] = src.m_xsqsum[i];
	}
	unsigned long l = 0;
	for (unsigned int x = 0; x < (m_xbins + 3); x++)
	{
		m_bin[l] = src.m_bin[l];
		++l;
	}
	return *this;
}

void
Hist1D::clear()
{
	for (int i = 0; i < 2; i++)
	{
		m_wsum[i] = 0.0;
		m_xsum[i] = 0.0;
		m_xsqsum[i] = 0.0;
	}
	unsigned long l = 0;
	for (unsigned int x = 0; x < (m_xbins + 3); x++)
		m_bin[l++] = 0.0;
}

void
Hist1D::setup (double xmin, double xmax, unsigned int xbins)
{
	// set default value
	m_xscale = 1.0;
	m_bin = 0;

	if (xmin > xmax)
	{
		m_xmin = xmax;
		m_xmax = xmin;
	}
	else
	{
		m_xmin = xmin;
		m_xmax = xmax;
	}
	m_xbins = xbins;
	if (xbins == 0)
		m_xbins = 1;
	if (m_xmax != m_xmin)
		m_xscale = (double)m_xbins / (m_xmax - m_xmin);
	//
	// allocate (xbins + 3)
	// +3 means total, underflow and overflow
	unsigned long l = (m_xbins + 3);
	m_bin = new double [l];
}

double
Hist1D::cumulate(double x, double weight)
{
	// whole total
	m_bin[0] += weight;

	// statistics
	m_wsum[0] += weight;
	m_xsum[0] += (x * weight);
	m_xsqsum[0] += (x * x * weight);

	bool xin = false;

	unsigned int ix;
	if (x < m_xmin)
		ix = 1;
	else if (x > m_xmax)
		ix = m_xbins + 2;
	else
	{
		ix = (unsigned int)((x - m_xmin) * m_xscale);
		if ( ix >= m_xbins )
			ix = m_xbins - 1;
		ix += 2;
		xin = true;
	}
	if (xin)
	{
		m_wsum[1] += weight;
		m_xsum[1] += (x * weight);
		m_xsqsum[1] += (x * x * weight);
	}
	unsigned long lx = (unsigned long)ix;
	m_bin[lx] += weight;
	return m_bin[lx];
}

double
Hist1D::bin(int xbin) const
{
	if ((xbin < 0) || (xbin >= (int)m_xbins))
		return 0.0;
	unsigned long l = (unsigned long)xbin + 2;
	return m_bin[l];
}

double
Hist1D::total() const
{
	return m_bin[0];
}

std::ostream& 
Hist1D::CSVdump(std::ostream& os) const
{
	unsigned int noldprec = os.precision();
	os.precision(10);

	os << "\"Hist1D::\",\"" << m_title << "\"" << std::endl;
	os << "\"min\",\"max\",\"bins\""
		<< ",\"wtotal(all)\",\"av.(all)\",\"std.(all)\""
		<< ",\"wtotal(in)\",\"av.(in)\",\"std.(in)\""
		<< std::endl;
	os << m_xmin << "," << m_xmax << "," << m_xbins;
	for (int i = 0; i < 2; i++)
	{
		double av = 0.0;
		double sd = 0.0;
		if (m_wsum[i] > 0.0)
		{
			av = m_xsum[i] / m_wsum[i];
			sd = (m_xsqsum[i] / m_wsum[i]) - (av * av);
			if (sd > 0.0)
				sd = sqrt(sd);
		}
		os << "," << m_wsum[i] << "," << av << "," << sd;
	}
	os << std::endl;

	double dx = (m_xmax - m_xmin) / m_xbins;

	os << "\"Total\",\"Under\",";
	double x0 = m_xmin + (dx * 0.5);
	for (unsigned int x = 0; x < m_xbins; x++)
	{
		double d = dx * x + x0;
		os << d << ",";
	}
	os << "\"Over\"" << std::endl;

	unsigned long l = 0;
	for (unsigned int x = 0; x < (m_xbins + 3); x++)
	{
		os << m_bin[l++];
		if (x < (m_xbins + 2))
			os << ",";
		else
			os << std::endl;
	}
	os.precision(noldprec);
	return os;
}

}	// namespace mylibrary
