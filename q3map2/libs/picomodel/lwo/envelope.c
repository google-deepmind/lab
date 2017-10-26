/*
   ======================================================================
   envelope.c

   Envelope functions for an LWO2 reader.

   Ernie Wright  16 Nov 00
   ====================================================================== */

#include "../picointernal.h"
#include "lwo2.h"

/*
   ======================================================================
   lwFreeEnvelope()

   Free the memory used by an lwEnvelope.
   ====================================================================== */

void lwFreeEnvelope( lwEnvelope *env ){
	if ( env ) {
		if ( env->name ) {
			_pico_free( env->name );
		}
		lwListFree( env->key, _pico_free );
		lwListFree( env->cfilter, (ListFreeFunc) lwFreePlugin );
		_pico_free( env );
	}
}


static int compare_keys( lwKey *k1, lwKey *k2 ){
	return k1->time > k2->time ? 1 : k1->time < k2->time ? -1 : 0;
}


/*
   ======================================================================
   lwGetEnvelope()

   Read an ENVL chunk from an LWO2 file.
   ====================================================================== */

lwEnvelope *lwGetEnvelope( picoMemStream_t *fp, int cksize ){
	lwEnvelope *env;
	lwKey *key;
	lwPlugin *plug;
	unsigned int id;
	unsigned short sz;
	float f[ 4 ];
	int i, nparams, pos, rlen;


	/* allocate the Envelope structure */

	env = _pico_calloc( 1, sizeof( lwEnvelope ) );
	if ( !env ) {
		goto Fail;
	}

	/* remember where we started */

	set_flen( 0 );
	pos = _pico_memstream_tell( fp );

	/* index */

	env->index = getVX( fp );

	/* first subchunk header */

	id = getU4( fp );
	sz = getU2( fp );
	if ( 0 > get_flen() ) {
		goto Fail;
	}

	key = NULL;

	/* process subchunks as they're encountered */

	while ( 1 ) {
		sz += sz & 1;
		set_flen( 0 );

		switch ( id ) {
		case ID_TYPE:
			env->type = getU2( fp );
			break;

		case ID_NAME:
			env->name = getS0( fp );
			break;

		case ID_PRE:
			env->behavior[ 0 ] = getU2( fp );
			break;

		case ID_POST:
			env->behavior[ 1 ] = getU2( fp );
			break;

		case ID_KEY:
			key = _pico_calloc( 1, sizeof( lwKey ) );
			if ( !key ) {
				goto Fail;
			}
			key->time = getF4( fp );
			key->value = getF4( fp );
			lwListInsert( (void **) &env->key, key, (ListCompareFunc) compare_keys );
			env->nkeys++;
			break;

		case ID_SPAN:
			if ( !key ) {
				goto Fail;
			}
			key->shape = getU4( fp );

			nparams = ( sz - 4 ) / 4;
			if ( nparams > 4 ) {
				nparams = 4;
			}
			for ( i = 0; i < nparams; i++ )
				f[ i ] = getF4( fp );

			switch ( key->shape ) {
			case ID_TCB:
				key->tension = f[ 0 ];
				key->continuity = f[ 1 ];
				key->bias = f[ 2 ];
				break;

			case ID_BEZI:
			case ID_HERM:
			case ID_BEZ2:
				for ( i = 0; i < nparams; i++ )
					key->param[ i ] = f[ i ];
				break;
			}
			break;

		case ID_CHAN:
			plug = _pico_calloc( 1, sizeof( lwPlugin ) );
			if ( !plug ) {
				goto Fail;
			}

			plug->name = getS0( fp );
			plug->flags = getU2( fp );
			plug->data = getbytes( fp, sz - get_flen() );

			lwListAdd( (void **) &env->cfilter, plug );
			env->ncfilters++;
			break;

		default:
			break;
		}

		/* error while reading current subchunk? */

		rlen = get_flen();
		if ( rlen < 0 || rlen > sz ) {
			goto Fail;
		}

		/* skip unread parts of the current subchunk */

		if ( rlen < sz ) {
			_pico_memstream_seek( fp, sz - rlen, PICO_SEEK_CUR );
		}

		/* end of the ENVL chunk? */

		rlen = _pico_memstream_tell( fp ) - pos;
		if ( cksize < rlen ) {
			goto Fail;
		}
		if ( cksize == rlen ) {
			break;
		}

		/* get the next subchunk header */

		set_flen( 0 );
		id = getU4( fp );
		sz = getU2( fp );
		if ( 6 != get_flen() ) {
			goto Fail;
		}
	}

	return env;

Fail:
	lwFreeEnvelope( env );
	return NULL;
}


