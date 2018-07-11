// acc1f2.cpp
// g++ -Wall acc1f2.cpp fit2dline.cpp hist2d.cpp mytimer.cpp ncpng.cpp -o acc1f2
//
// 2016-03-28 copy from muon1f1/genacc2
//
// generate acceptance

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#include "fit2dline.h"
#include "hist2d.h"
#include "mytimer.h"

static const int NUM_X_PIXELS = 101;
static const int NUM_Y_PIXELS = 101;

static const double PIXEL_XSIZE = 5.0;
static const double PIXEL_YSIZE = 5.0;

static const double UNIT_ZDIFF = 495.0;	// Unit distance in mm units
static const double UNIT_YDIFF = 139.0;	// Unit height difference in mm units

static const int NUM_X_CHANNELS = 48;	// 10mm pitch
static const int NUM_Y_CHANNELS = 48;	// 10mm pitch

static const double X_UNIT_H = 500.0;	// height of the X unit
static const double X_UNIT_W = 480.0;	// width of the X unit
static const double Y_UNIT_H = 480.0;	// height of the Y unit
static const double Y_UNIT_W = 500.0;	// width of the Y unit

// order is YX-XY from reactor side
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

double eopos (double x, double ramp)
{
	double r = 0.0;
	if (x < -0.5)
	{
		if (x < -1.5)
			r = 0.0;
		else
			r = -x - 0.5;
	}
	else if (x < 0.5)
		r = 0.5 + x;
	else if (x < 1.5)
		r = 1.5 - x;
	else
	{
std::cerr << "wrong eopos " << x << std::endl;
	 	r = 0.0;
	}
	return (r - 0.5) * ramp + 0.5;
}

double eoneg (double x, double ramp)
{
	double r = 0.0;
	if (x > 0.5)
	{
		if (x > 1.5)
			r = 0.0;
		else
			r = x - 0.5;
	}
	else if (x > -0.5)
		r = 0.5 - x;
	else if (x > -1.5)
		r = 1.5 + x;
	else
	{
std::cerr << "wrong eoneg " << x << std::endl;
		r = 0.0;
	}
	return (r - 0.5) * ramp + 0.5;
}

double digitize (double x, double xlow, double dx)
{
	double r = (x - xlow) / dx;
	int ir = (int)(r);
	return (((double)(ir) + 0.5) * dx + xlow);
}

