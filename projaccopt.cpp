// coinimgopt.cpp

#include <iostream>
#include <sstream>
#include <string>

#include "projaccopt.h"

namespace mylibrary
{

ProjAccOpt::ProjAccOpt ()
{
	reset ();
}

ProjAccOpt::~ProjAccOpt ()
{
}

std::ostream&
ProjAccOpt::usage (std::ostream& os, const char* pname) const
{
	os << "Usage: "
		<< pname
		<< " [options]\n"
		<< "Options:\n"
		<< " -out string  \t leading name for the output files (name)\n"
		<< " -comb name   \t layer combination (EEEO, EOEO etc. order Xf-Xr-Yf-Yr)\n"
		<< " -dist        \t distance to the projection plane (m)\n"
		<< " -dzshift     \t distance between units (mm)\n"
		<< std::endl;
	return os;
}

void
ProjAccOpt::reset ()
{
	// output name
	m_outname = "";
	// combination name
	m_combname = "";
	// shift parameters
	m_dxshift = 0.0;
	m_dyshift = 0.0;
	m_dzshift = 495.0;
	// distance parameter
	m_dist_given = false;
	m_dist = 25.5;
}

bool
ProjAccOpt::setargs (int argc, char* argv[])
{
	bool result = true;

	int iarg = 1;
	while (iarg < argc)
	{
		std::string sword (argv[iarg++]);
		if ((sword.size() > 0) && (sword[0] == '-'))
		{
			if ((sword == "-dxshift") || (sword == "-dyshift") || (sword == "-dzshift") ||
				(sword == "-dist") )
			{
				// option with a double value
				if (iarg >= argc)
				{
					result = false;
					std::cerr << "value is not given." << std::endl;
					break;
				}
				std::stringstream ss;
				ss << argv[iarg++];
				double dv;
				if (!(ss >>dv))
				{
					iarg--;
					std::cerr << "value is not a double, ignored." << std::endl;
				}
				else
				{
					if (sword == "-dxshift")
						m_dxshift = dv;
					else if (sword == "-dyshift")
						m_dyshift = dv;
					else if (sword == "-dzshift")
						m_dzshift = dv;
					else if (sword == "-dist")
					{
						m_dist = dv;
						m_dist_given = true;
					}
				}
			}
			else if ((sword == "-out") || (sword == "-outname")
				|| (sword == "-comb") || (sword == "-combination"))
			{
				// option with a name string
				if (iarg >= argc)
				{
					std::cerr << "You must specify name." << std::endl;
					result = false;
					break;
				}
				if ((sword == "-out") || (sword == "-outname"))
				{
					m_outname = std::string(argv[iarg++]);
					std::cerr << "Output name is " << m_outname << std::endl;
				}
				else if ((sword == "-comb") || (sword == "-combination"))
				{
					m_combname = std::string(argv[iarg++]);
					std::cerr << "Combination name is " << m_combname << std::endl;
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
			std::cerr << "ERROR: " << sword << " -- unknown option" << std::endl;
			result = false;
			break;
		}
	}
	return result;
}

}	// namespace mylibrary
