// coinhit.cpp
// g++ -Wall coinhit.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp hist1d.cpp mytimer.cpp -lz -o coinhit
//
// 17-Mar-2015 1st version
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "gzfstream.h"
#include "coinrecord.h"
#include "hist1d.h"
#include "mytimer.h"

static const unsigned int NUM_UNITS = 4;

static const int NUM_XBINS = 192;
static const int NUM_YBINS = 192;

static const int IMG_WIDTH = 192;
static const int IMG_HEIGHT = 192;

static const int NUM_HIST1D = 1;
mylibrary::Hist1D hist1d [NUM_HIST1D] =
{
	mylibrary::Hist1D ("Coincidence pattern", -0.5, 15.5, 16),
};

int main (int argc, char* argv[])
{

	long totalfile = 0;
	double duration = 0.0;
	double totalcoin = 0.0;

	// check the arguments
	std::string listfilename;
	bool listgiven = false;
	int iarg = 1;
	while (iarg < argc)
	{
		std::string word (argv[iarg++]);
		if ((word.size() > 0) && (word[0] == '-'))
		{
			std::cerr << "ERROR: " << argv[0]
				  << " unknown option ("
				  << word
				  << ")"
				  << std::endl;
			listgiven = false;
			break;
		}
		else
		{
			listfilename = word;
			listgiven = true;
		}
	}
		
	if (!listgiven)
	{
		// show usage.
		std::cout
			<< "Usage: "
			<< argv[0]
			<< " [options] list_filename"
			<< std::endl;
		return (-1);
	}

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

	mylibrary::MyTimer mytimer;

	// open logfile
	std::ofstream oflog ("coinhit.log");
	if (!oflog)
	{
		std::cerr << "ERROR: "
			<< argv[0] << " -- cannot open logfile"
			<< std::endl;
	}

	// read the filename from the list file
	std::string liststr;
	mytimer.start ();
	while (getline (ifl, liststr))
	{
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
			// open the data file
			mylibrary::igzfstream ifs (datfnam.c_str());
			if (!ifs)
			{
				// file open error
				std::cerr
					<< "ERROR: data file ("
					<< datfnam
					<< ") open error."
					<< std::endl;
				if (oflog)
				{
					oflog
						<< "ERROR: data file ("
						<< datfnam
						<< ") open error."
						<< std::endl;
				}
			}
			else
			{
				totalfile += 1;
				bool newrun = true;
				tsecnow = 0.0;
				tsecfirst = 0.0;
				tseclast = 0.0;
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
							int hittype = 0;
							int hitid = 1;
							for (unsigned int u = 0; u < NUM_UNITS; u++)
							{
								if (coinrec.numdat(u) > 0)
									hittype |= hitid;
								hitid = (hitid << 1);
							}
							if (hittype == 0)
							{
								std::cerr << "ERROR: Wrong COIN record (hittype 0) in"
									<< datfnam
									<< std::cerr;
							}
							hist1d[0].cumulate ((double)(hittype));
						}
					// end of "COIN" record processing
					}
				// end of while(getline(..)) loop
				}
				duration = duration + (tseclast - tsecfirst);
			}
		}
		liststr.clear();
	}
	mytimer.stop ();

	if (oflog)
	{
		oflog << "Total files " << totalfile
			<< " Total events " << totalcoin
			<< " duration(sec) " << duration
			<< std::endl;
	}

	for (int i = 0; i < NUM_HIST1D; i++)
	{
		std::stringstream ss;
		ss << "coinhit-" << i;
		std::string ofnam;
		ss >> ofnam;
		std::string ofncsv;
		ofncsv = ofnam + ".csv";
		std::ofstream ofcsv (ofncsv.c_str());
		mytimer.csvout (ofcsv);
		ofcsv << std::endl;
		ofcsv << "Total files," << totalfile
			<< ",Total events," << totalcoin
			<< ",duration(sec)," << duration
			<< std::endl;
		hist1d [i].CSVdump (ofcsv);
	}

	return 0;
}
