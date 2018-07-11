// gentrxopt.h

#ifndef GENTRXOPT_H_INCLUDED
#define GENTRXOPT_H_INCLUDED

#include <string.h>

namespace mylibrary
{
class GenTrxOpt
{
public:
	GenTrxOpt ();
	virtual ~GenTrxOpt ();
	bool setargs (int argc, char* argv[]);
	void reset ();
	std::ostream& usage (std::ostream& os, const char* pname) const;

public:

	// flux filename
	bool m_flxname_given;
	std::string m_flxfilename;

	// flux error filename
	bool m_errname_given;
	std::string m_errfilename;

	// output prefix name
	bool m_outname_given;
	std::string m_outname;

	// method (0: cos^2)
	int m_method;

	// parameters
	double m_p0;
	double m_p1;
};

}	// namespace mylibrary

#endif	// GENTRXOPT_H_INCLUDED
