// gencoinlistKEK.cpp
// g++ -Wall gencoinlistKEK.cpp -o gencoinlistKEK

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

class CheckOpt
{
public:
	CheckOpt ();
	void usage (std::ostream& os, const char* pname);
	bool setup (int argc, char* argv []);

public:
	bool m_infile_given;
	std::string m_infilename;
	bool m_outfile_given;
	std::string m_outfilename;
};

CheckOpt::CheckOpt ()
{
	m_infile_given = false;
	m_infilename = "";
	m_outfile_given = false;
	m_outfilename = "";
}

void
CheckOpt::usage (std::ostream& os, const char* pname)
{
	os
		<< "Usage: "
		<< pname
		<< " [option(s)] runno_filename\n"
		<< "Option(s):\n"
		<< "  -o[ut] filename \t specifies output filename\n"
		<< std::endl;
}

bool
CheckOpt::setup (int argc, char* argv[])
{
	int npos = 1;
	while (npos < argc)
	{
		std::string optword (argv[npos]);
		if ((optword == "-o") || (optword == "-out"))
		{
			++npos;
			if (npos < argc)
			{
				m_outfilename = std::string (argv[npos]);
				m_outfile_given = true;
			}
			else
			{
				std::cerr << "ERROR: missing output filename." << std::endl;
				return false;
			}
		}
		else
		{
			if (m_infile_given)
			{
				std::cerr << "WARNING: input filename already given, replaced." << std::endl;
			}
			m_infilename = optword;
			m_infile_given = true;
		}
		++npos;
	}

//	if (!m_infile_given)
//		return false;
	return true;
}

int main (int argc, char* argv[])
{
	CheckOpt opt;
	if ( !opt.setup (argc, argv) )
	{
		opt.usage (std::cerr, argv[0] );
		return (-1);
	}

	std::string infilename;
	if (opt.m_infile_given)
		infilename = opt.m_infilename;
	else
		infilename = "gencoinlistKEK.xml";
	std::ifstream ifs ( infilename.c_str() );
	if (!ifs)
	{
		std::cerr
			<< "ERROR: input file ("
			<< infilename
			<< ") cannot be opened."
			<< std::endl;
		return (-2);
	}

	std::string outfilename;
	if (opt.m_outfile_given)
		outfilename = opt.m_outfilename;
	else
		outfilename = "coinlist.txt";
	std::ofstream ofs ( outfilename.c_str() );
	if (!ofs)
	{
		std::cerr
			<< "ERROR: output file ("
			<< outfilename
			<< ") cannot be opened."
			<< std::endl;
		return (-2);
	}

	ofs
		<< "%";
	for (int i = 0; i < argc; i++)
		ofs << " " << argv[i];
	ofs << std::endl;

	std::string datafolder ("/data1/Detector5/data/");
	std::string foldersep ("/");
	std::string coinprefix ("d5-r");
	std::string coinpostfix ("_m24c32.dat.gz");

	std::string sline;
	while (std::getline (ifs, sline))
	{
		if ((sline.size () >= 10) &&
			(sline.substr(0,9) == "<RunRange"))
		{
			std::string::size_type n = 9;
			while (n < sline.size ())
			{
				if (sline[n] == '>')
					break;
				n++;
			}
			break;
		}
		sline.clear ();
	}

	while (std::getline (ifs, sline))
	{
		if ((sline.size () >= 4) && (sline.substr (0, 4) == "<!--"))
			continue;
		else if (sline.size () >= 3)
		{
			std::stringstream ss (sline);
			
		        int srun;
		        int erun;
			while ((ss >> srun >> erun))
			{
			        for (int runno = srun; runno <= erun; runno++)
				{
					ofs
						<< datafolder
						<< runno
						<< foldersep
						<< coinprefix
						<< runno
						<< coinpostfix
						<< std::endl;
				}
			}
		}
		sline.clear ();
	}
        return 0;
}
