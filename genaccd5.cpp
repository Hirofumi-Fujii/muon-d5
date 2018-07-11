// genaccd5.cpp
//
// g++ -Wall genaccd5.cpp hist2d.cpp mytimer.cpp jokisch.cpp miyake.cpp ncpng.cpp -o genaccd5
//
// 2017-10-26 cos table was wrong, wt00 is recalculated (old methed may be correct). (Version 4.3)
// 2017-10-18 copied from muon1f3/genacc0
// 2017-10-18 options -uxdiff, -uydiff, -uzdiff are added (Version 4.2)
// 2017-10-17 class DetectorLayer is introduced (Version 4.1)
// 2017-08-10 Histogram numbering is changed (Version 4.0), 0:nogap acc, 1:4-fold acc, 2:8-fold acc, 3:cos table, 4:Jokisch table, 5:Miyake table
// 2017-05-18 Copied from muon1f2/genacc0.cpp (Version 3.0)
//   - histograms has been changed (0:nogap acc, 1:cos table, 2:jokisch table, 3:4-fold acc, 4:8-foldacc
//   - -gap and -nogap options were removed
//
// 2017-04-24 -o[ut] option is added
// 2017-04-24 YDIFF restored (135 -> 139)
// 2017-03-19 *** TEST *** UNIT_YDIFF 139 -> 135
// 2017-03-14 pcut0 can be set in the command line
// 2017-03-06 more output parameters in csv file
// 2017-03-07 log file added
// 2017-03-02 bug fixed for negative csthe
// 2017-02-27 re-arrange histograms (similar to accpyramid, Jokisch flux added)
// 2016-07-06 simple (no gap) version
// 2016-03-28 copy from muon1f1/genacc2
//
// generate acceptance

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#include "hist2d.h"
#include "jokisch.h"
#include "miyake.h"
#include "mytimer.h"

static const int VERSION_MAJOR_NUMBER = 4;
static const int VERSION_MINOR_NUMBER = 3;

static const int NUM_DETECTOR_LAYERS = 8;

static const int NUM_X_CHANNELS = 97;	// 5mm pitch
static const int NUM_Y_CHANNELS = 97;	// 5mm pitch

static const double X_LAYER_H = 500.0;	// height of the X layer
static const double X_LAYER_W = 480.0;	// width of the X layer
static const double Y_LAYER_H = 480.0;	// height of the Y layer
static const double Y_LAYER_W = 500.0;	// width of the Y layer

static const double X_UNIT_H = X_LAYER_H;	// height of the X unit
static const double X_UNIT_W = (X_LAYER_W + 5.0);	// width of the X unit
static const double Y_UNIT_H = (Y_LAYER_H + 5.0);	// height of the Y unit
static const double Y_UNIT_W = Y_LAYER_W;	// width of the Y unit

static const double UNIT_XDIFF = 0.0;
static const double UNIT_YDIFF = 0.0;
static const double UNIT_ZDIFF = 360.0;

//
double g_det_uxdiff;	// detector unit X-position difference
double g_det_uydiff;	// detector unit Y-position differnece
double g_det_uzdiff;	// detector unit Z-position difference

double g_detzpos_xf;	// z-position of the front unit x-plane 
double g_detzpos_yf;	// z-position of the front unit y-plane 
double g_detzpos_xr;	// z-position of the rear unit x-plane 
double g_detzpos_yr;	// z-position of the rear unit y-plane 
	
double detxpos[NUM_DETECTOR_LAYERS];
double detypos[NUM_DETECTOR_LAYERS];
double detzpos[NUM_DETECTOR_LAYERS];

double detxsize[NUM_DETECTOR_LAYERS];
double detysize[NUM_DETECTOR_LAYERS];

/////
class DetectorLayer
{
public:
	enum
	{
		DETECT_DIR_X = 1,
		DETECT_DIR_Y,
	};

	DetectorLayer () :
		m_name (""), m_dir (0), m_xsize (0.0), m_ysize (0.0),
		m_xpos (0.0), m_ypos (0.0), m_zpos (0.0) {}

	DetectorLayer (const std::string& name, int dir, double xsize, double ysize,
		double xpos, double ypos, double zpos) :
		m_name (name), m_dir (dir), m_xsize (xsize), m_ysize (ysize),
		m_xpos (xpos), m_ypos (ypos), m_zpos (zpos) {}

