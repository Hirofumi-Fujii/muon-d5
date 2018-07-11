// genacc0.cpp
// g++ -Wall genacc0.cpp hist2d.cpp mytimer.cpp ncpng.cpp -o genacc0
//
// 2016-07-06 simple (no gap) version
// 2016-03-28 copy from muon1f1/genacc2
//
// generate acceptance

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#include "hist2d.h"
#include "mytimer.h"

static const int VERSION_MAJOR_NUMBER = 1;
static const int VERSION_MINOR_NUMBER = 0;

static const int NUM_X_CHANNELS = 97;	// 5mm pitch
static const int NUM_Y_CHANNELS = 97;	// 5mm pitch

static const double X_UNIT_H = 500.0;	// height of the X unit
static const double X_UNIT_W = 485.0;	// width of the X unit
static const double Y_UNIT_H = 485.0;	// height of the Y unit
static const double Y_UNIT_W = 500.0;	// width of the Y unit

// order is YX-XY from reactor side
static const double UNIT_XDIFF = 0.0;
static const double UNIT_YDIFF = 139.0;
static const double UNIT_ZDIFF = 495.0;

static const double ZPOS_FRONT_Y = -10.5;
static const double ZPOS_FRONT_X = 10.5;
static const double ZPOS_REAR_X = (UNIT_ZDIFF - 10.5);
static const double ZPOS_REAR_Y = (UNIT_ZDIFF + 10.5);

static const double XPOS_FRONT_Y = UNIT_XDIFF;
static const double XPOS_FRONT_X = UNIT_XDIFF;
static const double XPOS_REAR_X = 0.0;
static const double XPOS_REAR_Y = 0.0;

static const double YPOS_FRONT_Y = UNIT_YDIFF;
static const double YPOS_FRONT_X = UNIT_YDIFF;
static const double YPOS_REAR_X = 0.0;
static const double YPOS_REAR_Y = 0.0;

//
static const int NUM_DETECTOR_PLANES = 8;	// YA-YB-XA-XB XB-XA-YB-YA
static const double detzpos[NUM_DETECTOR_PLANES] =
{
	(ZPOS_FRONT_Y - 5.0),
	(ZPOS_FRONT_Y + 5.0),
	(ZPOS_FRONT_X - 5.0),
	(ZPOS_FRONT_X + 5.0),
	(ZPOS_REAR_X - 5.0),
	(ZPOS_REAR_X + 5.0),
	(ZPOS_REAR_Y - 5.0),
	(ZPOS_REAR_Y + 5.0),
};

static const double detxpos[NUM_DETECTOR_PLANES] =
{
	XPOS_FRONT_Y,
	XPOS_FRONT_Y,
	(XPOS_FRONT_X - 2.5),
	(XPOS_FRONT_X + 2.5),
	(XPOS_REAR_X + 2.5),
	(XPOS_REAR_X - 2.5),
	XPOS_REAR_Y,
	XPOS_REAR_Y,
	
};

static const double detypos[NUM_DETECTOR_PLANES] =
{
	(YPOS_FRONT_Y + 2.5),
	(YPOS_FRONT_Y - 2.5),
	YPOS_FRONT_X,
	YPOS_FRONT_X,
	YPOS_REAR_X,
	YPOS_REAR_X,
	(YPOS_REAR_Y + 2.5),
	(YPOS_REAR_Y - 2.5),	
};

static const double detxsize[NUM_DETECTOR_PLANES] =
{
	Y_UNIT_W,
	Y_UNIT_W,
	(X_UNIT_W - 5.0),
	(X_UNIT_W - 5.0),
	(X_UNIT_W - 5.0),
	(X_UNIT_W - 5.0),
	Y_UNIT_W,
	Y_UNIT_W,
};

static const double detysize[NUM_DETECTOR_PLANES] =
{
	(Y_UNIT_H - 5.0),
	(Y_UNIT_H - 5.0),
	X_UNIT_H,
	X_UNIT_H,
	X_UNIT_H,
	X_UNIT_H,
	(Y_UNIT_H - 5.0),
	(Y_UNIT_H - 5.0),
};

double overlapsize (int numdet, const double xpos[], const double ypos[], const double ysize[], double slope)
{
	// xpos and ypos is the position of the center of the detector
	// i.e., i-th detector covers the range (ypos[i] - ysize[i] * 0.5, ypos[i] + ysize[i] * 0.5).

	double ymin;
	double ymax;
	for (int n = 0; n < numdet; n++)
	{
		// range at xpos = 0.0
		double dy = slope * xpos[n];
		double halfsize = ysize[n] * 0.5;
		double yl = ypos[n] - halfsize - dy;
		double yh = yl + ysize[n];
		if (n == 0)
		{
			ymin = yl;
			ymax = yh;
		}
		else
		{
			if ((yh <= ymin) || (yl >= ymax))
				return 0.0;
			if (yh < ymax)
				ymax = yh;
			else if (yl > ymin)
				ymin = yl;
		}
	}
	return (ymax - ymin);
}

double
efflen (double dd, double width)
{
	// project on the front
	double xmax1 = width;
	double xmin1 = 0.0;
	double xmax2 = width + dd;
	double xmin2 = dd;
	if ((xmax2 <= xmin1) || (xmin2 >= xmax1))
		return 0.0;	// no overlap
	double x1 = xmax1;
	if (x1 > xmax2)
		x1 = xmax2;
	double x2 = xmin1;
	if (x2 < xmin2)
		x2 = xmin2;	
	return (x1 - x2);
}