/*
   ======================================================================
   lwFindEnvelope()

   Returns an lwEnvelope pointer, given an envelope index.
   ====================================================================== */

lwEnvelope *lwFindEnvelope( lwEnvelope *list, int index ){
	lwEnvelope *env;

	env = list;
	while ( env ) {
		if ( env->index == index ) {
			break;
		}
		env = env->next;
	}
	return env;
}


/*
   ======================================================================
   range()

   Given the value v of a periodic function, returns the equivalent value
   v2 in the principal interval [lo, hi].  If i isn't NULL, it receives
   the number of wavelengths between v and v2.

   v2 = v - i * (hi - lo)

   For example, range( 3 pi, 0, 2 pi, i ) returns pi, with i = 1.
   ====================================================================== */

static float range( float v, float lo, float hi, int *i ){
	float v2, r = hi - lo;

	if ( r == 0.0 ) {
		if ( i ) {
			*i = 0;
		}
		return lo;
	}

	v2 = lo + v - r * ( float ) floor( ( double ) v / r );
	if ( i ) {
		*i = -( int )( ( v2 - v ) / r + ( v2 > v ? 0.5 : -0.5 ) );
	}

	return v2;
}


/*
   ======================================================================
   hermite()

   Calculate the Hermite coefficients.
   ====================================================================== */

static void hermite( float t, float *h1, float *h2, float *h3, float *h4 ){
	float t2, t3;

	t2 = t * t;
	t3 = t * t2;

	*h2 = 3.0f * t2 - t3 - t3;
	*h1 = 1.0f - *h2;
	*h4 = t3 - t2;
	*h3 = *h4 - t2 + t;
}


/*
   ======================================================================
   bezier()

   Interpolate the value of a 1D Bezier curve.
   ====================================================================== */

static float bezier( float x0, float x1, float x2, float x3, float t ){
	float a, b, c, t2, t3;

	t2 = t * t;
	t3 = t2 * t;

	c = 3.0f * ( x1 - x0 );
	b = 3.0f * ( x2 - x1 ) - c;
	a = x3 - x0 - c - b;

	return a * t3 + b * t2 + c * t + x0;
}


/*
   ======================================================================
   bez2_time()

   Find the t for which bezier() returns the input time.  The handle
   endpoints of a BEZ2 curve represent the control points, and these have
   (time, value) coordinates, so time is used as both a coordinate and a
   parameter for this curve type.
   ====================================================================== */

static float bez2_time( float x0, float x1, float x2, float x3, float time,
						float *t0, float *t1 ){
	float v, t;

	t = *t0 + ( *t1 - *t0 ) * 0.5f;
	v = bezier( x0, x1, x2, x3, t );
	if ( fabs( time - v ) > .0001f ) {
		if ( v > time ) {
			*t1 = t;
		}
		else{
			*t0 = t;
		}
		return bez2_time( x0, x1, x2, x3, time, t0, t1 );
	}
	else{
		return t;
	}
}


/*
   ======================================================================
   bez2()

   Interpolate the value of a BEZ2 curve.
   ====================================================================== */

