// coinimgbr.cpp
// g++ -Wall coinimgbr.cpp coinimgopt.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp fit2dline.cpp hist2d.cpp ncpng.cpp mytimer.cpp -lz -o coinimgbr
//
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

static const int NUM_XBINS = 192;
static const int NUM_YBINS = 192;

static const int IMG_WIDTH = 192;
static const int IMG_HEIGHT = 192;

static const double unit_y_cntr[2] = { 139.0, 0.0 };
static const double unit_x_cntr[2] = { 0.0, 0.0 };

static const double unit_ydiff = 139.0;
static const double unit_zdiff = 495.0;

static const double xepos0[2] = { -237.5, -237.5 };
static const double xopos0[2] = { -232.5, -232.5 };
static const double yepos0[2] = { (-232.5 + unit_ydiff), -237.5 };
static const double yopos0[2] = { (-237.5 + unit_ydiff), -232.5 };

static const double xunit_zpos[4] = { 5.5, 15.5, (unit_zdiff - 5.5), (unit_zdiff - 15.5)};
static const double yunit_zpos[4] = { -15.5, -5.5, (unit_zdiff + 15.5), (unit_zdiff + 5.5)};

static const int NUM_HIST2D = 8;
mylibrary::Hist2D hist2d [NUM_HIST2D] =
{
	mylibrary::Hist2D ("front unit X-Y", -242.5, 242.5, 97, -242.5, 242.5, 97, true),
	mylibrary::Hist2D ("rear unit  X-Y", -242.5, 242.5, 97, -242.5, 242.5, 97, true),
	mylibrary::Hist2D ("dx-dy", -482.5, 482.5, 193, -482.5, 482.5, 193),
	mylibrary::Hist2D ("dx-dy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
	mylibrary::Hist2D ("ddx-ddy", -482.5, 482.5, 193, -482.5, 482.5, 193),
	mylibrary::Hist2D ("ddx-ddy smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
	mylibrary::Hist2D ("phi-cos", -0.965, 0.965, 193, -0.965, 0.965, 193),
	mylibrary::Hist2D ("phi-cos smooth", -0.965, 0.965, 193, -0.965, 0.965, 193, true),
};

static const char* monstr[] =
{
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec"
};

time_t
UnixTime (const std::string& timestr)
{
	if (timestr.size() < 24)
		return (time_t)(-1);

	struct tm tmtmp;

	// Month
	int m = -1;
	for (int i = 0; i < 12; i++)
	{
		if (timestr.substr(4,3) == monstr[i])
		{
			m = i;
			break;
		}
	}
	if (m < 0)
		return (time_t)(-1);
	tmtmp.tm_mon = m;

	int ibuf[4];

	// Day
	if (timestr[ 8] == ' ')
		ibuf[0] = 0;
	else
		ibuf[0] = (int)(timestr[ 8] - '0');
	ibuf[1] = (int)(timestr[ 9] - '0');
	tmtmp.tm_mday = ibuf[0] * 10 + ibuf[1];

	// Hour
	if (timestr[11] == ' ')
		ibuf[0] = 0;
	else
		ibuf[0] = (int)(timestr[11] - '0');
	ibuf[1] = (int)(timestr[12] - '0');
	tmtmp.tm_hour = ibuf[0] * 10 + ibuf[1];

	// Minuit
	if (timestr[14] == ' ')
		ibuf[0] = 0;
	else
		ibuf[0] = (int)(timestr[14] - '0');
	ibuf[1] = (int)(timestr[15] - '0');
	tmtmp.tm_min = ibuf[0] * 10 + ibuf[1];

	// Sec
	if (timestr[17] == ' ')
		ibuf[0] = 0;
	else
		ibuf[0] = (int)(timestr[17] - '0');
	ibuf[1] = (int)(timestr[18] - '0');
	tmtmp.tm_sec = ibuf[0] * 10 + ibuf[1];
	
	// Year
	ibuf[0] = (int)(timestr[20] - '0');
	ibuf[1] = (int)(timestr[21] - '0');
	ibuf[2] = (int)(timestr[22] - '0');
	ibuf[3] = (int)(timestr[23] - '0');
	tmtmp.tm_year = ibuf[0] * 1000 + ibuf[1] * 100 + ibuf[2] * 10 + ibuf[3] - 1900;

	// Other members
	tmtmp.tm_wday = 0;	// will be set by mktime
	tmtmp.tm_yday = 0;	// will be set by mktime
	tmtmp.tm_isdst = 0;	// will be set by mktime

	return mktime (&tmtmp);
}

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
		outname = "coinimgbr";
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

							for (unsigned int u = 0; u < NUM_UNITS; u++)
							{
								if (coinrec.numdat(u) > 0)
								{
									tsecnow = coinrec.microsec(u) / 1000000.0;
									break;
								}
							}
							if (newrun)
							{
								tsecfirst = tsecnow;
								newrun = false;
							}
							tseclast = tsecnow;

							// Unit assignments
							// int uidy2 = 0;  //UNIT#7
							// int uidx2 = 1;  //UNIT#8
							int uidx2 = 0;
							int uidy2 = 1;
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
								// position in the device plane
								double zx1 = (xunit_zpos[0] + xunit_zpos[1]) * 0.5;
								double zx2 = (xunit_zpos[2] + xunit_zpos[3]) * 0.5;
								double zy1 = (yunit_zpos[0] + yunit_zpos[1]) * 0.5;
								double zy2 = (yunit_zpos[2] + yunit_zpos[3]) * 0.5;
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
			time_t tstrt = UnixTime (runsrec);
			time_t tendt = UnixTime (runerec);
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
		<< "<head><title>coinimgbr results</title></head>\n"
		<< "<body>\n"
		<< "<hr>\n<h1>coinimgbr results</h1>\n<hr>\n"
		<< std::endl;

	ofh
		<< "<b>Command line:</b> ";
	for (int i = 0; i < argc; i++)
	{
		ofh << argv[i] << ' ';
	}
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
		if (eoselect == 1)
			ss << outname << "-" << i << "e";
		else if (eoselect == 2)
			ss << outname << "-" << i << "o";
		else
			ss << outname << "-" << i;
		std::string ofnam;
		ss >> ofnam;
		std::string ofncsv;
		ofncsv = ofnam + ".csv";
		std::ofstream ofcsv (ofncsv.c_str());
		mytimer.csvout (ofcsv);
		ofcsv
			<< ",maxhits," << maxhits
			<< ",eoselect," << eoselect
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