	std::string m_name;
	int m_dir;
	double m_xsize;
	double m_ysize;
	double m_xpos;
	double m_ypos;
	double m_zpos;
};

DetectorLayer layers [NUM_DETECTOR_LAYERS];

int setupd5 ()
{
	/// D5 setup
	/// The layer-order from the reactor-building is
	/// XB-XA-YB-YA - XB-XA-YB-YA

	double xygap = 21.0;
	double layer_xshift = X_UNIT_W - X_LAYER_W;
	double layer_yshift = Y_UNIT_H - Y_LAYER_H;
	double layer_xthick = 10.0;
	double layer_ythick = 10.0;

	double hxs = layer_xshift * 0.5;
	double hys = layer_yshift * 0.5;
	double hxt = layer_xthick * 0.5;
	double hyt = layer_ythick * 0.5;

	double x_layer_w = X_LAYER_W;
	double x_layer_h = X_LAYER_H;
	double y_layer_w = Y_LAYER_W;
	double y_layer_h = Y_LAYER_H;

	// Front Unit
	double xc = g_det_uxdiff;
	double yc = g_det_uydiff;
	double zc = -g_det_uzdiff;
	
	double zxc = zc - (xygap * 0.5);
	double zyc = zc + (xygap * 0.5);
	
	g_detzpos_xf = zxc;
	g_detzpos_yf = zyc;

	layers[0] = DetectorLayer
	(
		std::string ("XB"), DetectorLayer::DETECT_DIR_X, 
		x_layer_w, x_layer_h, xc + hxs, yc, zxc - hxt
	);
	layers[1] = DetectorLayer
	(
		std::string ("XA"), DetectorLayer::DETECT_DIR_X, 
		x_layer_w, x_layer_h, xc - hxs, yc, zxc + hxt
	);
	layers[2] = DetectorLayer
	(
		std::string ("YB"), DetectorLayer::DETECT_DIR_Y, 
		y_layer_w, y_layer_h, xc, yc + hys, zyc - hyt
	);
	layers[3] = DetectorLayer
	(
		std::string ("YA"), DetectorLayer::DETECT_DIR_Y, 
		y_layer_w, y_layer_h, xc, yc - hys, zyc + hyt
	);

	// Rear Unit
	xc = 0.0;
	yc = 0.0;
	zc = 0.0;

	zxc = zc - (xygap * 0.5);
	zyc = zc + (xygap * 0.5);

	g_detzpos_xr = zxc;
	g_detzpos_yr = zyc;

	layers[4] = DetectorLayer
	(
		std::string ("XB"), DetectorLayer::DETECT_DIR_X, 
		x_layer_w, x_layer_h, xc + hxs, yc, zxc - hxt
	);
	layers[5] = DetectorLayer
	(
		std::string ("XA"), DetectorLayer::DETECT_DIR_X, 
		x_layer_w, x_layer_h, xc - hxs, yc, zxc + hxt
	);
	layers[6] = DetectorLayer
	(
		std::string ("YB"), DetectorLayer::DETECT_DIR_Y, 
		y_layer_w, y_layer_h, xc, yc + hys, zyc - hyt
	);
	layers[7] = DetectorLayer
	(
		std::string ("YA"), DetectorLayer::DETECT_DIR_Y, 
		y_layer_w, y_layer_h, xc, yc - hys, zyc + hyt
	);

	/// Copy to the local buffers
	for (int n = 0; n < NUM_DETECTOR_LAYERS; n++)
	{
		detxpos [n] = layers [n].m_xpos;
		detypos [n] = layers [n].m_ypos;
		detzpos [n] = layers [n].m_zpos;
		detxsize [n] = layers [n].m_xsize;
		detysize [n] = layers [n].m_ysize;
	}
	return 0;	
}

