/* -----------------------------------------------------------------------------

   DDS Library

   Based on code from Nvidia's DDS example:
   http://www.nvidia.com/object/dxtc_decompression_code.html

   Copyright (c) 2003 Randy Reddig
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the names of the copyright holders nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   ----------------------------------------------------------------------------- */



/* marker */
#define DDSLIB_C



/* dependencies */
#include "ddslib.h"



/* endian tomfoolery */
typedef union
{
	float f;
	char c[ 4 ];
}
floatSwapUnion;


#ifndef __BIG_ENDIAN__
	#ifdef _SGI_SOURCE
		#define __BIG_ENDIAN__
	#endif
#endif


#ifdef __BIG_ENDIAN__

int   DDSBigLong( int src ) { return src; }
short DDSBigShort( short src ) { return src; }
float DDSBigFloat( float src ) { return src; }

int DDSLittleLong( int src ){
	return ( ( src & 0xFF000000 ) >> 24 ) |
		   ( ( src & 0x00FF0000 ) >> 8 ) |
		   ( ( src & 0x0000FF00 ) << 8 ) |
		   ( ( src & 0x000000FF ) << 24 );
}

short DDSLittleShort( short src ){
	return ( ( src & 0xFF00 ) >> 8 ) |
		   ( ( src & 0x00FF ) << 8 );
}

float DDSLittleFloat( float src ){
	floatSwapUnion in,out;
	in.f = src;
	out.c[ 0 ] = in.c[ 3 ];
	out.c[ 1 ] = in.c[ 2 ];
	out.c[ 2 ] = in.c[ 1 ];
	out.c[ 3 ] = in.c[ 0 ];
	return out.f;
}

#else /*__BIG_ENDIAN__*/

int   DDSLittleLong( int src ) { return src; }
short DDSLittleShort( short src ) { return src; }
float DDSLittleFloat( float src ) { return src; }

int DDSBigLong( int src ){
	return ( ( src & 0xFF000000 ) >> 24 ) |
		   ( ( src & 0x00FF0000 ) >> 8 ) |
		   ( ( src & 0x0000FF00 ) << 8 ) |
		   ( ( src & 0x000000FF ) << 24 );
}

short DDSBigShort( short src ){
	return ( ( src & 0xFF00 ) >> 8 ) |
		   ( ( src & 0x00FF ) << 8 );
}

float DDSBigFloat( float src ){
	floatSwapUnion in,out;
	in.f = src;
	out.c[ 0 ] = in.c[ 3 ];
	out.c[ 1 ] = in.c[ 2 ];
	out.c[ 2 ] = in.c[ 1 ];
	out.c[ 3 ] = in.c[ 0 ];
	return out.f;
}

#endif /*__BIG_ENDIAN__*/



/*
   DDSDecodePixelFormat()
   determines which pixel format the dds texture is in
 */

static void DDSDecodePixelFormat( ddsBuffer_t *dds, ddsPF_t *pf ){
	unsigned int fourCC;


	/* dummy check */
	if ( dds == NULL || pf == NULL ) {
		return;
	}

	/* extract fourCC */
	fourCC = dds->pixelFormat.fourCC;

	/* test it */
	if ( fourCC == 0 ) {
		*pf = DDS_PF_ARGB8888;
	}
	else if ( fourCC == *( (unsigned int*) "DXT1" ) ) {
		*pf = DDS_PF_DXT1;
	}
	else if ( fourCC == *( (unsigned int*) "DXT2" ) ) {
		*pf = DDS_PF_DXT2;
	}
	else if ( fourCC == *( (unsigned int*) "DXT3" ) ) {
		*pf = DDS_PF_DXT3;
	}
	else if ( fourCC == *( (unsigned int*) "DXT4" ) ) {
		*pf = DDS_PF_DXT4;
	}
	else if ( fourCC == *( (unsigned int*) "DXT5" ) ) {
		*pf = DDS_PF_DXT5;
	}
	else{
		*pf = DDS_PF_UNKNOWN;
	}
}



