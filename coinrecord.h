// coinrecord.h
//
// 2017-11-30 numAhits() and numBhits() are added.
//

#ifndef COINRECORD_H_INCLUDED
#define COINRECORD_H_INCLUDED

#include <iostream>

namespace MUONDAQ
{
class CoinXYUnit
{
public:
	CoinXYUnit ();
	virtual ~CoinXYUnit ();
	std::istream& Read (std::istream& is);

public:
	int m_dataready;
	int m_numxhits;
	int m_numxclust;
	int m_xav;
	int m_numyhits;
	int m_numyclust;
	int m_yav;
	double m_usec;
	double m_nsec;
};

class CoinRecord
{
public:
	static const int NUM_XYUNITS = 4;
	static const int MAX_XPOS = 475;
	static const int MAX_YPOS = 475;
public:
	CoinRecord ();
	virtual ~CoinRecord ();
	std::istream& Read (std::istream& is);
	const CoinXYUnit& XYUnit (int unitno) const;
	int numdat (int unitno) const;
	int numAhits (int unitno) const;
	int numBhits (int unitno) const;
	int numxhits (int unitno) const { return numAhits (unitno); }
	int numyhits (int unitno) const { return numBhits (unitno); }
	int numAclusters (int unitno) const;
	int numBclusters (int unitno) const;
	bool A1cluster (int unitno, int maxhits=0) const;
	bool B1cluster (int unitno, int maxhits=0) const;
	bool AB1cluster (int unitno, int maxhits=0) const;
	bool xy1cluster (int unitno, int maxhits=0) const;
	int Apos (int unitno, int dir=1) const;
	int Bpos (int unitno, int dir=1) const;
	int xpos (int unitno, int dir=1) const;
	int ypos (int unitno, int dir=1) const;
	int epos (int unitno, int dir=1) const { return xpos(unitno, dir); }
	// position from even channels (small detector only)
	int opos (int unitno, int dir=1) const { return ypos(unitno, dir) + ((dir>0) ? 5 : -5); }
	// position from odd channels (small detector only)
	int rxpos (int unitno) const { return xpos (unitno, -1); }
	int rypos (int unitno) const { return ypos (unitno, -1); }
	double microsec (int unitno) const;
	double nanosec (int unitno) const;

protected:
	int m_numdat[NUM_XYUNITS];
	CoinXYUnit m_xy[NUM_XYUNITS];
};

std::ostream&
operator << (std::ostream& os, const CoinRecord& coinrec);

std::istream&
operator >> (std::istream& is, CoinRecord& coinrec);

}	// namespace MUONDAQ
#endif	// COINRECORD_H_INCLUDED
