// anaacc1f2.cpp
// g++ -Wall anaacc1f2.cpp hist2d.cpp ncpng.cpp jokisch.cpp -o anaacc1f2

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include "hist2d.h"
#include "jokisch.h"

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
double g_udist;

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

double g_effxlen [NUM_X_PIXELS];
double g_effylen [NUM_Y_PIXELS];

double g_pcut;

void setup (const std::string& datsetname)
{
	// 0   1   2   3   4   5   6   7
	// Y0E Y0O X0E X0O X1O X1E Y1O Y1E

	g_plane [0] = Y_PLANE;
	g_plane [1] = Y_PLANE;
	g_plane [2] = X_PLANE;
	g_plane [3] = X_PLANE;
	g_plane [4] = X_PLANE;
	g_plane [5] = X_PLANE;
	g_plane [6] = Y_PLANE;
	g_plane [7] = Y_PLANE;

	g_pcut = 1.0;	// in GeV/c

	g_width = 48.0;	// in cm
	g_length = 50.0;	// in cm
	g_udist = 49.5;	// in cm
	g_xygap = 2.1;	// in cm

	g_dxsize = g_width / double (NUM_X_CHANNELS);
	g_dysize = g_width / double (NUM_Y_CHANNELS);

	g_xdiff = 0.0;
	g_ydiff = 13.9;

	g_xpos [7] =  0.0;
	g_xpos [6] =  0.0;
	g_xpos [5] = -2.5;
	g_xpos [4] =  2.5;
	g_xpos [3] =  2.5 + g_xdiff;
	g_xpos [2] = -2.5 + g_xdiff;
	g_xpos [1] =  0.0 + g_xdiff;
	g_xpos [0] =  0.0 + g_xdiff;

	g_ypos [7] = -2.5;
	g_ypos [6] =  2.5;
	g_ypos [5] =  0.0;
	g_ypos [4] =  0.0;
	g_ypos [3] =  0.0 + g_ydiff;
	g_ypos [2] =  0.0 + g_ydiff;
	g_ypos [1] = -2.5 + g_ydiff;
	g_ypos [0] =  2.5 + g_ydiff;

	g_zpos [0] = 0.0;
	g_zpos [1] = g_zpos [0] + 1.0;
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

double EffXlen (int baseid, double rx)
{
	double xmax = g_xmax [baseid];
	double xmin = g_xmin [baseid];
	for (int n = 0; n < NUM_PLANES; n++)
	{
		// get the n-th center position on the [baseid] plane
		double dx = (rx * (g_zpos[n] - g_zpos[baseid]));
		if ((dx + g_xmax[n]) < xmax)
			xmax = dx + g_xmax[n];
		if ((dx + g_xmin[n]) > xmin)
			xmin = dx + g_xmin[n];
	}
	if (xmax < xmin)
		return 0.0;
	return (xmax - xmin);
}

double EffYlen (int baseid, double ry)
{
	double ymax = g_ymax [baseid];
	double ymin = g_ymin [baseid];
	for (int n = 0; n < NUM_PLANES; n++)
	{
		double dy = (ry * (g_zpos[n] - g_zpos[baseid]));
		if ((dy + g_ymax[n]) < ymax)
			ymax = dy + g_ymax[n];
		if ((dy + g_ymin[n]) > ymin)
			ymin = dy + g_ymin[n];
	}
	if (ymax < ymin)
		return 0.0;
	return (ymax - ymin);
}

int main (int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "ERROR: dataset name is missing.\n"
			<< "Usage: " << argv[0] << " dataset_name\n"
			<< "     where dataset_name is one of\n"
			<< "       EOEO EOOE EOEE EOOO OEEO OEOE OEOE OEEE OEOO\n"
			<< "       EEEO EEOE EEEE EEOO OOEO OOOE OOOE OOEE OOOO\n"
			<< std::endl;
		return (-1);
	}

	std::string datsetnam (argv[1]);

	// 0   1   2   3   4   5   6   7
	// Y0E Y0O X0E X0O X1O X1E Y1O Y1E
	int idx0 = 2;
	int idx1 = 4;
	int idy0 = 0;
	int idy1 = 6;

	if (datsetnam[0] == 'E')
		idx0 = 2;
	else if (datsetnam[0] == 'O')
		idx0 = 3;
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
		idy0 = 0;
	else if (datsetnam[2] == 'O')
		idy0 = 1;
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

	std::cout << "Dataset " << datsetnam
		<< " idy0=" << idy0 << " idx0=" << idx0
		<< " idx1=" << idx1 << " idy1=" << idy1
		<< std::endl;

	setup (datsetnam);
	calcgeom ();

	g_xdist = g_zpos [idx1] - g_zpos [idx0];
	g_ydist = g_zpos [idy1] - g_zpos [idy0];

	// Histograms
	static const int numhist2d = 3;
	mylibrary::Hist2D hist2d [numhist2d] =
	{
		mylibrary::Hist2D ("dsdo sr*cm^2",
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

	for (int ix = 0; ix < NUM_X_PIXELS; ix++)
	{
		double dx = g_dx[ix];
		double rx = dx / g_xdist;
		g_effxlen [ix] = EffXlen (idx1, rx);
	}

	for (int iy = 0; iy < NUM_Y_PIXELS; iy++)
	{
		double dy = g_dy[iy];
		double ry = dy / g_ydist;
		g_effylen [iy] = EffYlen (idy1, ry);
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
			double cs2f = 1.0 / (1.0 + (rx * rx));
//			double csf = sqrt (cs2f);
			double sn2t = 1.0 / (1.0 + (ry * ry * cs2f));
			double cst = sqrt (1.0 - sn2t);
			if (ry < 0.0)
				cst = -cst;
			double dsdo = cs2f * sn2t * cs2f * sn2t * dxdy * g_effxlen [ix] * g_effylen [iy];
			double flx = Jokisch::integral_flux (g_pcut, fabs (cst));
			hist2d [0].cumulate (g_ddx[ix], g_ddy[iy], dsdo);
			hist2d [1].cumulate (g_ddx[ix], g_ddy[iy], cst);
			hist2d [2].cumulate (g_ddx[ix], g_ddy[iy], flx);
		}
	}

	for (int nh = 0; nh < numhist2d; nh++)
	{
		std::stringstream ss;
		ss << "anaacc-" << datsetnam << "-" << nh << ".csv";
		std::string ofcsvnam;
		ss >> ofcsvnam;
		std::ofstream ofcsv (ofcsvnam.c_str());
		if (ofcsv)
		{
			ofcsv << "\"";
			for (int i = 0; i < argc; i++)
				ofcsv << argv[i] << ' ';
			ofcsv << "\"" << std::endl;
			ofcsv << "Datasetname," << datsetnam
				<< ",width," << g_width
				<< ",length," << g_length
				<< ",xchannels," << NUM_X_CHANNELS
				<< ",ychannels," << NUM_Y_CHANNELS
				<< ",xdiff," << g_xdiff
				<< ",ydiff," << g_ydiff
				<< ",xdist," << g_xdist
				<< ",ydist," << g_ydist
				<< ",pcut(GeV/c)," << g_pcut
				<< std::endl;
			hist2d[nh].CSVdump (ofcsv);
		}
		else
			std::cerr << "ERROR: output file (" << ofcsvnam << ") open error."
				<< std::endl;
	}

	return 0;
}
