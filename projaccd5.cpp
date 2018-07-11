// projaccd5.cpp
//
// g++ -Wall projaccd5.cpp projaccopt.cpp mytimer.cpp hist2d.cpp ncpng.cpp -Wall -o projaccd5
//
// Acceptance for finite distance projection.
// 2018-06-04 correct csv output for 'proj-dist(m)' (comma was missing).
// 2017-12-08 range on projection plane in the case of (xscale < 1.0) and/or (yscale < 1.0) is added.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "projaccopt.h"
#include "mytimer.h"
#include "hist2d.h"

static const int VERSION_MAJOR_NUMBER = 1;
static const int VERSION_MINOR_NUMBER = 1;

static const int NUM_X_CHANNELS = 97;
static const int NUM_Y_CHANNELS = 97;

static const int NUM_PROJ_XBINS = (NUM_X_CHANNELS * 2 - 1);
static const int NUM_PROJ_YBINS = (NUM_Y_CHANNELS * 2 - 1);

double funit_xsize = 485.0;
double funit_ysize = 485.0;
double runit_xsize = 485.0;
double runit_ysize = 485.0;

double xyplane_dist = 21.0;

double funit_xplane_zpos;
double funit_yplane_zpos;
double runit_xplane_zpos;
double runit_yplane_zpos;

double xf_xpos [NUM_X_CHANNELS];
double yf_ypos [NUM_Y_CHANNELS];
double xr_xpos [NUM_X_CHANNELS];
double yr_ypos [NUM_Y_CHANNELS];

