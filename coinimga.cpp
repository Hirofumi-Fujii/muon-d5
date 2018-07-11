// coinimga.cpp
// g++ -Wall coinimga.cpp coinimgopt.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp hist2d.cpp ncpng.cpp mytimer.cpp -lz -o coinimga
//
// 07-Dec-2017 range on projection plane in the case of (xscale < 1.0) and/or (yscale < 1.0) is added.
// 30-Nov-2017 front average position (hist[18]) was wrong (uxdiff, uydiff must be subtracted), corrected. (VERION 308)
// 29-Nov-2017 new calculation for projection plane from muon-d5/coinimga.cpp (VERSION 307)
// 20-Oct-2017 single unit coincidece, hist[18], hist[19] are added. (VERSION 305)
// 03-Aug-2017 coinimgf-old3.cpp is renamed coinimga.cpp (switchs the function by -average/-fitting) (VERSION 304)
// 01-Aug-2017 Copied from coinimga.cpp (old coinimgf.cpp is renamed coinimgf-old2.cpp)
// 01-Jun-2017 uxdiff, uydiff, uzdiff, pxdist and pydist are added (VERSION 303)
// 29-May-2017 Histgram ranges for X-Y are modified (VERSION_NUMBER 302) -5.0 480.0 -> -2.5 482.5
// 23-May-2017 Add new trigger conditions (VERSION_NUMBER 301) output precision 16-digits
//
// 21-Feb-2017 UnixTime is moved into MyTimer class.
// 20-Feb-2017 Protect against all 0 COIN events, added FirstEventTime and LastEventTime in log file.
//
// 05-Aug-2016 ppos_shift[][] are added
// 03-Aug-2016 XF-XR and YF-YR histograms are added
// 27-May-2016 command line added to the csv outputs
// 06-May-2016 output position of the 'duration' has been changed
// 12-Mar-2016 coinimga -- test for coinrec
// 31-Mar-2015 modified for detector3
// 5-Mar-2015 modified for detector4
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "gzfstream.h"
#include "coinimgopt.h"
#include "coinrecord.h"
#include "hist2d.h"
#include "fit2dline.h"
#include "ncpng.h"
#include "mytimer.h"

static const unsigned int VERSION_NUMBER = 308;

static const unsigned int NUM_UNITS = 4;
static const unsigned int NUM_LAYERS = 4;

static const int NUM_XBINS = 193;
static const int NUM_YBINS = 193;

static const int IMG_WIDTH = 193;
static const int IMG_HEIGHT = 193;

double xlayer_zpos [NUM_LAYERS];
double xlayer_xpos [NUM_LAYERS];	// HF-2017-11-29, x-position of the low-edge X-channel in world coordinate
double ylayer_zpos [NUM_LAYERS];
double ylayer_ypos [NUM_LAYERS];	// HF-2017-11-29, y-position of the low-edge Y-channel in world coordinate

