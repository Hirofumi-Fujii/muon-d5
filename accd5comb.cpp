// accd5comb.cpp
// 2017-11-10 log (geometrical parameters) output is added. introduced g_uxdiff and g_uydiff.
//            change the direction of the g_xdist and g_ydist (right-handed) (version 4,2)
// 2017-10-27 dsdo calculation formula is changed (version 4.2)
// 2017-10-17 -udist, -out, -pcut options are added (version 4,1)
// g++ -Wall accd5comb.cpp hist2d.cpp ncpng.cpp mytimer.cpp jokisch.cpp -o accd5comb

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include "hist2d.h"
#include "mytimer.h"
#include "jokisch.h"

static const int VERSION_MAJOR_NUMBER = 4;
static const int VERSION_MINOR_NUMBER = 2;

static const int NUM_UNITS = 4;
static const int NUM_X_CHANNELS = 48;
static const int NUM_Y_CHANNELS = 48;

static const int NUM_PLANES = (NUM_UNITS * 2);
static const int NUM_X_PIXELS = (NUM_X_CHANNELS * 2 - 1);
static const int NUM_Y_PIXELS = (NUM_Y_CHANNELS * 2 - 1);

enum
{
	X_PLANE = 0,
	Y_PLANE,
};

// Z positions
double g_x_zpos [NUM_UNITS];	// for X planes
double g_y_zpos [NUM_UNITS];	// for Y planes

// XY gap
double g_xygap;

// XY position differences
double g_dx [NUM_X_PIXELS];
double g_dy [NUM_Y_PIXELS];

// XY position differences in device coordinate
double g_ddx [NUM_X_PIXELS];
double g_ddy [NUM_Y_PIXELS];

double g_width;
double g_length;
double g_layergap;
double g_layershift;
double g_udist;
double g_uxdiff;
double g_uydiff;

double g_dxsize;
double g_dysize;

double g_xdiff;
double g_ydiff;

double g_xdist;
double g_ydist;

double g_xpos [NUM_PLANES];
double g_ypos [NUM_PLANES];
double g_zpos [NUM_PLANES];

double g_xmax [NUM_PLANES];
double g_xmin [NUM_PLANES];
double g_ymax [NUM_PLANES];
double g_ymin [NUM_PLANES];

int g_plane [NUM_PLANES];
bool g_selected [NUM_PLANES];

double g_effxlen0 [NUM_X_PIXELS];
double g_effylen0 [NUM_Y_PIXELS];

double g_effxlenS [NUM_X_PIXELS];
double g_effylenS [NUM_Y_PIXELS];

double g_effxlenA [NUM_X_PIXELS];
double g_effylenA [NUM_Y_PIXELS];

double g_pcut;