int main (int argc, char* argv[])
{
	using namespace mylibrary;

	bool enable_gap = true;
	int iarg = 1;
	while (iarg < argc)
	{
		std::string word(argv[iarg++]);
		if ((word.size() > 0) && (word[0] == '-'))
		{
			if (word == "-gap")
				enable_gap = true;
			else if (word == "-nogap")
				enable_gap = false;
			else
			{
				std::cerr << "ERROR " << argv[0]
					<< " unknown option " << word
					<< std::endl;
				return (-1);
			}
		}
	}

	static const int NUM_HIST2D = 4;
	Hist2D hist2d[NUM_HIST2D] =
	{
		Hist2D ("dxdy uniform", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("dxdy uniform smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("dxdy cos^2", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("dxdy cos^2 smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
	};

	double zx1 = ZPOS_FRONT_X;
	double zy1 = ZPOS_FRONT_Y;
	double zx2 = ZPOS_REAR_X;
	double zy2 = ZPOS_REAR_Y;

	double distx = zx1 - zx2;
	double disty = zy1 - zy2;

	double dist = (distx + disty) * 0.5;
	double scalex = dist / distx;
	double scaley = dist / disty;

	double dsx = (double)(X_UNIT_W) / ((double)(NUM_X_CHANNELS));
	double dsy = (double)(Y_UNIT_H) / ((double)(NUM_Y_CHANNELS));

	double xunitdiff = UNIT_XDIFF;
	double yunitdiff = UNIT_YDIFF;

	int num_x_bins = NUM_X_CHANNELS * 2 - 1;
	int num_y_bins = NUM_Y_CHANNELS * 2 - 1;

	// channel 0 position on the projection plane	
	double x0 = (dsx * 0.5) - (double)(num_x_bins) * dsx * 0.5;
	double y0 = (dsy * 0.5) - (double)(num_y_bins) * dsy * 0.5;

	std::cout << "ch size " << dsx << ' ' << dsy
		<< " ch0 pos " << x0 << ' ' << y0
		<< std::endl;

	// cell acceptance
	double ds = (dsx / distx) * (dsy / disty)
			 * (dsx / 10.0) * (dsy / 10.0);	// cm^2 units; channel size is mm units.

	mylibrary::MyTimer mytimer;
	mytimer.start ();
	for (int ych2 = 0; ych2 < num_y_bins; ych2++)
	{
		double ddy = (double)(ych2) * dsy + y0;
		for (int xch2 = 0; xch2 < num_x_bins; xch2++)
		{
			double ddx = (double)(xch2) * dsx + x0;
			double rdx = (ddx + xunitdiff) / distx;
			double rdy = (ddy + yunitdiff) / disty;
			double cs2phi = 1.0 / (1.0 + (rdx * rdx));
			double sn2the = 1.0 / (1.0 + (rdy * rdy * cs2phi));
			double cs2the = 1.0 - sn2the;
			// get number of combinations
			double effxsize;
			double effysize;
			if (enable_gap)
			{
				effxsize = overlapsize(NUM_DETECTOR_PLANES, detzpos, detxpos, detxsize, rdx);
				effysize = overlapsize(NUM_DETECTOR_PLANES, detzpos, detypos, detysize, rdy);
			}
			else
			{
				effxsize = efflen (ddx, double(X_UNIT_W));
				effysize = efflen (ddy, double(Y_UNIT_H));
			}
			double wt0 = cs2phi * sn2the * cs2phi * sn2the * ds * (effxsize /dsx) * (effysize / dsy);
			double wt1 = wt0 * cs2the;
			hist2d[0].cumulate (ddx, ddy, wt0);
			hist2d[1].cumulate (ddx, ddy, wt0);
			hist2d[2].cumulate (ddx, ddy, wt1);
			hist2d[3].cumulate (ddx, ddy, wt1);
		}
	}
	mytimer.stop ();

	// Show results.
	std::string outname = "genacc0";
	for (int n = 0; n < NUM_HIST2D; n++)
	{
		std::stringstream ss;
		ss << outname << '-' << n;
		std::string outfnam;
		ss >> outfnam;

		std::string outcsvnam = outfnam + ".csv";
		std::ofstream ofcsv (outcsvnam.c_str());

		if (ofcsv)
		{
			ofcsv << '\"';
			for (int i = 0; i < argc; i++)
			{
				if (i)
					ofcsv << ' ';
				ofcsv << argv[i];
			}
			ofcsv << "\",";
			mytimer.csvout (ofcsv);
			ofcsv
				<< ",\"Version(major,minor)\""
				<< ',' << VERSION_MAJOR_NUMBER
				<< ',' << VERSION_MINOR_NUMBER
				<< std::endl;

			ofcsv
				<< "\"channel-size(x,y)(mm)\"," << dsx << ',' << dsy
				<< ",\"dist(av,x,y)(mm)\"," << dist << ',' << distx << ',' << disty
				<< ",scalex," << scalex << ",scaley," << scaley
				<< ",enable_gap";
			if (enable_gap)
				ofcsv << ",true";
			else
				ofcsv << ",false";
			ofcsv
				<< std::endl;

			hist2d[n].CSVdump (ofcsv);
			ofcsv.close ();
		}
	}
	return 0;
}