/*
   DDSGetInfo()
   extracts relevant info from a dds texture, returns 0 on success
 */

int DDSGetInfo( ddsBuffer_t *dds, int *width, int *height, ddsPF_t *pf ){
	/* dummy test */
	if ( dds == NULL ) {
		return -1;
	}

	/* test dds header */
	if ( *( (int*) dds->magic ) != *( (int*) "DDS " ) ) {
		return -1;
	}
	if ( DDSLittleLong( dds->size ) != 124 ) {
		return -1;
	}

	/* extract width and height */
	if ( width != NULL ) {
		*width = DDSLittleLong( dds->width );
	}
	if ( height != NULL ) {
		*height = DDSLittleLong( dds->height );
	}

	/* get pixel format */
	DDSDecodePixelFormat( dds, pf );

	/* return ok */
	return 0;
}



/*
   DDSGetColorBlockColors()
   extracts colors from a dds color block
 */

static void DDSGetColorBlockColors( ddsColorBlock_t *block, ddsColor_t colors[ 4 ] ){
	unsigned short word;


	/* color 0 */
	word = DDSLittleShort( block->colors[ 0 ] );
	colors[ 0 ].a = 0xff;

	/* extract rgb bits */
	colors[ 0 ].b = (unsigned char) word;
	colors[ 0 ].b <<= 3;
	colors[ 0 ].b |= ( colors[ 0 ].b >> 5 );
	word >>= 5;
	colors[ 0 ].g = (unsigned char) word;
	colors[ 0 ].g <<= 2;
	colors[ 0 ].g |= ( colors[ 0 ].g >> 5 );
	word >>= 6;
	colors[ 0 ].r = (unsigned char) word;
	colors[ 0 ].r <<= 3;
	colors[ 0 ].r |= ( colors[ 0 ].r >> 5 );

	/* same for color 1 */
	word = DDSLittleShort( block->colors[ 1 ] );
	colors[ 1 ].a = 0xff;

	/* extract rgb bits */
	colors[ 1 ].b = (unsigned char) word;
	colors[ 1 ].b <<= 3;
	colors[ 1 ].b |= ( colors[ 1 ].b >> 5 );
	word >>= 5;
	colors[ 1 ].g = (unsigned char) word;
	colors[ 1 ].g <<= 2;
	colors[ 1 ].g |= ( colors[ 1 ].g >> 5 );
	word >>= 6;
	colors[ 1 ].r = (unsigned char) word;
	colors[ 1 ].r <<= 3;
	colors[ 1 ].r |= ( colors[ 1 ].r >> 5 );

	/* use this for all but the super-freak math method */
	if ( block->colors[ 0 ] > block->colors[ 1 ] ) {
		/* four-color block: derive the other two colors.
		   00 = color 0, 01 = color 1, 10 = color 2, 11 = color 3
		   these two bit codes correspond to the 2-bit fields
		   stored in the 64-bit block. */

		word = ( (unsigned short) colors[ 0 ].r * 2 + (unsigned short) colors[ 1 ].r ) / 3;
		/* no +1 for rounding */
		/* as bits have been shifted to 888 */
		colors[ 2 ].r = (unsigned char) word;
		word = ( (unsigned short) colors[ 0 ].g * 2 + (unsigned short) colors[ 1 ].g ) / 3;
		colors[ 2 ].g = (unsigned char) word;
		word = ( (unsigned short) colors[ 0 ].b * 2 + (unsigned short) colors[ 1 ].b ) / 3;
		colors[ 2 ].b = (unsigned char) word;
		colors[ 2 ].a = 0xff;

		word = ( (unsigned short) colors[ 0 ].r + (unsigned short) colors[ 1 ].r * 2 ) / 3;
		colors[ 3 ].r = (unsigned char) word;
		word = ( (unsigned short) colors[ 0 ].g + (unsigned short) colors[ 1 ].g * 2 ) / 3;
		colors[ 3 ].g = (unsigned char) word;
		word = ( (unsigned short) colors[ 0 ].b + (unsigned short) colors[ 1 ].b * 2 ) / 3;
		colors[ 3 ].b = (unsigned char) word;
		colors[ 3 ].a = 0xff;
	}
	else
	{
		/* three-color block: derive the other color.
		   00 = color 0, 01 = color 1, 10 = color 2,
		   11 = transparent.
		   These two bit codes correspond to the 2-bit fields
		   stored in the 64-bit block */

		word = ( (unsigned short) colors[ 0 ].r + (unsigned short) colors[ 1 ].r ) / 2;
		colors[ 2 ].r = (unsigned char) word;
		word = ( (unsigned short) colors[ 0 ].g + (unsigned short) colors[ 1 ].g ) / 2;
		colors[ 2 ].g = (unsigned char) word;
		word = ( (unsigned short) colors[ 0 ].b + (unsigned short) colors[ 1 ].b ) / 2;
		colors[ 2 ].b = (unsigned char) word;
		colors[ 2 ].a = 0xff;

		/* random color to indicate alpha */
		colors[ 3 ].r = 0x00;
		colors[ 3 ].g = 0xff;
		colors[ 3 ].b = 0xff;
		colors[ 3 ].a = 0x00;
	}
}



