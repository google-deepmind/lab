/*
   ======================================================================
   lwio.c

   Functions for reading basic LWO2 data types.

   Ernie Wright  17 Sep 00
   ====================================================================== */

#include <limits.h>

#include "../picointernal.h"
#include "lwo2.h"


/*
   ======================================================================
   flen

   This accumulates a count of the number of bytes read.  Callers can set
   it at the beginning of a sequence of reads and then retrieve it to get
   the number of bytes actually read.  If one of the I/O functions fails,
   flen is set to an error code, after which the I/O functions ignore
   read requests until flen is reset.
   ====================================================================== */

#define FLEN_ERROR INT_MIN

static int flen;

void set_flen( int i ) { flen = i; }

int get_flen( void ) { return flen; }


#ifdef _WIN32
/*
   =====================================================================
   revbytes()

   Reverses byte order in place.

   INPUTS
   bp       bytes to reverse
   elsize   size of the underlying data type
   elcount  number of elements to swap

   RESULTS
   Reverses the byte order in each of elcount elements.

   This only needs to be defined on little-endian platforms, most
   notably Windows.  lwo2.h replaces this with a #define on big-endian
   platforms.
   ===================================================================== */

void revbytes( void *bp, int elsize, int elcount ){
	register unsigned char *p, *q;

	p = ( unsigned char * ) bp;

	if ( elsize == 2 ) {
		q = p + 1;
		while ( elcount-- ) {
			*p ^= *q;
			*q ^= *p;
			*p ^= *q;
			p += 2;
			q += 2;
		}
		return;
	}

	while ( elcount-- ) {
		q = p + elsize - 1;
		while ( p < q ) {
			*p ^= *q;
			*q ^= *p;
			*p ^= *q;
			++p;
			--q;
		}
		p += elsize >> 1;
	}
}
#endif


void *getbytes( picoMemStream_t *fp, int size ){
	void *data;

	if ( flen == FLEN_ERROR ) {
		return NULL;
	}
	if ( size < 0 ) {
		flen = FLEN_ERROR;
		return NULL;
	}
	data = _pico_alloc( size );
	if ( !data ) {
		flen = FLEN_ERROR;
		return NULL;
	}
	if ( 1 != _pico_memstream_read( fp, data, size ) ) {
		flen = FLEN_ERROR;
		_pico_free( data );
		return NULL;
	}

	flen += size;
	return data;
}


void skipbytes( picoMemStream_t *fp, int n ){
	if ( flen == FLEN_ERROR ) {
		return;
	}
	if ( _pico_memstream_seek( fp, n, PICO_SEEK_CUR ) ) {
		flen = FLEN_ERROR;
	}
	else{
		flen += n;
	}
}


int getI1( picoMemStream_t *fp ){
	int i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	i = _pico_memstream_getc( fp );
	if ( i < 0 ) {
		flen = FLEN_ERROR;
		return 0;
	}
	if ( i > 127 ) {
		i -= 256;
	}
	flen += 1;
	return i;
}


short getI2( picoMemStream_t *fp ){
	short i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	if ( 1 != _pico_memstream_read( fp, &i, 2 ) ) {
		flen = FLEN_ERROR;
		return 0;
	}
	revbytes( &i, 2, 1 );
	flen += 2;
	return i;
}


int getI4( picoMemStream_t *fp ){
	int i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	if ( 1 != _pico_memstream_read( fp, &i, 4 ) ) {
		flen = FLEN_ERROR;
		return 0;
	}
	revbytes( &i, 4, 1 );
	flen += 4;
	return i;
}


unsigned char getU1( picoMemStream_t *fp ){
	int i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	i = _pico_memstream_getc( fp );
	if ( i < 0 ) {
		flen = FLEN_ERROR;
		return 0;
	}
	flen += 1;
	return i;
}


unsigned short getU2( picoMemStream_t *fp ){
	unsigned short i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	if ( 1 != _pico_memstream_read( fp, &i, 2 ) ) {
		flen = FLEN_ERROR;
		return 0;
	}
	revbytes( &i, 2, 1 );
	flen += 2;
	return i;
}


unsigned int getU4( picoMemStream_t *fp ){
	unsigned int i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	if ( 1 != _pico_memstream_read( fp, &i, 4 ) ) {
		flen = FLEN_ERROR;
		return 0;
	}
	revbytes( &i, 4, 1 );
	flen += 4;
	return i;
}