void setup (const std::string& datsetname)
{

	// 1F2/1F3
	// 0   1   2   3   4   5   6   7
	// Y0E Y0O X0E X0O X1O X1E Y1O Y1E
	//
	// d5
	// 0   1   2   3   4   5   6   7
	// X0O X0E Y0O Y0E X1O X1E Y1O Y1E

	g_plane [0] = X_PLANE;
	g_plane [1] = X_PLANE;
	g_plane [2] = Y_PLANE;
	g_plane [3] = Y_PLANE;
	g_plane [4] = X_PLANE;
	g_plane [5] = X_PLANE;
	g_plane [6] = Y_PLANE;
	g_plane [7] = Y_PLANE;

	g_width = 480.0;	// in mm
	g_length = 500.0;	// in mm

	g_layergap = 10.0;	// in mm
	g_layershift = 5.0;	// in mm

	g_xygap = 21.0;	// in mm

	g_dxsize = g_width / double (NUM_X_CHANNELS);
	g_dysize = g_width / double (NUM_Y_CHANNELS);

	double halfshift = g_layershift * 0.5;
	g_xpos [7] =  0.0;
	g_xpos [6] =  0.0;
	g_xpos [5] = -halfshift;
	g_xpos [4] =  halfshift;
	g_xpos [3] =  0.0 + g_uxdiff;
	g_xpos [2] =  0.0 + g_uxdiff;
	g_xpos [1] = -halfshift + g_uxdiff;
	g_xpos [0] =  halfshift + g_uxdiff;

	g_ypos [7] = -halfshift;
	g_ypos [6] =  halfshift;
	g_ypos [5] =  0.0;
	g_ypos [4] =  0.0;
	g_ypos [3] = -halfshift + g_uydiff;
	g_ypos [2] =  halfshift + g_uydiff;
	g_ypos [1] =  0.0 + g_uydiff;
	g_ypos [0] =  0.0 + g_uydiff;

	g_zpos [0] = 0.0;
	g_zpos [1] = g_zpos [0] + g_layergap;
	g_zpos [2] = g_zpos [0] + g_xygap;
	g_zpos [3] = g_zpos [1] + g_xygap;
	g_zpos [4] = g_zpos [0] + g_udist;
	g_zpos [5] = g_zpos [1] + g_udist;
	g_zpos [6] = g_zpos [2] + g_udist;
	g_zpos [7] = g_zpos [3] + g_udist;


	for (int n = 0; n < NUM_PLANES; n++)
	{
		if (g_plane [n] == X_PLANE)
		{
			g_xmax [n] = g_width * 0.5 + g_xpos [n];
			g_xmin [n] = g_xmax [n] - g_width;
			g_ymax [n] = g_length * 0.5 + g_ypos [n];
			g_ymin [n] = g_ymax [n] - g_length;
		}
		else
		{
			g_xmax [n] = g_length * 0.5 + g_xpos [n];
			g_xmin [n] = g_xmax [n] - g_length;
			g_ymax [n] = g_width * 0.5 + g_ypos [n];
			g_ymin [n] = g_ymax [n] - g_width;
		}
	}
}

void calcgeom ()
{

	for (int n = 0; n < NUM_X_PIXELS; n++)
	{
		g_ddx [n] = (g_dxsize * double (n + 1)) - g_width;
		g_dx[n] = g_ddx[n] + g_xdiff;
	}

	// utsana0 is using right-handed coordinate
	for (int n = 0; n < NUM_Y_PIXELS; n++)
	{
		g_ddy [n] = (g_dysize * double (n + 1)) - g_width;
		g_dy[n] = g_ddy [n] + g_ydiff;
	}
}

double EffXlen0 (int frontid, int baseid, double rx)
{
	double xmax = g_xmax [baseid];
	double xmin = g_xmin [baseid];
//	for (int n = 0; n < NUM_PLANES; n++)
//	{
		// get the n-th center position on the [baseid] plane
//		double dx = (rx * (g_zpos[frontid] - g_zpos[baseid]));
		double dx = (rx * (g_zpos[baseid] - g_zpos[frontid]));
		if ((dx + g_xmax[frontid]) < xmax)
			xmax = dx + g_xmax[frontid];
		if ((dx + g_xmin[frontid]) > xmin)
			xmin = dx + g_xmin[frontid];
//	}
	if (xmax < xmin)
		return 0.0;
	return (xmax - xmin);
}

double EffYlen0 (int frontid, int baseid, double ry)
{
	double ymax = g_ymax [baseid];
	double ymin = g_ymin [baseid];
//	for (int n = 0; n < NUM_PLANES; n++)
//	{
		// get the n-th center position on the [baseid] plane
//		double dy = (ry * (g_zpos[frontid] - g_zpos[baseid]));
		double dy = (ry * (g_zpos[baseid] - g_zpos[frontid]));
		if ((dy + g_ymax[frontid]) < ymax)
			ymax = dy + g_ymax[frontid];
		if ((dy + g_ymin[frontid]) > ymin)
			ymin = dy + g_ymin[frontid];
//	}
	if (ymax < ymin)
		return 0.0;
	return (ymax - ymin);
}