/*
   DDSDecodeColorBlock()
   decodes a dds color block
   fixme: make endian-safe
 */

static void DDSDecodeColorBlock( unsigned int *pixel, ddsColorBlock_t *block, int width, unsigned int colors[ 4 ] ){
	int r, n;
	unsigned int bits;
	unsigned int masks[] = { 3, 12, 3 << 4, 3 << 6 };       /* bit masks = 00000011, 00001100, 00110000, 11000000 */
	int shift[] = { 0, 2, 4, 6 };


	/* r steps through lines in y */
	for ( r = 0; r < 4; r++, pixel += ( width - 4 ) )  /* no width * 4 as unsigned int ptr inc will * 4 */
	{
		/* width * 4 bytes per pixel per line, each j dxtc row is 4 lines of pixels */

		/* n steps through pixels */
		for ( n = 0; n < 4; n++ )
		{
			bits = block->row[ r ] & masks[ n ];
			bits >>= shift[ n ];

			switch ( bits )
			{
			case 0:
				*pixel = colors[ 0 ];
				pixel++;
				break;

			case 1:
				*pixel = colors[ 1 ];
				pixel++;
				break;

			case 2:
				*pixel = colors[ 2 ];
				pixel++;
				break;

			case 3:
				*pixel = colors[ 3 ];
				pixel++;
				break;

			default:
				/* invalid */
				pixel++;
				break;
			}
		}
	}
}



/*
   DDSDecodeAlphaExplicit()
   decodes a dds explicit alpha block
 */

static void DDSDecodeAlphaExplicit( unsigned int *pixel, ddsAlphaBlockExplicit_t *alphaBlock, int width, unsigned int alphaZero ){
	int row, pix;
	unsigned short word;
	ddsColor_t color;


	/* clear color */
	color.r = 0;
	color.g = 0;
	color.b = 0;

	/* walk rows */
	for ( row = 0; row < 4; row++, pixel += ( width - 4 ) )
	{
		word = DDSLittleShort( alphaBlock->row[ row ] );

		/* walk pixels */
		for ( pix = 0; pix < 4; pix++ )
		{
			/* zero the alpha bits of image pixel */
			*pixel &= alphaZero;
			color.a = word & 0x000F;
			color.a = color.a | ( color.a << 4 );
			*pixel |= *( (unsigned int*) &color );
			word >>= 4;     /* move next bits to lowest 4 */
			pixel++;        /* move to next pixel in the row */

		}
	}
}



/*
   DDSDecodeAlpha3BitLinear()
   decodes interpolated alpha block
 */

