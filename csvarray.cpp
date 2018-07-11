// csvarray.cpp

#include "csvarray.h"
#include <sstream>

namespace mylibrary
{

const char CSVArray::CSV_LF = 0x0a;
const char CSVArray::CSV_CR = 0x0a;
const char CSVArray::CSV_DQUOTE = 0x22;
const char CSVArray::CSV_COMMA = 0x2c;
const std::string CSVArray::m_nullstr = std::string("");

CSVArray::CSVArray ()
{
}

CSVArray::~CSVArray ()
{
}

const std::string&
CSVArray::CellString (int nline, int ncol) const
{
	if ((nline < 0) || (nline >= (int)(m_array.size())) )
		return m_nullstr;
	if ((ncol < 0) || (ncol >= (int)(m_array[nline].size())) )
		return m_nullstr;
	return m_array[nline][ncol];
}

double
CSVArray::CellDoubleValue (int nline, int ncol) const
{
	if ((nline < 0) || (nline >= (int)(m_array.size())) )
		return 0.0;
	if ((ncol < 0) || (ncol >= (int)(m_array[nline].size())) )
		return 0.0;
	std::stringstream ss (m_array[nline][ncol]);
	double dv = 0.0;
	if (ss >> dv)
		return dv;
	return 0.0;
}

long
CSVArray::CellLongValue (int nline, int ncol) const
{
	if ((nline < 0) || (nline >= (int)(m_array.size())) )
		return (long)(0);
	if ((ncol < 0) || (ncol >= (int)(m_array[nline].size())) )
		return (long)(0);
	std::stringstream ss (m_array[nline][ncol]);
	double lv = (long)(0);
	if (ss >> lv)
		return lv;
	return (long)(0);
}

std::istream& 
CSVArray::Read (std::istream& is)
{
	std::string cellstr;
	std::vector<std::string> csvline;
	bool newcell = true;
	bool newline = true;
	char c;
	while (is.get ( c ))
	{
		if (newcell && (c == CSV_DQUOTE))
		{
			char cprev = 0;
			while (is.get (c))
			{
				if (c == CSV_DQUOTE)
				{
					if (cprev == CSV_DQUOTE)
					{
						cellstr.push_back (cprev);
						c = 0;
					}
				}
				else if (cprev == CSV_DQUOTE)
					break;
				else
					cellstr.push_back (c);
				cprev = c;					
			}
			newcell = false;
			newline = false;
		}

		if (c == CSV_COMMA)
		{
			csvline.push_back (cellstr);
			cellstr.clear ();
			newcell = true;
			newline = false;
		}
		else if (c == CSV_LF)
		{
			csvline.push_back (cellstr);
			m_array.push_back (csvline);
			cellstr.clear ();
			csvline.clear ();
			newcell = true;
			newline = true;
		}
		else
		{
			cellstr.push_back (c);
			newcell = false;
			newline = false;
		}
	}
	if (!newcell)
		csvline.push_back (cellstr);
	if (!newline)
		m_array.push_back (csvline);
	return is;
}

}	// namespace mylibrary
