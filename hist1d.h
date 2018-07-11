// hist1d.h
#ifndef HIST1D_H_INCLUDED
#define HIST1D_H_INCLUDED
#include <ostream>
#include <string>

namespace mylibrary
{

class Hist1D
{
public:
	Hist1D ();
	Hist1D (const std::string& title,
		double xmin, double xmax, unsigned int xbins);
	Hist1D (const Hist1D& src);
	virtual ~Hist1D ();
	Hist1D& operator= (const Hist1D& src);
	double cumulate (double x, double weight = 1.0 );
	void clear ();
	double bin (int xbin) const;
	double total () const;
	std::ostream& CSVdump (std::ostream& os) const;

private:
	void setup (double xmin, double xmax, unsigned int xbins);

protected:
	std::string m_title;
	double m_xmin;
	double m_xmax;
	unsigned int m_xbins;
	double m_xscale;
	double m_wsum[2];
	double m_xsum[2];
	double m_xsqsum[2];
	double* m_bin;
};

}	// namespace mylibrary

#endif	// HIST1D_H_INCLUDED
