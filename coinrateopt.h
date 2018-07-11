// coinrateopt.h

#ifndef COINRATEOPT_H_INCLUDED
#define COINRATEOPT_H_INCLUDED

#include <string.h>

namespace mylibrary
{
class CoinrateOpt
{
public:
	CoinrateOpt ();
	virtual ~CoinrateOpt ();
	bool setargs (int argc, char* argv[]);
	void reset ();
	std::ostream& usage (std::ostream& os, const char* pname) const;

public:
	// listfilename
	std::string m_listfilename;
	// output name
	std::string m_outname;
	// maximum hits (0 means no limit)
	int m_maxhits;
};

}	// namespace mylibrary

#endif	// COINRATEOPT_H_INCLUDED
