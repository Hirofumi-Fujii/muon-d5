// coinimgb.cpp
// g++ -Wall coinimgb.cpp coinimgopt.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp fit2dline.cpp hist2d.cpp ncpng.cpp mytimer.cpp -lz -o coinimgb
//
// 21-Feb-2017 UnixTime () is moved into MyTimer class.
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

	// check the arguments
	if (!opt.setargs (argc, argv))
	{
		opt.usage (std::cout, argv[0]);
		return (-1);
	}

	std::string listfilename = opt.m_listfilename;
	std::string outname = opt.m_outname;
	if (outname == "")
		outname = "coinimgb";
	int maxhits = opt.m_maxhits;
	int eoselect = opt.m_evenodd_select;

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
	double obj_z = -opt.m_dist;

	// Scale factor
	double scale = (0.0 - obj_z) / udist;

	// Range on the projection plane
	double xpmax =  235.0 * (scale + 1.0) + 235.0 * scale;
	double xpmin = -235.0 * (scale + 1.0) - 235.0 * scale;
	double ypmax = (unit_ydiff + 235.0) * (scale + 1.0) + 235.0 * scale;
	double ypmin = (unit_ydiff - 235.0) * (scale + 1.0) - 235.0 * scale;

	static const int NUM_HIST2D = 10;
	Hist2D hist2d [NUM_HIST2D] =
	{
		Hist2D ("front unit X-Y", -242.5, 242.5, 97, -242.5, 242.5, 97, true),
		Hist2D ("rear unit  X-Y", -242.5, 242.5, 97, -242.5, 242.5, 97, true),
		Hist2D ("dx-dy", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("dx-dy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("ddx-ddy", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("ddx-ddy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("phi-cos", -0.965, 0.965, 193, -0.965, 0.965, 193),
		Hist2D ("phi-cos smooth", -0.965, 0.965, 193, -0.965, 0.965, 193, true),
		Hist2D ("xp-yp", xpmin, xpmax, 193, ypmin, ypmax, 193),
		Hist2D ("xp-yp smooth", xpmin, xpmax, 193, ypmin, ypmax, 193, true),
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
		<< ",CoinRate(1/sec)"
		<< ",CoinAllRate(1/sec)"
		<< ",EventRate(1/sec)"
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

							bool histit = false;
							if (eoselect == 1)
							{
								if
								(
									coinrec.A1cluster(uidx1, maxhits) &&
									coinrec.A1cluster(uidy1, maxhits) &&
									coinrec.A1cluster(uidx2, maxhits) &&
									coinrec.A1cluster(uidy2, maxhits)
								)
									histit = true;
							}
							else if (eoselect == 2)
							{
								if
								(
									coinrec.B1cluster(uidx1, maxhits) &&
									coinrec.B1cluster(uidy1, maxhits) &&
									coinrec.B1cluster(uidx2, maxhits) &&
									coinrec.B1cluster(uidy2, maxhits)
								)
									histit = true;
							}
							else
							{
								if
								(
									coinrec.xy1cluster(uidx1, maxhits) &&
									coinrec.xy1cluster(uidy1, maxhits) &&
									coinrec.xy1cluster(uidx2, maxhits) &&
									coinrec.xy1cluster(uidy2, maxhits)
								)
									histit = true;
							}

							if (histit)
							{
								totalevt += 1.0;
								runnumevt += 1.0;
								double xdat[4];
								double xpos[4];
								int nxdat;
								double ydat[4];
								double ypos[4];
								int nydat;
								if (eoselect == 1)
								{
									xdat[0] = (double)(coinrec.xpos(uidx1)) + xepos0[0];
									xdat[1] = (double)(coinrec.xpos(uidx2)) + xepos0[1];
									xpos[0] = xunit_zpos[0];
									xpos[1] = xunit_zpos[2];
									nxdat = 2;
									ydat[0] = 470.0 - (double)(coinrec.xpos(uidy1)) + yepos0[0];
									ydat[1] = (double)(coinrec.xpos(uidy2)) + yepos0[1];
									ypos[0] = yunit_zpos[0];
									ypos[1] = yunit_zpos[2];
									nydat = 2;
								}
								else if (eoselect == 2)
								{
									xdat[0] = (double)(coinrec.ypos(uidx1)) + xopos0[0];
									xdat[1] = (double)(coinrec.ypos(uidx2)) + xopos0[1];
									xpos[0] = xunit_zpos[1];
									xpos[1] = xunit_zpos[3];
									nxdat = 2;
									ydat[0] = 470.0 - (double)(coinrec.ypos(uidy1)) + yopos0[0];
									ydat[1] = (double)(coinrec.ypos(uidy2)) + yopos0[1];
									ypos[0] = yunit_zpos[1];
									ypos[1] = yunit_zpos[3];
									nydat = 2;
								}
								else
								{
									xdat[0] = (double)(coinrec.xpos(uidx1)) + xepos0[0];
									xdat[1] = (double)(coinrec.ypos(uidx1)) + xopos0[0];
									xdat[2] = (double)(coinrec.xpos(uidx2)) + xepos0[1];
									xdat[3] = (double)(coinrec.ypos(uidx2)) + xopos0[1];
									xpos[0] = xunit_zpos[0];
									xpos[1] = xunit_zpos[1];
									xpos[2] = xunit_zpos[2];
									xpos[3] = xunit_zpos[3];
									nxdat = 4;
									ydat[0] = 470.0 - (double)(coinrec.xpos(uidy1)) + yepos0[0];
									ydat[1] = 470.0 - (double)(coinrec.ypos(uidy1)) + yopos0[0];
									ydat[2] = (double)(coinrec.xpos(uidy2)) + yepos0[1];
									ydat[3] = (double)(coinrec.ypos(uidy2)) + yopos0[1];
									ypos[0] = yunit_zpos[0];
									ypos[1] = yunit_zpos[1];
									ypos[2] = yunit_zpos[2];
									ypos[3] = yunit_zpos[3];
									nydat = 4;
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
								hist2d[2].cumulate (dx, dy);
								hist2d[3].cumulate (dx, dy);
								double xd1 = xline.y (zx1);
								double yd1 = yline.y (zy1) - unit_ydiff;
								double xd2 = xline.y (zx2);
								double yd2 = yline.y (zy2);
								double ddx = xd1 - xd2;
								double ddy = yd1 - yd2;
								hist2d[4].cumulate (ddx, ddy);
								hist2d[5].cumulate (ddx, ddy);
								// phi-cos(theta)
								double ax = -xline.a();
								double ay = -yline.a();
								double phi = atan (ax);
								double cst = ay / sqrt(ax * ax + ay * ay + 1.0);
								hist2d[6].cumulate (phi, cst);
								hist2d[7].cumulate (phi, cst);
								// position on the projection plane
								double xp = xline.y (obj_z);
								double yp = yline.y (obj_z);
								hist2d[8].cumulate (xp, yp);
								hist2d[9].cumulate (xp, yp);
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
			if (tsecdiff > 0.0)
			{
				coinrate = runnumcoin / tsecdiff;
				coinallrate = runcoinall / tsecdiff;
				evtrate = runnumevt / tsecdiff;
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
				<< ',' << coinrate
				<< ',' << coinallrate
				<< ',' << evtrate
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
		<< "<head><title>coinimgb results</title></head>\n"
		<< "<body>\n"
		<< "<hr>\n<h1>coinimgb results</h1>\n<hr>\n"
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
			<< std::endl;

		ofcsv
			<< "Duration(sec)," << duration
			<< ",Total Files," << totalfile
			<< ",Total CoinRec," << totalcoin
			<< ",Total CoinAll," << totalcoinall
			<< ",Total Events," << totalevt
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
