// coinimgopt.h

#ifndef COINIMGOPT_H_INCLUDED
#define COINIMGOPT_H_INCLUDED

#include <string.h>

namespace mylibrary
{
class CoinimgOpt
{
public:
	CoinimgOpt ();
	virtual ~CoinimgOpt ();
	bool setargs (int argc, char* argv[]);
	void reset ();
	std::ostream& usage (std::ostream& os, const char* pname) const;

	enum
	{
		AVERAGE = 0, FITTING,
	};

public:
	// listfilename
	std::string m_listfilename;
	// output name
	std::string m_outname;
	// combination (EOEO etc.) name
	std::string m_combname;
	// maximum hits (0 means no limit)
	int m_maxhits;
	// even-odd selection
	int m_evenodd_select;
	// narrow X range
	bool m_xenarrow;
	bool m_xonarrow;
	// shift parameters
	double m_dxshift;
	double m_dyshift;
	double m_dzshift;	// average z-distance between units
	// distance parameters
	bool m_dist_given;
	double m_dist;
	// method selection (0:average, 1:fitting)
	int m_method;
	// edge correction
	bool m_edge_corr;
	// straight track allowence
	double m_dxmax;
	double m_dymax;
};

}	// namespace mylibrary

#endif	// COINIMGOPT_H_INCLUDED
