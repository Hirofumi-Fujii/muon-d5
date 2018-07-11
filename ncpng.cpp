// ncpng.cpp

#include "ncpng.h"

namespace mypnglib
{

const unsigned char
NCPNG::m_signature[NCPNG::SIGNATURE_SIZE]
	= { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};

int
NCPNG::crc_table_computed = 0;

unsigned long
NCPNG::crc_table[256] = { 0 };

unsigned char
NCPNG::m_biton[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

//
NCPNG::GREY_PIXEL::GREY_PIXEL (double intensity) :
	m_intensity (intensity), m_alpha (1.0)
{
}

NCPNG::GREY_PIXEL::GREY_PIXEL (double intensity, double alpha) :
	m_intensity (intensity), m_alpha (alpha)
{
}

NCPNG::COLOUR_PIXEL::COLOUR_PIXEL (double red, double green, double blue) :
	m_red (red), m_green (green), m_blue (blue), m_alpha (1.0)
{
}

NCPNG::COLOUR_PIXEL::COLOUR_PIXEL (double red, double green, double blue, double alpha) :
	m_red (red), m_green (green), m_blue (blue), m_alpha (alpha)
{
}

//
// Make the table for a fast CRC.
void
NCPNG::make_crc_table(void)
{
	unsigned long c;
	int n, k;

	for (n = 0; n < 256; n++)
	{
		c = (unsigned long) n;
		for (k = 0; k < 8; k++)
		{
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[n] = c;
	}
	crc_table_computed = 1;
}

// Update a running CRC with the bytes buf[0..len-1]--the CRC
// should be initialized to all 1's, and the transmitted value
// is the 1's complement of the final running CRC.
   
unsigned long
NCPNG::update_crc
	(unsigned long crc, const unsigned char *buf, unsigned long len)
{
	unsigned long c = crc;
	unsigned long n;

	if (!crc_table_computed)
		make_crc_table();
	for (n = 0; n < len; n++)
	{
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c;
}

unsigned char*
NCPNG::fill_ulong (unsigned char* buf, unsigned long u)
{
	*buf++ = (unsigned char)((u >> 24) & 255);
	*buf++ = (unsigned char)((u >> 16) & 255);
	*buf++ = (unsigned char)((u >>  8) & 255);
	*buf++ = (unsigned char)( u        & 255);
	return buf;
}

void
NCPNG::write_chunk
	(std::ostream& os, const char* name, const unsigned char* buf, unsigned long len)
{
	unsigned char tmpbuf[8];	// len & name
	if (buf == 0)
		len = 0;
	unsigned char* pname = fill_ulong(tmpbuf, len);
	unsigned char* p = pname;

	*p++ = (unsigned char)name[0];
	*p++ = (unsigned char)name[1];
	*p++ = (unsigned char)name[2];
	*p++ = (unsigned char)name[3];

	unsigned long crc = 0xffffffffL;
	crc = update_crc (crc, pname, (p - pname));
	os.write ((const char*)tmpbuf, (p - tmpbuf));
	if (len > 0)
	{
		crc = update_crc (crc, buf, len);
		os.write ((const char*)buf, len);
	}
	crc = crc ^ 0xffffffffL;
	p = fill_ulong (tmpbuf, crc);
	os.write ((const char*)tmpbuf, (p - tmpbuf));
}

NCPNG::NCPNG (unsigned long imgw, unsigned long imgh,
	COLOURTYPE coltype, unsigned int bitdepth ) :
	m_width (imgw), m_height (imgh),
	m_colour_type (coltype), m_bit_depth (bitdepth),
	m_compression_method (0), m_filter_method (0), m_interlace_method (0),
	m_imgbuf (0), m_bytes_per_scanline (0), m_num_palette(0)
{
	unsigned long numelems = 1;
	switch (m_colour_type)
	{
	case TRUECOLOUR:
		numelems = 3;
		break;
	case GREYSCALE_WITH_ALPHA:
		numelems = 2;
		break;
	case TRUECOLOUR_WITH_ALPHA:
		numelems = 4;
		break;
	default:
		numelems = 1;
		break;
	}
	unsigned long ubytes = (numelems * m_bit_depth * m_width + 7) / 8;	// bytes_per_scanline
	unsigned long ualloc = (ubytes + 1) * m_height;	// total size (+1 for filter_type)
	if (ualloc > 0)
	{
		m_imgbuf = new unsigned char [ualloc];
		unsigned long u = 0;
		for (unsigned long h = 0; h < m_height; h++)
		{
			m_imgbuf [u++] = 0;	// filter_type = 0
			for (unsigned long w = 0; w < ubytes; w++)
				m_imgbuf [u++] = 0;
		}
		m_bytes_per_scanline = ubytes;
	}
	if (m_colour_type == INDEXED_COLOUR)
	{
		if (m_bit_depth == 1)
			m_num_palette = 2;
		else if (m_bit_depth == 2)
			m_num_palette = 4;
		else if (m_bit_depth == 4)
			m_num_palette = 16;
		else if (m_bit_depth == 8)
			m_num_palette = 256;
	}
}

NCPNG::~NCPNG ()
{
	delete [] m_imgbuf;
}

std::ostream&
NCPNG::IDATWriter::put (std::ostream& os, unsigned char c)
{
	if (m_filled >= IDATBUF_SIZE)
	{
		write_chunk (os, "IDAT", m_buf, m_filled);
		m_filled = 0;
	}
	m_buf[m_filled++] = c;
	return os;
}

std::ostream&
NCPNG::IDATWriter::flush (std::ostream& os)
{
	if (m_filled > 0)
	{
		write_chunk (os, "IDAT", m_buf, m_filled);
		os.flush ();
	}
	m_filled = 0;
	return os;
}

void
NCPNG::write_IHDR (std::ostream& os) const
{
	unsigned char hbuf[16];	// 13 is enough
	unsigned char* p = hbuf;
	p = fill_ulong (p, m_width);
	p = fill_ulong (p, m_height);
	*p++ = (unsigned char)(m_bit_depth & 255);
	*p++ = (unsigned char)(m_colour_type & 255);
	*p++ = (unsigned char)(m_compression_method & 255);
	*p++ = (unsigned char)(m_filter_method & 255);
	*p++ = (unsigned char)(m_interlace_method & 255);
	write_chunk (os, "IHDR", hbuf, (p - hbuf));
}

void
NCPNG::write_PLTE (std::ostream& os) const
{
	if (m_num_palette > 0)
		write_chunk (os, "PLTE", m_palette, (m_num_palette * 3));
}

void
NCPNG::write_IDAT (std::ostream& os) const
{
	IDATWriter idatwriter;

	idatwriter.put (os, 0x78);
	idatwriter.put (os, 0x9c);	//  Level=2 (default) + FCHECK

	unsigned long adler32_s1 = 1;
	unsigned long adler32_s2 = 0;

	unsigned long imgsize = (m_bytes_per_scanline + 1) * m_height;
	unsigned long left = imgsize;
	unsigned char* p = m_imgbuf;
	while (left)
	{
		unsigned long len = left;
		if (left < 32000)
			idatwriter.put (os, 0x01);	// last block
		else
		{
			idatwriter.put (os, 0x00);
			len = 32000;
		}
		// LEN and NLEN are both 2-byte integer.
		// Store them in LITTLE ENDIAN.
		idatwriter.put (os, ( len       & 255));
		idatwriter.put (os, ((len >> 8) & 255));
		unsigned long nlen = ~len;
		idatwriter.put (os, ( nlen       & 255));
		idatwriter.put (os, ((nlen >> 8) & 255));
		for (unsigned long n = 0; n < len; n++)
		{
			unsigned char c = *p++;
			idatwriter.put (os, c);
			adler32_s1 = (adler32_s1 + c) % ADLER32_BASE;
			adler32_s2 = (adler32_s2 + adler32_s1) % ADLER32_BASE;
		}
		left -= len;
	}

	// Adler32 check sums should be stored in BIG ENDIAN.
	idatwriter.put (os, ((adler32_s2 >> 8) & 255));
	idatwriter.put (os, ( adler32_s2       & 255));
	idatwriter.put (os, ((adler32_s1 >> 8) & 255));
	idatwriter.put (os, ( adler32_s1       & 255));

	idatwriter.flush (os);
}

void
NCPNG::write_IEND (std::ostream& os) const
{
	write_chunk (os, "IEND", 0, 0);
}

std::ostream&
NCPNG::write (std::ostream& os)
{
	// write signature
	os.write ((const char*)m_signature, SIGNATURE_SIZE);
	// write IHDR chunk
	write_IHDR (os);
	// write PLTE
	write_PLTE (os);
	// write IDAT
	write_IDAT (os);
	// write IEND
	write_IEND (os);
	return os;
}

void
NCPNG::putpixel (unsigned long x, unsigned long y,
	unsigned int nelem, double dat[])
{
	unsigned long upos = (m_bytes_per_scanline + 1) * y + 1;
	if (m_bit_depth == 1)
	{
		unsigned int ubyte = x / 8;
		unsigned int ubit = x % 8;
		upos = upos + ubyte;
		if (dat[0] >= 0.5)
			m_imgbuf[upos] = m_imgbuf[upos] | m_biton[ubit];
		else
			m_imgbuf[upos] = m_imgbuf[upos] & (~m_biton[ubit]);
	}
	else if (m_bit_depth == 8)
	{
		upos = upos + (x * nelem);
		for (unsigned int i = 0; i < nelem; i++)
		{
			unsigned int u = (unsigned int)(dat[i] * 256.0);
			if (u > 255)
				u = 255;
			m_imgbuf[upos++] = u;
		}
	}
	else if (m_bit_depth == 16)
	{
		upos = upos + (x * nelem * 2);
		for (unsigned int i = 0; i < nelem; i++)
		{
			unsigned int u = (unsigned int)(dat[i] * 65536.0);
			if (u > 65535)
				u = 65535;
			m_imgbuf[upos++] = ((u >> 8) & 255);
			m_imgbuf[upos++] = ( u       & 255);
		}
	}
}

void
NCPNG::putpixel (unsigned long x, unsigned long y,
	unsigned int palindex)
{
	unsigned long upos = (m_bytes_per_scanline + 1) * y + 1;
	if (m_bit_depth == 1)
	{
		unsigned int ubyte = x / 8;
		unsigned int ubit = x % 8;
		upos = upos + ubyte;
		if (palindex > 0)
			m_imgbuf[upos] = m_imgbuf[upos] | m_biton[ubit];
		else
			m_imgbuf[upos] = m_imgbuf[upos] & (~m_biton[ubit]);
	}
	else if (m_bit_depth == 8)
	{
		upos = upos + x;
		unsigned int u = palindex;
		if (u > 255)
			u = 255;
		m_imgbuf[upos] = u;
	}
}

void
NCPNG::put (unsigned long x, unsigned long y, const COLOUR_PIXEL& pixel)
{
	if ((x >= m_width) || (y >= m_height))
		return;

	double dat[4];
	unsigned int nelem = 3;

//	double dmax = 256.0;
//	if (m_bit_depth == 16)
//		dmax = 65536.0;
	if ((m_bit_depth > 1) && (m_bit_depth < 8))
		return;	// not supported yet.

	switch (m_colour_type)
	{
	case GREYSCALE:
		nelem = 1;
		dat[0] = rgbtogrey (pixel.m_red, pixel.m_green, pixel.m_blue);
		break;
	case TRUECOLOUR:
		nelem = 3;
		dat[0] = pixel.m_red;
		dat[1] = pixel.m_green;
		dat[2] = pixel.m_blue;
		break;
	case GREYSCALE_WITH_ALPHA:
		nelem = 2;
		dat[0] = rgbtogrey (pixel.m_red, pixel.m_green, pixel.m_blue);
		dat[1] = pixel.m_alpha;
		break;
	case TRUECOLOUR_WITH_ALPHA:
		nelem = 4;
		dat[0] = pixel.m_red;
		dat[1] = pixel.m_green;
		dat[2] = pixel.m_blue;
		dat[3] = pixel.m_alpha;
		break;
	default:
		return;
	}
	putpixel (x, y, nelem, dat);
}

void
NCPNG::put (unsigned long x, unsigned long y, const GREY_PIXEL& pixel)
{
	if ((x >= m_width) || (y >= m_height))
		return;

	double dat[4];
	unsigned int nelem = 3;

	if ((m_bit_depth > 1) && (m_bit_depth < 8))
		return;	// not supported yet.

	switch (m_colour_type)
	{
	case GREYSCALE:
		nelem = 1;
		dat[0] = pixel.m_intensity;
		break;
	case TRUECOLOUR:
		nelem = 3;
		dat[0] = pixel.m_intensity;
		dat[1] = pixel.m_intensity;
		dat[2] = pixel.m_intensity;
		break;
	case GREYSCALE_WITH_ALPHA:
		nelem = 2;
		dat[0] = pixel.m_intensity;
		dat[1] = pixel.m_alpha;
		break;
	case TRUECOLOUR_WITH_ALPHA:
		nelem = 4;
		dat[0] = pixel.m_intensity;
		dat[1] = pixel.m_intensity;
		dat[2] = pixel.m_intensity;
		dat[3] = pixel.m_alpha;
		break;
	default:
		return;
	}
	putpixel (x, y, nelem, dat);
}

void
NCPNG::put (unsigned long x, unsigned long y, unsigned int palindex)
{
	if ((x >= m_width) || (y >= m_height))
		return;
	if (m_colour_type != INDEXED_COLOUR)
		return;
	putpixel (x, y, palindex);
}

void
NCPNG::palette (unsigned int palindex, unsigned int ired, unsigned int igreen, unsigned int iblue)
{
	if (palindex >= m_num_palette)
		return;
	unsigned int idx = palindex * 3;
	m_palette[idx    ] = (unsigned char) (ired & 255);
	m_palette[idx + 1] = (unsigned char) (igreen & 255);
	m_palette[idx + 2] = (unsigned char) (iblue & 255);
}

}	// namespace mypnglib
