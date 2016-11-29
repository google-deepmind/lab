/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// Functions to load JPEG files from a buffer, based on jdatasrc.c
//
// Leonardo Zide (leo@lokigames.com)
//

#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jpeglib.h>
#include <jerror.h>

/* Expanded data source object for stdio input */

typedef struct {
	struct jpeg_source_mgr pub; /* public fields */

	int src_size;
	JOCTET * src_buffer;

	JOCTET * buffer;    /* start of buffer */
	boolean start_of_file;  /* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

#define INPUT_BUF_SIZE  4096    /* choose an efficiently fread'able size */


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

static void my_init_source( j_decompress_ptr cinfo ){
	my_src_ptr src = (my_src_ptr) cinfo->src;

	/* We reset the empty-input-file flag for each image,
	 * but we don't clear the input buffer.
	 * This is correct behavior for reading a series of images from one source.
	 */
	src->start_of_file = TRUE;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

static boolean my_fill_input_buffer( j_decompress_ptr cinfo ){
	my_src_ptr src = (my_src_ptr) cinfo->src;
	size_t nbytes;

	if ( src->src_size > INPUT_BUF_SIZE ) {
		nbytes = INPUT_BUF_SIZE;
	}
	else{
		nbytes = src->src_size;
	}

	memcpy( src->buffer, src->src_buffer, nbytes );
	src->src_buffer += nbytes;
	src->src_size -= nbytes;

	if ( nbytes <= 0 ) {
		if ( src->start_of_file ) { /* Treat empty input file as fatal error */
			ERREXIT( cinfo, JERR_INPUT_EMPTY );
		}
		WARNMS( cinfo, JWRN_JPEG_EOF );
		/* Insert a fake EOI marker */
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

static void my_skip_input_data( j_decompress_ptr cinfo, long num_bytes ){
	my_src_ptr src = (my_src_ptr) cinfo->src;

	/* Just a dumb implementation for now.  Could use fseek() except
	 * it doesn't work on pipes.  Not clear that being smart is worth
	 * any trouble anyway --- large skips are infrequent.
	 */
	if ( num_bytes > 0 ) {
		while ( num_bytes > (long) src->pub.bytes_in_buffer ) {
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void) my_fill_input_buffer( cinfo );
			/* note we assume that fill_input_buffer will never return FALSE,
			 * so suspension need not be handled.
			 */
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

static void my_term_source( j_decompress_ptr cinfo ){
	/* no work necessary here */
}


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

static void jpeg_buffer_src( j_decompress_ptr cinfo, void* buffer, int bufsize ){
	my_src_ptr src;

	/* The source object and input buffer are made permanent so that a series
	 * of JPEG images can be read from the same file by calling jpeg_stdio_src
	 * only before the first one.  (If we discarded the buffer at the end of
	 * one image, we'd likely lose the start of the next one.)
	 * This makes it unsafe to use this manager and a different source
	 * manager serially with the same JPEG object.  Caveat programmer.
	 */
	if ( cinfo->src == NULL ) { /* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
					 ( *cinfo->mem->alloc_small )( (j_common_ptr) cinfo, JPOOL_PERMANENT,
												   sizeof( my_source_mgr ) );
		src = (my_src_ptr) cinfo->src;
		src->buffer = (JOCTET *)
					  ( *cinfo->mem->alloc_small )( (j_common_ptr) cinfo, JPOOL_PERMANENT,
													INPUT_BUF_SIZE * sizeof( JOCTET ) );
	}

	src = (my_src_ptr) cinfo->src;
	src->pub.init_source = my_init_source;
	src->pub.fill_input_buffer = my_fill_input_buffer;
	src->pub.skip_input_data = my_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = my_term_source;
	src->src_buffer = (JOCTET *)buffer;
	src->src_size = bufsize;
	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
}

// =============================================================================

static char errormsg[JMSG_LENGTH_MAX];

typedef struct my_jpeg_error_mgr
{
	struct jpeg_error_mgr pub; // "public" fields
	jmp_buf setjmp_buffer;    // for return to caller
} bt_jpeg_error_mgr;

static void my_jpeg_error_exit( j_common_ptr cinfo ){
	bt_jpeg_error_mgr* myerr = (bt_jpeg_error_mgr*) cinfo->err;

	( *cinfo->err->format_message )( cinfo, errormsg );

	longjmp( myerr->setjmp_buffer, 1 );
}

// stash a scanline
static void j_putRGBScanline( unsigned char* jpegline, int widthPix, unsigned char* outBuf, int row ){
	int offset = row * widthPix * 4;
	int count;

	for ( count = 0; count < widthPix; count++ )
	{
		unsigned char iRed, iBlu, iGrn;
		unsigned char *oRed, *oBlu, *oGrn, *oAlp;

		iRed = *( jpegline + count * 3 + 0 );
		iGrn = *( jpegline + count * 3 + 1 );
		iBlu = *( jpegline + count * 3 + 2 );

		oRed = outBuf + offset + count * 4 + 0;
		oGrn = outBuf + offset + count * 4 + 1;
		oBlu = outBuf + offset + count * 4 + 2;
		oAlp = outBuf + offset + count * 4 + 3;

		*oRed = iRed;
		*oGrn = iGrn;
		*oBlu = iBlu;
		*oAlp = 255;
	}
}

// stash a scanline
static void j_putRGBAScanline( unsigned char* jpegline, int widthPix, unsigned char* outBuf, int row ){
	int offset = row * widthPix * 4;
	int count;

	for ( count = 0; count < widthPix; count++ )
	{
		unsigned char iRed, iBlu, iGrn, iAlp;
		unsigned char *oRed, *oBlu, *oGrn, *oAlp;

		iRed = *( jpegline + count * 4 + 0 );
		iGrn = *( jpegline + count * 4 + 1 );
		iBlu = *( jpegline + count * 4 + 2 );
		iAlp = *( jpegline + count * 4 + 3 );

		oRed = outBuf + offset + count * 4 + 0;
		oGrn = outBuf + offset + count * 4 + 1;
		oBlu = outBuf + offset + count * 4 + 2;
		oAlp = outBuf + offset + count * 4 + 3;

		*oRed = iRed;
		*oGrn = iGrn;
		*oBlu = iBlu;
		// ydnar: see bug 900
		*oAlp = 255; //%	iAlp;
	}
}

// stash a gray scanline
static void j_putGrayScanlineToRGB( unsigned char* jpegline, int widthPix, unsigned char* outBuf, int row ){
	int offset = row * widthPix * 4;
	int count;

	for ( count = 0; count < widthPix; count++ )
	{
		unsigned char iGray;
		unsigned char *oRed, *oBlu, *oGrn, *oAlp;

		// get our grayscale value
		iGray = *( jpegline + count );

		oRed = outBuf + offset + count * 4;
		oGrn = outBuf + offset + count * 4 + 1;
		oBlu = outBuf + offset + count * 4 + 2;
		oAlp = outBuf + offset + count * 4 + 3;

		*oRed = iGray;
		*oGrn = iGray;
		*oBlu = iGray;
		*oAlp = 255;
	}
}

int LoadJPGBuff( void *src_buffer, int src_size, unsigned char **pic, int *width, int *height ) {
	struct jpeg_decompress_struct cinfo;
	struct my_jpeg_error_mgr jerr;
	JSAMPARRAY buffer;
	int row_stride, size;

	cinfo.err = jpeg_std_error( &jerr.pub );
	jerr.pub.error_exit = my_jpeg_error_exit;

	if ( setjmp( jerr.setjmp_buffer ) ) {
		*pic = (unsigned char*)errormsg;
		jpeg_destroy_decompress( &cinfo );
		return -1;
	}

	jpeg_create_decompress( &cinfo );
	jpeg_buffer_src( &cinfo, src_buffer, src_size );
	jpeg_read_header( &cinfo, TRUE );
	jpeg_start_decompress( &cinfo );

	row_stride = cinfo.output_width * cinfo.output_components;

	size = cinfo.output_width * cinfo.output_height * 4;
	*width = cinfo.output_width;
	*height = cinfo.output_height;
	*pic = (unsigned char*)( malloc( size + 1 ) );
	memset( *pic, 0, size + 1 );

	buffer = ( *cinfo.mem->alloc_sarray )( ( j_common_ptr ) & cinfo, JPOOL_IMAGE, row_stride, 1 );

	while ( cinfo.output_scanline < cinfo.output_height )
	{
		jpeg_read_scanlines( &cinfo, buffer, 1 );

		if ( cinfo.out_color_components == 4 ) {
			j_putRGBAScanline( buffer[0], cinfo.output_width, *pic, cinfo.output_scanline - 1 );
		}
		else if ( cinfo.out_color_components == 3 ) {
			j_putRGBScanline( buffer[0], cinfo.output_width, *pic, cinfo.output_scanline - 1 );
		}
		else if ( cinfo.out_color_components == 1 ) {
			j_putGrayScanlineToRGB( buffer[0], cinfo.output_width, *pic, cinfo.output_scanline - 1 );
		}
	}

	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	return 0;
}
