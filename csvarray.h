// csvarray.h

// RFC 4180
// The ABNF grammar:
//
//   file = [header CRLF] record *(CRLF record) [CRLF]
//   header = name *(COMMA name)
//   record = field *(COMMA field)
//   name = field
//   field = (escaped / non-escaped)
//   escaped = DQUOTE *(TEXTDATA / COMMA / CR / LF / 2DQUOTE) DQUOTE
//   non-escaped = *TEXTDATA
//   COMMA = %x2C
//   CR = %x0D ;as per section 6.1 of RFC 2234 [2]
//   DQUOTE =  %x22 ;as per section 6.1 of RFC 2234 [2]
//   LF = %x0A ;as per section 6.1 of RFC 2234 [2]
//   CRLF = CR LF ;as per section 6.1 of RFC 2234 [2]
//   TEXTDATA =  %x20-21 / %x23-2B / %x2D-7E
//

#ifndef CSVARRAY_H_INCLUDED
#define CSVARRAY_H_INCLUDED

#include <iostream>
#include <string>
#include <vector>

namespace mylibrary
{

class CSVArray
{
public:
	static const char CSV_LF;
	static const char CSV_CR;
	static const char CSV_DQUOTE;
	static const char CSV_COMMA;

public:
	CSVArray ();
	virtual ~CSVArray ();
	std::istream& Read (std::istream& is);
	const std::string& CellString (int nline, int ncol) const;
	double CellDoubleValue (int nline, int ncol) const;
	long CellLongValue (int nline, int ncol) const;

public:
	std::vector<std::vector<std::string> > m_array;
public:
	static const std::string m_nullstr;
};

}	// namespace mylibrary

#endif	// CSVARRAY_H_INCLUDED
