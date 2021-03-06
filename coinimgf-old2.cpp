// coinimgf.cpp
// g++ -Wall coinimgf.cpp coinimgopt.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp fit2dline.cpp hist2d.cpp ncpng.cpp mytimer.cpp -lz -o coinimgf
//
// Make hit image by fitting method.
// 10-May-2017 coinrecord.Read () function is corrected to allow single group hit.
// 21-Feb-2017 Copy from coinimgb.cpp
//
// 20-Feb-2017 Protect against all 0 COIN events, added FirstEventTime and LastEventTime in log file.
//
// 27-May-2016 Add command line for each csv-file.
// 26-May-2016 Add opt.m_dxshift and opt.m_dyshift for xunit[0] and yunit[0]
// 06-May-2016 output position of the 'duration' has been changed
// 06-May-2016 logfile added
// 27-Apr-2016 coinimgopt added
// 26-Mar-2016 fit2dline version
// 17-Mar-2016 coinimgb -- test for coinrec
// 31-Mar-2016 modified for detector3
// 5-Mar-2016 modified for detector4
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include "gzfstream.h"
#include "coinimgopt.h"
#include "coinrecord.h"
#include "fit2dline.h"
#include "hist2d.h"
#include "ncpng.h"
#include "mytimer.h"

static const unsigned int NUM_UNITS = 4;

static const double unit_ydiff = 139.0;
static const double unit_zdiff = 495.0;

// channel 0 center position in the unit
static const double unit_xch0pos = -235.0;
static const double unit_ych0pos = -235.0;

// channel 0 center position in physical coordinate
// [0] is front unit and [1] is rear unit
static const double def_xepos0[2] = { (unit_xch0pos - 2.5), (unit_xch0pos - 2.5) };
static const double def_xopos0[2] = { (unit_xch0pos + 2.5), (unit_xch0pos + 2.5) };
static const double def_yepos0[2] = { (unit_ych0pos + 2.5 + unit_ydiff), (unit_ych0pos - 2.5) };
static const double def_yopos0[2] = { (unit_ych0pos - 2.5 + unit_ydiff), (unit_ych0pos + 2.5) };

// The followings are correspondig to {front-even, front-odd, rear-even, rear-odd}
// Position zero is the center of the front-unit
static const double xunit_zpos[4] = { 5.5, 15.5, (unit_zdiff - 5.5), (unit_zdiff - 15.5)};
static const double yunit_zpos[4] = { -15.5, -5.5, (unit_zdiff + 15.5), (unit_zdiff + 5.5)};

