// coinimgopt.cpp

#include <iostream>
#include <sstream>
#include <string>

#include "coinrateopt.h"

namespace mylibrary
{

CoinrateOpt::CoinrateOpt ()
{
	reset ();
}

CoinrateOpt::~CoinrateOpt ()
{
}

std::ostream&
CoinrateOpt::usage (std::ostream& os, const char* pname) const
{
	os << "Usage: "
		<< pname
		<< " [options] list_filename\n"
		<< "Options:\n"
		<< " -out string  \t leading name for the output files (name)\n"
		<< " -maxhits n   \t maximum number of hits (0: no limit)\n"
		<< std::endl;
	return os;
}

void
CoinrateOpt::reset ()
{
	// listfilename
	m_listfilename = "";
	// output name
	m_outname = "";
	// maxhits
	m_maxhits = 0;
}

bool
CoinrateOpt::setargs (int argc, char* argv[])
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
			else if ((sword == "-out") || (sword == "-outname"))
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
