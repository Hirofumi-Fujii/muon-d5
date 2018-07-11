// accxtest.cpp

#include <iostream>

int main()
{
	double thickness = 10.0;
	for (int i = 0; i < 193; i++)
	{
		double dx = (double)(i) * 5.0 - 480.0;
		double xdist = 495.0;
		// get the upper x position
		double x1 = thickness / xdist * dx;
		std::cout << i << ',' << dx << ',' << x1 << std::endl;
	}
	return 0;
}
