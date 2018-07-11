// genfluxopt.h
//
// 2018-06-12 option for projection-plane data-file is added.
// 2018-06-07 Parameters for E/O correction are added
//

#ifndef GENFLUXOPT_H_INCLUDED
#define GENFLUXOPT_H_INCLUDED

#include <string.h>

namespace mylibrary
{
class GenFluxOpt
{
public:
	GenFluxOpt ();
	virtual ~GenFluxOpt ();
	bool setargs (int argc, char* argv[]);
	void reset ();
	std::ostream& usage (std::ostream& os, const char* pname) const;

public:

	// data filename
	bool m_datname_given;
	std::string m_datfilename;
	double m_datcut;	// cutoff value for data selection

	// acceptance filename
	bool m_accname_given;
	std::string m_accfilename;
	double m_acccut;	// cutoff value for mininum acc

	// output prefix name
	bool m_outname_given;
	std::string m_outname;

	// E/O correction
	bool m_eocorr_given;
	int m_eocorr;

	// Option for projection-plane histogram
	bool m_projection;

	// output controls
	bool m_png_out;
	bool m_gpl_out;
};

}	// namespace mylibrary

#endif	// GENFLUXOPT_H_INCLUDED
