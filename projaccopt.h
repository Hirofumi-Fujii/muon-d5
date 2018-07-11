// ProjAccOpt.h

#ifndef PROJACCOPT_H_INCLUDED
#define PROJACCOPT_H_INCLUDED

#include <string.h>

namespace mylibrary
{
class ProjAccOpt
{
public:
	ProjAccOpt ();
	virtual ~ProjAccOpt ();
	bool setargs (int argc, char* argv[]);
	void reset ();
	std::ostream& usage (std::ostream& os, const char* pname) const;

public:
	// output name
	std::string m_outname;
	// combination (EOEO etc.) name
	std::string m_combname;
	// shift parameters
	double m_dxshift;
	double m_dyshift;
	double m_dzshift;	// average z-distance between units
	// distance parameters
	bool m_dist_given;
	double m_dist;
};

}	// namespace mylibrary

#endif	// PROJACCOPT_H_INCLUDED
