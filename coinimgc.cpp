// coinimgc.cpp
// g++ -Wall coinimgc.cpp coinimgopt.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp hist2d.cpp ncpng.cpp mytimer.cpp -lz -o coinimgc
//
// 27-Nov-2017 straight track selection parameters dxmax, dymax are added.
// 11-Oct-2017 modified for d5.
// 21-Feb-2017 UnixTime () is moved into MyTimer class.
// 20-Feb-2017 Protect against all 0 COIN events, added FirstEventTime and LastEventTime in log file.
//
// 07-Oct-2016 Single plane histogram
// 18-Jul-2016 copy from coinimga
//
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

static const int NUM_X_CHANNELS = 48;
static const int NUM_Y_CHANNELS = 48;

static const int NUM_XBINS = (NUM_X_CHANNELS * 2 - 1);
static const int NUM_YBINS = (NUM_Y_CHANNELS * 2 - 1);

static const int NUM_HIST2D = 9;
mylibrary::Hist2D hist2d [NUM_HIST2D] =
{
	mylibrary::Hist2D ("All front unit X-Y", -5.0, 475.0, NUM_X_CHANNELS, -5.0, 475.0, NUM_Y_CHANNELS),
	mylibrary::Hist2D ("All rear unit X-Y", -5.0, 475.0, NUM_X_CHANNELS, -5.0, 475.0, NUM_Y_CHANNELS),
	mylibrary::Hist2D ("Coin front unit X-Y", -5.0, 475.0, NUM_X_CHANNELS, -5.0, 475.0, NUM_Y_CHANNELS),
	mylibrary::Hist2D ("Coin rear unit X-Y", -5.0, 475.0, NUM_X_CHANNELS, -5.0, 475.0, NUM_Y_CHANNELS),
	mylibrary::Hist2D ("0:dx-dy", -475.0, 475.0, NUM_XBINS, -475.0, 475.0, NUM_YBINS),
	mylibrary::Hist2D ("1:dx-dy", -475.0, 475.0, NUM_XBINS, -475.0, 475.0, NUM_YBINS),
	mylibrary::Hist2D ("2:dx-dy", -475.0, 475.0, NUM_XBINS, -475.0, 475.0, NUM_YBINS),
	mylibrary::Hist2D ("ST front unit X-Y", -5.0, 475.0, NUM_X_CHANNELS, -5.0, 475.0, NUM_Y_CHANNELS),
	mylibrary::Hist2D ("ST rear unit X-Y", -5.0, 475.0, NUM_X_CHANNELS, -5.0, 475.0, NUM_Y_CHANNELS),
};


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

	bool x1even = false;
	bool x2even = false;
	bool y1even = false;
	bool y2even = false;

	// check the arguments
	if (!opt.setargs (argc, argv))
	{
		opt.usage (std::cout, argv[0]);
		return (-1);
	}
	std::string listfilename;
	listfilename = opt.m_listfilename;
	std::string combname = opt.m_combname;
	if (combname == "")
		combname = "EOEO";
	if (combname.size() != 4)
	{
		std::cerr << "ERROR: wrong combname (" << combname << ")" << std::endl;
		return (-2);
	}
	if (combname[0] == 'E')
		x1even = true;
	if (combname[1] == 'E')
		x2even = true;
	if (combname[2] == 'E')
		y1even = true;
	if (combname[3] == 'E')
		y2even = true;

	std::string outname;
	if (opt.m_outname == "")
		outname = "coinimgc-" + combname;
	else
		outname = opt.m_outname + "-" + combname;

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

							bool allhit = true;
							bool allsingle = true;

							// Front X
							bool x1ok = false;
							double x1;
							if (x1even)
							{
								if (coinrec.A1cluster (uidx1, maxhits))
								{
									x1 = (double)(coinrec.xpos(uidx1));
									x1ok = true;
								}
								int nopposit = coinrec.numBclusters (uidx1);
								if (nopposit == 0)
								{
									allhit = false;
									allsingle = false;
								}
								else if (nopposit > 1)
									allsingle = false;
							}
							else
							{
								if (coinrec.B1cluster (uidx1, maxhits))
								{
									x1 = (double)(coinrec.ypos(uidx1));
									x1ok = true;
								}
								int nopposit = coinrec.numAclusters (uidx1);
								if (nopposit == 0)
								{
									allhit = false;
									allsingle = false;
								}
								else if (nopposit > 1)
									allsingle = false;
							}


							// Rear X
							bool x2ok = false;
							double x2;
							if (x2even)
							{
								if (coinrec.A1cluster (uidx2, maxhits))
								{
									x2 = (double)(coinrec.xpos(uidx2));
									x2ok = true;
								}
								int nopposit = coinrec.numBclusters (uidx2);
								if (nopposit == 0)
								{
									allhit = false;
									allsingle = false;
								}
								else if (nopposit > 1)
									allsingle = false;
							}
							else
							{
								if (coinrec.B1cluster (uidx2, maxhits))
								{
									x2 = (double)(coinrec.ypos(uidx2));
									x2ok = true;
								}
								int nopposit = coinrec.numAclusters (uidx2);
								if (nopposit == 0)
								{
									allhit = false;
									allsingle = false;
								}
								else if (nopposit > 1)
									allsingle = false;
							}

							// Front Y
							bool y1ok = false;
							double y1;
							if (y1even)
							{
								if (coinrec.A1cluster (uidy1, maxhits))
								{
									y1 = (double)(coinrec.xpos(uidy1));
									y1ok = true;
								}
								int nopposit = coinrec.numBclusters (uidy1);
								if (nopposit == 0)
								{
									allhit = false;
									allsingle = false;
								}
								else if (nopposit > 1)
									allsingle = false;
							}
							else
							{
								if (coinrec.B1cluster (uidy1, maxhits))
								{
									y1 = (double)(coinrec.ypos(uidy1));
									y1ok = true;
								}
								int nopposit = coinrec.numAclusters (uidy1);
								if (nopposit == 0)
								{
									allhit = false;
									allsingle = false;
								}
								else if (nopposit > 1)
									allsingle = false;
							}

							// Rear Y
							bool y2ok = false;
							double y2;
							if (y2even)
							{
								if (coinrec.A1cluster (uidy2, maxhits))
								{
									y2 = (double)(coinrec.xpos(uidy2));
									y2ok = true;
								}
								int nopposit = coinrec.numBclusters (uidy2);
								if (nopposit == 0)
								{
									allhit = false;
									allsingle = false;
								}
								else if (nopposit > 1)
									allsingle = false;
							}
							else
							{
								if (coinrec.B1cluster (uidy2, maxhits))
								{
									y2 = (double)(coinrec.ypos(uidy2));
									y2ok = true;
								}
								int nopposit = coinrec.numAclusters (uidy2);
								if (nopposit == 0)
								{
									allhit = false;
									allsingle = false;
								}
								else if (nopposit > 1)
									allsingle = false;
							}


							if (x1ok && y1ok)
								hist2d[0].cumulate(x1, y1);
							if (x2ok && y2ok)
								hist2d[1].cumulate(x2, y2);
							if (x1ok && y1ok && x2ok && y2ok)
							{
								hist2d[2].cumulate(x1, y1);
								hist2d[3].cumulate(x2, y2);
								double dx = x1 - x2;
								double dy = y1 - y2;
								// @@@ d5 is not upside-down @@@ y1 = 470.0 - y1;
								hist2d[4].cumulate(dx, dy);
								if (allhit)
									hist2d[5].cumulate (dx, dy);
								if (allsingle)
									hist2d[6].cumulate (dx, dy);
								runnumevt += 1.0;
								if ((fabs (dx) < opt.m_dxmax) && (fabs (dy) < opt.m_dymax))
								{
									hist2d[7].cumulate (x1, y1);
									hist2d[8].cumulate (x2, y2);
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
		<< "<head><title>coinimgc-" << combname << " results</title></head>\n"
		<< "<body>\n"
		<< "<hr>\n<h1>coinimgc-" << combname << " results</h1>\n<hr>\n"
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
			<< ",maxhits," << maxhits
			<< ",dxmax," << opt.m_dxmax
			<< ",dymax," << opt.m_dymax;
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
