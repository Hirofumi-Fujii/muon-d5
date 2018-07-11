// gencoinlist.cpp
// g++ -Wall gencoinlist.cpp -o gencoinlist.exe


#include <iostream>
#include <sstream>

int main (int argc, char* argv[])
{
        if (argc != 3)
        {
                std::cerr << "Usage: " << argv[0]
                        << " start_runno end_runno"
                        << std::endl;
                return (-1);
        }
        std::stringstream ss;
        ss << argv[1] << ' ' << argv[2];
        int srun;
        int erun;
        ss >> srun;
        ss >> erun;
        for (int n = srun; n <= erun; n++)
        {
                std::cout << "/data1/1F3/detector4"
                        << "/data/" << n
                        << "/d4-r" << n
                        << "_m24c32.dat.gz"
			<< std::endl;
        }
        return 0;
}
