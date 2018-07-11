// gentrxxopt.cpp

#include <iostream>
#include <sstream>
#include <string>

#include "gentrxopt.h"

namespace mylibrary
{

GenTrxOpt::GenTrxOpt ()
{
	reset ();
}

GenTrxOpt::~GenTrxOpt ()
{
}

std::ostream&
GenTrxOpt::usage (std::ostream& os, const char* pname) const
{
	os << "Usage: "
		<< pname
		<< " [options] flux_filename flux-error_filename\n"
		<< "Options:\n"
		<< " -out string  \t leading name for the output files (name)\n"
		<< " -p0 value \t propotional parameter for flux calculation\n"
		<< " -p1 value \t constant parameter for flux calculation\n"
		<< std::endl;
	return os;
}

void
GenTrxOpt::reset ()
{
	// flux filename
	m_flxname_given = false;
	m_flxfilename = "";

	// flux-error filename
	m_errname_given = false;
	m_errfilename = "";

	// output prefix name
	m_outname_given = false;
	m_outname = "";

	// method
	m_method = 0;

	// parameters
	m_p0 = 73.0;
	m_p1 = 0.73;
}

bool
GenTrxOpt::setargs (int argc, char* argv[])
{
	bool result = true;

	int iarg = 1;
	while (iarg < argc)
	{
		std::string sword (argv[iarg++]);
		if ((sword.size() > 0) && (sword[0] == '-'))
		{
			if ((sword == "-outname") || (sword == "-out") || (sword == "-o"))
			{
				// option with a name string
				if (iarg >= argc)
				{
					std::cerr << "You must specify name." << std::endl;
					result = false;
					break;
				}
				if ((sword == "-outname") || (sword == "-out") || (sword == "-o"))
				{
					m_outname = std::string(argv[iarg++]);
					m_outname_given = true;
					std::cerr << "Output name is " << m_outname << std::endl;
				}
			}
			else if ((sword == "-p0") || (sword == "-p1"))
			{
				// option with a numerical value
				if (iarg >= argc)
				{
					std::cerr << "You must specify the numerical value." << std::endl;
					result = false;
					break;
				}
				std::stringstream ss;
				ss << argv[iarg++];
				double dv;
				if (!(ss >>dv))
				{
					iarg--;
					std::cerr << "value is not a numerical, ignored." << std::endl;
				}
				else if (sword == "-p0")
				{
					m_p0 = dv;
					std::cerr << "p0 value is set to " << m_p0 << std::endl;
				}
				else if (sword == "-p1")
				{
					m_p1 = dv;
					std::cerr << "p1 value is set to " << m_p1 << std::endl;
				}
			}
			else
			{
				std::cerr << "ERROR: " << sword << " -- no such option" << std::endl;
				result = false;
				break;
			}
		}
		else
		{
			// may be data filename or acceptance filename
			if (!m_flxname_given)
			{
				m_flxfilename = sword;
				m_flxname_given = true;
				std::cerr << "flux filename is " << m_flxfilename << std::endl;
			}
			else if (!m_errname_given)
			{
				m_errfilename = sword;
				m_errname_given = true;
				std::cerr << "flux-error filename is " << m_errfilename << std::endl;
			}
			else
			{
				std::cerr << "WARNING: flux filename and flux-error filename are already given, ignored." << std::endl;
			}
		}
	}
	if ((!m_flxname_given) || (!m_errname_given))
		result = false;
	return result;
}

}	// namespace mylibrary