int getVX( picoMemStream_t *fp ){
	int i, c;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}

	c = _pico_memstream_getc( fp );
	if ( c != 0xFF ) {
		i = c << 8;
		c = _pico_memstream_getc( fp );
		i |= c;
		flen += 2;
	}
	else {
		c = _pico_memstream_getc( fp );
		i = c << 16;
		c = _pico_memstream_getc( fp );
		i |= c << 8;
		c = _pico_memstream_getc( fp );
		i |= c;
		flen += 4;
	}

	if ( _pico_memstream_error( fp ) ) {
		flen = FLEN_ERROR;
		return 0;
	}
	return i;
}


float getF4( picoMemStream_t *fp ){
	float f;

	if ( flen == FLEN_ERROR ) {
		return 0.0f;
	}
	if ( 1 != _pico_memstream_read( fp, &f, 4 ) ) {
		flen = FLEN_ERROR;
		return 0.0f;
	}
	revbytes( &f, 4, 1 );
	flen += 4;
	return f;
}


char *getS0( picoMemStream_t *fp ){
	char *s;
	int i, c, len, pos;

	if ( flen == FLEN_ERROR ) {
		return NULL;
	}

	pos = _pico_memstream_tell( fp );
	for ( i = 1; ; i++ ) {
		c = _pico_memstream_getc( fp );
		if ( c <= 0 ) {
			break;
		}
	}
	if ( c < 0 ) {
		flen = FLEN_ERROR;
		return NULL;
	}

	if ( i == 1 ) {
		if ( _pico_memstream_seek( fp, pos + 2, PICO_SEEK_SET ) ) {
			flen = FLEN_ERROR;
		}
		else{
			flen += 2;
		}
		return NULL;
	}

	len = i + ( i & 1 );
	s = _pico_alloc( len );
	if ( !s ) {
		flen = FLEN_ERROR;
		return NULL;
	}

	if ( _pico_memstream_seek( fp, pos, PICO_SEEK_SET ) ) {
		flen = FLEN_ERROR;
		return NULL;
	}
	if ( 1 != _pico_memstream_read( fp, s, len ) ) {
		flen = FLEN_ERROR;
		return NULL;
	}

	flen += len;
	return s;
}


int sgetI1( unsigned char **bp ){
	int i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	i = **bp;
	if ( i > 127 ) {
		i -= 256;
	}
	flen += 1;
	( *bp )++;
	return i;
}


short sgetI2( unsigned char **bp ){
	short i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	memcpy( &i, *bp, 2 );
	revbytes( &i, 2, 1 );
	flen += 2;
	*bp += 2;
	return i;
}


int sgetI4( unsigned char **bp ){
	int i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	memcpy( &i, *bp, 4 );
	revbytes( &i, 4, 1 );
	flen += 4;
	*bp += 4;
	return i;
}


unsigned char sgetU1( unsigned char **bp ){
	unsigned char c;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	c = **bp;
	flen += 1;
	( *bp )++;
	return c;
}


unsigned short sgetU2( unsigned char **bp ){
	unsigned char *buf = *bp;
	unsigned short i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	i = ( buf[ 0 ] << 8 ) | buf[ 1 ];
	flen += 2;
	*bp += 2;
	return i;
}


unsigned int sgetU4( unsigned char **bp ){
	unsigned int i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}
	memcpy( &i, *bp, 4 );
	revbytes( &i, 4, 1 );
	flen += 4;
	*bp += 4;
	return i;
}


int sgetVX( unsigned char **bp ){
	unsigned char *buf = *bp;
	int i;

	if ( flen == FLEN_ERROR ) {
		return 0;
	}

	if ( buf[ 0 ] != 0xFF ) {
		i = buf[ 0 ] << 8 | buf[ 1 ];
		flen += 2;
		*bp += 2;
	}
	else {
		i = ( buf[ 1 ] << 16 ) | ( buf[ 2 ] << 8 ) | buf[ 3 ];
		flen += 4;
		*bp += 4;
	}
	return i;
}


float sgetF4( unsigned char **bp ){
	float f;

	if ( flen == FLEN_ERROR ) {
		return 0.0f;
	}
	memcpy( &f, *bp, 4 );
	revbytes( &f, 4, 1 );
	flen += 4;
	*bp += 4;
	return f;
}


char *sgetS0( unsigned char **bp ){
	char *s;
	unsigned char *buf = *bp;
	int len;

	if ( flen == FLEN_ERROR ) {
		return NULL;
	}

	len = strlen( (char *) buf ) + 1;
	if ( len == 1 ) {
		flen += 2;
		*bp += 2;
		return NULL;
	}
	len += len & 1;
	s = _pico_alloc( len );
	if ( !s ) {
		flen = FLEN_ERROR;
		return NULL;
	}

	memcpy( s, buf, len );
	flen += len;
	*bp += len;
	return s;
}
