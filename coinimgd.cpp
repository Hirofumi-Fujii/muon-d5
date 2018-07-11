// coinimgd.cpp
// g++ -Wall coinimgd.cpp coinimgopt.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp hist2d.cpp ncpng.cpp mytimer.cpp -lz -o coinimgd
//
// 08-Nov-2016 copy from coinimga, coinimgopt has been changed (added m_dist_given and m_dist)
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
#include <cmath>
#include "gzfstream.h"
#include "coinimgopt.h"
#include "coinrecord.h"
#include "hist2d.h"
#include "ncpng.h"
#include "mytimer.h"

static const unsigned int NUM_UNITS = 4;

static const int NUM_XBINS = 193;
static const int NUM_YBINS = 193;

static const int IMG_WIDTH = 193;
static const int IMG_HEIGHT = 193;

static const int NUM_HIST2D = 12;
mylibrary::Hist2D hist2d [NUM_HIST2D] =
{
	mylibrary::Hist2D ("front unit even-odd X", -2.5, 472.5, 95, -2.5, 472.5, 95),
	mylibrary::Hist2D ("front unit even-odd Y", -2.5, 472.5, 95, -2.5, 472.5, 95),
	mylibrary::Hist2D ("front unit even-odd diff.", -237.5, 237.5, 95, -237.5, 237.5, 95),
	mylibrary::Hist2D ("front unit X-Y", -2.5, 472.5, 95, -2.5, 472.5, 95),
	mylibrary::Hist2D ("rear unit even-odd X", -2.5, 472.5, 95, -2.5, 472.5, 95),
	mylibrary::Hist2D ("rear unit even-odd Y", -2.5, 472.5, 95, -2.5, 472.5, 95),
	mylibrary::Hist2D ("rear unit even-odd diff.", -237.5, 237.5, 95, -237.5, 237.5, 95),
	mylibrary::Hist2D ("rear unit X-Y", -2.5, 472.5, 95, -2.5, 472.5, 95),
	mylibrary::Hist2D ("straight front X-Y", -2.5, 472.5, 95, -2.5, 472.5, 95),
	mylibrary::Hist2D ("straight front X-Y smooth", -2.5, 472.5, 95, -2.5, 472.5, 95, true),
	mylibrary::Hist2D ("straight rear X-Y", -2.5, 472.5, 95, -2.5, 472.5, 95),
	mylibrary::Hist2D ("straight rear X-Y smooth", -2.5, 472.5, 95, -2.5, 472.5, 95, true),
};

// shits of the plane positions in mm units.
// 0:DAQ#7(Y2),1:DAQ#8(X2),2:DAQ#9(Y1),3:DAQ#10(X1)

static const double ppos_shift[NUM_UNITS][2] =
{
	{ -2.5,  2.5 },
	{ -2.5,  2.5 },
	{  2.5, -2.5 },
	{ -2.5,  2.5 },
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
	std::string listfilename;
	listfilename = opt.m_listfilename;
	std::string outname = opt.m_outname;
	if (outname == "")
		outname = "coinimgd";
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


							if (coinrec.xy1cluster(uidx1, maxhits) &&
							    coinrec.xy1cluster(uidy1, maxhits) &&
							    coinrec.xy1cluster(uidx2, maxhits) &&
							    coinrec.xy1cluster(uidy2, maxhits))
							{
								// all layers are single-cluster
								// front unit
								double xe1 = (double)(coinrec.xpos(uidx1)) + ppos_shift[uidx1][0];
								double xo1 = (double)(coinrec.ypos(uidx1)) + ppos_shift[uidx1][1];
								double ye1 = 470.0 - (double)(coinrec.xpos(uidy1)) + ppos_shift[uidy1][0];
								double yo1 = 470.0 - (double)(coinrec.ypos(uidy1)) + ppos_shift[uidy1][1];
								double xa1 = (xe1 + xo1) * 0.5;
								double ya1 = (ye1 + yo1) * 0.5;
								hist2d[0].cumulate(xe1, xo1);
								hist2d[1].cumulate(ye1, yo1);
								hist2d[2].cumulate((xe1 - xo1), (ye1 - yo1));
								hist2d[3].cumulate(xa1, ya1);
								// rear unit
								double xe2 = (double)(coinrec.xpos(uidx2)) + ppos_shift[uidx2][0];
								double xo2 = (double)(coinrec.ypos(uidx2)) + ppos_shift[uidx2][1];
								double ye2 = (double)(coinrec.xpos(uidy2)) + ppos_shift[uidy2][0];
								double yo2 = (double)(coinrec.ypos(uidy2)) + ppos_shift[uidy2][1];
								double xa2 = (xe2 + xo2) * 0.5;
								double ya2 = (ye2 + yo2) * 0.5;
								hist2d[4].cumulate(xe2, xo2);
								hist2d[5].cumulate(ye2, yo2);
								hist2d[6].cumulate((xe2 - xo2), (ye2 - yo2));
								hist2d[7].cumulate(xa2, ya2);
								//
								// dx-dy
								bool histit = true;
								if ( fabs(xa1 - xa2) > 12.5 )
									histit = false;
								if ( fabs(ya1 - ya2) > 12.5 )
									histit = false;
								if (histit)
								{
									totalevt += 1.0;
									runnumevt += 1.0;
									double dx = xa1 - xa2;
									double dy = ya1 - ya2;
									hist2d[8].cumulate(xa1, ya1);
									hist2d[9].cumulate(xa1, ya1);
									hist2d[10].cumulate(xa2, ya2);
									hist2d[11].cumulate(xa2, ya2);
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
		<< "<head><title>coinimga results</title></head>\n"
		<< "<body>\n"
		<< "<hr>\n<h1>coinimga results</h1>\n<hr>\n"
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
			<< ",maxhits," << maxhits;
		if (opt.m_xenarrow)
			ofcsv << ",xenarrow";
		else
			ofcsv << ",xewide";
		if (opt.m_xonarrow)
			ofcsv << ",xonarrow";
		else
			ofcsv << ",xowide";
		ofcsv
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