static float bez2( lwKey *key0, lwKey *key1, float time ){
	float x, y, t, t0 = 0.0f, t1 = 1.0f;

	if ( key0->shape == ID_BEZ2 ) {
		x = key0->time + key0->param[ 2 ];
	}
	else{
		x = key0->time + ( key1->time - key0->time ) / 3.0f;
	}

	t = bez2_time( key0->time, x, key1->time + key1->param[ 0 ], key1->time,
				   time, &t0, &t1 );

	if ( key0->shape == ID_BEZ2 ) {
		y = key0->value + key0->param[ 3 ];
	}
	else{
		y = key0->value + key0->param[ 1 ] / 3.0f;
	}

	return bezier( key0->value, y, key1->param[ 1 ] + key1->value, key1->value, t );
}


/*
   ======================================================================
   outgoing()

   Return the outgoing tangent to the curve at key0.  The value returned
   for the BEZ2 case is used when extrapolating a linear pre behavior and
   when interpolating a non-BEZ2 span.
   ====================================================================== */

static float outgoing( lwKey *key0, lwKey *key1 ){
	float a, b, d, t, out;

	switch ( key0->shape )
	{
	case ID_TCB:
		a = ( 1.0f - key0->tension )
			* ( 1.0f + key0->continuity )
			* ( 1.0f + key0->bias );
		b = ( 1.0f - key0->tension )
			* ( 1.0f - key0->continuity )
			* ( 1.0f - key0->bias );
		d = key1->value - key0->value;

		if ( key0->prev ) {
			t = ( key1->time - key0->time ) / ( key1->time - key0->prev->time );
			out = t * ( a * ( key0->value - key0->prev->value ) + b * d );
		}
		else{
			out = b * d;
		}
		break;

	case ID_LINE:
		d = key1->value - key0->value;
		if ( key0->prev ) {
			t = ( key1->time - key0->time ) / ( key1->time - key0->prev->time );
			out = t * ( key0->value - key0->prev->value + d );
		}
		else{
			out = d;
		}
		break;

	case ID_BEZI:
	case ID_HERM:
		out = key0->param[ 1 ];
		if ( key0->prev ) {
			out *= ( key1->time - key0->time ) / ( key1->time - key0->prev->time );
		}
		break;

	case ID_BEZ2:
		out = key0->param[ 3 ] * ( key1->time - key0->time );
		if ( fabs( key0->param[ 2 ] ) > 1e-5f ) {
			out /= key0->param[ 2 ];
		}
		else{
			out *= 1e5f;
		}
		break;

	case ID_STEP:
	default:
		out = 0.0f;
		break;
	}

	return out;
}


/*
   ======================================================================
   incoming()

   Return the incoming tangent to the curve at key1.  The value returned
   for the BEZ2 case is used when extrapolating a linear post behavior.
   ====================================================================== */

static float incoming( lwKey *key0, lwKey *key1 ){
	float a, b, d, t, in;

	switch ( key1->shape )
	{
	case ID_LINE:
		d = key1->value - key0->value;
		if ( key1->next ) {
			t = ( key1->time - key0->time ) / ( key1->next->time - key0->time );
			in = t * ( key1->next->value - key1->value + d );
		}
		else{
			in = d;
		}
		break;

	case ID_TCB:
		a = ( 1.0f - key1->tension )
			* ( 1.0f - key1->continuity )
			* ( 1.0f + key1->bias );
		b = ( 1.0f - key1->tension )
			* ( 1.0f + key1->continuity )
			* ( 1.0f - key1->bias );
		d = key1->value - key0->value;

		if ( key1->next ) {
			t = ( key1->time - key0->time ) / ( key1->next->time - key0->time );
			in = t * ( b * ( key1->next->value - key1->value ) + a * d );
		}
		else{
			in = a * d;
		}
		break;

	case ID_BEZI:
	case ID_HERM:
		in = key1->param[ 0 ];
		if ( key1->next ) {
			in *= ( key1->time - key0->time ) / ( key1->next->time - key0->time );
		}
		break;
		return in;

	case ID_BEZ2:
		in = key1->param[ 1 ] * ( key1->time - key0->time );
		if ( fabs( key1->param[ 0 ] ) > 1e-5f ) {
			in /= key1->param[ 0 ];
		}
		else{
			in *= 1e5f;
		}
		break;

	case ID_STEP:
	default:
		in = 0.0f;
		break;
	}

	return in;
}