static void DDSDecodeAlpha3BitLinear( unsigned int *pixel, ddsAlphaBlock3BitLinear_t *alphaBlock, int width, unsigned int alphaZero ){

	int row, pix;
	unsigned int stuff;
	unsigned char bits[ 4 ][ 4 ];
	unsigned short alphas[ 8 ];
	ddsColor_t aColors[ 4 ][ 4 ];


	/* get initial alphas */
	alphas[ 0 ] = alphaBlock->alpha0;
	alphas[ 1 ] = alphaBlock->alpha1;

	/* 8-alpha block */
	if ( alphas[ 0 ] > alphas[ 1 ] ) {
		/* 000 = alpha_0, 001 = alpha_1, others are interpolated */
		alphas[ 2 ] = ( 6 * alphas[ 0 ] +     alphas[ 1 ] ) / 7; /* bit code 010 */
		alphas[ 3 ] = ( 5 * alphas[ 0 ] + 2 * alphas[ 1 ] ) / 7; /* bit code 011 */
		alphas[ 4 ] = ( 4 * alphas[ 0 ] + 3 * alphas[ 1 ] ) / 7; /* bit code 100 */
		alphas[ 5 ] = ( 3 * alphas[ 0 ] + 4 * alphas[ 1 ] ) / 7; /* bit code 101 */
		alphas[ 6 ] = ( 2 * alphas[ 0 ] + 5 * alphas[ 1 ] ) / 7; /* bit code 110 */
		alphas[ 7 ] = (     alphas[ 0 ] + 6 * alphas[ 1 ] ) / 7; /* bit code 111 */
	}

	/* 6-alpha block */
	else
	{
		/* 000 = alpha_0, 001 = alpha_1, others are interpolated */
		alphas[ 2 ] = ( 4 * alphas[ 0 ] +     alphas[ 1 ] ) / 5;  /* bit code 010 */
		alphas[ 3 ] = ( 3 * alphas[ 0 ] + 2 * alphas[ 1 ] ) / 5;  /* bit code 011 */
		alphas[ 4 ] = ( 2 * alphas[ 0 ] + 3 * alphas[ 1 ] ) / 5;  /* bit code 100 */
		alphas[ 5 ] = (    alphas[ 0 ] + 4 * alphas[ 1 ] ) / 5;  /* bit code 101 */
		alphas[ 6 ] = 0;                                        /* bit code 110 */
		alphas[ 7 ] = 255;                                      /* bit code 111 */
	}

	/* decode 3-bit fields into array of 16 bytes with same value */

	/* first two rows of 4 pixels each */
	stuff = *( (unsigned int*) &( alphaBlock->stuff[ 0 ] ) );

	bits[ 0 ][ 0 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 0 ][ 1 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 0 ][ 2 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 0 ][ 3 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 1 ][ 0 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 1 ][ 1 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 1 ][ 2 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 1 ][ 3 ] = (unsigned char) ( stuff & 0x00000007 );

	/* last two rows */
	stuff = *( (unsigned int*) &( alphaBlock->stuff[ 3 ] ) ); /* last 3 bytes */

	bits[ 2 ][ 0 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 2 ][ 1 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 2 ][ 2 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 2 ][ 3 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 3 ][ 0 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 3 ][ 1 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 3 ][ 2 ] = (unsigned char) ( stuff & 0x00000007 );
	stuff >>= 3;
	bits[ 3 ][ 3 ] = (unsigned char) ( stuff & 0x00000007 );

	/* decode the codes into alpha values */
	for ( row = 0; row < 4; row++ )
	{
		for ( pix = 0; pix < 4; pix++ )
		{
			aColors[ row ][ pix ].r = 0;
			aColors[ row ][ pix ].g = 0;
			aColors[ row ][ pix ].b = 0;
			aColors[ row ][ pix ].a = (unsigned char) alphas[ bits[ row ][ pix ] ];
		}
	}

	/* write out alpha values to the image bits */
	for ( row = 0; row < 4; row++, pixel += width - 4 )
	{
		for ( pix = 0; pix < 4; pix++ )
		{
			/* zero the alpha bits of image pixel */
			*pixel &= alphaZero;

			/* or the bits into the prev. nulled alpha */
			*pixel |= *( (unsigned int*) &( aColors[ row ][ pix ] ) );
			pixel++;
		}
	}
}