double EffXlenA (int baseid, double rx)
{
	double xmax = g_xmax [baseid];
	double xmin = g_xmin [baseid];
	for (int n = 0; n < NUM_PLANES; n++)
	{
		// get the n-th center position on the [baseid] plane
//		double dx = (rx * (g_zpos[n] - g_zpos[baseid]));
		double dx = (rx * (g_zpos[baseid] - g_zpos[n]));
		if ((dx + g_xmax[n]) < xmax)
			xmax = dx + g_xmax[n];
		if ((dx + g_xmin[n]) > xmin)
			xmin = dx + g_xmin[n];
	}
	if (xmax < xmin)
		return 0.0;
	return (xmax - xmin);
}

double EffYlenA (int baseid, double ry)
{
	double ymax = g_ymax [baseid];
	double ymin = g_ymin [baseid];
	for (int n = 0; n < NUM_PLANES; n++)
	{
		// get the n-th center position on the [baseid] plane
//		double dy = (ry * (g_zpos[n] - g_zpos[baseid]));
		double dy = (ry * (g_zpos[baseid] - g_zpos[n]));
		if ((dy + g_ymax[n]) < ymax)
			ymax = dy + g_ymax[n];
		if ((dy + g_ymin[n]) > ymin)
			ymin = dy + g_ymin[n];
	}
	if (ymax < ymin)
		return 0.0;
	return (ymax - ymin);
}

double EffXlenS (int baseid, double rx)
{
	double xmax = g_xmax [baseid];
	double xmin = g_xmin [baseid];
	for (int n = 0; n < NUM_PLANES; n++)
	{
		if (g_selected [n])
		{
			// get the n-th center position on the [baseid] plane
//			double dx = (rx * (g_zpos[n] - g_zpos[baseid]));
			double dx = (rx * (g_zpos[baseid] - g_zpos[n]));
			if ((dx + g_xmax[n]) < xmax)
				xmax = dx + g_xmax[n];
			if ((dx + g_xmin[n]) > xmin)
				xmin = dx + g_xmin[n];
		}
	}
	if (xmax < xmin)
		return 0.0;
	return (xmax - xmin);
}

double EffYlenS (int baseid, double ry)
{
	double ymax = g_ymax [baseid];
	double ymin = g_ymin [baseid];
	for (int n = 0; n < NUM_PLANES; n++)
	{
		if (g_selected [n])
		{
			// get the n-th center position on the [baseid] plane
//			double dy = (ry * (g_zpos[n] - g_zpos[baseid]));
			double dy = (ry * (g_zpos[baseid] - g_zpos[n]));
			if ((dy + g_ymax[n]) < ymax)
				ymax = dy + g_ymax[n];
			if ((dy + g_ymin[n]) > ymin)
				ymin = dy + g_ymin[n];
		}
	}
	if (ymax < ymin)
		return 0.0;
	return (ymax - ymin);
}