double 
overlapsize8 (int numdet, const double xpos[], const double ypos[], const double ysize[], double slope)
{
	// overlapsize for 8-fold coincidence
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
overlapsize4 (int numdet, const double xpos[], const double ypos[], const double ysize[], double slope)
{
	// numdet must be even number
	// overlapsize for 4-fold coincidence
	// xpos and ypos is the position of the center of the detector
	// i.e., i-th detector covers the range (ypos[i] - ysize[i] * 0.5, ypos[i] + ysize[i] * 0.5).

	double ymin = 0.0;
	double ymax = 0.0;
	int n = 0;	// layer number
	int nmax = (numdet / 2) * 2;
	while (n < nmax)
	{
		double yorl;
		double yorh;
		for (int k = 0; k < 2; k++)
		{
			// range at xpos = 0.0
			double dy = slope * xpos[n + k];
			double halfsize = ysize[n + k] * 0.5;
			double yl = ypos[n + k] - halfsize - dy;
			double yh = yl + ysize[n + k];
			if (k == 0)
			{
				yorl = yl;
				yorh = yh;
			}
			else
			{
				if (yl < yorl)
					yorl = yl;
				if (yh > yorh)
					yorh = yh;
			}
		}
		if (n == 0)
		{
			ymin = yorl;
			ymax = yorh;
		}
		else
		{
			if ((yorh <= ymin) || (yorl >= ymax))
				return 0.0;
			if (yorh < ymax)
				ymax = yorh;
			else if (yorl > ymin)
				ymin = yorl;
		}
		n = n + 2;
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

	// defalut settings
	double pcut0 = 0.0;
	std::string outname;
	g_det_uxdiff = UNIT_XDIFF;
	g_det_uydiff = UNIT_YDIFF;
	g_det_uzdiff = UNIT_ZDIFF;

	bool outnamegiven = false;
	int iarg = 1;
	while (iarg < argc)
	{
		std::string word(argv[iarg++]);
		if ((word.size() > 0) && (word[0] == '-'))
		{
			if (
				(word == "-pcut") || (word == "-pcut0") ||
				(word == "-udist") || (word == "-unitdist") ||
				(word == "-uxdiff") || (word == "-uydiff") || (word == "-uzdiff")
			)
			{
				if (iarg >= argc)
				{
					std::cerr << "ERROR " << argv[0]
						<< " parameter for " << word
						<< " is missing."
						<< std::endl;
					return (-1);
				}
				std::stringstream ss;
				ss << argv[iarg];
				iarg++;
				double dv(0.0);
				if (ss >> dv)
				{
					if ((word == "-pcut") || (word == "-pcut0"))
					{
						pcut0 = dv;
					}
					else if (word == "-uxdiff")
					{
						g_det_uxdiff = dv;
					}
					else if (word == "-uydiff")
					{
						g_det_uydiff = dv;
					}
					else if ((word == "-uzdiff") || 
						(word == "-unitdist") || (word == "-udist"))
					{
						g_det_uzdiff = dv;
					}
				}
				else
				{
					std::cerr << "ERROR " << argv[0]
						<< " the parameter value for " << word
						<< " is not numerical."
						<< std::endl;
					return (-1);
				}
			}
			else if ((word == "-out") || (word == "-o"))
			{
				if (iarg >= argc)
				{
					std::cerr << "ERROR " << argv[0]
						<< " parameter for " << word
						<< " is missing."
						<< std::endl;
					return (-1);
				}
				outname = std::string(argv[iarg]);
				outnamegiven = true;
				++iarg;
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

	setupd5 ();

	double zx1 = g_detzpos_xf;
	double zy1 = g_detzpos_yf;
	double zx2 = g_detzpos_xr;
	double zy2 = g_detzpos_yr;

	double distx = zx1 - zx2;
	double disty = zy1 - zy2;

	double dist = (distx + disty) * 0.5;
	double scalex = dist / distx;
	double scaley = dist / disty;

	double dsx = (double)(X_UNIT_W) / ((double)(NUM_X_CHANNELS));
	double dsy = (double)(Y_UNIT_H) / ((double)(NUM_Y_CHANNELS));

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
	
	static const int NUM_HIST2D = 6;
	Hist2D hist2d[NUM_HIST2D] =
	{
		Hist2D ("dsdo-0 sr*cm^2", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("dsdo-4 sr*cm^2", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("dsdo-8 sr*cm^2", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("cos(theta)", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("Jokisch flux (1/(s*sr*m^2)", -482.5, 482.5, 193, -482.5, 482.5, 193),
		Hist2D ("Miyake flux (1/(s*sr*m^2)", -482.5, 482.5, 193, -482.5, 482.5, 193),
	};

	mylibrary::MyTimer mytimer;
	mytimer.start ();
	for (int ych2 = 0; ych2 < num_y_bins; ych2++)
	{
		double ddy = (double)(ych2) * dsy + y0;
		for (int xch2 = 0; xch2 < num_x_bins; xch2++)
		{
			double ddx = (double)(xch2) * dsx + x0;
			double rdx = (ddx + g_det_uxdiff) / distx;
			double rdy = (ddy + g_det_uydiff) / disty;
//			double cs2phi = 1.0 / (1.0 + (rdx * rdx));
//			double sn2the = 1.0 / (1.0 + (rdy * rdy * cs2phi));
//			double cs2the = 1.0 - sn2the;
			double cs2the = 1.0 / ((rdx * rdx) + (rdy * rdy) + 1.0);
			double csthe = sqrt (cs2the);
//			if (rdy > 0.0)
//				csthe = -csthe;
			// get number of combinations
			double effxsize0 = efflen (ddx, double(X_UNIT_W));
			double effysize0 = efflen (ddy, double(Y_UNIT_H));
			double effxsize4 = overlapsize4(NUM_DETECTOR_LAYERS, detzpos, detxpos, detxsize, rdx);
			double effysize4 = overlapsize4(NUM_DETECTOR_LAYERS, detzpos, detypos, detysize, rdy);
			double effxsize8 = overlapsize8(NUM_DETECTOR_LAYERS, detzpos, detxpos, detxsize, rdx);
			double effysize8 = overlapsize8(NUM_DETECTOR_LAYERS, detzpos, detypos, detysize, rdy);

//			double wt0 = cs2phi * sn2the * cs2phi * sn2the * ds * (effxsize /dsx) * (effysize / dsy);
//			double wt1 = wt0 * cs2the;

//			double wt00 = cs2phi * sn2the * cs2phi * sn2the * ds /dsx / dsy;
			double wt00 = cs2the * cs2the * ds / dsx / dsy;	// for d5
			double wt0 = wt00 * effxsize0 * effysize0;
			double wt4 = wt00 * effxsize4 * effysize4;
			double wt8 = wt00 * effxsize8 * effysize8;
			double flxj = Jokisch::integral_flux (pcut0, fabs (csthe));
			double flxm = Miyake::integral_flux (pcut0, fabs (csthe));

			hist2d[0].cumulate (ddx, ddy, wt0);
			hist2d[1].cumulate (ddx, ddy, wt4);
			hist2d[2].cumulate (ddx, ddy, wt8);
			hist2d[3].cumulate (ddx, ddy, csthe);
			hist2d[4].cumulate (ddx, ddy, flxj);
			hist2d[5].cumulate (ddx, ddy, flxm);
		}
	}
	mytimer.stop ();

	// Show results.
	if (!outnamegiven)
		outname = std::string("genaccd5");
	std::string logcsvnam = outname + "-log.csv";
	std::ofstream oflog (logcsvnam.c_str());
	if (oflog)
	{
		oflog << '\"';
		for (int i = 0; i < argc; i++)
		{
			if (i)
				oflog << ' ';
			oflog << argv[i];
		}
		oflog << "\",";
		mytimer.csvout (oflog);
		oflog
			<< ",\"Version(major,minor)\""
			<< ',' << VERSION_MAJOR_NUMBER
			<< ',' << VERSION_MINOR_NUMBER
			<< ",\"Number of layers\""
			<< ',' << NUM_DETECTOR_LAYERS
			<< ",pcut0(GeV/c)"
			<< ',' << pcut0
			<< std::endl;
		oflog
			<< "plane-id"
			<< ",plane-name"
			<< ",xpos"
			<< ",ypos"
			<< ",zpos"
			<< ",xsize"
			<< ",ysize"
			<< std::endl;

		for (int n = 0; n < NUM_DETECTOR_LAYERS; n++)
		{
			oflog
				<< n
				<< ',' << layers[n].m_name
				<< ',' << layers[n].m_xpos
				<< ',' << layers[n].m_ypos
				<< ',' << layers[n].m_zpos
				<< ',' << layers[n].m_xsize
				<< ',' << layers[n].m_ysize
				<< std::endl;
		}
	}

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
				<< ",\"diff(x,y)(mm)\"," << g_det_uxdiff << ',' << g_det_uydiff 
				<< ",\"scale(x,y)\"," << scalex << ',' << scaley
				<< ",enable_gap,true"
				<< ",pcut0(GeV/c)"
				<< ',' << pcut0
				<< std::endl;

			hist2d[n].CSVdump (ofcsv);
			ofcsv.close ();
		}
		else
		{
			std::cerr << "ERROR: " << outcsvnam << " cannot be created." << std::endl;
		}
	}
	return 0;
}