int main (int argc, char* argv [])
{

//	Options
	mylibrary::ProjAccOpt opt;

//	Set our defalut parameters
	opt.m_outname = "projaccd5";
	opt.m_dist = 1.15;
	opt.m_dxshift = 0.0;
	opt.m_dyshift = 0.0;
	opt.m_dzshift = 660.0;
	// check the arguments
	if (!opt.setargs (argc, argv))
	{
		opt.usage (std::cout, argv[0]);
		return (-1);
	}

//	Setup
	// Unit position differences
	double uxdiff = opt.m_dxshift;
	double uydiff = opt.m_dyshift;
	double uzdiff = opt.m_dzshift;

	double proj_zpos = -(opt.m_dist * 1000.0);
	
	runit_xplane_zpos = -(xyplane_dist * 0.5);
	runit_yplane_zpos = runit_xplane_zpos + xyplane_dist;
	funit_xplane_zpos = runit_xplane_zpos - uzdiff;
	funit_yplane_zpos = runit_yplane_zpos - uzdiff;

	std::cout
		<< " funit_xplane_zpos " << funit_xplane_zpos
		<< " funit_yplane_zpos " << funit_yplane_zpos
		<< " runit_xplane_zpos " << runit_xplane_zpos
		<< " runit_yplane_zpos " << runit_yplane_zpos
		<< std::endl;

	std::string outname = opt.m_outname;

	double xf;
	double yf;
	double xr;
	double yr;

	double xzdiff = funit_xplane_zpos - runit_xplane_zpos;
	double yzdiff = funit_yplane_zpos - runit_yplane_zpos;
	double avzdiff = (xzdiff + yzdiff) * 0.5;

	// Front Unit
	double xf_chsize = funit_xsize / double (NUM_X_CHANNELS);
	double xf_xmin = -(funit_xsize * 0.5) + uxdiff;
	double xf_xmax = xf_xmin + funit_xsize;

	for (int i = 0; i < NUM_X_CHANNELS; i++)
	{
		xf_xpos [i] = (double (i) + 0.5) * xf_chsize + xf_xmin;
	}

	double yf_chsize = funit_ysize / double (NUM_Y_CHANNELS);
	double yf_ymin = -(funit_ysize * 0.5) + uydiff;
	double yf_ymax = yf_ymin + funit_ysize;

	for (int i = 0; i < NUM_Y_CHANNELS; i++)
	{
		yf_ypos [i] = (double (i) + 0.5) * yf_chsize + yf_ymin;
	}

	// Rear Unit
	double xr_chsize = runit_xsize / double (NUM_X_CHANNELS);
	double xr_xmin = -(runit_xsize * 0.5);
	double xr_xmax = xr_xmin + runit_xsize;

	for (int i = 0; i < NUM_X_CHANNELS; i++)
	{
		xr_xpos [i] = (double (i) + 0.5) * xr_chsize + xr_xmin;
	}

	double yr_chsize = runit_ysize / double (NUM_Y_CHANNELS);
	double yr_ymin = -(runit_ysize * 0.5);
	double yr_ymax = yr_ymin + runit_ysize;

	for (int i = 0; i < NUM_Y_CHANNELS; i++)
	{
		yr_ypos [i] = (double (i) + 0.5) * yr_chsize + yr_ymin;
	}

	double xscale = (proj_zpos - runit_xplane_zpos) / (funit_xplane_zpos - runit_xplane_zpos);
	double yscale = (proj_zpos - runit_yplane_zpos) / (funit_yplane_zpos - runit_yplane_zpos);

	std::cout
		<< " xscale " << xscale
		<< " yscale " << yscale
		<< std::endl;

	// Range on the projection plane in meter units
	double xp_xmax;
	double xp_xmin;
	if (xscale > 1.0)
	{
		xp_xmax = ((xf_xmax - xr_xmin) * xscale + xr_xmin) / 1000.0;
		xp_xmin = ((xf_xmin - xr_xmax) * xscale + xr_xmax) / 1000.0;
	}
	else
	{
		xp_xmax = ((xf_xmax - xr_xmax) * xscale + xr_xmax) / 1000.0;
		xp_xmin = ((xf_xmin - xr_xmin) * xscale + xr_xmin) / 1000.0;
	}
	double yp_ymax;
	double yp_ymin;
	if (yscale > 1.0)
	{
		yp_ymax = ((yf_ymax - yr_ymin) * yscale + yr_ymin) / 1000.0;
		yp_ymin = ((yf_ymin - yr_ymax) * yscale + yr_ymax) / 1000.0;
	}
	else
	{
		yp_ymax = ((yf_ymax - yr_ymax) * yscale + yr_ymax) / 1000.0;
		yp_ymin = ((yf_ymin - yr_ymin) * yscale + yr_ymin) / 1000.0;
	}

	std::cout
		<< " xp_xmin " << xp_xmin
		<< " xp_xmax " << xp_xmax
		<< " yp_ymin " << yp_ymin
		<< " yp_ymax " << yp_ymax
		<< std::endl;

	static const int NUM_HIST2D = 6;
	mylibrary::Hist2D hist2d[NUM_HIST2D] =
	{
		mylibrary::Hist2D ("xp-yp", xp_xmin, xp_xmax, NUM_PROJ_XBINS, yp_ymin, yp_ymax, NUM_PROJ_YBINS),
		mylibrary::Hist2D ("xp-yp-smooth", xp_xmin, xp_xmax, NUM_PROJ_XBINS, yp_ymin, yp_ymax, NUM_PROJ_YBINS, true),
		mylibrary::Hist2D ("acc", xp_xmin, xp_xmax, NUM_PROJ_XBINS, yp_ymin, yp_ymax, NUM_PROJ_YBINS),
		mylibrary::Hist2D ("acc-smooth", xp_xmin, xp_xmax, NUM_PROJ_XBINS, yp_ymin, yp_ymax, NUM_PROJ_YBINS, true),
		mylibrary::Hist2D ("acc-cos2", xp_xmin, xp_xmax, NUM_PROJ_XBINS, yp_ymin, yp_ymax, NUM_PROJ_YBINS),
		mylibrary::Hist2D ("acc-cos2-smooth", xp_xmin, xp_xmax, NUM_PROJ_XBINS, yp_ymin, yp_ymax, NUM_PROJ_YBINS, true),
	};

	mylibrary::MyTimer mytimer;
	mytimer.start ();

	double dSdOmega0 = (xf_chsize / xzdiff) * (yf_chsize / yzdiff)
				* (xr_chsize / 1000.0) * (yr_chsize / 1000.0);

	for (int iyf = 0; iyf < NUM_Y_CHANNELS; iyf++)
	{
		yf = yf_ypos [iyf];
		for (int ixf = 0; ixf < NUM_X_CHANNELS; ixf++)
		{
			xf = xf_xpos [ixf];
			for (int iyr = 0; iyr < NUM_Y_CHANNELS; iyr++)
			{
				yr = yr_ypos [iyr];
				for (int ixr = 0; ixr < NUM_X_CHANNELS; ixr++)
				{
					xr = xr_xpos [ixr];
					double dx = xf - xr;
					double dy = yf - yr;
					double xp = (dx * xscale + xr) / 1000.0;
					double yp = (dy * yscale + yr) / 1000.0;
					double rx = dx / xzdiff;
					double ry = dy / yzdiff;
					double cs2 = 1.0 / ((rx * rx) + (ry * ry) + 1.0);
					double wt = dSdOmega0 * cs2 * cs2;
					double wtcs2 = wt * cs2;
					hist2d [0].cumulate (xp, yp);
					hist2d [1].cumulate (xp, yp);
					hist2d [2].cumulate (xp, yp, wt);
					hist2d [3].cumulate (xp, yp, wt);
					hist2d [4].cumulate (xp, yp, wtcs2);
					hist2d [5].cumulate (xp, yp, wtcs2);
				}
			}
		}
	}
	mytimer.stop ();

	// Show results
	for (int nh = 0; nh < NUM_HIST2D; nh++)
	{
		std::stringstream ss;
		ss << outname << '-' << nh << ".csv";
		std::string ofcsvnam;
		ss >> ofcsvnam;
		std::ofstream ofcsv (ofcsvnam.c_str());
		if (ofcsv)
		{
			ofcsv << "\"";
			for (int i = 0; i < argc; i++)
				ofcsv << argv[i] << ' ';
			ofcsv << "\",";
			mytimer.csvout (ofcsv);
			ofcsv
				<< ",\"Version(major,minor)\""
				<< ',' << VERSION_MAJOR_NUMBER
				<< ',' << VERSION_MINOR_NUMBER
				<< ",Detector5" << ',' << "vertial-setup"
				<< ",\"channels(x,y)\""
				<< ',' << NUM_X_CHANNELS
				<< ',' << NUM_Y_CHANNELS
				<< std::endl;
			ofcsv
				<< "\"channel-size(x,y)(mm)\""
				<< ',' << xr_chsize << ',' << yr_chsize
				<< ",\"u-dist(av,x,y)(mm)\""
				<< ',' << avzdiff << ',' << xzdiff << ',' << yzdiff
				<< ",\"u-shift(x,y,z)(mm)\""
				<< ',' << uxdiff << ',' << uydiff << ',' << uzdiff
				<< ",proj-dist(m)" << ',' << opt.m_dist
				<< ",\"scale(x,y)\""
				<< ',' << xscale << ',' << yscale
				<< std::endl;
			hist2d[nh].CSVdump (ofcsv);
		}
		else
			std::cerr << "ERROR: output file (" << ofcsvnam << ") open error."
				<< std::endl;
	}
	return 0;
}
