// coinimga.cpp
// g++ -Wall coinimga.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp hist2d.cpp ncpng.cpp mytimer.cpp -lz -o coinimga
//
// 12-Mar-2015 coinimga -- test for coinrec
// 31-Mar-2015 modified for detector3
// 5-Mar-2015 modified for detector4
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "gzfstream.h"
#include "coinrecord.h"
#include "hist2d.h"
#include "ncpng.h"
#include "mytimer.h"

static const unsigned int NUM_UNITS = 4;

static const int NUM_XBINS = 192;
static const int NUM_YBINS = 192;

static const int IMG_WIDTH = 192;
static const int IMG_HEIGHT = 192;

static const int NUM_HIST2D = 15;
mylibrary::Hist2D hist2d [NUM_HIST2D] =
{
	mylibrary::Hist2D ("front unit even-odd X", -0.5, 47.5, 48, -0.5, 47.5, 48),
	mylibrary::Hist2D ("front unit even-odd Y", -0.5, 47.5, 48, -0.5, 47.5, 48),
	mylibrary::Hist2D ("front unit even-odd diff.", -47.5, 47.5, 95, -47.5, 47.5, 95),
	mylibrary::Hist2D ("front unit X-Y", -0.5, 94.5, 95, -0.5, 94.5, 95),
	mylibrary::Hist2D ("rear unit even-odd X", -0.5, 47.5, 48, -0.5, 47.5, 48),
	mylibrary::Hist2D ("rear unit even-odd Y", -0.5, 47.5, 48, -0.5, 47.5, 48),
	mylibrary::Hist2D ("rear unit even-odd diff.", -47.5, 47.5, 95, -47.5, 47.5, 95),
	mylibrary::Hist2D ("rear unit X-Y", -0.5, 94.5, 95, -0.5, 94.5, 95),
	mylibrary::Hist2D ("dx-dy", -94.5, 94.5, 189, -94.5, 94.5, 189),
	mylibrary::Hist2D ("dx-dy smooth", -94.5, 94.5, 189, -94.5, 94.5, 189, true),
	mylibrary::Hist2D ("dx-dy all single hit", -94.5, 94.5, 189, -94.5, 94.5, 189),
	mylibrary::Hist2D ("dx-front x-even hits", -94.5, 94.5, 189, 0.5, 10.5, 10),
	mylibrary::Hist2D ("dx-front x-odd hits", -94.5, 94.5, 189, 0.5, 10.5, 10),
	mylibrary::Hist2D ("dy-front y-even hits", -94.5, 94.5, 189, 0.5, 10.5, 10),
	mylibrary::Hist2D ("dy-front y-odd hits", -94.5, 94.5, 189, 0.5, 10.5, 10),
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
							// 4-fold coincidence
							int uidy2 = 0;  //UNIT#7
							int uidx2 = 1;  //UNIT#8
							int uidy1 = 2;  //UNIT#9
							int uidx1 = 3;  //UNIT#10

							if (coinrec.xy1cluster(uidx1) &&
							    coinrec.xy1cluster(uidy1) &&
							    coinrec.xy1cluster(uidx2) &&
							    coinrec.xy1cluster(uidy2))
							{
								// all layers are single-cluster
								// front unit
								double xe1 = coinrec.xpos(uidx1) * 0.1;
								double xo1 = coinrec.ypos(uidx1) * 0.1;
								double ye1 = 47.0 - (coinrec.xpos(uidy1) * 0.1);
								double yo1 = 47.0 - (coinrec.ypos(uidy1) * 0.1);
								hist2d[0].cumulate(xe1, xo1);
								hist2d[1].cumulate(ye1, yo1);
								hist2d[2].cumulate((xe1 - xo1), (ye1 - yo1));
								hist2d[3].cumulate((xe1 + xo1), (ye1 + yo1));
								// rear unit
								double xe2 = coinrec.xpos(uidx2) * 0.1;
								double xo2 = coinrec.ypos(uidx2) * 0.1;
								double ye2 = coinrec.xpos(uidy2) * 0.1;
								double yo2 = coinrec.ypos(uidy2) * 0.1;
								hist2d[4].cumulate(xe2, xo2);
								hist2d[5].cumulate(ye2, yo2);
								hist2d[6].cumulate((xe2 - xo2), (ye2 - yo2));
								hist2d[7].cumulate((xe2 + xo2), (ye2 + yo2));
								//
								// dx-dy
								double dx = (xe1 + xo1) - (xe2 + xo2);
								double dy = (ye1 + yo1) - (ye2 + yo2);
								hist2d[8].cumulate(dx, dy);
								hist2d[9].cumulate(dx, dy);
								hist2d[11].cumulate(dx, (double)(coinrec.numxhits(uidx1)));
								hist2d[12].cumulate(dx, (double)(coinrec.numyhits(uidx1)));
								hist2d[13].cumulate(dy, (double)(coinrec.numxhits(uidy1)));
								hist2d[14].cumulate(dy, (double)(coinrec.numyhits(uidy1)));
								if (
									coinrec.xy1cluster(uidx1, 1) &&
									coinrec.xy1cluster(uidy1, 1) &&
									coinrec.xy1cluster(uidx2, 1) &&
									coinrec.xy1cluster(uidy2, 1))
								{
									// all single hit
									hist2d[10].cumulate(dx, dy);
								}
							}
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

	for (int i = 0; i < NUM_HIST2D; i++)
	{
		std::stringstream ss;
		ss << "coinimga-" << i;
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
		hist2d [i].CSVdump (ofcsv);
		std::string ofnpng;
		ofnpng = ofnam + ".png";
		std::ofstream ofpng (ofnpng.c_str(), std::ios::binary);
		hist2d [i].PNGdump (ofpng);
	}

	return 0;
}
