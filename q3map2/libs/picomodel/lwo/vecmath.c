/*
   ======================================================================
   vecmath.c

   Basic vector and matrix functions.

   Ernie Wright  17 Sep 00
   ====================================================================== */

#include <math.h>


float dot( float a[], float b[] ){
	return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
}


void cross( float a[], float b[], float c[] ){
	c[ 0 ] = a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
	c[ 1 ] = a[ 2 ] * b[ 0 ] - a[ 0 ] * b[ 2 ];
	c[ 2 ] = a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ];
}


void normalize( float v[] ){
	float r;

	r = ( float ) sqrt( dot( v, v ) );
	if ( r > 0 ) {
		v[ 0 ] /= r;
		v[ 1 ] /= r;
		v[ 2 ] /= r;
	}
}