int main (int argc, char* argv[])
{
	using namespace mylibrary;

	CoinimgOpt opt;

	long totalfile = 0;
	unsigned long listlineno = 0;
	double duration = 0.0;
	double totalcoin = 0.0;
	double totalcoinall = 0.0;
	double totalevt = 0.0;
	double totalall1evt = 0.0;

	// check the arguments
	if (!opt.setargs (argc, argv))
	{
		opt.usage (std::cout, argv[0]);
		return (-1);
	}

	std::string listfilename = opt.m_listfilename;
	std::string outname = opt.m_outname;
	if (outname == "")
		outname = "coinimgf";
	int maxhits = opt.m_maxhits;
	int eoselect = opt.m_evenodd_select;
	double projdist = 25.3;
	if (opt.m_dist_given)
		projdist = opt.m_dist;

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

	double tsecnow = 0.0;
	double tsecfirst = 0.0;
	double tseclast = 0.0;
	double tsecdiff = 0.0;
	double runnumcoin = 0.0;
	double runcoinall = 0.0;
	double runnumevt = 0.0;
	double runall1evt = 0.0;
	std::string runsrec = "";
	std::string runerec = "";

	double xepos0[2];
	double xopos0[2];
	double yepos0[2];
	double yopos0[2];

	xepos0[0] = def_xepos0[0] + opt.m_dxshift;
	xopos0[0] = def_xopos0[0] + opt.m_dxshift;
	xepos0[1] = def_xepos0[1];
	xopos0[1] = def_xopos0[1];

	yepos0[0] = def_yepos0[0] + opt.m_dyshift;
	yopos0[0] = def_yopos0[0] + opt.m_dyshift;
	yepos0[1] = def_yepos0[1];
	yopos0[1] = def_yopos0[1];

	// Z position of the device plane
	double zx1 = (xunit_zpos[0] + xunit_zpos[1]) * 0.5;
	double zx2 = (xunit_zpos[2] + xunit_zpos[3]) * 0.5;
	double zy1 = (yunit_zpos[0] + yunit_zpos[1]) * 0.5;
	double zy2 = (yunit_zpos[2] + yunit_zpos[3]) * 0.5;

	// Average unit distance
	double udist = ((zx2 - zx1) + (zy2 - zy1)) * 0.5;

	// Z position of the measuring object
	double obj_z = -(projdist * 1000.0);	// m -> mm

	// Scale factor
	double scale = (0.0 - obj_z) / udist;

	// Range on the projection plane
	double xpmax =  235.0 * (scale + 1.0) + 235.0 * scale;
	double xpmin = -235.0 * (scale + 1.0) - 235.0 * scale;
	double ypmax = (unit_ydiff + 235.0) * (scale + 1.0) + 235.0 * scale;
	double ypmin = (unit_ydiff - 235.0) * (scale + 1.0) - 235.0 * scale;

	static const int NUM_HIST2D = 20;
	Hist2D hist2d [NUM_HIST2D] =
	{
		Hist2D ("front unit X-Y", -242.5, 242.5, 97, -242.5, 242.5, 97, true),
		Hist2D ("rear unit  X-Y", -242.5, 242.5, 97, -242.5, 242.5, 97, true),
		Hist2D ("1:front unit X-Y", -242.5, 242.5, 97, -242.5, 242.5, 97, true),
		Hist2D ("1:rear unit  X-Y", -242.5, 242.5, 97, -242.5, 242.5, 97, true),
		Hist2D ("dx-dy", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("dx-dy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("1:dx-dy", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("1:dx-dy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("ddx-ddy", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("ddx-ddy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("1:ddx-ddy", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("1:ddx-ddy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("phi-cos", -0.965, 0.965, 193, -0.965, 0.965, 193),
		Hist2D ("phi-cos smooth", -0.965, 0.965, 193, -0.965, 0.965, 193, true),
		Hist2D ("1:phi-cos", -0.965, 0.965, 193, -0.965, 0.965, 193),
		Hist2D ("1:phi-cos smooth", -0.965, 0.965, 193, -0.965, 0.965, 193, true),
		Hist2D ("xp-yp", xpmin, xpmax, 193, ypmin, ypmax, 193),
		Hist2D ("xp-yp smooth", xpmin, xpmax, 193, ypmin, ypmax, 193, true),
		Hist2D ("1:xp-yp", xpmin, xpmax, 193, ypmin, ypmax, 193),
		Hist2D ("1:xp-yp smooth", xpmin, xpmax, 193, ypmin, ypmax, 193, true),
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
		<< ",CoinAll"
		<< ",Events"
		<< ",All1Events"		// 2017-05-11
		<< ",CoinRate(1/sec)"
		<< ",CoinAllRate(1/sec)"
		<< ",EventRate(1/sec)"
		<< ",All1Rate(1/sec)"		// 2017-05-11
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
			runall1evt = 0.0;
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
							}

							bool x1ok = false;
							bool y1ok = false;
							bool x2ok = false;
							bool y2ok = false;

							int nxdat = 0;
							int nydat = 0;
							double xdat [NUM_UNITS];
							double xpos [NUM_UNITS];
							double ydat [NUM_UNITS];
							double ypos [NUM_UNITS];

							// Front X
							if (coinrec.A1cluster (uidx1, maxhits))
							{					
								xdat[nxdat] = (double)(coinrec.Apos(uidx1)) + xepos0[0];
								xpos[nxdat] = xunit_zpos[0];
								++nxdat;
								x1ok = true;								
							}
							if (coinrec.B1cluster (uidx1, maxhits))
							{
								xdat[nxdat] = (double)(coinrec.Bpos(uidx1)) + xopos0[0];
								xpos[nxdat] = xunit_zpos[1];
								++nxdat;
								x1ok = true;
							}

							// Front Y (Note that Y-direction is from top to bottom)
							if (coinrec.A1cluster (uidy1, maxhits))
							{					
								ydat[nydat] = 470.0 - (double)(coinrec.Apos(uidy1)) + yepos0[0];
								ypos[nydat] = yunit_zpos[0];
								++nydat;
								y1ok = true;
							}
							if (coinrec.B1cluster (uidy1, maxhits))
							{
								ydat[nydat] = 470.0 - (double)(coinrec.Bpos(uidy1)) + yopos0[0];
								ypos[nydat] = yunit_zpos[1];
								++nydat;
								y1ok = true;
							}

							// Rear X
							if (coinrec.A1cluster (uidx2, maxhits))
							{					
								xdat[nxdat] = (double)(coinrec.Apos(uidx2)) + xepos0[1];
								xpos[nxdat] = xunit_zpos[2];
								++nxdat;
								x2ok = true;
							}
							if (coinrec.B1cluster (uidx2, maxhits))
							{
								xdat[nxdat] = (double)(coinrec.Bpos(uidx2)) + xopos0[1];
								xpos[nxdat] = xunit_zpos[3];
								++nxdat;
								x2ok = true;
							}

							// Rear Y
							if (coinrec.A1cluster (uidy2, maxhits))
							{					
								ydat[nydat] = (double)(coinrec.Apos(uidy2)) + yepos0[1];
								ypos[nydat] = yunit_zpos[2];
								++nydat;
								y2ok = true;
							}
							if (coinrec.B1cluster (uidy2, maxhits))
							{
								ydat[nydat] = (double)(coinrec.Bpos(uidy2)) + yopos0[1];
								ypos[nydat] = yunit_zpos[3];
								++nydat;
								y2ok = true;
							}

							if (x1ok && x2ok && y1ok && y2ok)
							{
								bool all1evt = false;
								if ((nxdat > 3) && (nydat > 3))
									all1evt = true;
								totalevt += 1.0;
								runnumevt += 1.0;
								if (all1evt)
								{
									totalall1evt += 1.0;
									runall1evt += 1.0;
								}
								mylibrary::Fit2DLine xline (nxdat, xpos, xdat);
								mylibrary::Fit2DLine yline (nydat, ypos, ydat);
								double x1 = xline.y (0.0);
								double x2 = xline.y (unit_zdiff);
								double y1 = yline.y (0.0) - unit_ydiff;
								double y2 = yline.y (unit_zdiff);
								double dx = x1 - x2;
								double dy = y1 - y2;
								hist2d[0].cumulate (x1, y1);
								hist2d[1].cumulate (x2, y2);
								if (all1evt)
								{
									hist2d[2].cumulate (x1, y1);
									hist2d[3].cumulate (x2, y2);
								}
								hist2d[4].cumulate (dx, dy);
								hist2d[5].cumulate (dx, dy);
								if (all1evt)
								{
									hist2d[6].cumulate (dx, dy);
									hist2d[7].cumulate (dx, dy);
								}
								double xd1 = xline.y (zx1);
								double yd1 = yline.y (zy1) - unit_ydiff;
								double xd2 = xline.y (zx2);
								double yd2 = yline.y (zy2);
								double ddx = xd1 - xd2;
								double ddy = yd1 - yd2;
								hist2d[8].cumulate (ddx, ddy);
								hist2d[9].cumulate (ddx, ddy);
								if (all1evt)
								{
									hist2d[10].cumulate (ddx, ddy);
									hist2d[11].cumulate (ddx, ddy);
								}
								// phi-cos(theta)
								double ax = -xline.a();
								double ay = -yline.a();
								double phi = atan (ax);
								double cst = ay / sqrt(ax * ax + ay * ay + 1.0);
								hist2d[12].cumulate (phi, cst);
								hist2d[13].cumulate (phi, cst);
								if (all1evt)
								{
									hist2d[14].cumulate (phi, cst);
									hist2d[15].cumulate (phi, cst);
								}
								// position on the projection plane
								double xp = xline.y (obj_z);
								double yp = yline.y (obj_z);
								hist2d[16].cumulate (xp, yp);
								hist2d[17].cumulate (xp, yp);
								if (all1evt)
								{
									hist2d[18].cumulate (xp, yp);
									hist2d[19].cumulate (xp, yp);
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
			} // if (!ifs) ... else
			time_t tstrt = MyTimer::UnixTime (runsrec);
			time_t tendt = MyTimer::UnixTime (runerec);
			double dft = 0.0;
			if ((tstrt != (time_t)(-1)) && (tendt != (time_t)(-1)))
				dft = difftime (tendt, tstrt);
			double coinrate = 0.0;
			double coinallrate = 0.0;
			double evtrate = 0.0;
			double all1rate = 0.0;
			if (tsecdiff > 0.0)
			{
				coinrate = runnumcoin / tsecdiff;
				coinallrate = runcoinall / tsecdiff;
				evtrate = runnumevt / tsecdiff;
				all1rate = runall1evt / tsecdiff;
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
				<< ',' << runall1evt	// 2017-05-11
				<< ',' << coinrate
				<< ',' << coinallrate
				<< ',' << evtrate
				<< ',' << all1rate	// 2017-05-11
				<< std::endl;
		} // if (doit)
		liststr.clear();
	} // while (getline (ifl, liststr))
	mytimer.stop ();

///// OUTPUT section /////

	std::string hname = outname + ".html";

	std::ofstream ofh (hname.c_str());
	ofh
		<< "<!DOCTYPE html>\n"
		<< "<html>\n"
		<< "<head><title>coinimgf results ("
		<< MyTimer::TimeToStr (mytimer.start_time())
		<< ")</title></head>\n"
		<< "<body>\n"
		<< "<hr>\n<h1>coinimgf results ("
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
		<< "<b>Date and time:</b> ";
	mytimer.txtout (ofh);
	ofh << "<br>" << std::endl;

	ofh
		<< "Log file: <a href=\"" << logfnam << "\">"
		<< logfnam << "</a>"
		<< "<br>"
		<< std::endl;

	ofh
		<< "DAQtime(days): " << (duration / 3600.0 / 24.0)
		<< ", Files: " << totalfile
		<< ", CoinRecords: " << totalcoin
		<< ", CoinAll: " << totalcoinall
		<< ", Events: " << totalevt
		<< ", AllSingle: " << totalall1evt
		<< "<br>"
		<< std::endl;

	ofh << "<hr>" << std::endl;

	ofh
		<< "<table border=\"1\" >\n"
		<< "<tr>"
		<< std::endl;

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

		ofcsv << "\"";
		for (int ia = 0; ia < argc; ia++)
		{
			if (ia)
				ofcsv << ' ';
			ofcsv << argv[ia];
		}
		ofcsv << "\",";
		mytimer.csvout (ofcsv);
		ofcsv
			<< ",maxhits," << maxhits
			<< ",eoselect," << eoselect
			<< ",unit_zdiff," << unit_zdiff
			<< ",unit_ydiff," << unit_ydiff
			<< ",xdist," << (zx2 - zx1)
			<< ",ydist," << (zy2 - zy1)
			<< ",dxshift," << opt.m_dxshift
			<< ",dyshift," << opt.m_dyshift
			<< ",projdist," << projdist
			<< std::endl;

		ofcsv
			<< "Duration(sec)," << duration
			<< ",Total Files," << totalfile
			<< ",Total CoinRec," << totalcoin
			<< ",Total CoinAll," << totalcoinall
			<< ",Total Events," << totalevt
			<< ",Total AllSingle," << totalall1evt
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
		if (numtd >= 4)
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
