// coinimga.cpp
// g++ -Wall coinrate.cpp coinrateopt.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp mytimer.cpp -lz -o coinrate
//
// 30-Jun-2018 VERSION 100
//
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "gzfstream.h"
#include "coinrateopt.h"
#include "coinrecord.h"
#include "mytimer.h"

static const unsigned int VERSION_NUMBER = 100;

static const unsigned int NUM_UNITS = 4;
static const unsigned int NUM_LAYERS = 4;

int main (int argc, char* argv[])
{
	using namespace mylibrary;

//	Option object
	CoinrateOpt opt;

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
		outname = "coinrate";
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

//
//	Initialization
//
	
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

								// *** Front Unit ***
								// X-plane
								if (coinrec.A1cluster (uidx1, maxhits))
									++nxf;
								if (coinrec.B1cluster (uidx1, maxhits))
									++nxf;
								// Y-plane
								if (coinrec.A1cluster (uidy1, maxhits))
									++nyf;
								if (coinrec.B1cluster (uidy1, maxhits))
									++nyf;

								// *** Rear Unit ***
								// X-plane
								if (coinrec.A1cluster (uidx2, maxhits))
									++nxr;
								if (coinrec.B1cluster (uidx2, maxhits))
									++nxr;
								// Y-plane
								if (coinrec.A1cluster (uidy2, maxhits))
									++nyr;
								if (coinrec.B1cluster (uidy2, maxhits))
									++nyr;

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

	std::string hname = outname + ".html";
	std::ofstream ofh (hname.c_str());
	ofh
		<< "<!DOCTYPE html>\n"
		<< "<html>\n"
		<< "<head><title>coinrate version "
		<< VERSION_NUMBER
		<< " results ("
		<< MyTimer::TimeToStr (mytimer.start_time())
		<< ")</title></head>\n"
		<< "<body>\n"
		<< "<hr>\n<h1>coinrate version "
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
		<< "<b>Log file:</b> <a href=\"" << logfnam << "\">" << logfnam << "</a><br>\n"
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
		<< "</body>\n"
		<< "</html>"
		<< std::endl;

	return 0;
}