int main (int argc, char* argv[])
{
	for (int i = 0; i < NUM_PLANES; i++)
		g_selected [i] = false;

	// default values
	g_udist = 360.0;
	g_uxdiff = 0.0;
	g_uydiff = 0.0;	// in mm
	g_pcut = 0.0;	// in GeV/c

	std::string datsetnam;
	std::string outname;
	bool outname_given = false;
	bool datset_given = false;

	int iarg = 1;
	while (iarg < argc)
	{
		std::string word (argv [iarg]);
		++iarg;
		if ((word == "-udist") || (word == "-pcut0") || (word == "-pcut"))
		{
			if (iarg < argc)
			{
				std::stringstream ss;
				ss << argv[iarg] << ' ';
				++iarg;
				double dval;
				if (ss >> dval)
				{
					if (word == "-udist")
						g_udist = dval;
					else if ((word == "-pcut0") || (word == "-pcut"))
						g_pcut = dval;
				}
				else
				{
					std::cerr << "ERROR: cannot read the value."
						<< std::endl;
					return (-1);
				}
			}
			else
			{
				std::cerr << "ERROR: value is missing."
					<< std::endl;
				return (-1);
			}
		}
		else if ((word == "-out") || (word == "-o"))
		{
			if (iarg < argc)
			{
				outname = std::string (argv[iarg]);
				++iarg;
				outname_given = true;
			}
			else
			{
				std::cerr << "ERROR: name is missing."
					<< std::endl;
				return (-1);
			}
		}
		else
		{
			datset_given = true;
			datsetnam = word;
		}
	}

	if (!datset_given)
	{
		std::cerr << "ERROR: dataset name is missing.\n"
			<< "Usage: " << argv[0] << " [options] dataset_name\n"
			<< "     where dataset_name is one of\n"
			<< "       EOEO EOOE EOEE EOOO OEEO OEOE OEOE OEEE OEOO\n"
			<< "       EEEO EEOE EEEE EEOO OOEO OOOE OOOE OOEE OOOO\n"
			<< "options\n"
			<< " -out name        Prefix name for output files.\n"
			<< " -pcut0 value     Cut-off momentum (GeV/c) for detector.\n"
			<< " -udist value     XY-unit distance in mm units.\n"
			<< std::endl;
		return (-1);
	}

	if (!outname_given)
		outname = "accd5comb";

	std::cout << "datsetnam = " << datsetnam << std::endl;
	std::cout << "outname = " << outname << std::endl;

	// 1F2/1F3
	// 0   1   2   3   4   5   6   7
	// Y0E Y0O X0E X0O X1O X1E Y1O Y1E
	//
	// d5
	// 0   1   2   3   4   5   6   7
	// X0O X0E Y0O Y0E X1O X1E Y1O Y1E

	int idx0 = 0;
	int idx1 = 4;
	int idy0 = 2;
	int idy1 = 6;

	if (datsetnam[0] == 'E')
		idx0 = 1;
	else if (datsetnam[0] == 'O')
		idx0 = 0;
	else
	{
		std::cerr << "ERROR: no such dataset name [" << datsetnam << "]."
			<< std::endl;
		return (-2);
	}

	if (datsetnam[1] == 'E')
		idx1 = 5;
	else if (datsetnam[1] == 'O')
		idx1 = 4;
	else
	{
		std::cerr << "ERROR: no such dataset name [" << datsetnam << "]."
			<< std::endl;
		return (-2);
	}

	if (datsetnam[2] == 'E')
		idy0 = 3;
	else if (datsetnam[2] == 'O')
		idy0 = 2;
	else
	{
		std::cerr << "ERROR: no such dataset name [" << datsetnam << "]."
			<< std::endl;
		return (-2);
	}

	if (datsetnam[3] == 'E')
		idy1 = 7;
	else if (datsetnam[3] == 'O')
		idy1 = 6;
	else
	{
		std::cerr << "ERROR: no such dataset name [" << datsetnam << "]."
			<< std::endl;
		return (-2);
	}

	g_selected [idx0] = true;
	g_selected [idy0] = true;
	g_selected [idx1] = true;
	g_selected [idy1] = true;

	std::cout << "Dataset " << datsetnam
		<< " idy0=" << idy0 << " idx0=" << idx0
		<< " idx1=" << idx1 << " idy1=" << idy1
		<< std::endl;

	setup (datsetnam);
//--	g_xdist = g_zpos [idx1] - g_zpos [idx0];
//--	g_ydist = g_zpos [idy1] - g_zpos [idy0];
	g_xdist = g_zpos [idx0] - g_zpos [idx1];
	g_ydist = g_zpos [idy0] - g_zpos [idy1];
	g_xdiff = g_xpos [idx0] - g_xpos [idx1];
	g_ydiff = g_ypos [idy0] - g_ypos [idy1];

//@@@
	std::cout << "xdiff " << g_xdiff << ", ydiff " << g_ydiff << std::endl;
	calcgeom ();

	// Histograms
	static const int numhist2d = 5;
	mylibrary::Hist2D hist2d [numhist2d] =
	{
		mylibrary::Hist2D ("dsdo-0 sr*cm^2",
				- (g_width - (g_dxsize * 0.5)), g_width - (g_dxsize * 0.5), NUM_X_PIXELS,
				- (g_width - (g_dysize * 0.5)), g_width - (g_dysize * 0.5), NUM_Y_PIXELS ),
		mylibrary::Hist2D ("dsdo-4 sr*cm^2",
				- (g_width - (g_dxsize * 0.5)), g_width - (g_dxsize * 0.5), NUM_X_PIXELS,
				- (g_width - (g_dysize * 0.5)), g_width - (g_dysize * 0.5), NUM_Y_PIXELS ),
		mylibrary::Hist2D ("dsdo-8 sr*cm^2",
				- (g_width - (g_dxsize * 0.5)), g_width - (g_dxsize * 0.5), NUM_X_PIXELS,
				- (g_width - (g_dysize * 0.5)), g_width - (g_dysize * 0.5), NUM_Y_PIXELS ),
		mylibrary::Hist2D ("cos(theta)",
				- (g_width - (g_dxsize * 0.5)), g_width - (g_dxsize * 0.5), NUM_X_PIXELS,
				- (g_width - (g_dysize * 0.5)), g_width - (g_dysize * 0.5), NUM_Y_PIXELS ),
		mylibrary::Hist2D ("Jokisch flux sr*m^2",
				- (g_width - (g_dxsize * 0.5)), g_width - (g_dxsize * 0.5), NUM_X_PIXELS,
				- (g_width - (g_dysize * 0.5)), g_width - (g_dysize * 0.5), NUM_Y_PIXELS ),
	};

	for (int n = 0; n < NUM_PLANES; n++)
	{
		std::cout << n
			<< ',' << g_xpos[n]
			<< ',' << g_ypos[n]
			<< ',' << g_zpos[n]
			<< ',' << g_xmin[n]
			<< ',' << g_xmax[n]
			<< ',' << g_ymin[n]
			<< ',' << g_ymax[n]
			<< std::endl;
	}

	mylibrary::MyTimer mytimer;
	mytimer.start ();

	for (int ix = 0; ix < NUM_X_PIXELS; ix++)
	{
		double dx = g_dx[ix];
		double rx = dx / g_xdist;
		g_effxlen0 [ix] = EffXlen0 (idx0, idx1, rx);
		g_effxlenA [ix] = EffXlenA (idx1, rx);
		g_effxlenS [ix] = EffXlenS (idx1, rx);
	}

	for (int iy = 0; iy < NUM_Y_PIXELS; iy++)
	{
		double dy = g_dy[iy];
		double ry = dy / g_ydist;
		g_effylen0 [iy] = EffYlen0 (idy0, idy1, ry);
		g_effylenA [iy] = EffYlenA (idy1, ry);
		g_effylenS [iy] = EffYlenS (idy1, ry);
	}

	double dxdy = g_dxsize / g_xdist * g_dysize / g_ydist;

	for (int iy = 0; iy < NUM_Y_PIXELS; iy++)
	{
		double dy = g_dy[iy];
		double ry = dy / g_ydist;
		for (int ix = 0; ix < NUM_X_PIXELS; ix++)
		{
			double dx = g_dx[ix];
			double rx = dx / g_xdist;

//			double cs2f = 1.0 / (1.0 + (rx * rx));
//			double csf = sqrt (cs2f);
//			double sn2t = 1.0 / (1.0 + (ry * ry * cs2f));
//			double cst = sqrt (1.0 - sn2t);
//			if (ry < 0.0)
//				cst = -cst;
//			double dsdo = cs2f * sn2t * cs2f * sn2t * dxdy;

			double cs2t = 1.0 / (rx * rx + ry * ry + 1.0);
			double cst = sqrt (cs2t);
			double dsdo = cs2t * cs2t * dxdy;
			
			double eff0 = g_effxlen0 [ix] * g_effylen0 [iy] * 0.01;	// 0.01 = mm^2->cm^2
			double effs = g_effxlenS [ix] * g_effylenS [iy] * 0.01;	// 0.01 = mm^2->cm^2
			double effa = g_effxlenA [ix] * g_effylenA [iy] * 0.01;	// 0.01 = mm^2->cm^2
			double flx = Jokisch::integral_flux (g_pcut, fabs (cst));
			hist2d [0].cumulate (g_ddx[ix], g_ddy[iy], (dsdo * eff0));
			hist2d [1].cumulate (g_ddx[ix], g_ddy[iy], (dsdo * effs));
			hist2d [2].cumulate (g_ddx[ix], g_ddy[iy], (dsdo * effa));
			hist2d [3].cumulate (g_ddx[ix], g_ddy[iy], cst);
			hist2d [4].cumulate (g_ddx[ix], g_ddy[iy], flx);
		}
	}
	mytimer.stop ();

	double avdist = (g_xdist + g_ydist) * 0.5;
	double scalex = avdist / g_xdist;
	double scaley = avdist / g_ydist;

	// Show results
	for (int nh = 0; nh < numhist2d; nh++)
	{
		std::stringstream ss;
		ss << outname << "-" << datsetnam << "-" << nh << ".csv";
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
				<< ",Datasetname" << ',' << datsetnam
				<< ",\"channels(x,y)\""
				<< ',' << NUM_X_CHANNELS
				<< ',' << NUM_Y_CHANNELS
				<< ",\"width,length(mm)\""
				<< ',' << g_width << ',' << g_length
				<< std::endl;
			ofcsv
				<< "\"channel-size(x,y)(mm)\""
				<< ',' << g_dxsize << ',' << g_dysize
				<< ",\"dist(av,x,y)(mm)\""
				<< ',' << avdist << ',' << g_xdist << ',' << g_ydist
				<< ",\"diff(x,y)(mm)\""
				<< ',' << g_xdiff << ',' << g_ydiff 
				<< ",\"scale(x,y)\""
				<< ',' << scalex << ',' << scaley
				<< ",enable_gap,true"
				<< ",pcut0(GeV/c)" << ',' << g_pcut
				<< std::endl;
			hist2d[nh].CSVdump (ofcsv);
		}
		else
			std::cerr << "ERROR: output file (" << ofcsvnam << ") open error."
				<< std::endl;
	}

	// output geometrical parameters
	std::stringstream ss;
	ss << outname << "-" << datsetnam << "-log.csv";
	std::string oflognam;
	ss >> oflognam;
	std::ofstream oflogcsv (oflognam.c_str());
	if (oflogcsv)
	{
		oflogcsv << "\"";
		for (int i = 0; i < argc; i++)
			oflogcsv << argv[i] << ' ';
		oflogcsv << "\",";
		mytimer.csvout (oflogcsv);
		oflogcsv
			<< ",\"Version(major,minor)\""
			<< ',' << VERSION_MAJOR_NUMBER
			<< ',' << VERSION_MINOR_NUMBER
			<< ",Datasetname" << ',' << datsetnam
			<< ",\"channels(x,y)\""
			<< ',' << NUM_X_CHANNELS
			<< ',' << NUM_Y_CHANNELS
			<< ",\"width,length(mm)\""
			<< ',' << g_width << ',' << g_length
			<< std::endl;
		oflogcsv
			<< "\"channel-size(x,y)(mm)\""
			<< ',' << g_dxsize << ',' << g_dysize
			<< ",\"dist(av,x,y)(mm)\""
			<< ',' << avdist << ',' << g_xdist << ',' << g_ydist
			<< ",\"diff(x,y)(mm)\""
			<< ',' << g_xdiff << ',' << g_ydiff 
			<< ",\"scale(x,y)\""
			<< ',' << scalex << ',' << scaley
			<< ",enable_gap,true"
			<< ",pcut0(GeV/c)" << ',' << g_pcut
			<< std::endl;
		oflogcsv
			<< "Planes"
			<< ',' << NUM_PLANES
			<< std::endl;
		oflogcsv
			<< "Selected"
			<< ",Plane#"
			<< ",X(0)Y(1)"
			<< ",xpos,ypos,zpos"
			<< ",xmin,xmax,ymin,ymax"
			<< std::endl;
		for (int n = 0; n < NUM_PLANES; n++)
		{
			if (g_selected [n])
				oflogcsv << '+';
			else
				oflogcsv << ' ';
			oflogcsv << ',' << n
				<< ',' << g_plane [n]
				<< ',' << g_xpos [n] << ',' << g_ypos [n] << ',' << g_zpos [n]
				<< ',' << g_xmin [n] << ',' << g_xmax [n]
				<< ',' << g_ymin [n] << ',' << g_ymax [n]
				<< std::endl;
		}
		oflogcsv
			<< "X-pixels"
			<< ',' << NUM_X_PIXELS
			<< std::endl;
		for (int n = 0; n < NUM_X_PIXELS; n++)
		{
			oflogcsv << ',' << n;
		}
		oflogcsv << std::endl;
		oflogcsv << "ddx";
		for (int n = 0; n < NUM_X_PIXELS; n++)
		{
			oflogcsv << ',' << g_ddx [n];
		}
		oflogcsv << std::endl;
		oflogcsv << "dx";
		for (int n = 0; n < NUM_X_PIXELS; n++)
		{
			oflogcsv << ',' << g_dx [n];
		}
		oflogcsv << std::endl;
		oflogcsv << "effxlen0";
		for (int n = 0; n < NUM_X_PIXELS; n++)
		{
			oflogcsv << ',' << g_effxlen0 [n];
		}
		oflogcsv << std::endl;
		oflogcsv << "effxlenA";
		for (int n = 0; n < NUM_X_PIXELS; n++)
		{
			oflogcsv << ',' << g_effxlenA [n];
		}
		oflogcsv << std::endl;
		oflogcsv << "effxlenS";
		for (int n = 0; n < NUM_X_PIXELS; n++)
		{
			oflogcsv << ',' << g_effxlenS [n];
		}
		oflogcsv << std::endl;
		oflogcsv
			<< "Y-pixels"
			<< ',' << NUM_Y_PIXELS
			<< std::endl;
		for (int n = 0; n < NUM_Y_PIXELS; n++)
		{
			oflogcsv << ',' << n;
		}
		oflogcsv << std::endl;
		oflogcsv << "ddy";
		for (int n = 0; n < NUM_Y_PIXELS; n++)
		{
			oflogcsv << ',' << g_ddy [n];
		}
		oflogcsv << std::endl;
		oflogcsv << "dy";
		for (int n = 0; n < NUM_Y_PIXELS; n++)
		{
			oflogcsv << ',' << g_dy [n];
		}
		oflogcsv << std::endl;
		oflogcsv << "effylen0";
		for (int n = 0; n < NUM_Y_PIXELS; n++)
		{
			oflogcsv << ',' << g_effylen0 [n];
		}
		oflogcsv << std::endl;
		oflogcsv << "effylenA";
		for (int n = 0; n < NUM_Y_PIXELS; n++)
		{
			oflogcsv << ',' << g_effylenA [n];
		}
		oflogcsv << std::endl;
		oflogcsv << "effylenS";
		for (int n = 0; n < NUM_Y_PIXELS; n++)
		{
			oflogcsv << ',' << g_effylenS [n];
		}
		oflogcsv << std::endl;
	}
	else
		std::cerr << "ERROR: log output file (" << oflognam << ") open error."
			<< std::endl;

	return 0;
}
