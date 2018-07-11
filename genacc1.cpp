// genacc1.cpp
// g++ -Wall genacc1.cpp hist2d.cpp mytimer.cpp ncpng.cpp -o genacc1
//
// 2016-03-28 copy from muon1f1/genacc2
//
// generate acceptance

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#include "hist2d.h"
#include "mytimer.h"

static const int NUM_X_CHANNELS = 97;	// 5mm pitch
static const int NUM_Y_CHANNELS = 97;	// 5mm pitch

static const double X_UNIT_H = 500.0;	// height of the X unit
static const double X_UNIT_W = 485.0;	// width of the X unit
static const double Y_UNIT_H = 485.0;	// height of the Y unit
static const double Y_UNIT_W = 500.0;	// width of the Y unit

// order is YX-XY from reactor side
static const double UNIT_YDIFF = 139.0;
static const double UNIT_ZDIFF = 495.0;
static const double ZPOS_FRONT_Y = -10.5;
static const double ZPOS_FRONT_X = 10.5;
static const double ZPOS_REAR_X = (UNIT_ZDIFF - 10.5);
static const double ZPOS_REAR_Y = (UNIT_ZDIFF + 10.5);

double overlapsize (double xmin, double xmax, double width)
{
	// initial condition:
	//    (xmax >= xmin) && (width >= (xmax - xmin))
	// xmin, xmax are measured from the center of width
	// i.e., accepted range is [-width/2, width/2]

	double halfwidth = width * 0.5;
	double wmin = -halfwidth;
	double wmax = halfwidth;

	if ((xmax <= wmin) || (xmin >= wmax))
		return 0.0;

	if (xmin < wmin)
		xmin = wmin;
	if (xmax > wmax)
		xmax = wmax;
	return (xmax - xmin);
}

/* @@@@@@@
double eopos (double x)
{
	if (x < -0.5)
		return 0.0;
	else if (x < 0.5)
		return (0.5 + x);
	return (1.5 - x);
}

double eoneg (double x)
{
	if (x > 0.5)
		return 0.0;
	else if (x > -0.5)
		return (0.5 - x);
	return (1.5 + x);
}
@@@@@@ */

double eopos (double x)
{
	return 0.0;
}

double eoneg (double x)
{
	return 0.0;
}

