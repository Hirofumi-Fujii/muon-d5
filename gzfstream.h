// gzfstream.h
//

#ifndef GZFSTREAM_H_INCLUDED
#define GZFSTREAM_H_INCLUDED

#include <iostream>
#include "gzipfilebuf.h"

namespace mylibrary
{

class igzfstream : public std::istream
{
public:
	igzfstream ();
	explicit igzfstream (const char* filename, std::ios_base::openmode mode = std::ios_base::in);
	virtual ~igzfstream ();
	void open (const char* filename, std::ios_base::openmode mode = std::ios_base::in);
	bool is_open ();
	void close ();

protected:
	GzipFileBuf m_buf;
};

class ogzfstream : public std::ostream
{
public:
	ogzfstream ();
	explicit ogzfstream (const char* filename, std::ios_base::openmode mode = std::ios_base::out);
	virtual ~ogzfstream ();
	void open (const char* filename, std::ios_base::openmode mode = std::ios_base::out);
	bool is_open ();
	void close ();

protected:
	GzipFileBuf m_buf;
};

}	// namespace mylibrary
#endif	// GZFSTREAM_H_INCLUDED