/*
   DDSDecompressDXT1()
   decompresses a dxt1 format texture
 */

static int DDSDecompressDXT1( ddsBuffer_t *dds, int width, int height, unsigned char *pixels ){
	int x, y, xBlocks, yBlocks;
	unsigned int    *pixel;
	ddsColorBlock_t *block;
	ddsColor_t colors[ 4 ];


	/* setup */
	xBlocks = width / 4;
	yBlocks = height / 4;

	/* walk y */
	for ( y = 0; y < yBlocks; y++ )
	{
		/* 8 bytes per block */
		block = (ddsColorBlock_t*) ( (size_t) dds->data + y * xBlocks * 8 );

		/* walk x */
		for ( x = 0; x < xBlocks; x++, block++ )
		{
			DDSGetColorBlockColors( block, colors );
			pixel = (unsigned int*) ( pixels + x * 16 + ( y * 4 ) * width * 4 );
			DDSDecodeColorBlock( pixel, block, width, (unsigned int*) colors );
		}
	}

	/* return ok */
	return 0;
}



/*
   DDSDecompressDXT3()
   decompresses a dxt3 format texture
 */

static int DDSDecompressDXT3( ddsBuffer_t *dds, int width, int height, unsigned char *pixels ){
	int x, y, xBlocks, yBlocks;
	unsigned int            *pixel, alphaZero;
	ddsColorBlock_t         *block;
	ddsAlphaBlockExplicit_t *alphaBlock;
	ddsColor_t colors[ 4 ];


	/* setup */
	xBlocks = width / 4;
	yBlocks = height / 4;

	/* create zero alpha */
	colors[ 0 ].a = 0;
	colors[ 0 ].r = 0xFF;
	colors[ 0 ].g = 0xFF;
	colors[ 0 ].b = 0xFF;
	alphaZero = *( (unsigned int*) &colors[ 0 ] );

	/* walk y */
	for ( y = 0; y < yBlocks; y++ )
	{
		/* 8 bytes per block, 1 block for alpha, 1 block for color */
		block = (ddsColorBlock_t*) ( (size_t) dds->data + y * xBlocks * 16 );

		/* walk x */
		for ( x = 0; x < xBlocks; x++, block++ )
		{
			/* get alpha block */
			alphaBlock = (ddsAlphaBlockExplicit_t*) block;

			/* get color block */
			block++;
			DDSGetColorBlockColors( block, colors );

			/* decode color block */
			pixel = (unsigned int*) ( pixels + x * 16 + ( y * 4 ) * width * 4 );
			DDSDecodeColorBlock( pixel, block, width, (unsigned int*) colors );

			/* overwrite alpha bits with alpha block */
			DDSDecodeAlphaExplicit( pixel, alphaBlock, width, alphaZero );
		}
	}

	/* return ok */
	return 0;
}



/*
   DDSDecompressDXT5()
   decompresses a dxt5 format texture
 */