int main (int argc, char* argv[])
{
	using namespace mylibrary;

	bool enable_gap = true;
	bool enable_eo = true;
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
			else if (word == "-eo")
				enable_eo = true;
			else if (word == "-noeo")
				enable_eo = false;
			else
			{
				std::cerr << "ERROR " << argv[0]
					<< " unknown option " << word
					<< std::endl;
				return (-1);
			}
		}
	}

	static const int NUM_HIST2D = 8;
	Hist2D hist2d[NUM_HIST2D] =
	{
		Hist2D ("dxdy uniform", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("dxdy uniform smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("dxdy cos^2", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("dxdy cos^2 smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("phicos uniform", -0.965, 0.965, 193, -0.965, 0.965, 193),
		Hist2D ("phicos uniform smooth", -0.965, 0.965, 193, -0.965, 0.965, 193, true),
		Hist2D ("phicos cos^2", -0.965, 0.965, 193, -0.965, 0.965, 193),
		Hist2D ("phicos cos^2 smooth", -0.965, 0.965, 193, -0.965, 0.965, 193, true),
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

	double dsx1 = (double)(X_UNIT_W) / ((double)(NUM_X_CHANNELS));
	double dsy1 = (double)(Y_UNIT_H) / ((double)(NUM_Y_CHANNELS));
	double dsx2 = dsx1;
	double dsy2 = dsy1;

	// center position
	double xc1 = 0.0;
	double yc1 = UNIT_YDIFF;
	double xc2 = 0.0;
	double yc2 = 0.0;

	// channel 0 position
	double x01 = xc1 - (double)(NUM_X_CHANNELS - 1) * dsx1 * 0.5;
	double y01 = yc1 - (double)(NUM_Y_CHANNELS - 1) * dsy1 * 0.5;
	double x02 = xc2 - (double)(NUM_X_CHANNELS - 1) * dsx2 * 0.5;
	double y02 = yc2 - (double)(NUM_Y_CHANNELS - 1) * dsy2 * 0.5;

	std::cout << "chsize [Front] " << dsx1 << ' ' << dsy1
		<< " [Rear] " << dsx2 << ' ' << dsy2
		<< std::endl;

	double diffx = xc1 - xc2;
	double diffy = yc1 - yc2;

	double sizex1 = X_UNIT_H;
	double sizey1 = Y_UNIT_W;
	double sizex2 = X_UNIT_H;
	double sizey2 = Y_UNIT_W;

	double hfdsx1 = dsx1 * 0.5;
	double hfdsy1 = dsy1 * 0.5;
	double hfdsx2 = dsx2 * 0.5;
	double hfdsy2 = dsy2 * 0.5;

	// our digitizer gives the half of the channel size
	double ds = (hfdsx1 / distx) * (hfdsy1 / disty)
			 * (hfdsx2 / 10.0) * (hfdsy2 / 10.0);	// cm^2 units; channel size is mm units.

	double x1;
	double y1;
	double x2;
	double y2;

	double wt0max = 0.0;
	double wt1max = 0.0;

	mylibrary::MyTimer mytimer;
	mytimer.start ();
	for (int ych2 = 0; ych2 < (NUM_Y_CHANNELS * 2 - 1); ych2++)
	{
		y2 = (double)(ych2) * dsy2 * 0.5 + y02;
		for (int xch2 = 0; xch2 < (NUM_X_CHANNELS * 2 - 1); xch2++)
		{
			x2 = (double)(xch2) * dsx2 * 0.5 + x02;
			for (int ych1 = 0; ych1 < (NUM_Y_CHANNELS * 2 - 1); ych1++)
			{
				y1 = (double)(ych1) * dsy1 * 0.5 + y01;
				for (int xch1 = 0; xch1 < (NUM_X_CHANNELS * 2 - 1); xch1++)
				{
					x1 = (double)(xch1) * dsx1 * 0.5 + x01;
					double rdx = (x1 - x2) / distx;
					double rdy = (y1 - y2) / disty;
					double cs2phi = 1.0 / (1.0 + (rdx * rdx));
					double sn2the = 1.0 / (1.0 + (rdy * rdy * cs2phi));
					double cs2the = 1.0 - sn2the;
					double wt0 = cs2phi * sn2the * cs2phi * sn2the * ds;
					double wt1 = wt0 * cs2the;
					if (wt0 > wt0max)
						wt0max = wt0;
					if (wt1 > wt1max)
						wt1max = wt1;
					if (enable_gap)
					{
						// check the hit posion on the pair plane
						// Y plane
						double xony1 = rdx * (zy1 - zx2) + x2 - xc1;
						double xony2 = rdx * (zy2 - zx2) + x2 - xc2;
						// X plane
						double yonx1 = rdy * (zx1 - zy2) + y2 - yc1;
						double yonx2 = rdy * (zx2 - zy2) + y2 - yc2;
						double effdx1 = overlapsize((xony1 - hfdsx1), (xony1 + hfdsx1), sizex1);
						double effdy1 = overlapsize((yonx1 - hfdsy1), (yonx1 + hfdsy1), sizey1);
						double effdx2 = overlapsize((xony2 - hfdsx2), (xony2 + hfdsx2), sizex2);
						double effdy2 = overlapsize((yonx2 - hfdsy2), (yonx2 + hfdsy2), sizey2);
						double effarea = effdx1 / dsx1 * effdy1 / dsy1 * effdx2 / dsx2 * effdy2 / dsy2;
						wt0 = wt0 * effarea;
						wt1 = wt1 * effarea;
					}
					if (enable_eo)
					{
						double wx1 = (xch1 & 1) ? eopos(rdx) : eoneg(rdx);
						double wx2 = (xch2 & 1) ? eoneg(rdx) : eopos(rdx);
						double wy1 = (ych1 & 1) ? eoneg(rdy) : eopos(rdy);
						double wy2 = (ych2 & 1) ? eoneg(rdy) : eopos(rdy);
						wt0 = wt0 * wx1 * wx2 * wy1 * wy2 * 16.0;
						wt1 = wt1 * wx1 * wx2 * wy1 * wy2 * 16.0;
					}
					double dx = ((x1 - x2) * scalex - diffx);
					double dy = ((y1 - y2) * scaley - diffy);
					hist2d[0].cumulate (dx, dy, wt0);
					hist2d[1].cumulate (dx, dy, wt0);
					hist2d[2].cumulate (dx, dy, wt1);
					hist2d[3].cumulate (dx, dy, wt1);
					double phi = atan (-rdx);
					double cst = (-rdy) / sqrt(rdx * rdx + rdy * rdy + 1.0);
					hist2d[4].cumulate (phi, cst, wt0);
					hist2d[5].cumulate (phi, cst, wt0);
					hist2d[6].cumulate (phi, cst, wt1);
					hist2d[7].cumulate (phi, cst, wt1);
				}
			}
		}
	}
	mytimer.stop ();
	// Show results.
	std::string ofname = std::string("genacc1.csv");
	std::ofstream ofcsv (ofname.c_str());
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
		if (enable_gap)
			ofcsv << ",eneble_gap";
		else
			ofcsv << ",diable_gap";
		if (enable_eo)
			ofcsv << ",enable_eo";
		else
			ofcsv << ",disable_eo";
		ofcsv << std::endl;

		ofcsv << "xdist," << distx << ",ydist," << disty
			<< ",scalex," << scalex << ",scaley," << scaley
			<< std::endl;

		for (int n = 0; n < NUM_HIST2D; n++)
			hist2d[n].CSVdump (ofcsv);
		ofcsv.close ();
	}
	return 0;
}
