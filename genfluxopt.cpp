// genfluxopt.cpp

#include <iostream>
#include <sstream>
#include <string>

#include "genfluxopt.h"

namespace mylibrary
{

GenFluxOpt::GenFluxOpt ()
{
	reset ();
}

GenFluxOpt::~GenFluxOpt ()
{
}

std::ostream&
GenFluxOpt::usage (std::ostream& os, const char* pname) const
{
	os << "Usage: "
		<< pname
		<< " [options] data_filename acceptance_filename\n"
		<< "Options:\n"
		<< " -out string  \t leading name for the output files (name)\n"
		<< " -cut value \t cutoff (non-nogative) value for data selection\n"
		<< " -png \t output the png image file\n"
		<< " -nopng \t disable the png-image file output\n"
		<< " -gpl \t output the gnuplot data file\n"
		<< " -nogpl \t disable the gnuplot data output\n"
		<< " -eocorr \t enable E/O coorection\n"
		<< " -noeocorr \t disable E/O correction\n"
		<< " -projection \t for projection-plane data file\n"
		<< std::endl;
	return os;
}

void
GenFluxOpt::reset ()
{
	// data filename
	m_datname_given = false;
	m_datfilename = "";
	m_datcut = 0.0;

	// acceptance filename
	m_accname_given = false;
	m_accfilename = "";
	m_acccut = 0.0;

	// output prefix name
	m_outname_given = false;
	m_outname = "";

	// E/O correction
	m_eocorr_given = false;
	m_eocorr = 0;

	// projection-plane option
	m_projection = false;

	// output control for png-image
	m_png_out = false;

	// output control for gnuplot-data
	m_gpl_out = true;
}

bool
GenFluxOpt::setargs (int argc, char* argv[])
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
			else if ((sword == "-datacut") || (sword == "-datcut") || (sword == "-cut"))
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
				else if ((sword == "-datacut") || (sword == "-datcut") || (sword == "-cut"))
				{
					if (dv < 0.0)
					{
						std::cerr << "value is not non-negative, ignored." << std::endl;
					}
					else
					{
						m_datcut = dv;
						std::cerr << "data cutoff is set to " << m_datcut << std::endl;
					}
				}
			}
			else if (sword == "-eocorr")
			{
				m_eocorr_given = true;
				m_eocorr = 1;
			}
			else if ((sword == "-noeo") || (sword == "-noeocorr") || (sword == "-nocorr"))
			{
				m_eocorr_given = true;
				m_eocorr = 0;
			}
			else if (sword == "-png")
				m_png_out = true;
			else if (sword == "-nopng")
				m_png_out = false;
			else if (sword == "-gpl")
				m_gpl_out = true;
			else if (sword == "-nogpl")
				m_gpl_out = false;
			else if ((sword == "-proj") || (sword == "-projection"))
				m_projection = true;
			else if ((sword == "-noproj") || (sword == "-noprojection"))
				m_projection = false;
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
			if (!m_datname_given)
			{
				m_datfilename = sword;
				m_datname_given = true;
				std::cerr << "data filename is " << m_datfilename << std::endl;
			}
			else if (!m_accname_given)
			{
				m_accfilename = sword;
				m_accname_given = true;
				std::cerr << "acceptance filename is " << m_accfilename << std::endl;
			}
			else
			{
				std::cerr << "WARNING: data filename and acceptance filename are already given, ignored." << std::endl;
			}
		}
	}
	if ((!m_datname_given) || (!m_accname_given))
		result = false;
	return result;
}

}	// namespace mylibrary
