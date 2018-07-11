// gzipfilebuf.cpp

#include "gzipfilebuf.h"
#include <cstring>

namespace mylibrary
{

GzipFileBuf::GzipFileBuf ()
{
	m_gzfile = NULL;
	m_openmode = std::ios_base::openmode(0);
	m_gbuf = 0;
	m_pbuf = 0;
}

GzipFileBuf::~GzipFileBuf ()
{
	close ();
}

GzipFileBuf*
GzipFileBuf::open (const char* filename, std::ios_base::openmode mode)
{
	char modechar[8];
	int n = 0;
	if (mode & std::ios_base::in)
		modechar[n++] = 'r';
	if (mode & std::ios_base::out)
		modechar[n++] = 'w';
	if (mode & std::ios_base::binary)
		modechar[n++] = 'b';
	modechar[n] = char(0);

	m_gzfile = gzopen (filename, modechar);
	if (m_gzfile == NULL)
		return NULL;

	if (mode & std::ios_base::in)
		m_gbuf = m_rbuf + GZIPFBACKUPSIZE;
	if (mode & std::ios_base::out)
		m_pbuf = m_rbuf + GZIPFBACKUPSIZE + GZIPFILEBUFSIZE;
	setg (m_gbuf, m_gbuf, m_gbuf);
	setp (m_pbuf, m_pbuf);
	return this;
}

GzipFileBuf*
GzipFileBuf::close ()
{
	if (m_gzfile == NULL)
		return NULL;
	if (m_openmode & std::ios_base::out)
	{
		sync ();
		gzflush (m_gzfile, Z_FINISH);
	}
	gzclose (m_gzfile);
	m_gzfile = NULL;
	return this;
}

bool
GzipFileBuf::is_open () const
{
	return (m_gzfile != NULL);
}

int
GzipFileBuf::underflow ()
{
	if(m_gzfile == NULL)
		return std::char_traits<char>::eof();
	int c;
	if((gptr() >= m_gbuf) && (gptr() < egptr()))
	{
		// Why call me ?
		// Still we have enough data in the buffer.
		c = (int)*gptr(); c &= 255;
		return c;
	}
	int nback = (int)(egptr() - eback());
	if(nback > GZIPFBACKUPSIZE)
		nback = GZIPFBACKUPSIZE;
	if(nback > 0)
		::memmove((char*)(m_gbuf - nback), (egptr() - nback), nback);
	int n;
	if((n = gzread (m_gzfile, m_gbuf, GZIPFILEBUFSIZE)) <= 0)
		return std::char_traits<char>::eof();
	setg((m_gbuf - nback), m_gbuf, (m_gbuf + n));
	c = (int)m_gbuf[0]; c &= 255;
	return c;
}

int
GzipFileBuf::overflow (int c)
{
	if (m_gzfile == NULL)
		return std::char_traits<char>::eof();

	std::streamsize n = pptr() - pbase();
	if (n > 0)
	{
		if ((std::streamsize)gzwrite(m_gzfile, pbase(), n) != n)
			return std::char_traits<char>::eof();
	}
	setp (m_pbuf, m_pbuf + GZIPFILEBUFSIZE);
	if (c == std::char_traits<char>::eof())
		return 0;
	*m_pbuf = c;
	pbump(1);
	return c;
}

int
GzipFileBuf::sync ()
{
	// Returns zero, which indicates success.
	// A value of -1 would indicate failure.

	if (m_gzfile == NULL)
		return (-1);

	if (overflow () == std::char_traits<char>::eof())
		return (-1);
	return 0;
}

}	// namespace mylibrary
