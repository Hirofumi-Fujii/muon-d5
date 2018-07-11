// ncpng.h
// No-Compressed PNG

#ifndef NCPNG_H_INCLUDED
#define NCPNG_H_INCLUDED

#include <iostream>

namespace mypnglib
{

class NCPNG
{
public:
	enum COLOURTYPE
	{
		GREYSCALE = 0, TRUECOLOUR = 2, 
		INDEXED_COLOUR = 3,
		GREYSCALE_WITH_ALPHA = 4, TRUECOLOUR_WITH_ALPHA = 6
	};

	static const int SIGNATURE_SIZE = 8;
	static const unsigned char m_signature[SIGNATURE_SIZE];
	enum { ADLER32_BASE = 65521 };	// largest prime smaller than 65536

public:
	class COLOUR_PIXEL
	{
	public:
		COLOUR_PIXEL () : m_red (0.0), m_green (0.0), m_blue (0.0), m_alpha (1.0) { return; }
		COLOUR_PIXEL (double red, double green, double blue);
		COLOUR_PIXEL (double red, double green, double blue, double alpha);
		double m_red;
		double m_green;
		double m_blue;
		double m_alpha;
	};

	class GREY_PIXEL
	{
	public:
		GREY_PIXEL () : m_intensity (0.0), m_alpha (1.0) { return; }
		GREY_PIXEL (double intensity);
		GREY_PIXEL (double intensity, double alpha);
		double m_intensity;
		double m_alpha;
	};

public:
	NCPNG(unsigned long imgw, unsigned long imgh,
		COLOURTYPE coltype = TRUECOLOUR, unsigned int bitdepth = 8);
	virtual ~NCPNG();
	virtual void put (unsigned long x, unsigned long y, const COLOUR_PIXEL& pixel);
	virtual void put (unsigned long x, unsigned long y, const GREY_PIXEL& pixel);
	virtual void put (unsigned long x, unsigned long y, unsigned int palindex);
	virtual void palette (unsigned int palindex, unsigned int ired, unsigned int igreen, unsigned int iblue);
	std::ostream& write (std::ostream& os);

protected:
	static unsigned long crc_table[256];
	static int crc_table_computed;
	static unsigned char m_biton[8];

	static void make_crc_table (void);
	static unsigned long update_crc
		(unsigned long crc, const unsigned char* buf, unsigned long len);
	static unsigned char* fill_ulong (unsigned char* buf, unsigned long u);
	static double rgbtogrey (double r, double g, double b)
		{ return (r * 0.299 + g * 0.587 + b * 0.114); }
	static void write_chunk
		(std::ostream& os,
		const char* name, const unsigned char* buf, unsigned long len);

protected:
	class IDATWriter
	{
	public:
		IDATWriter () : m_filled (0) { return; }
		virtual ~IDATWriter() { return; }
		std::ostream& put (std::ostream& os, unsigned char c);
		std::ostream& flush (std::ostream& os); 
	protected:
		static const unsigned int IDATBUF_SIZE = 64000;
	protected:
		unsigned long m_filled;
		unsigned char m_buf [IDATBUF_SIZE];
	};

protected:
	void write_IHDR (std::ostream& os) const;
	void write_PLTE (std::ostream& os) const;
	void write_IDAT (std::ostream& os) const;
	void write_IEND (std::ostream& os) const;

//	static const unsigned long BASE = 65521; /* largest prime smaller than 65536 */

protected:
	void putpixel(unsigned long x, unsigned long y,
		unsigned int nelem, double dat[]);
	void putpixel(unsigned long x, unsigned long y,
		unsigned int palindex);

protected:
	unsigned long m_width;
	unsigned long m_height;
	COLOURTYPE m_colour_type;
	unsigned int m_bit_depth;
	unsigned int m_compression_method;
	unsigned int m_filter_method;
	unsigned int m_interlace_method;
	unsigned char* m_imgbuf;
	//
	unsigned long m_bytes_per_scanline;
	//
	unsigned int m_num_palette;
	unsigned char m_palette[768];	// 768 = 256 * 3(R,G,B)
};

}	// namespace mypnglib
#endif	// NCPNG_H_INCLUDED
