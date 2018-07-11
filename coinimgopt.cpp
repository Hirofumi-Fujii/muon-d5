// coinimgopt.cpp

#include <iostream>
#include <sstream>
#include <string>

#include "coinimgopt.h"

namespace mylibrary
{

CoinimgOpt::CoinimgOpt ()
{
	reset ();
}

CoinimgOpt::~CoinimgOpt ()
{
}

std::ostream&
CoinimgOpt::usage (std::ostream& os, const char* pname) const
{
	os << "Usage: "
		<< pname
		<< " [options] list_filename\n"
		<< "Options:\n"
		<< " -maxhits n   \t maximum number of hits (0: no limit)\n"
		<< " -out string  \t leading name for the output files (name)\n"
		<< " -fit         \t use fitting for muon track\n"
		<< " -average     \t use average for muon track\n"
		<< " -comb name   \t layer combination (EEEO, EOEO etc. order Xf-Xr-Yf-Yr)\n"
		<< std::endl;
	return os;
}

void
CoinimgOpt::reset ()
{
	// listfilename
	m_listfilename = "";
	// output name
	m_outname = "";
	// combination name
	m_combname = "";
	// maximum hits (0 means no limit)
	m_maxhits = 0;
	// even-odd selection
	m_evenodd_select = 0;
	//
	m_xenarrow = false;
	m_xonarrow = false;
	// shift parameters
	m_dxshift = 0.0;
	m_dyshift = 139.0;
	m_dzshift = 495.0;
	// distance parameter
	m_dist_given = false;
	m_dist = 25.5;
	// method selection
	m_method = CoinimgOpt::AVERAGE;
	// edge correction
	m_edge_corr = true;
	// straight track allowence
	m_dxmax = 12.5;
	m_dymax = 12.5;
}

bool
CoinimgOpt::setargs (int argc, char* argv[])
{
	bool result = true;
	bool listgiven = false;

	int iarg = 1;
	while (iarg < argc)
	{
		std::string sword (argv[iarg++]);
		if ((sword.size() > 0) && (sword[0] == '-'))
		{
			if (sword == "-maxhits")
			{
				// option with an integer
				if (iarg >= argc)
				{
					result = false;
					std::cerr << "value is not given." << std::endl;
					break;
				}
				std::stringstream ss;
				ss << argv[iarg++];
				int iv;
				if (!(ss >>iv))
				{
					iarg--;
					std::cerr << "value is not an integer, ignored." << std::endl;
				}
				else
				{
					m_maxhits = iv;
				}
			}
			else if ((sword == "-dxshift") || (sword == "-dyshift") || (sword == "-dzshift") ||
				(sword == "-dist") )
			{
				// option with an double
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
			else if (sword == "-even")
				m_evenodd_select = 1;
			else if (sword == "-odd")
				m_evenodd_select = 2;
			else if (sword == "-evenodd")
				m_evenodd_select = 0;
			else if (sword == "-xenarrow")
				m_xenarrow = true;
			else if (sword == "-xonarrow")
				m_xonarrow = true;
			else if (sword == "-nodist")
				m_dist_given = false;
			else if (sword == "-average")
				m_method = CoinimgOpt::AVERAGE;
			else if ((sword == "-fiting") || (sword == "-fit"))
				m_method = CoinimgOpt::FITTING;
			else if (sword == "-edge")
				m_edge_corr = true;
			else if (sword == "-noedge")
				m_edge_corr = false;
			else
			{
				std::cerr << "ERROR: " << sword << " -- no such option" << std::endl;
				result = false;
				break;
			}
		}
		else
		{
			// may be listfilename
			if (listgiven)
			{
				std::cerr << "WARNING: list_filename is already given, replaced" << std::endl;
			}
			m_listfilename = sword;
			listgiven = true;
			std::cerr << "List filename is " << m_listfilename << std::endl;
		}
	}
	if (!listgiven)
		result = false;
	return result;
}

}	// namespace mylibrary