int main (int argc, char* argv[])
{
	using namespace mylibrary;

//	Options
	bool usefitting = true;

//	Option object
	CoinimgOpt opt;

///// ***** detector 5 defaults *****
	double projdist = 1.15;	// distance to the projection plane

	opt.m_dist = projdist;
	opt.m_dxshift = 0.0;
	opt.m_dyshift = 0.0;
	opt.m_dzshift = 660.0;
	opt.m_edge_corr = true;
///// *******************************

	// check the arguments
	if (!opt.setargs (argc, argv))
	{
		opt.usage (std::cout, argv[0]);
		return (-1);
	}

//
// Setting parameters
//
	double uxdiff = opt.m_dxshift;	// Unit x-position difference
	double uydiff = opt.m_dyshift;	// Unit y-position difference
	double uzdiff = opt.m_dzshift;	// Unit z-position difference

	// HF-2017-11-29 start
	double xlayer_size = 480.0;
	double xlayer_halfsize = xlayer_size * 0.5;
	double ylayer_size = 480.0;
	double ylayer_halfsize = ylayer_size * 0.5;

	double xplane_size = xlayer_size + 5.0;
	double xplane_halfsize = xplane_size * 0.5;
	double yplane_size = ylayer_size + 5.0;
	double yplane_halfsize = yplane_size * 0.5;
	// HF-2017-11-29 end

	// HF-2017-11-30 start
	double xlayer_dist = 10.0;	// distance between layers
	double xlayer_halfdist = xlayer_dist * 0.5;
	double ylayer_dist = 10.0;	// distance between layers
	double ylayer_halfdist = ylayer_dist * 0.5;
	double xyplane_dist = 21.0;	// distance between xy-planes
	double xyplane_halfdist = xyplane_dist * 0.5;
	double xch_width = 10.0;
	double xch_halfwidth = xch_width * 0.5;
	double ych_width = 10.0;
	double ych_halfwidth = ych_width * 0.5;

	// HF-2017-11-29 projdist is the distance from the center of the rear unit.
	if (opt.m_dist_given)
		projdist = opt.m_dist;

	double obj_z = -(projdist * 1000.0);	// z-origin is the center of the rear unit.

// ***** Detector 5 Setup Layer location *****
// The origin of our coordinate system is the center of the rear unit.

	double pxdist = uzdiff;	// x-plane distance
	double pydist = uzdiff;	// y-plane distance

// Front unit
// Order XO-XE-YO-YE from the reactor side
// In progam, 0 is even, 1 is odd
// The origin of z is rear-unit center
//
//	xlayer[0] = X front even (A-group)
	xlayer_zpos [0] = -pxdist - xyplane_halfdist + xlayer_halfdist;
	xlayer_xpos [0] = -xlayer_halfsize - 2.5 + uxdiff + xch_halfwidth;	// lowest channel center

//	xlayer[1] = X front odd (B-group)
	xlayer_zpos [1] = xlayer_zpos [0] - xlayer_dist;
	xlayer_xpos [1] = xlayer_xpos [0] + 5.0;

//	ylayer[0] = Y front even (A-group)
	ylayer_zpos [0] = -pydist + xyplane_halfdist + ylayer_halfdist;
	ylayer_ypos [0] = -ylayer_halfsize - 2.5 + uydiff + ych_halfwidth;	// lowest channel center

//	ylayer[1] = Y front odd (B-group)
	ylayer_zpos [1] = ylayer_zpos [0] - ylayer_dist;
	ylayer_ypos [1] = ylayer_ypos [0] + 5.0;

// Rear unit
// Order X-Y from the reactor side
// In progam, 0 is even, 1 is odd
// The origin of z is rear-unit center
//
//	xlayer[2] = X rear even (A-group)
	xlayer_zpos [2] = -xyplane_halfdist + xlayer_halfdist;
	xlayer_xpos [2] = -xlayer_halfsize - 2.5 + xch_halfwidth;		// lowest channel center

//	xlayer[3] = X rear odd (B-group)
	xlayer_zpos [3] = xlayer_zpos [2] - xlayer_dist;
	xlayer_xpos [3] = xlayer_xpos [2] + 5.0;

//	ylayer[2] = Y rear even (A-group)
	ylayer_zpos [2] = +xyplane_halfdist + ylayer_halfdist;
	ylayer_ypos [2] = -ylayer_halfsize - 2.5 + ych_halfwidth;		// lowest channel center

//	ylayer[3] = Y rear odd (B-group)
	ylayer_zpos [3] = ylayer_zpos [2] - ylayer_dist;
	ylayer_ypos [3] = ylayer_ypos [2] + 5.0;

// ***** End of detector setup

	std::string listfilename;
	listfilename = opt.m_listfilename;
	std::string outname = opt.m_outname;
	if (outname == "")
		outname = "coinimga";
	int maxhits = opt.m_maxhits;

	// open the list file
	std::ifstream ifl (listfilename.c_str());
	if (!ifl)
	{
		// file open error
		std::cerr
			<< "ERROR: list file ("
			<< listfilename
			<< ") open error."
			<< std::endl;
		return (-1);
	}
	if (opt.m_method == CoinimgOpt::FITTING)
		usefitting = true;
	else
		usefitting = false;

	std::string methodname = usefitting ? "fitting" : "average";

//
//	Initialization
//

	// HF-2017-11-29 start
	double zxp_f = (xlayer_zpos[0] + xlayer_zpos[1]) * 0.5;
	double zxp_r = (xlayer_zpos[2] + xlayer_zpos[3]) * 0.5;
	double zyp_f = (ylayer_zpos[0] + ylayer_zpos[1]) * 0.5;
	double zyp_r = (ylayer_zpos[2] + ylayer_zpos[3]) * 0.5;

	double xscale = (obj_z - zxp_r) / (zxp_f - zxp_r);
	double yscale = (obj_z - zyp_r) / (zyp_f - zyp_r);

	double xpmax;
	double xpmin;
	if (xscale > 1.0)
	{
		xpmax = (( xplane_halfsize + uxdiff + xplane_halfsize) * xscale - xplane_halfsize) / 1000.0;
		xpmin = ((-xplane_halfsize + uxdiff - xplane_halfsize) * xscale + xplane_halfsize) / 1000.0;
	}
	else
	{
		xpmax = (( xplane_halfsize + uxdiff - xplane_halfsize) * xscale + xplane_halfsize) / 1000.0;
		xpmin = ((-xplane_halfsize + uxdiff + xplane_halfsize) * xscale - xplane_halfsize) / 1000.0;
	}
	double ypmax;
	double ypmin;
	if (yscale > 1.0)
	{
		ypmax = (( yplane_halfsize + uydiff + yplane_halfsize) * yscale - yplane_halfsize) / 1000.0;
		ypmin = ((-yplane_halfsize + uydiff - yplane_halfsize) * yscale + yplane_halfsize) / 1000.0;
	}
	else
	{
		ypmax = (( yplane_halfsize + uydiff - yplane_halfsize) * yscale + yplane_halfsize) / 1000.0;
		ypmin = ((-yplane_halfsize + uydiff + yplane_halfsize) * yscale - yplane_halfsize) / 1000.0;
	}

	// HF-2017-11-29 end
	
	long totalfile = 0;
	unsigned long listlineno = 0;
	double duration = 0.0;
	double totalcoin = 0.0;
	double totalcoinall = 0.0;
	double totalevt = 0.0;
	double totalallhcoin = 0.0;	// 2017-05-23
	double totalallhevt = 0.0;	// 2017-05-23
	double totalall1cevt = 0.0;

	double tsecnow = 0.0;
	double tsecfirst = 0.0;
	double tseclast = 0.0;
	double tsecdiff = 0.0;
	double runnumcoin = 0.0;
	double runcoinall = 0.0;
	double runnumevt = 0.0;
	double runallhcoin = 0.0;
	double runallhevt = 0.0;
	double runall1cevt = 0.0;
	std::string runsrec = "";
	std::string runerec = "";
	std::string prcsrec = "";	// Process start record
	std::string prcerec = "";	// Process end record

	static const int NUM_HIST2D = 20;
	mylibrary::Hist2D hist2d [NUM_HIST2D] =
	{
		// HF-2017-11-29 start
		Hist2D ("0:front unit X-Y", -xplane_halfsize, xplane_halfsize, 97, -yplane_halfsize, yplane_halfsize, 97),
		Hist2D ("0:rear unit  X-Y", -xplane_halfsize, xplane_halfsize, 97, -yplane_halfsize, yplane_halfsize, 97),
		Hist2D ("1:front unit X-Y", -xplane_halfsize, xplane_halfsize, 97, -yplane_halfsize, yplane_halfsize, 97),
		Hist2D ("1:rear unit  X-Y", -xplane_halfsize, xplane_halfsize, 97, -yplane_halfsize, yplane_halfsize, 97),
		Hist2D ("2:front unit X-Y", -xplane_halfsize, xplane_halfsize, 97, -yplane_halfsize, yplane_halfsize, 97),
		Hist2D ("2:rear unit  X-Y", -xplane_halfsize, xplane_halfsize, 97, -yplane_halfsize, yplane_halfsize, 97),
		Hist2D ("0:dx-dy", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("0:dx-dy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("1:dx-dy", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("1:dx-dy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("2:dx-dy", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("2:dx-dy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("0:xp-yp", xpmin, xpmax, 193, ypmin, ypmax, 193),
		Hist2D ("0:xp-yp smooth", xpmin, xpmax, 193, ypmin, ypmax, 193, true),
		Hist2D ("1:xp-yp", xpmin, xpmax, 193, ypmin, ypmax, 193),
		Hist2D ("1:xp-yp smooth", xpmin, xpmax, 193, ypmin, ypmax, 193, true),
		Hist2D ("2:xp-yp", xpmin, xpmax, 193, ypmin, ypmax, 193),
		Hist2D ("2:xp-yp smooth", xpmin, xpmax, 193, ypmin, ypmax, 193, true),
		Hist2D ("front only X-Y", -xplane_halfsize, xplane_halfsize, 97, -yplane_halfsize, yplane_halfsize, 97),
		Hist2D ("rear only  X-Y", -xplane_halfsize, xplane_halfsize, 97, -yplane_halfsize, yplane_halfsize, 97),
		// HF-2017-11-29 end
	};

	mylibrary::MyTimer mytimer;

	std::string logfnam = outname + "-log.csv";
	std::ofstream ofl (logfnam.c_str());
	if (!ofl)
	{
		std::cerr
			<< "ERROR: log file ("
			<< logfnam
			<< ") open error."
			<< std::endl;
		return (-1);
	}
	ofl
		<< "ListLineNo."
		<< ",Filename"
		<< ",Open"
		<< ",RunStart"
		<< ",RunEnd"
		<< ",Start(Unix-sec)"
		<< ",End(Unix-sec)"
		<< ",Difftime(sec)"
		<< ",FirstEventTime(sec)"	// 2017-02-20
		<< ",LastEventTime(sec)"	// 2017-02-20
		<< ",EventDiffTime(sec)"
		<< ",CoinRecords"
		<< ",CoinAllUnits"
		<< ",Events"
		<< ",AllHitCoin"		// 2017-05-23
		<< ",AllHitEvents"		// 2017-05-23
		<< ",All1CEvents"		// 2017-05-11
		<< ",CoinRate(1/sec)"
		<< ",CoinAllRate(1/sec)"
		<< ",EventRate(1/sec)"
		<< ",AllHitCoinRate(1/sec)"	// 2017-05-23
		<< ",AllHitEvtRate(1/sec)"	// 2017-05-23
		<< ",All1CRate(1/sec)"		// 2017-05-11
		<< std::endl;

	// read the filename from the list file
	std::string liststr;
	mytimer.start ();
	while (getline (ifl, liststr))
	{
		listlineno += (unsigned long)(1);
		std::string datfnam;
		bool doit = true;
		if ((liststr.size() <= 0) || (liststr[0] == '%'))
			doit = false;
		if (doit)
		{
			std::stringstream ssl (liststr);
			if (!(ssl >> datfnam))
				doit = false;
		}
		if (doit)
		{
			bool newrun = true;
			tsecnow = 0.0;
			tsecfirst = 0.0;
			tseclast = 0.0;
			tsecdiff = 0.0;
			runnumcoin = 0.0;
			runcoinall = 0.0;
			runnumevt = 0.0;
			runallhcoin = 0.0;
			runallhevt = 0.0;
			runall1cevt = 0.0;
			runsrec = "";
			runerec = "";

			ofl << listlineno << ",\"" << datfnam << "\"";
			// open the data file
			mylibrary::igzfstream ifs (datfnam.c_str());
			if (!ifs)
			{
				ofl << ",0";
				// file open error
				std::cerr
					<< "ERROR: data file ("
					<< datfnam
					<< ") open error."
					<< std::endl;
			}
			else
			{
				totalfile += 1;
				ofl << ",1";

				// read file and count the data-records
				std::string recstr;
				while (getline (ifs, recstr))
				{
					if ((recstr.size() > 4) &&
					    (recstr.substr(0,4) == "COIN"))
					{
						std::stringstream ss (recstr);
						std::string recid;
						ss >> recid;
						MUONDAQ::CoinRecord coinrec;
						if ((ss >> coinrec))
						{
							totalcoin += 1.0;
							runnumcoin += 1.0;
							int numhitunits = 0;	// 2017-02-20
							for (unsigned int u = 0; u < NUM_UNITS; u++)
							{
								if (coinrec.numdat(u) > 0)
								{
									tsecnow = coinrec.microsec(u) / 1000000.0;
									numhitunits += 1;	// 2017-02-20
								}
							}
							if (numhitunits > 0)	// 2017-02-20
							{
								if (newrun)
								{
									tsecfirst = tsecnow;
									newrun = false;
								}
								tseclast = tsecnow;
							}	// 2017-02-20

							// Unit assignments
							int uidy2 = 0;  //UNIT#7
							int uidx2 = 1;  //UNIT#8
							int uidy1 = 2;  //UNIT#9
							int uidx1 = 3;  //UNIT#10

							if
							(
								(coinrec.numdat (uidx1) > 0) &&
								(coinrec.numdat (uidy1) > 0) &&
								(coinrec.numdat (uidx2) > 0) &&
								(coinrec.numdat (uidy2) > 0)
							)
							{
								totalcoinall += 1.0;
								runcoinall += 1.0;

								// A-group: even
								// B-group: odd
								bool allhcoin = false;

								if
								(
									(coinrec.numAclusters (uidx1) > 0) &&
									(coinrec.numBclusters (uidx1) > 0) &&
									(coinrec.numAclusters (uidy1) > 0) &&
									(coinrec.numBclusters (uidy1) > 0) &&
									(coinrec.numAclusters (uidx2) > 0) &&
									(coinrec.numBclusters (uidx2) > 0) &&
									(coinrec.numAclusters (uidy2) > 0) &&
									(coinrec.numBclusters (uidy2) > 0)
								)
								{
									totalallhcoin += 1.0;
									runallhcoin += 1.0;
									allhcoin = true;
								}

								bool all1cluster = false;
								if
								(
									coinrec.AB1cluster (uidx1, maxhits) &&
									coinrec.AB1cluster (uidy1, maxhits) &&
									coinrec.AB1cluster (uidx2, maxhits) &&
									coinrec.AB1cluster (uidy2, maxhits)
								)
									all1cluster = true;

								int nxf = 0;	// number of single x-layers in the front unit
								int nyf = 0;	// number of single y-layers in the front unit
								int nxr = 0;	// number of single x-layers in the rear unit
								int nyr = 0;	// number of single y-layers in the rear unit

								double xsumf = 0.0;
								double zxsumf = 0.0;	// HF-2017-11-29
								double ysumf = 0.0;
								double zysumf = 0.0;	// HF-2017-11-29
								double xsumr = 0.0;
								double zxsumr = 0.0;	// HF-2017-11-29
								double ysumr = 0.0;
								double zysumr = 0.0;	// HF-2017-11-29

								int nxdat = 0;	// number of single x-layers
								int nydat = 0;	// number of single y-layers
								double xdat [NUM_LAYERS];
								double xpos [NUM_LAYERS];
								double ydat [NUM_LAYERS];
								double ypos [NUM_LAYERS];

								// *** Front Unit ***
								// X-plane
								if (coinrec.A1cluster (uidx1, maxhits))
								{
									double xdev = (double)(coinrec.Apos(uidx1));
									double x = xdev + xlayer_xpos [0];	// HF-2017-11-29
									xsumf += x;
									zxsumf += xlayer_zpos[0];	// HF-2017-11-29
									++nxf;
									xdat [nxdat] = x;
									xpos [nxdat] = xlayer_zpos[0];
									++nxdat;
									if (opt.m_edge_corr && (xdev < 2.5) && (coinrec.numBhits (uidx1) == 0))
									{
										x = -10.0 + xlayer_xpos[1];
										xsumf += x;
										zxsumf += xlayer_zpos[1];
										++nxf;
										xdat [nxdat] = x;
										xpos [nxdat] = xlayer_zpos[1];
										++nxdat;
									}
								}
								if (coinrec.B1cluster (uidx1, maxhits))
								{
									double xdev = (double)(coinrec.Bpos(uidx1));
									double x = xdev + xlayer_xpos [1];	// HF-2017-11-29
									xsumf += x;
									zxsumf += xlayer_zpos[1];	// HF-2017-11-29
									++nxf;
									xdat [nxdat] = x;
									xpos [nxdat] = xlayer_zpos[1];
									++nxdat;
									if (opt.m_edge_corr && (xdev > 467.5) && (coinrec.numAhits (uidx1) == 0))
									{
										x = 480.0 + xlayer_xpos[0];
										xsumf += x;
										zxsumf += xlayer_zpos[0];
										++nxf;
										xdat [nxdat] = x;
										xpos [nxdat] = xlayer_zpos[0];
										++nxdat;
									}
								}
								// Y-plane
								if (coinrec.A1cluster (uidy1, maxhits))
								{
									double ydev = (double)(coinrec.Apos(uidy1));
								//@@@	double y = 470.0 - ydev + ylayer_ypos [0];	// HF-2017-11-29
									double y = ydev + ylayer_ypos [0];
									ysumf += y;
									zysumf += ylayer_zpos[0];	// HF-2017-11-29
									++nyf;
									ydat [nydat] = y;
									ypos [nydat] = ylayer_zpos[0];
									++nydat;
									if (opt.m_edge_corr && (ydev < 2.5) && (coinrec.numBhits (uidy1) == 0))
									{
										ydev = -10.0;
									//@@@	y = 470.0 - ydev + ylayer_ypos[1];
										y = ydev + ylayer_ypos[1];
										ysumf += y;
										zysumf += ylayer_zpos[1];
										++nyf;
										ydat [nydat] = y;
										ypos [nydat] = ylayer_zpos[1];
										++nydat;
									}
								}
								if (coinrec.B1cluster (uidy1, maxhits))
								{
									double ydev = (double)(coinrec.Bpos(uidy1));
								//@@@	double y = 470.0 - ydev + ylayer_ypos [1];	// HF-2017-11-29
									double y = ydev + ylayer_ypos[1];
									ysumf += y;
									zysumf += ylayer_zpos[1];	// HF-2017-11-29
									++nyf;
									ydat [nydat] = y;
									ypos [nydat] = ylayer_zpos[1];
									++nydat;
									if (opt.m_edge_corr && (ydev > 467.5) && (coinrec.numAhits (uidy1) == 0))
									{
										ydev = 480.0;
									//@@@	y = 470.0 - ydev + ylayer_ypos[0];
										y = ydev + ylayer_ypos[0];
										ysumf += y;
										zysumf += ylayer_zpos[0];
										++nyf;
										ydat [nydat] = y;
										ypos [nydat] = ylayer_zpos[0];
										++nydat;
									}
								}

								// *** Rear Unit ***
								// X-plane
								if (coinrec.A1cluster (uidx2, maxhits))
								{
									double xdev = (double)(coinrec.Apos(uidx2));
									double x = xdev + xlayer_xpos [2];	// HF-2017-11-29
									xsumr += x;
									zxsumr += xlayer_zpos[2];	// HF-2017-11-29
									++nxr;
									xdat [nxdat] = x;
									xpos [nxdat] = xlayer_zpos[2];
									++nxdat;
									if (opt.m_edge_corr && (xdev < 2.5) && (coinrec.numBhits (uidx2) == 0))
									{
										x = -10.0 + xlayer_xpos[3];
										xsumr += x;
										zxsumr += xlayer_zpos[3];
										++nxr;
										xdat [nxdat] = x;
										xpos [nxdat] = xlayer_zpos[3];
										++nxdat;
									}
								}
								if (coinrec.B1cluster (uidx2, maxhits))
								{
									double xdev = (double)(coinrec.Bpos(uidx2));
									double x = xdev + xlayer_xpos [3];	// HF-2017-11-29
									xsumr += x;
									zxsumr += xlayer_zpos[3];	// HF-2017-11-29
									++nxr;
									xdat [nxdat] = x;
									xpos [nxdat] = xlayer_zpos[3];
									++nxdat;
									if (opt.m_edge_corr && (xdev > 467.5) && (coinrec.numAhits (uidx2) == 0))
									{
										x = 480.0 + xlayer_xpos[2];
										xsumr += x;
										zxsumr += xlayer_zpos[2];
										++nxr;
										xdat [nxdat] = x;
										xpos [nxdat] = xlayer_zpos[2];
										++nxdat;
									}
								}
								// Y-plane
								if (coinrec.A1cluster (uidy2, maxhits))
								{
									double ydev = (double)(coinrec.Apos(uidy2));
									double y = ydev + ylayer_ypos [2];	// HF-2017-11-29
									ysumr += y;
									zysumr += ylayer_zpos[2];	// HF-2017-11-29
									++nyr;
									ydat [nydat] = y;
									ypos [nydat] = ylayer_zpos[2];
									++nydat;
									if (opt.m_edge_corr && (ydev < 2.5) && (coinrec.numBhits (uidy2) == 0))
									{
										ydev = -10.0;
										y = ydev + ylayer_ypos[3];
										ysumr += y;
										zysumr += ylayer_zpos[3];
										++nyr;
										ydat [nydat] = y;
										ypos [nydat] = ylayer_zpos[3];
										++nydat;
									}
								}
								if (coinrec.B1cluster (uidy2, maxhits))
								{
									double ydev = (double)(coinrec.Bpos(uidy2));
									double y = ydev + ylayer_ypos [3];	// HF-2017-11-29
									ysumr += y;
									zysumr += ylayer_zpos[3];	// HF-2017-11-29
									++nyr;
									ydat [nydat] = y;
									ypos [nydat] = ylayer_zpos[3];
									++nydat;
									if (opt.m_edge_corr && (ydev > 467.5) && (coinrec.numAhits (uidy2) == 0))
									{
										ydev = 480.0;
										y = ydev + ylayer_ypos[2];
										ysumr += y;
										zysumr += ylayer_zpos[2];
										++nyr;
										ydat [nydat] = y;
										ypos [nydat] = ylayer_zpos[2];
										++nydat;
									}
								}

								// HF-2017-10-20 start
								if ((nxf > 0) && (nyf > 0))
								{
									double xav = (xsumf / double (nxf)) - uxdiff;	// HF-2017-11-30
									double yav = (ysumf / double (nyf)) - uydiff;	// HF-2017-11-30
									hist2d[18].cumulate(xav, yav);
								}
								if ((nxr > 0) && (nyr > 0))
								{
									double xav = xsumr / double (nxr);
									double yav = ysumr / double (nyr);
									hist2d[19].cumulate(xav, yav);
								}
								// HF-2017-10-20 end

								if ((nxf > 0) && (nyf > 0) && (nxr > 0) && (nyr > 0))
								{
									totalevt += 1.0;
									runnumevt += 1.0;
									if (allhcoin)
									{
										totalallhevt += 1.0;
										runallhevt += 1.0;
									}

									if (all1cluster)
									{
										totalall1cevt += 1.0;
										runall1cevt += 1.0;
									}

									mylibrary::Fit2DLine xline (nxdat, xpos, xdat);
									mylibrary::Fit2DLine yline (nydat, ypos, ydat);

									double xf;
									double yf;
									double xr;
									double yr;
									double xp;
									double yp;
									if (usefitting)
									{
										// HF-2017-11-29 start
										xf = xline.y (zxp_f) - uxdiff;
										yf = yline.y (zyp_f) - uydiff;
										xr = xline.y (zxp_r);
										yr = yline.y (zyp_r);
										xp = xline.y (obj_z) / 1000.0;
										yp = yline.y (obj_z) / 1000.0;
										// HF-2017-11-29 end
									}
									else
									{
										// HF-2017-11-29 start
										double axf = (xsumf / (double)(nxf));
										double zxf = zxsumf / (double)(nxf);
										double ayf = (ysumf / (double)(nyf));
										double zyf = zysumf / (double)(nyf);
										double axr = xsumr / (double)(nxr);
										double zxr = zxsumr / (double)(nxr);
										double ayr = ysumr / (double)(nyr);
										double zyr = zysumr / (double)(nyr);
										// Units of xp and yp is meter
										xf = axf - uxdiff;
										yf = ayf - uydiff;
										xr = axr;
										yr = ayr;
										xp = ((axf - axr) / (zxf - zxr) * (obj_z - zxr) + axr) / 1000.0;
										yp = ((ayf - ayr) / (zyf - zyr) * (obj_z - zyr) + ayr) / 1000.0;
										// HF-2017-11-29 end
									}
									hist2d[0].cumulate(xf, yf);
									hist2d[1].cumulate(xr, yr);
									if (allhcoin)
									{
										hist2d[2].cumulate(xf, yf);
										hist2d[3].cumulate(xr, yr);
									}
									if (all1cluster)
									{
										hist2d[4].cumulate(xf, yf);
										hist2d[5].cumulate(xr, yr);
									}
									double dx = xf - xr;
									double dy = yf - yr;
									hist2d[6].cumulate(dx, dy);
									hist2d[7].cumulate(dx, dy);
									if (allhcoin)
									{
										hist2d[8].cumulate(dx, dy);
										hist2d[9].cumulate(dx, dy);
									}
									if (all1cluster)
									{
										hist2d[10].cumulate(dx, dy);
										hist2d[11].cumulate(dx, dy);
									}
									hist2d[12].cumulate(xp, yp);
									hist2d[13].cumulate(xp, yp);
									if (allhcoin)
									{
										hist2d[14].cumulate(xp, yp);
										hist2d[15].cumulate(xp, yp);
									}
									if (all1cluster)
									{
										hist2d[16].cumulate(xp, yp);
										hist2d[17].cumulate(xp, yp);
									}
								}
							}
						}
					}
					else if ((recstr.size() > 4) &&
							(recstr.substr(0,4) == "RUNS"))
					{
						runsrec = recstr.substr(5);
					}
					else if ((recstr.size() > 4) &&
							(recstr.substr(0,4) == "RUNE"))
					{
						runerec = recstr.substr(5);
					} // end of "COIN" "RUNS" "RUNE" record processing
				} // while (getline (ifs, recstr))
				tsecdiff = tseclast - tsecfirst;
				duration = duration + tsecdiff;
				if ((prcsrec == "") && (runsrec != ""))
					prcsrec = runsrec;
				if (runerec != "")
					prcerec = runerec;
			} // if (!ifs) ... else
			time_t tstrt = MyTimer::UnixTime (runsrec);
			time_t tendt = MyTimer::UnixTime (runerec);
			double dft = 0.0;
			if ((tstrt != (time_t)(-1)) && (tendt != (time_t)(-1)))
				dft = difftime (tendt, tstrt);
			double coinrate = 0.0;
			double coinallrate = 0.0;
			double evtrate = 0.0;
			double allhcoinrate = 0.0;
			double allhevtrate = 0.0;
			double all1rate = 0.0;
			if (tsecdiff > 0.0)
			{
				coinrate = runnumcoin / tsecdiff;
				coinallrate = runcoinall / tsecdiff;
				evtrate = runnumevt / tsecdiff;
				allhcoinrate = runallhcoin / tsecdiff;
				allhevtrate = runallhevt / tsecdiff;
				all1rate = runall1cevt / tsecdiff;
			}
			ofl
				<< ',' << runsrec
				<< ',' << runerec
				<< ',' << tstrt
				<< ',' << tendt
				<< ',' << dft
				<< ',' << tsecfirst	// 2017-02-20
				<< ',' << tseclast	// 2017-02-20
				<< ',' << tsecdiff
				<< ',' << runnumcoin
				<< ',' << runcoinall
				<< ',' << runnumevt
				<< ',' << runallhcoin	// 2017-05-23
				<< ',' << runallhevt	// 2017-05-23
				<< ',' << runall1cevt	// 2017-05-11
				<< ',' << coinrate
				<< ',' << coinallrate
				<< ',' << evtrate
				<< ',' << allhcoinrate	// 2017-05-23
				<< ',' << allhevtrate	// 2017-05-23
				<< ',' << all1rate	// 2017-05-11
				<< std::endl;
		} // if (doit)
		liststr.clear();
	} // while (getline (ifl, liststr))
	mytimer.stop ();

///// OUTPUT section /////

	std::string edgecorryn = opt.m_edge_corr ? "yes" : "no";

	std::string setupfname = outname + "-setup.csv";
	std::ofstream ofsetup (setupfname.c_str());
	ofsetup.precision(16);
	ofsetup << "\"";
	for (int ia = 0; ia < argc; ia++)
	{
		if (ia)
			ofsetup << ' ';
		ofsetup << argv[ia];
	}
	ofsetup << "\",version," << VERSION_NUMBER << ',';
	mytimer.csvout (ofsetup);
	ofsetup << std::endl;
	ofsetup << "Name,z-pos,x0-pos,y0-pos"
		<< std::endl;
	ofsetup << "Proj.Plane" << ',' << obj_z << '\n';
	for (unsigned int n = 0; n < NUM_LAYERS; n++)
	{
		ofsetup << "X" << n
			<< ',' << xlayer_zpos [n]
			<< ',' << xlayer_xpos [n] << ','
			<< '\n';
	}
	for (unsigned int n = 0; n < NUM_LAYERS; n++)
	{
		ofsetup	<< "Y" << n
			<< ',' << ylayer_zpos [n]
			<< ',' << ',' << ylayer_ypos [n]
			<< '\n';
	}
	ofsetup << std::endl;
	ofsetup.close ();

	std::string hname = outname + ".html";
	std::ofstream ofh (hname.c_str());
	ofh
		<< "<!DOCTYPE html>\n"
		<< "<html>\n"
		<< "<head><title>coinimga version "
		<< VERSION_NUMBER
		<< " results ("
		<< MyTimer::TimeToStr (mytimer.start_time())
		<< ")</title></head>\n"
		<< "<body>\n"
		<< "<hr>\n<h1>coinimga version "
		<< VERSION_NUMBER
		<< " results ("
		<< MyTimer::TimeToStr (mytimer.start_time())
		<< ")</h1>\n<hr>\n"
		<< std::endl;

	ofh
		<< "<b>Command line:</b> ";
	for (int i = 0; i < argc; i++)
	{
		ofh << argv[i] << ' ';
	}
	ofh << "<br>" << std::endl;

	ofh
		<< "<b>Date and Time:</b>";
	mytimer.txtout (ofh);
	ofh << "<br>" << std::endl;

	ofh
		<< "<b>Method:</b> " << methodname << "<br>\n"
		<< "<b>Edge correction:</b> " << edgecorryn << "<br>\n"
		<< "<b>Projection plane:</b> " << projdist << " (m)<br>\n"
		<< "<b>Log file:</b> <a href=\"" << logfnam << "\">" << logfnam << "</a><br>\n"
		<< "<b>Setup file:</b> <a href=\"" << setupfname << "\">" << setupfname << "</a><br>\n"
		<< "<b>1st run start time:</b> " << prcsrec << "<br>\n"
		<< "<b>last run end time:</b> " << prcerec << "<br>\n"
		<< std::endl;

	ofh
		<< "<b>DAQtime(days):</b> " << (duration / 3600.0 / 24.0)
		<< ", Files: " << totalfile
		<< ", CoinRecords: " << totalcoin
		<< ", CoinAll: " << totalcoinall
		<< ", Events: " << totalevt
		<< ", AllHit: " << totalallhcoin
		<< ", AllHitEvents: " << totalallhevt
		<< ", AllSingle: " << totalall1cevt
		<< "<br>"
		<< std::endl;

	ofh << "<hr>" << std::endl;

	ofh
		<< "<table border=\"1\" >\n"
		<< "<tr>"
		<< std::endl;

	int maxtd = 6;
        int numtd = 0;
	for (int i = 0; i < NUM_HIST2D; i++)
	{
		std::stringstream ss;
		ss << outname << "-" << i;
		std::string ofnam;
		ss >> ofnam;
		std::string ofncsv;
		ofncsv = ofnam + ".csv";
		std::ofstream ofcsv (ofncsv.c_str());
		ofcsv.precision(16);
		ofcsv << "\"";
		for (int ia = 0; ia < argc; ia++)
		{
			if (ia)
				ofcsv << ' ';
			ofcsv << argv[ia];
		}
		ofcsv << "\",version," << VERSION_NUMBER << ',';
		mytimer.csvout (ofcsv);
		ofcsv
			<< ",maxhits," << maxhits
			<< ",\"unitdiff(x,y,z)\","
			<< uxdiff << ',' << uydiff << ',' << uzdiff
			<< ",\"planedist(x,y)\","
			<< pxdist << ',' << pydist
			<< ",projdist(m)" << ',' << projdist
			<< ",obj_z(mm)" << ',' << obj_z	// HF-2017-11-30
			<< ",method" << ',' << methodname
			<< ",edge_correction" << "," << edgecorryn	// HF-2017-12-01
			<< std::endl;
		ofcsv
			<< "Duration(sec)," << duration
			<< ",FirstRun Start," << prcsrec
			<< ",LastRun End," << prcerec
			<< ",Total Files," << totalfile
			<< ",Total CoinRec," << totalcoin
			<< ",Total CoinAll," << totalcoinall
			<< ",Total Events," << totalevt
			<< ",AllHit Coin," << totalallhcoin
			<< ",AllHit Events," << totalallhevt
			<< ",AllSingle Events," << totalall1cevt
			<< std::endl;
		hist2d [i].CSVdump (ofcsv);
		std::string ofnpng;
		ofnpng = ofnam + ".png";
		std::ofstream ofpng (ofnpng.c_str(), std::ios::binary);
		hist2d [i].PNGdump (ofpng);

		if (numtd == 0)
		{
			ofh << "<tr>"
				<< std::endl;
		}

		ofh
			<< "<td>"
			<< "<img src=\"" << ofnpng << "\" alt=\"" << ofnpng
			<< "\"><br>\n"
			<< hist2d[i].title()
			<< "<br>\n"
			<< "csv-file: <a href=\"" << ofncsv << "\">"
			<< ofncsv << "</a>"
			<< std::endl;
		++numtd;
		if (numtd >= maxtd)
		{
			ofh << "</tr>"
				<< std::endl;
			numtd = 0;
		}
	}
	if (numtd > 0)
	{
		ofh
			<< "</tr>"
			<< std::endl;
	}

	ofh
		<< "</table>"
		<< std::endl;

	ofh
		<< "</body>\n"
		<< "</html>"
		<< std::endl;

	return 0;
}
