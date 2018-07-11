// gzipfilebuf.h

#ifndef GZIPFILEBUF_H_INCLUDED
#define GZIPFILEBUF_H_INCLUDED

#include <streambuf>
#include <zlib.h>

namespace mylibrary
{
class GzipFileBuf : public std::streambuf
{
public:
	GzipFileBuf ();
	virtual ~GzipFileBuf ();
	GzipFileBuf* open (const char* filename, std::ios_base::openmode mode);
	bool is_open () const;
	GzipFileBuf* close ();

protected:
	virtual int underflow ();
	virtual int overflow (int c = std::char_traits<char>::eof());
	virtual int sync ();

protected:
	static const int GZIPFILEBUFSIZE = 8192;
	static const int GZIPFBACKUPSIZE = 128;
	gzFile m_gzfile;
	std::ios_base::openmode m_openmode;
	char* m_gbuf;	// get buffer pointer
	char* m_pbuf;	// put buffer pointer
	char m_rbuf[(GZIPFBACKUPSIZE + GZIPFILEBUFSIZE + GZIPFILEBUFSIZE + 1)];
};

}	// namespace mylibrary
#endif	// GZIPFILEBUF_H_INCLUDED