int main (int argc, char* argv[])
{
	using namespace mylibrary;

	bool enable_gap = true;
	bool enable_eo = true;
	bool nodigitize = false;

	double eoamp = 1.0;

	double dsx1 = (double)(X_UNIT_W) / ((double)(NUM_X_CHANNELS));
	double dsy1 = (double)(Y_UNIT_H) / ((double)(NUM_Y_CHANNELS));
	double dsx2 = dsx1;
	double dsy2 = dsy1;

	double digsx1 = dsx1;
	double digsy1 = dsy1;
	double digsx2 = dsx2;
	double digsy2 = dsy2;

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
			else if (word == "-nodigitize")
			{
				digsx1 = 0.0;
				digsy1 = 0.0;
				digsx2 = 0.0;
				digsy2 = 0.0;
				nodigitize = true;
			}
			else if ((word == "-digitize") || (word == "-eoamp"))
			{
				if (iarg >= argc)
				{
					std::cerr << "ERROR " << argv[0]
						<< " requires numerial value."
						<< std::endl;
					return (-1);
				}
				std::stringstream ss;
				ss << argv[iarg++];
				double rval = 0.0;
				if (!(ss >> rval))
				{
					std::cerr << "ERROR " << argv[0]
						<< " requires numerical value."
						<< std::endl;
					return (-1);
				}
				if (word == "-digitize")
				{
					if (rval > 0.0)
					{
						digsx1 = rval;
						digsy1 = rval;
						digsx2 = rval;
						digsy2 = rval;
						nodigitize = false;
					}
					else
					{
						digsx1 = 0.0;
						digsy1 = 0.0;
						digsx2 = 0.0;
						digsy2 = 0.0;
						nodigitize = true;
					}
				}
				else if (word == "-eoamp")
				{
					if ((rval >= 0.0) && (rval <= 1.0))
					{
						eoamp = rval;
					}
					else
					{
						std::cerr << "ERROR " << argv[0]
							<< "-eoamp must be in the range [0.0,1.0] ("
							<< rval
							<< ")"
							<< std::endl;
						return (-1);
					}
				}
			}
			else
			{
				std::cerr << "ERROR " << argv[0]
					<< " unknown option " << word
					<< std::endl;
				return (-1);
			}
		}
	}

	static const int NUM_HIST2D = 16;
	Hist2D hist2d[NUM_HIST2D] =
	{
		Hist2D ("av dxdy uniform", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("av dxdy uniform smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("av dxdy cos^2", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("av dxdy cos^2 smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("fit phicos uniform", -0.965, 0.965, 193, -0.965, 0.965, 193),
		Hist2D ("fit phicos uniform smooth", -0.965, 0.965, 193, -0.965, 0.965, 193, true),
		Hist2D ("fit phicos cos^2", -0.965, 0.965, 193, -0.965, 0.965, 193),
		Hist2D ("fit phicos cos^2 smooth", -0.965, 0.965, 193, -0.965, 0.965, 193, true),
		Hist2D ("sp dxdy uniform", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("sp dxdy uniform smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("sp dxdy cos^2", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("sp dxdy cos^2 smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("fit dxdy uniform", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("fit dxdy uniform smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
		Hist2D ("fit dxdy cos^2", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("fit dxdy cos^2 smooth", -482.5, 482.5, 193, -482.5, 482.5, 193, true),
	};

	double zx1 = ZPOS_FRONT_X;
	double zy1 = ZPOS_FRONT_Y;
	double zx2 = ZPOS_REAR_X;
	double zy2 = ZPOS_REAR_Y;

	double distx = zx2 - zx1;
	double disty = zy2 - zy1;

//	double dist = (distx + disty) * 0.5;
	double scalex = UNIT_ZDIFF / distx;
	double scaley = UNIT_ZDIFF / disty;

	// center position
	double xc1 = 0.0;
	double yc1 = UNIT_YDIFF;
	double xc2 = 0.0;
	double yc2 = 0.0;

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

	double ypxc = (double)(NUM_X_PIXELS) * 0.5;
	double xpxc = (double)(NUM_Y_PIXELS) * 0.5;

	// IMPORTANT:
	// We are going to calculate from 0 to NUM_*_PIXELS in each direction.
	// It is assumed that
	// (1) 0 is the lowest position in the direction (for all even-odd, and X-Y combinations)
	// (2) The even-plane is lower then the odd-plane

	double x1ez = ZPOS_FRONT_X - 5.0;
	double x1oz = ZPOS_FRONT_X + 5.0;
	double y1ez = ZPOS_FRONT_Y + 5.0;
	double y1oz = ZPOS_FRONT_Y - 5.0;
	bool x1front_even = false;
	bool y1front_even = false;
	if (x1ez < x1oz)
		x1front_even = true;
	if (y1ez < y1oz)
		y1front_even = true;

	double x2ez = ZPOS_REAR_X + 5.0;
	double x2oz = ZPOS_REAR_X - 5.0;
	double y2ez = ZPOS_REAR_Y + 5.0;
	double y2oz = ZPOS_REAR_Y - 5.0;
	bool x2front_even = false;
	bool y2front_even = false;
	if (x2ez < x2oz)
		x2front_even = true;
	if (y2ez < y2oz)
		y2front_even = true;

	int x1eoflag = (x1front_even) ? 1 : 0;
	int y1eoflag = (y1front_even) ? 1 : 0;
	int x2eoflag = (x2front_even) ? 1 : 0;
	int y2eoflag = (y2front_even) ? 1 : 0;

	// low edges
	double x1e0 = xc1 - ((double)(NUM_X_CHANNELS) * dsx1 * 0.5) - 2.5;
	double x1o0 = x1e0 + 5.0;
	double y1e0 = yc1 - ((double)(NUM_Y_CHANNELS) * dsy1 * 0.5) - 2.5;
	double y1o0 = y1e0 + 5.0;

	double x2e0 = xc2 - ((double)(NUM_X_CHANNELS) * dsx2 * 0.5) - 2.5;
	double x2o0 = x2e0 + 5.0;
	double y2e0 = yc2 - ((double)(NUM_Y_CHANNELS) * dsy2 * 0.5) - 2.5;
	double y2o0 = y2e0 + 5.0;

	mylibrary::MyTimer mytimer;
	mytimer.start ();
	for (int ypx2 = 0; ypx2 < NUM_Y_PIXELS; ypx2++)
	{
		y2 = ((double)(ypx2) - ypxc + 0.5) * PIXEL_YSIZE + yc2;
		for (int xpx2 = 0; xpx2 < NUM_X_PIXELS; xpx2++)
		{
			x2 = ((double)(xpx2) - xpxc + 0.5) * PIXEL_XSIZE + xc2;
			for (int ypx1 = 0; ypx1 < NUM_Y_PIXELS; ypx1++)
			{
				y1 = ((double)(ypx1) - ypxc + 0.5) * PIXEL_YSIZE + yc1;
				for (int xpx1 = 0; xpx1 < NUM_X_PIXELS; xpx1++)
				{
					x1 = ((double)(xpx1) - xpxc + 0.5) * PIXEL_XSIZE + xc1;
					// calculate the track
					double rdx = (x1 - x2) / UNIT_ZDIFF;
					double rdy = (y1 - y2) / UNIT_ZDIFF;
					// calculate the acceptance
					double cs2phi = 1.0 / (1.0 + (rdx * rdx));
					double sn2the = 1.0 / (1.0 + (rdy * rdy * cs2phi));
					double cs2the = 1.0 - sn2the;
					double wt0 = cs2phi * sn2the * cs2phi * sn2the * ds;
					double wt1 = wt0 * cs2the;
					// calculate the hit position (8 planes)
					// Unit1 hit position
					double x1eh = (UNIT_ZDIFF - x1ez) * rdx + x2;
					double x1oh = (UNIT_ZDIFF - x1oz) * rdx + x2;
					double y1eh = (UNIT_ZDIFF - y1ez) * rdy + y2;
					double y1oh = (UNIT_ZDIFF - y1oz) * rdy + y2;
					if ( ((x1eh - x1e0) < 0.0) ||
					     ((x1eh - x1e0) > X_UNIT_W) ||
					     ((x1oh - x1o0) < 0.0) ||
					     ((x1oh - x1o0) > X_UNIT_W) )
						wt0 = wt1 = 0.0;
					if ( ((y1eh - y1e0) < 0.0) ||
					     ((y1eh - y1e0) > Y_UNIT_H) ||
					     ((y1oh - y1o0) < 0.0) ||
					     ((y1oh - y1o0) > Y_UNIT_H) )
						wt0 = wt1 = 0.0;
					// Unit2 hit position
					double x2eh = (UNIT_ZDIFF - x2ez) * rdx + x2;
					double x2oh = (UNIT_ZDIFF - x2oz) * rdx + x2;
					double y2eh = (UNIT_ZDIFF - y2ez) * rdy + y2;
					double y2oh = (UNIT_ZDIFF - y2oz) * rdy + y2;
					if ( ((x2eh - x2e0) < 0.0) ||
					     ((x2eh - x2e0) > X_UNIT_W) ||
					     ((x2oh - x2o0) < 0.0) ||
					     ((x2oh - x2o0) > X_UNIT_W) )
						wt0 = wt1 = 0.0;
					if ( ((y2eh - y2e0) < 0.0) ||
					     ((y2eh - y2e0) > Y_UNIT_H) ||
					     ((y2oh - y2o0) < 0.0) ||
					     ((y2oh - y2o0) > Y_UNIT_H) )
						wt0 = wt1 = 0.0;

					// Digitization
					double x1ed;
					double x1od;
					double y1ed;
					double y1od;
					double x2ed;
					double x2od;
					double y2ed;
					double y2od;
					if (nodigitize)
					{
						// Use hit positions for digitized potions
						x1ed = x1eh;
						x1od = x1oh;
						y1ed = y1eh;
						y1od = y1oh;
						x2ed = x2eh;
						x2od = x2oh;
						y2ed = y2eh;
						y2od = y2oh;
					}
					else
					{
						// Unit1 digitized position
						x1ed = digitize (x1eh, x1e0, digsx1);
						x1od = digitize (x1oh, x1o0, digsx1);
						y1ed = digitize (y1eh, y1e0, digsy1);
						y1od = digitize (y1oh, y1o0, digsy1);
						// Unit2 digitized position
						x2ed = digitize (x2eh, x2e0, digsx2);
						x2od = digitize (x2oh, x2o0, digsx2);
						y2ed = digitize (y2eh, y2e0, digsy2);
						y2od = digitize (y2oh, y2o0, digsy2);
					}

					// Averaged digitized position
					// Unit1 digitized average
					double x1av = (x1ed + x1od) * 0.5;
					double y1av = (y1ed + y1od) * 0.5;
					// Unit2 digitized average
					double x2av = (x2ed + x2od) * 0.5;
					double y2av = (y2ed + y2od) * 0.5;

					// Fitting using digitized positions
					double xpos[4];
					double xdat[4];
					int nxdat = 4;
					xpos[0] = x1ez;
					xdat[0] = x1ed;
					xpos[1] = x1oz;
					xdat[1] = x1od;
					xpos[2] = x2ez;
					xdat[2] = x2ed;
					xpos[3] = x2oz;
					xdat[3] = x2od;
					Fit2DLine xline (nxdat, xpos, xdat);
					double x1fit = xline.y (0.0);
					double x2fit = xline.y (UNIT_ZDIFF);

					double ypos[4];
					double ydat[4];
					int nydat = 4;
					ypos[0] = y1ez;
					ydat[0] = y1ed;
					ypos[1] = y1oz;
					ydat[1] = y1od;
					ypos[2] = y2ez;
					ydat[2] = y2ed;
					ypos[3] = y2oz;
					ydat[3] = y2od;
					Fit2DLine yline (nydat, ypos, ydat);
					double y1fit = yline.y (0.0);
					double y2fit = yline.y (UNIT_ZDIFF);

					if (wt0 > wt0max)
						wt0max = wt0;
					if (wt1 > wt1max)
						wt1max = wt1;
					if (enable_gap)
					{
						// check the hit posion on the pair plane
						// Y plane
						double xony1e = rdx * (UNIT_ZDIFF - y1ez) + x2 - xc1;
						double xony1o = rdx * (UNIT_ZDIFF - y1oz) + x2 - xc1;
						double effdx1e = overlapsize((xony1e - hfdsx1), (xony1e + hfdsx1), sizex1);
						double effdx1o = overlapsize((xony1o - hfdsx1), (xony1o + hfdsx1), sizex1);
						double effdx1 = effdx1e;
						if (effdx1 > effdx1o)
							effdx1 = effdx1o;
						double xony2e = rdx * (UNIT_ZDIFF - y2ez) + x2 - xc2;
						double xony2o = rdx * (UNIT_ZDIFF - y2oz) + x2 - xc2;
						double effdx2e = overlapsize((xony2e - hfdsx2), (xony2e + hfdsx2), sizex2);
						double effdx2o = overlapsize((xony2o - hfdsx2), (xony2o + hfdsx2), sizex2);
						double effdx2 = effdx2e;
						if (effdx2 > effdx2o)
							effdx2 = effdx2o;
						// X plane
						double yonx1e = rdy * (UNIT_ZDIFF - x1ez) + y2 - yc1;
						double yonx1o = rdy * (UNIT_ZDIFF - x1oz) + y2 - yc1;
						double effdy1e = overlapsize((yonx1e - hfdsy1), (yonx1e + hfdsy1), sizey1);
						double effdy1o = overlapsize((yonx1o - hfdsy1), (yonx1o + hfdsy1), sizey1);
						double effdy1 = effdy1e;
						if (effdy1 > effdy1o)
							effdy1 = effdy1o;
						double yonx2e = rdy * (UNIT_ZDIFF - x2ez) + y2 - yc2;
						double yonx2o = rdy * (UNIT_ZDIFF - x2oz) + y2 - yc2;
						double effdy2e = overlapsize((yonx2e - hfdsy2), (yonx2e + hfdsy2), sizey2);
						double effdy2o = overlapsize((yonx2o - hfdsy2), (yonx2o + hfdsy2), sizey2);
						double effdy2 = effdy2e;
						if (effdy2 > effdy2o)
							effdy2 = effdy2o;
						double effarea = effdx1 / dsx1 * effdy1 / dsy1 * effdx2 / dsx2 * effdy2 / dsy2;
						wt0 = wt0 * effarea;
						wt1 = wt1 * effarea;
					}
					if (enable_eo)
					{
						double wx1 = ((xpx1 & 1) == x1eoflag) ? eopos(rdx, eoamp) : eoneg(rdx, eoamp);
						double wy1 = ((ypx1 & 1) == y1eoflag) ? eopos(rdy, eoamp) : eoneg(rdy, eoamp);
						double wx2 = ((xpx2 & 1) == x2eoflag) ? eopos(rdx, eoamp) : eoneg(rdx, eoamp);
						double wy2 = ((ypx2 & 1) == y2eoflag) ? eopos(rdy, eoamp) : eoneg(rdy, eoamp);
						wt0 = wt0 * wx1 * wx2 * wy1 * wy2 * 16.0;
						wt1 = wt1 * wx1 * wx2 * wy1 * wy2 * 16.0;
					}
					double dx = (x1av - x2av) * scalex - diffx;
					double dy = (y1av - y2av) * scaley - diffy;
					hist2d[0].cumulate (dx, dy, wt0);
					hist2d[1].cumulate (dx, dy, wt0);
					hist2d[2].cumulate (dx, dy, wt1);
					hist2d[3].cumulate (dx, dy, wt1);

//					double phi = atan (-rdx);
//					double cst = (-rdy) / sqrt(rdx * rdx + rdy * rdy + 1.0);
//					(phi,cos(theta)) using fitted positions

					double rdxfit = xline.a();
					double rdyfit = yline.a();
					double phi = atan (-rdxfit);
					double cst = (-rdyfit) / sqrt(rdxfit * rdxfit + rdyfit * rdyfit + 1.0);
					hist2d[4].cumulate (phi, cst, wt0);
					hist2d[5].cumulate (phi, cst, wt0);
					hist2d[6].cumulate (phi, cst, wt1);
					hist2d[7].cumulate (phi, cst, wt1);

					// on the generator plane
					dx = (x1 - x2) - diffx;
					dy = (y1 - y2) - diffy;
					hist2d[8].cumulate (dx, dy, wt0);
					hist2d[9].cumulate (dx, dy, wt0);
					hist2d[10].cumulate (dx, dy, wt1);
					hist2d[11].cumulate (dx, dy, wt1);

					// using fitted positions
					dx = (x1fit - x2fit) - diffx;
					dy = (y1fit - y2fit) - diffy;
					hist2d[12].cumulate (dx, dy, wt0);
					hist2d[13].cumulate (dx, dy, wt0);
					hist2d[14].cumulate (dx, dy, wt1);
					hist2d[15].cumulate (dx, dy, wt1);

				}
			}
		}
	}
	mytimer.stop ();
	// Show results.
	std::string ofname = std::string("acc1f2.csv");
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
		if (nodigitize)
			ofcsv << ",nodigitize";
		else
			ofcsv << ",digitize";
		ofcsv
			<< ",digtize1xy," << digsx1 << ',' << digsy1
			<< ",digtize2xy," << digsx2 << ',' << digsy2;
		if (enable_gap)
			ofcsv << ",eneble_gap";
		else
			ofcsv << ",diable_gap";
		if (enable_eo)
			ofcsv << ",enable_eo";
		else
			ofcsv << ",disable_eo";
		ofcsv << ",eoamp," << eoamp;
		if (x1front_even)
			ofcsv << ",x1-EO";
		else
			ofcsv << ",x1-OE";
		if (y1front_even)
			ofcsv << ",y1-EO";
		else
			ofcsv << ",y1-OE";
		if (x2front_even)
			ofcsv << ",x2-EO";
		else
			ofcsv << ",x2-OE";
		if (y2front_even)
			ofcsv << ",y2-EO";
		else
			ofcsv << ",y2-OE";
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
