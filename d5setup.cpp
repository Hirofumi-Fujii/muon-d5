//
// Setting parameters
//
	double pxdist = opt.m_dzshift;	// x-plane distance
	double pydist = pxdist;	// y-plane distance
	double uxdiff = 0.0;	// Unit x-position difference
	double uydiff = 0.0;	// Unit y-position difference
	double uzdiff = (pxdist + pydist) *  0.5;	// Unit z-position difference

	double xlayer_size = 480.0;
	double xlayer_halfsize = xlayer_size * 0.5;
	double ylayer_size = 480.0;
	double ylayer_halfsize = ylayer_size * 0.5;

	double xplane_size = xlayer_size + 5.0;
	double xplane_halfsize = xplane_size * 0.5;
	double yplane_size = ylayer_size + 5.0;
	double yplane_halfsize = yplane_size * 0.5;

	double projdist = opt.m_dist;	// distance to the projection plane

// Front unit
// Order XO-XE-YO-YE from the reactor side
// In progam, 0 is even, 1 is odd
// The origin of z is rear-unit center
//
//	xlayer[0] = X front even (A-group)
	xlayer_zpos [0] = -pxdist - 10.5 + 5.0;
	xlayer_xpos [0] = -xlayer_halfsize - 2.5 + uxdiff + 5.0;	// + 5.0 is channel 0 center from edge

//	xlayer[1] = X front odd (B-group)
	xlayer_zpos [1] = xlayer_zpos [0] - 10.0;
	xlayer_xpos [1] = xlayer_xpos [0] + 5.0;

//	ylayer[0] = Y front even (A-group)
	ylayer_zpos [0] = -pydist + 10.5 + 5.0;
	ylayer_ypos [0] = -ylayer_halfsize - 2.5 + uydiff + 5.0;	// + 5.0 is channel 0 center from edge

//	ylayer[1] = Y front odd (B-group)
	ylayer_zpos [1] = ylayer_zpos [0] - 10.0;
	ylayer_ypos [1] = ylayer_ypos [0] + 5.0;

// Rear unit
// Order X-Y from the reactor side
// In progam, 0 is even, 1 is odd
// The origin of z is rear-unit center
//
//	xlayer[2] = X rear even (A-group)
	xlayer_zpos [2] = -10.5 + 5.0;
	xlayer_xpos [2] = -xlayer_halfsize - 2.5 + 5.0;		// + 5.0 is channel 0 center from edge

//	xlayer[3] = X rear odd (B-group)
	xlayer_zpos [3] = xlayer_zpos [2] - 10.0;
	xlayer_xpos [3] = xlayer_xpos [2] + 5.0;

//	ylayer[2] = Y rear even (A-group)
	ylayer_zpos [2] = +10.5 + 5.0;
	ylayer_ypos [2] = -ylayer_halfsize - 2.5 + 5.0;		// + 5.0 is channel 0 center from edge

//	ylayer[3] = Y rear odd (B-group)
	ylayer_zpos [3] = ylayer_zpos [2] - 10.0;
	ylayer_ypos [3] = ylayer_ypos [2] + 5.0;
