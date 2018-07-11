// hist2d.cpp
#include "hist2d.h"
#include "ncpng.h"
#include <cmath>

namespace mylibrary
{

Hist2D::Hist2D () :
	m_title ("")
{
	m_smooth = false;
	setup (0.0, 1.0, 1, 0.0, 1.0, 1);
	clear ();
}

Hist2D::Hist2D (const std::string& title,
		double xmin, double xmax, unsigned int xbins,
		double ymin, double ymax, unsigned int ybins,
		bool smooth ) :
	m_title(title)
{
	m_smooth = smooth;
	setup (xmin, xmax,  xbins, ymin, ymax, ybins);
	clear ();
}

Hist2D::Hist2D (const Hist2D& src) :
	m_title (src.m_title)
{
	m_smooth = src.m_smooth;
	setup (src.m_xmin, src.m_xmax, src.m_xbins,
		src.m_ymin, src.m_ymax, src.m_ybins);
	copystatistics (src);
}

Hist2D::~Hist2D()
{
	delete [] m_bin;
}

Hist2D&
Hist2D::operator= (const Hist2D& src)
{
	if (&src == this)
		return *this;
	delete [] m_bin;
	m_bin = 0;

	m_title = src.m_title;
	m_smooth = src.m_smooth;
	setup (src.m_xmin, src.m_xmax, src.m_xbins,
		src.m_ymin, src.m_ymax, src.m_ybins);
	copystatistics (src);
	return *this;
}

void
Hist2D::setup (double xmin, double xmax, unsigned int xbins,
	double ymin, double ymax, unsigned int ybins)
{
	m_xscale = 1.0;
	m_yscale = 1.0;
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

	if (ymin > ymax)
	{
		m_ymin = ymax;
		m_ymax = ymin;
	}
	else
	{
		m_ymin = ymin;
		m_ymax = ymax;
	}
	m_ybins = ybins;
	if (ybins == 0)
		m_ybins = 1;

	if (m_xmax != m_xmin)
		m_xscale = (double)m_xbins / (m_xmax - m_xmin);
	if (m_ymax != m_ymin)
		m_yscale = (double)m_ybins / (m_ymax - m_ymin);
	//
	// allocate (xbins + 3) * (ybins + 3)
	// +3 means total, underflow and overflow
	unsigned long l = (m_xbins + 3) * (m_ybins + 3);
	m_bin = new double [l];
}

void
Hist2D::copystatistics (const Hist2D& src)
{
	// Copy histogram contents and statistical values from src.
	for (int i = 0; i < 2; i++)
	{
		m_wsum[i] = src.m_wsum[i];
		m_xsum[i] = src.m_xsum[i];
		m_ysum[i] = src.m_ysum[i];
		m_xsqsum[i] = src.m_xsqsum[i];
		m_ysqsum[i] = src.m_ysqsum[i];
	}
	unsigned long l = 0;
	for (unsigned int y = 0; y < (m_ybins + 3); y++)
	{
		for (unsigned int x = 0; x < (m_xbins + 3); x++)
		{
			m_bin[l] = src.m_bin[l];
			++l;
		}
	}
}

void
Hist2D::clear()
{
	for (int i = 0; i < 2; i++)
	{
		m_wsum[i] = 0.0;
		m_xsum[i] = 0.0;
		m_ysum[i] = 0.0;
		m_xsqsum[i] = 0.0;
		m_ysqsum[i] = 0.0;
	}
	unsigned long l = 0;
	for (unsigned int y = 0; y < (m_ybins + 3); y++)
		for (unsigned int x = 0; x < (m_xbins + 3); x++)
			m_bin[l++] = 0.0;
}

double
Hist2D::cumulate(double x, double y, double weight)
{
	// whole total
	m_bin[0] += weight;

	// statistics
	m_wsum[0] += weight;
	m_xsum[0] += (x * weight);
	m_ysum[0] += (y * weight);
	m_xsqsum[0] += (x * x * weight);
	m_ysqsum[0] += (y * y * weight);

	if (m_smooth)
		return cums(x, y, weight);

	bool xin = false;
	bool yin = false;

	unsigned int ix;
	unsigned int iy;
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
	if (y < m_ymin)
		iy = 1;
	else if (y > m_ymax)
		iy = m_ybins + 2;
	else
	{
		iy = (unsigned int)((y - m_ymin) * m_yscale);
		if ( iy >= m_ybins )
			iy = m_ybins - 1;
		iy += 2;
		yin = true;
	}
	if (xin && yin)
	{
		m_wsum[1] += weight;
		m_xsum[1] += (x * weight);
		m_ysum[1] += (y * weight);
		m_xsqsum[1] += (x * x * weight);
		m_ysqsum[1] += (y * y * weight);
	}
	unsigned long lx = (unsigned long)ix;
	unsigned long ly = (unsigned long)iy * (m_xbins + 3);
	m_bin[lx] += weight;
	m_bin[ly] += weight;
	m_bin[lx + ly] += weight;
	return m_bin[lx + ly];
}

double
Hist2D::cums(double x, double y, double weight)
{

	bool xin = false;
	bool yin = false;

	double rx = (x - m_xmin) * m_xscale;
	double ry = (y - m_ymin) * m_yscale;
	double wx = 1.0;
	double wy = 1.0;

	unsigned int ix;
	unsigned int iy;

	unsigned int ix1;
	unsigned int iy1;

	if (x < m_xmin)
	{
		ix = 1;
		ix1 = ix;
	}
	else if (x > m_xmax)
	{
		ix = m_xbins + 2;
		ix1 = ix;
	}
	else
	{
		ix = (unsigned int)(rx);
		if ( ix >= m_xbins )
			ix = m_xbins - 1;
		if ((rx - double(ix)) < 0.5)
		{
			ix1 = ix - 1;
			wx = rx - double(ix) + 0.5;
		}
		else
		{
			ix1 = ix + 1;
			wx = double(ix1) - rx + 0.5;
		}
		ix += 2;
		ix1 += 2;
		xin = true;
	}
	if (y < m_ymin)
	{
		iy = 1;
		iy1 = iy;
	}
	else if (y > m_ymax)
	{
		iy = m_ybins + 2;
		iy1 = iy;
	}
	else
	{
		iy = (unsigned int)(ry);
		if ( iy >= m_ybins )
			iy = m_ybins - 1;
		if ((ry - double(iy)) < 0.5)
		{
			iy1 = iy - 1;
			wy = ry - double(iy) + 0.5;
		}
		else
		{
			iy1 = iy + 1;
			wy = double(iy1) - ry + 0.5;
		}
		iy += 2;
		iy1 += 2;
		yin = true;
	}
	if (wx < 0.0)
		wx = 0.0;
	if (wy < 0.0)
		wy = 0.0;
	if (wx > 1.0)
		wx = 1.0;
	if (wy > 1.0)
		wy = 1.0;
	if (xin && yin)
	{
		m_wsum[1] += weight;
		m_xsum[1] += (x * weight);
		m_ysum[1] += (y * weight);
		m_xsqsum[1] += (x * x * weight);
		m_ysqsum[1] += (y * y * weight);
	}
	unsigned long lx = (unsigned long)ix;
	unsigned long ly = (unsigned long)iy * (m_xbins + 3);
	double w = weight * wx * wy;
	m_bin[lx] += w;
	m_bin[ly] += w;
	m_bin[lx + ly] += w;
	double cntval = m_bin[lx + ly];
	lx = (unsigned long)ix1;
	w = weight * (1.0 - wx) * wy;
	m_bin[lx] += w;
	m_bin[ly] += w;
	m_bin[lx + ly] += w;
	lx = (unsigned long)ix;
	ly = (unsigned long)iy1 * (m_xbins + 3);
	w = weight * wx * (1.0 - wy);
	m_bin[lx] += w;
	m_bin[ly] += w;
	m_bin[lx + ly] += w;
	lx = (unsigned long)ix1;
	ly = (unsigned long)iy1 * (m_xbins + 3);
	w = weight * (1.0 - wx) * (1.0 - wy);
	m_bin[lx] += w;
	m_bin[ly] += w;
	m_bin[lx + ly] += w;

	return cntval;
}

double
Hist2D::bin(int xbin, int ybin) const
{
	if ((xbin < 0) || (xbin >= (int)m_xbins) || 
		(ybin < 0) || (ybin >= (int)m_ybins))
		return 0.0;
	unsigned long l = (unsigned long)(ybin + 2) * (m_xbins + 3)
			+ (unsigned long)xbin + 2;
	return m_bin[l];
}

double
Hist2D::total() const
{
	return m_bin[0];
}

std::ostream& 
Hist2D::CSVdump (std::ostream& os) const
{
	unsigned int noldprec = os.precision();
	os.precision(16);

	os << "\"Hist2D::\",\"" << m_title << "\"" << std::endl;
	os << "\"X/Y\",\"min\",\"max\",\"bins\""
		<< ",\"wtotal(all)\",\"av.(all)\",\"std.(all)\""
		<< ",\"wtotal(in)\",\"av.(in)\",\"std.(in)\""
		<< ",\"smoothing\""
		<< std::endl;
	os << "\"X\"," << m_xmin << "," << m_xmax << "," << m_xbins;
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
	if (m_smooth)
		os << ",\"true\"";
	else
		os << ",\"false\"";
	os << std::endl;
	os << "\"Y\"," << m_ymin << "," << m_ymax << "," << m_ybins;
	for (int i = 0; i < 2; i++)
	{
		double av = 0.0;
		double sd = 0.0;
		if (m_wsum[i] > 0.0)
		{
			av = m_ysum[i] / m_wsum[i];
			sd = (m_ysqsum[i] / m_wsum[i])  - (av * av);
			if (sd > 0.0)
				sd = sqrt(sd);
		}
		os << "," << m_wsum[i] << "," << av << "," << sd;
	}
	os << std::endl;

	double dx = (m_xmax - m_xmin) / m_xbins;
	double dy = (m_ymax - m_ymin) / m_ybins;

	os << "\"Y/X\",\"Total\",\"Under\",";
	double x0 = m_xmin + (dx * 0.5);
	double y0 = m_ymin + (dy * 0.5);
	for (unsigned int x = 0; x < m_xbins; x++)
	{
		double d = dx * x + x0;
		os << d << ",";
	}
	os << "\"Over\"" << std::endl;

	unsigned long l = 0;
	for (unsigned int y = 0; y < (m_ybins + 3); y++)
	{
		if (y == 0)
			os << "\"Total\",";
		else if (y == 1)
			os << "\"Under\",";
		else if (y == (m_ybins + 2))
			os << "\"Over\",";
		else
		{
			double d = dy * (y - 2) + y0;
			os << d << ",";
		}
		for (unsigned int x = 0; x < (m_xbins + 3); x++)
		{
			os << m_bin[l++];
			if (x < (m_xbins + 2))
				os << ",";
			else
				os << std::endl;
		}
	}
	os.precision(noldprec);
	return os;
}

std::ostream&
Hist2D::PNGdump (std::ostream& os) const
{
	// Serch for min. and max. values in the normal region
	double vmin = m_bin[(m_xbins + 3) * 2 + 2];
	double vmax = vmin;
	for (unsigned int y = 2; y < (m_ybins + 2); y++)
	{
		unsigned long l = (unsigned long)y * (unsigned long)(m_xbins + 3);
		for (unsigned int x = 2; x < (m_xbins + 2); x++)
		{
			double v = m_bin[l + x];
			if (v < vmin)
				vmin = v;
			if (v > vmax)
				vmax = v;
		}
	}
	double dv = vmax - vmin;
	if (dv == 0.0)
		dv = 1.0;

	mypnglib::NCPNG pngimg (m_xbins, m_ybins, mypnglib::NCPNG::GREYSCALE, 16);
	for (unsigned int j = 2; j < (m_ybins + 2); j++)
	{
		int y = int(m_ybins + 1 - j);
		unsigned long l = (unsigned long)j * (unsigned long)(m_xbins + 3);
		for (unsigned int i = 2; i < (m_xbins + 2); i++)
		{
			int x = int(i - 2);
			double v = (m_bin[l + i] - vmin) / dv;
			if (v > 1.0)
				v = 1.0;
			else if (v < 0.0)
				v = 0.0;
			mypnglib::NCPNG::GREY_PIXEL pixel(v);
			pngimg.put (x, y, pixel);
		}
	}
	pngimg.write (os);
	return os;
}

std::ostream& 
Hist2D::GPLdump(std::ostream& os) const
{
	// output for gnuplot pm3d map

	unsigned int noldprec = os.precision();
	os.precision(16);

	double dx = (m_xmax - m_xmin) / m_xbins;
	double dy = (m_ymax - m_ymin) / m_ybins;
	double x0 = m_xmin + (dx * 0.5);
	double y0 = m_ymin + (dy * 0.5);

	for (unsigned int y = 0; y < m_ybins; y++)
	{
		double yv = dy * y + y0;
		for (unsigned int x = 0; x < m_xbins; x++)
		{
			double xv = dx * x + x0;
			os
				<< ' ' << xv
				<< ' ' << yv
				<< ' ' << bin(int(x), int(y)) << std::endl;
		}
		os << std::endl;
	}
	os.precision(noldprec);
	return os;
}

}	// namespace mylibrary