/*
   ======================================================================
   evalEnvelope()

   Given a list of keys and a time, returns the interpolated value of the
   envelope at that time.
   ====================================================================== */

float evalEnvelope( lwEnvelope *env, float time ){
	lwKey *key0, *key1, *skey, *ekey;
	float t, h1, h2, h3, h4, in, out, offset = 0.0f;
	int noff;


	/* if there's no key, the value is 0 */

	if ( env->nkeys == 0 ) {
		return 0.0f;
	}

	/* if there's only one key, the value is constant */

	if ( env->nkeys == 1 ) {
		return env->key->value;
	}

	/* find the first and last keys */

	skey = ekey = env->key;
	while ( ekey->next ) ekey = ekey->next;

	/* use pre-behavior if time is before first key time */

	if ( time < skey->time ) {
		switch ( env->behavior[ 0 ] )
		{
		case BEH_RESET:
			return 0.0f;

		case BEH_CONSTANT:
			return skey->value;

		case BEH_REPEAT:
			time = range( time, skey->time, ekey->time, NULL );
			break;

		case BEH_OSCILLATE:
			time = range( time, skey->time, ekey->time, &noff );
			if ( noff % 2 ) {
				time = ekey->time - skey->time - time;
			}
			break;

		case BEH_OFFSET:
			time = range( time, skey->time, ekey->time, &noff );
			offset = noff * ( ekey->value - skey->value );
			break;

		case BEH_LINEAR:
			out = outgoing( skey, skey->next )
				  / ( skey->next->time - skey->time );
			return out * ( time - skey->time ) + skey->value;
		}
	}

	/* use post-behavior if time is after last key time */

	else if ( time > ekey->time ) {
		switch ( env->behavior[ 1 ] )
		{
		case BEH_RESET:
			return 0.0f;

		case BEH_CONSTANT:
			return ekey->value;

		case BEH_REPEAT:
			time = range( time, skey->time, ekey->time, NULL );
			break;

		case BEH_OSCILLATE:
			time = range( time, skey->time, ekey->time, &noff );
			if ( noff % 2 ) {
				time = ekey->time - skey->time - time;
			}
			break;

		case BEH_OFFSET:
			time = range( time, skey->time, ekey->time, &noff );
			offset = noff * ( ekey->value - skey->value );
			break;

		case BEH_LINEAR:
			in = incoming( ekey->prev, ekey )
				 / ( ekey->time - ekey->prev->time );
			return in * ( time - ekey->time ) + ekey->value;
		}
	}

	/* get the endpoints of the interval being evaluated */

	key0 = env->key;
	while ( time > key0->next->time )
		key0 = key0->next;
	key1 = key0->next;

	/* check for singularities first */

	if ( time == key0->time ) {
		return key0->value + offset;
	}
	else if ( time == key1->time ) {
		return key1->value + offset;
	}

	/* get interval length, time in [0, 1] */

	t = ( time - key0->time ) / ( key1->time - key0->time );

	/* interpolate */

	switch ( key1->shape )
	{
	case ID_TCB:
	case ID_BEZI:
	case ID_HERM:
		out = outgoing( key0, key1 );
		in = incoming( key0, key1 );
		hermite( t, &h1, &h2, &h3, &h4 );
		return h1 * key0->value + h2 * key1->value + h3 * out + h4 * in + offset;

	case ID_BEZ2:
		return bez2( key0, key1, time ) + offset;

	case ID_LINE:
		return key0->value + t * ( key1->value - key0->value ) + offset;

	case ID_STEP:
		return key0->value + offset;

	default:
		return offset;
	}
}
