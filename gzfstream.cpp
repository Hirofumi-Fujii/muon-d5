// gzfstream.cpp

#include "gzfstream.h"

namespace mylibrary
{
igzfstream::igzfstream ()
{
	this->init (&m_buf);
}

igzfstream::igzfstream (const char* filename, std::ios_base::openmode mode)
{
	this->init (&m_buf);
	open (filename, mode);
}

igzfstream::~igzfstream ()
{
	close ();
}

void
igzfstream::open (const char* filename, std::ios_base::openmode mode)
{
	if (m_buf.open (filename, (mode | std::ios_base::in)) == NULL)
		this->setstate (std::ios::failbit);
	else
		this->clear ();
}	

bool
igzfstream::is_open ()
{
	return m_buf.is_open ();
}

void
igzfstream::close ()
{
	if (m_buf.close() == NULL)
		this->setstate (std::ios::failbit);
	else
		this->clear ();
}

///// ostream
ogzfstream::ogzfstream ()
{
	this->init (&m_buf);
}

ogzfstream::ogzfstream (const char* filename, std::ios_base::openmode mode)
{
	this->init (&m_buf);
	open (filename, mode);
}

ogzfstream::~ogzfstream ()
{
	close ();
}

void
ogzfstream::open (const char* filename, std::ios_base::openmode mode)
{
	if (m_buf.open (filename, (mode | std::ios_base::out)) == NULL)
		this->setstate (std::ios::failbit);
	else
		this->clear ();
}	

bool
ogzfstream::is_open ()
{
	return m_buf.is_open ();
}

void
ogzfstream::close ()
{
	if (m_buf.close() == NULL)
		this->setstate (std::ios::failbit);
	else
		this->clear ();
}	
	
}	// namespace mylibrary
