// coinimg_d4.cpp
// g++ -Wall coinimg_d4.cpp coinrecord.cpp gzfstream.cpp gzipfilebuf.cpp  ncpng.cpp mytimer.cpp -lz -o coinimg_d4
//
// 31-Mar-2015 modified for detector3
// 5-Mar-2015 modified for detector4
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "gzfstream.h"
#include "coinrecord.h"
#include "ncpng.h"
#include "mytimer.h"
#include "Zav.h"

static const unsigned int NUM_UNITS = 4;

static const int NUM_XBINS = 192;
static const int NUM_YBINS = 192;

static const int IMG_WIDTH = 192;
static const int IMG_HEIGHT = 192;

double cntmax;
double cntbuf[NUM_YBINS][NUM_XBINS];

static const char header[4] =
{
	'I', 'M', 'G', 'S',
};

int main (int argc, char* argv[])
{
	bool newimage = false;
	bool saveimage = true;


	cntmax = 0.0;
	for (int j = 0; j < NUM_YBINS; j++)
		for (int i = 0; i < NUM_XBINS; i++)
			cntbuf[j][i] = 0.0;

	long totalfile = 0;
	long detectsys = 0;	// detector system, 0 is unknown
	double duration = 0.0;
	double totalcoin = 0.0;

	// check the arguments
	std::string listfilename;
	bool listgiven = false;
	int iarg = 1;
	while (iarg < argc)
	{
		std::string word (argv[iarg++]);
		if (word == "-new")
			newimage = true;
		else if (word == "-add")
			newimage = false;
		else if (word == "-save")
			saveimage = true;
		else if (word == "-nosave")
			saveimage = false;
		else if ((word.size() > 0) && (word[0] == '-'))
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
		std::cout
			<< "options:\n"
			<< "  -new     create new images (does not read the saved images)\n"
			<< "  -nosave  does not save the created images (only creates \'coinimgs.png\')"
			<< std::endl;
		return (-1);
	}

	if (!newimage)
	{
		newimage = true;	// in the case of read error.
		// open the saved file
		std::ifstream ifs ("coinimgs.sav", std::ios::binary);
		if (ifs)
		{
			char ch[4];
			ifs.read (ch, 4);
			int np[3];
			ifs.read ((char*)(np), sizeof(int) * 3);
			ifs.read ((char*)(&cntmax), sizeof(double));
			ifs.read ((char*)(cntbuf), sizeof(double) * NUM_YBINS * NUM_XBINS);
			int isize[2];
			ifs.read ((char*)(isize), sizeof(int) * 2);
			long linfo[4];
			ifs.read ((char*)(linfo), sizeof(long) * 4);
			double dinfo[4];
			ifs.read ((char*)(dinfo), sizeof(double) * 4);
			if (ifs)
			{
				totalfile = linfo[0];
				detectsys = linfo[1];
				duration = dinfo[0];
				totalcoin = dinfo[1];
				newimage = false;	// all data are successfully restored.
			}
			else
			{
				std::cerr << "ERROR: " << argv[0]
					<< " data read error in (coinimgs.sav). New images will be created."
					<< std::endl;
			}
		}
		else
		{
			std::cerr << "ERROR: " << argv[0]
				<< " file open error (coinimgs.sav). New images will be created."
				<< std::endl;
		}
	}

	if (newimage)
	{
		cntmax = 0.0;
		for (int j = 0; j < NUM_YBINS; j++)
			for (int i = 0; i < NUM_XBINS; i++)
				cntbuf[j][i] = 0.0;
		totalfile = 0;
		duration = 0.0;
		totalcoin = 0.0;
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
								int xcls[] = {coinrec.epos(uidx1, 1),  // even ch
									      coinrec.opos(uidx1, 1),  // odd ch
									      coinrec.opos(uidx2, 1),  // odd ch
									      coinrec.epos(uidx2, 1)}; // even ch
								int ycls[] = {coinrec.epos(uidy1, -1) + 139, // even ch
									      coinrec.opos(uidy1, -1) + 139, // odd ch
									      coinrec.opos(uidy2, 1), // odd ch
									      coinrec.epos(uidy2, 1)}; // even ch
#if 1
								// line fit to obtain the gradient of the muon track
								int z_for_x[4] = {-490, -480, -15, -5};
								int z_for_y[4] = {-510, -500, 5, 15};
								Zav zx, zy;
								const double err = 2.9; // 10./sqrt(12)
								for (int i = 0; i < 4; i++){
									zx.add_point(z_for_x[i], xcls[i], err);
									zy.add_point(z_for_y[i], ycls[i], err);
								}
								zx.fit();
								zy.fit();
								// gradient of the track
								const double dxdz = zx.a();
								const double dydz = zy.a();
								int nx = (95.5-dxdz/(5./475.));
								int ny = (123.5+dydz/(5./515.));
								//int ny = (95.5+(dydz+139./515.)/(5./515.));
#else								
								const double dx = ((xcls[0]+xcls[1])-(xcls[2]+xcls[3]));
								const double dy = ((ycls[0]+ycls[1])-(ycls[2]+ycls[3]))-139*2.;
								int nx = dx / 10.0 + 95.5;
								int ny = -dy / 10.0 + 95.5;
#endif
								if (nx>=0 && ny>=0)
								{
									if ((nx < NUM_XBINS) && (ny < NUM_YBINS))
									{
										cntbuf[ny][nx] += 1.0;
										if (cntbuf[ny][nx] > cntmax)
											cntmax = cntbuf[ny][nx];
									}
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

	if (saveimage)
	{
		std::ofstream ofs ("coinimgs.sav", std::ios::binary);
		if (ofs)
		{
			ofs.write (header, 4);
			int np[3];
			np[0] = 1;
			np[1] = NUM_YBINS;
			np[2] = NUM_XBINS;
			ofs.write ((const char*)(np), sizeof(int) * 3);
			ofs.write ((const char*)(&cntmax), sizeof(double));
			ofs.write ((const char*)(cntbuf), sizeof(double) * NUM_YBINS * NUM_XBINS);
			// information size
			int isize[2];
			isize[0] = 4;	// long information
			isize[1] = 4;	// double information
			ofs.write ((const char*)(isize), sizeof(int) * 2);
			long linfo[4];
			linfo[0] = totalfile;
			linfo[1] = detectsys;
			linfo[2] = 0;	// dummy
			linfo[3] = 0;	// dummy
			ofs.write ((const char*)(linfo), sizeof(long) * 4);
			double dinfo[4];
			dinfo[0] = duration;
			dinfo[1] = totalcoin;
			dinfo[2] = 0.0;	// dummy
			dinfo[3] = 0.0;	// dummy
			ofs.write ((const char*)(dinfo), sizeof(double) * 4);
		}
	}

	mytimer.stop ();
#if 0
	double proj_x[NUM_XBINS] = { 0 };
	double proj_y[NUM_YBINS] = { 0 };
	double proj_x_norm[NUM_XBINS/2] = { 0 };
	double proj_y_norm[NUM_YBINS/2] = { 0 };
	for (int y = 0; y < NUM_YBINS; y++){
		for (int x = 0; x < NUM_XBINS; x++){
			proj_y[y] += cntbuf[y][x];
			proj_x[x] += cntbuf[y][x];
			proj_y_norm[y/2] += cntbuf[y][x];
			proj_x_norm[x/2] += cntbuf[y][x];
		}
	}
	std::ofstream foutx("norm_x.dat");
	for (int x = 0; x < NUM_XBINS; x++){
		if (proj_x_norm[x/2] > 0) proj_x[x] /= proj_x_norm[x/2];
		foutx << 2.*proj_x[x] << std::endl;
	}
	std::ofstream fouty("norm_y.dat");
	for (int y = 0; y < NUM_YBINS; y++){
		if (proj_y_norm[y/2] > 0) proj_y[y] /= proj_y_norm[y/2];
		fouty << 2.*proj_y[y] << std::endl;
	}
#endif
	double norm_x[NUM_XBINS];
	double norm_y[NUM_YBINS];
	std::ifstream finx("norm_x.dat");	
	std::ifstream finy("norm_y.dat");
	if (finx && finy){
		cntmax = 0.;
		int index = 0;
		while (!finx.eof() && index < NUM_XBINS) finx >> norm_x[index++];
		index = 0;
		while (!finy.eof() && index < NUM_YBINS) finy >> norm_y[index++];
		for (int y = 0; y < NUM_YBINS; y++){
			for (int x = 0; x < NUM_XBINS; x++){
				if (norm_x[x] > 0 && norm_y[y] > 0) cntbuf[y][x] /= (norm_x[x]*norm_y[y]);
				if (norm_x[x]*norm_y[y] > 1 && cntbuf[y][x] > cntmax) cntmax = cntbuf[y][x];
			}
		}
	}

	double offsimg = 0.0;
	double normimg = 1.0;
	if (cntmax > normimg)
	  normimg = cntmax;
	// make image
	mypnglib::NCPNG pngimg (IMG_WIDTH, IMG_HEIGHT, mypnglib::NCPNG::GREYSCALE, 16);
	for (int y = 0; y < NUM_YBINS; y++)
	  {
	    for (int x = 0; x < NUM_XBINS; x++)
	      {
		double v = (cntbuf[y][x] - offsimg) / normimg;
		if (v > 1.0)
		  v = 1.0;
		else if (v < 0.0)
		  v = 0.0;
		mypnglib::NCPNG::GREY_PIXEL pixel(v);
		pngimg.put (x, y, pixel);
	      }
	  }
	std::ofstream ofs ("coinimgs.png", std::ios::binary);
	pngimg.write (ofs);

	return 0;
}
