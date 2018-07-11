// hist2d.h
//
// 'm_bin' is the body to store the value. Its size is (m_xbins + 3) * (m_ybins + 3) and
// the index assignments are
//    0                 total (in 1-dim)
//    1                 underflow
//    2 - (m_xbins + 1) normal region
//    (m_xbins + 2)     overflow

#ifndef HIST2D_H_INCLUDED
#define HIST2D_H_INCLUDED
#include <ostream>
#include <string>

namespace mylibrary
{

class Hist2D
{
public:
	Hist2D ();
	Hist2D (const std::string& title,
		double xmin, double xmax, unsigned int xbins,
		double ymin, double ymax, unsigned int ybins,
		bool smooth = false );
	Hist2D (const Hist2D& src);
	virtual ~Hist2D ();
	Hist2D& operator= (const Hist2D& src);
	double cumulate (double x, double y, double weight = 1.0 );
	void clear ();
	const std::string& title () const { return m_title; }
	double bin (int xbin, int ybin) const;
	double total () const;
	std::ostream& CSVdump (std::ostream& os) const;
	std::ostream& PNGdump (std::ostream& os) const;
	std::ostream& GPLdump (std::ostream& os) const;

protected:
	double cums (double x, double y, double weight);
	void setup (double xmin, double xmax, unsigned int xbins,
		double ymin, double ymax, unsigned int ybins);
	void copystatistics (const Hist2D& src);

protected:
	std::string m_title;
	double m_xmin;
	double m_xmax;
	unsigned int m_xbins;
	double m_ymin;
	double m_ymax;
	unsigned int m_ybins;
	double m_xscale;
	double m_yscale;
	bool m_smooth;
	double m_wsum[2];
	double m_xsum[2];
	double m_ysum[2];
	double m_xsqsum[2];
	double m_ysqsum[2];
	double* m_bin;
};

}	// namespace mylibrary

#endif	// HIST2D_H_INCLUDED