static int DDSDecompressDXT5( ddsBuffer_t *dds, int width, int height, unsigned char *pixels ){
	int x, y, xBlocks, yBlocks;
	unsigned int                *pixel, alphaZero;
	ddsColorBlock_t             *block;
	ddsAlphaBlock3BitLinear_t   *alphaBlock;
	ddsColor_t colors[ 4 ];


	/* setup */
	xBlocks = width / 4;
	yBlocks = height / 4;

	/* create zero alpha */
	colors[ 0 ].a = 0;
	colors[ 0 ].r = 0xFF;
	colors[ 0 ].g = 0xFF;
	colors[ 0 ].b = 0xFF;
	alphaZero = *( (unsigned int*) &colors[ 0 ] );

	/* walk y */
	for ( y = 0; y < yBlocks; y++ )
	{
		/* 8 bytes per block, 1 block for alpha, 1 block for color */
		block = (ddsColorBlock_t*) ( (size_t) dds->data + y * xBlocks * 16 );

		/* walk x */
		for ( x = 0; x < xBlocks; x++, block++ )
		{
			/* get alpha block */
			alphaBlock = (ddsAlphaBlock3BitLinear_t*) block;

			/* get color block */
			block++;
			DDSGetColorBlockColors( block, colors );

			/* decode color block */
			pixel = (unsigned int*) ( pixels + x * 16 + ( y * 4 ) * width * 4 );
			DDSDecodeColorBlock( pixel, block, width, (unsigned int*) colors );

			/* overwrite alpha bits with alpha block */
			DDSDecodeAlpha3BitLinear( pixel, alphaBlock, width, alphaZero );
		}
	}

	/* return ok */
	return 0;
}



/*
   DDSDecompressDXT2()
   decompresses a dxt2 format texture (fixme: un-premultiply alpha)
 */

static int DDSDecompressDXT2( ddsBuffer_t *dds, int width, int height, unsigned char *pixels ){
	int r;


	/* decompress dxt3 first */
	r = DDSDecompressDXT3( dds, width, height, pixels );

	/* return to sender */
	return r;
}



/*
   DDSDecompressDXT4()
   decompresses a dxt4 format texture (fixme: un-premultiply alpha)
 */

static int DDSDecompressDXT4( ddsBuffer_t *dds, int width, int height, unsigned char *pixels ){
	int r;


	/* decompress dxt5 first */
	r = DDSDecompressDXT5( dds, width, height, pixels );

	/* return to sender */
	return r;
}



/*
   DDSDecompressARGB8888()
   decompresses an argb 8888 format texture
 */

static int DDSDecompressARGB8888( ddsBuffer_t *dds, int width, int height, unsigned char *pixels ){
	int x, y;
	unsigned char               *in, *out;


	/* setup */
	in = dds->data;
	out = pixels;

	/* walk y */
	for ( y = 0; y < height; y++ )
	{
		/* walk x */
		for ( x = 0; x < width; x++ )
		{
			*out++ = *in++;
			*out++ = *in++;
			*out++ = *in++;
			*out++ = *in++;
		}
	}

	/* return ok */
	return 0;
}



/*
   DDSDecompress()
   decompresses a dds texture into an rgba image buffer, returns 0 on success
 */

int DDSDecompress( ddsBuffer_t *dds, unsigned char *pixels ){
	int width, height, r;
	ddsPF_t pf;


	/* get dds info */
	r = DDSGetInfo( dds, &width, &height, &pf );
	if ( r ) {
		return r;
	}

	/* decompress */
	switch ( pf )
	{
	case DDS_PF_ARGB8888:
		/* fixme: support other [a]rgb formats */
		r = DDSDecompressARGB8888( dds, width, height, pixels );
		break;

	case DDS_PF_DXT1:
		r = DDSDecompressDXT1( dds, width, height, pixels );
		break;

	case DDS_PF_DXT2:
		r = DDSDecompressDXT2( dds, width, height, pixels );
		break;

	case DDS_PF_DXT3:
		r = DDSDecompressDXT3( dds, width, height, pixels );
		break;

	case DDS_PF_DXT4:
		r = DDSDecompressDXT4( dds, width, height, pixels );
		break;

	case DDS_PF_DXT5:
		r = DDSDecompressDXT5( dds, width, height, pixels );
		break;

	default:
	case DDS_PF_UNKNOWN:
		memset( pixels, 0xFF, width * height * 4 );
		r = -1;
		break;
	}

	/* return to sender */
	return r;
}
