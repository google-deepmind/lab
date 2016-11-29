/*
   ======================================================================
   pntspols.c

   Point and polygon functions for an LWO2 reader.

   Ernie Wright  17 Sep 00
   ====================================================================== */

#include "../picointernal.h"
#include "lwo2.h"


/*
   ======================================================================
   lwFreePoints()

   Free the memory used by an lwPointList.
   ====================================================================== */

void lwFreePoints( lwPointList *point ){
	int i;

	if ( point ) {
		if ( point->pt ) {
			for ( i = 0; i < point->count; i++ ) {
				if ( point->pt[ i ].pol ) {
					_pico_free( point->pt[ i ].pol );
				}
				if ( point->pt[ i ].vm ) {
					_pico_free( point->pt[ i ].vm );
				}
			}
			_pico_free( point->pt );
		}
		memset( point, 0, sizeof( lwPointList ) );
	}
}


/*
   ======================================================================
   lwFreePolygons()

   Free the memory used by an lwPolygonList.
   ====================================================================== */

void lwFreePolygons( lwPolygonList *plist ){
	int i, j;

	if ( plist ) {
		if ( plist->pol ) {
			for ( i = 0; i < plist->count; i++ ) {
				if ( plist->pol[ i ].v ) {
					for ( j = 0; j < plist->pol[ i ].nverts; j++ )
						if ( plist->pol[ i ].v[ j ].vm ) {
							_pico_free( plist->pol[ i ].v[ j ].vm );
						}
				}
			}
			if ( plist->pol[ 0 ].v ) {
				_pico_free( plist->pol[ 0 ].v );
			}
			_pico_free( plist->pol );
		}
		memset( plist, 0, sizeof( lwPolygonList ) );
	}
}


/*
   ======================================================================
   lwGetPoints()

   Read point records from a PNTS chunk in an LWO2 file.  The points are
   added to the array in the lwPointList.
   ====================================================================== */

int lwGetPoints( picoMemStream_t *fp, int cksize, lwPointList *point ){
	float *f;
	int np, i, j;

	if ( cksize == 1 ) {
		return 1;
	}

	/* extend the point array to hold the new points */

	np = cksize / 12;
	point->offset = point->count;
	point->count += np;
	if ( !_pico_realloc( (void *) &point->pt, ( point->count - np ) * sizeof( lwPoint ), point->count * sizeof( lwPoint ) ) ) {
		return 0;
	}
	memset( &point->pt[ point->offset ], 0, np * sizeof( lwPoint ) );

	/* read the whole chunk */

	f = ( float * ) getbytes( fp, cksize );
	if ( !f ) {
		return 0;
	}
	revbytes( f, 4, np * 3 );

	/* assign position values */

	for ( i = 0, j = 0; i < np; i++, j += 3 ) {
		point->pt[ i ].pos[ 0 ] = f[ j ];
		point->pt[ i ].pos[ 1 ] = f[ j + 1 ];
		point->pt[ i ].pos[ 2 ] = f[ j + 2 ];
	}

	_pico_free( f );
	return 1;
}


/*
   ======================================================================
   lwGetBoundingBox()

   Calculate the bounding box for a point list, but only if the bounding
   box hasn't already been initialized.
   ====================================================================== */

void lwGetBoundingBox( lwPointList *point, float bbox[] ){
	int i, j;

	if ( point->count == 0 ) {
		return;
	}

	for ( i = 0; i < 6; i++ )
		if ( bbox[ i ] != 0.0f ) {
			return;
		}

	bbox[ 0 ] = bbox[ 1 ] = bbox[ 2 ] = 1e20f;
	bbox[ 3 ] = bbox[ 4 ] = bbox[ 5 ] = -1e20f;
	for ( i = 0; i < point->count; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			if ( bbox[ j ] > point->pt[ i ].pos[ j ] ) {
				bbox[ j ] = point->pt[ i ].pos[ j ];
			}
			if ( bbox[ j + 3 ] < point->pt[ i ].pos[ j ] ) {
				bbox[ j + 3 ] = point->pt[ i ].pos[ j ];
			}
		}
	}
}


/*
   ======================================================================
   lwAllocPolygons()

   Allocate or extend the polygon arrays to hold new records.
   ====================================================================== */

int lwAllocPolygons( lwPolygonList *plist, int npols, int nverts ){
	int i;

	plist->offset = plist->count;
	plist->count += npols;
	if ( !_pico_realloc( (void *) &plist->pol, ( plist->count - npols ) * sizeof( lwPolygon ), plist->count * sizeof( lwPolygon ) ) ) {
		return 0;
	}
	memset( plist->pol + plist->offset, 0, npols * sizeof( lwPolygon ) );

	plist->voffset = plist->vcount;
	plist->vcount += nverts;
	if ( !_pico_realloc( (void *) &plist->pol[ 0 ].v, ( plist->vcount - nverts ) * sizeof( lwPolVert ), plist->vcount * sizeof( lwPolVert ) ) ) {
		return 0;
	}
	memset( plist->pol[ 0 ].v + plist->voffset, 0, nverts * sizeof( lwPolVert ) );

	/* fix up the old vertex pointers */

	for ( i = 1; i < plist->offset; i++ )
		plist->pol[ i ].v = plist->pol[ i - 1 ].v + plist->pol[ i - 1 ].nverts;

	return 1;
}


/*
   ======================================================================
   lwGetPolygons()

   Read polygon records from a POLS chunk in an LWO2 file.  The polygons
   are added to the array in the lwPolygonList.
   ====================================================================== */

int lwGetPolygons( picoMemStream_t *fp, int cksize, lwPolygonList *plist, int ptoffset ){
	lwPolygon *pp;
	lwPolVert *pv;
	unsigned char *buf, *bp;
	int i, j, flags, nv, nverts, npols;
	unsigned int type;


	if ( cksize == 0 ) {
		return 1;
	}

	/* read the whole chunk */

	set_flen( 0 );
	type = getU4( fp );
	buf = getbytes( fp, cksize - 4 );
	if ( cksize != get_flen() ) {
		goto Fail;
	}

	/* count the polygons and vertices */

	nverts = 0;
	npols = 0;
	bp = buf;

	while ( bp < buf + cksize - 4 ) {
		nv = sgetU2( &bp );
		nv &= 0x03FF;
		nverts += nv;
		npols++;
		for ( i = 0; i < nv; i++ )
			j = sgetVX( &bp );
	}

	if ( !lwAllocPolygons( plist, npols, nverts ) ) {
		goto Fail;
	}

	/* fill in the new polygons */

	bp = buf;
	pp = plist->pol + plist->offset;
	pv = plist->pol[ 0 ].v + plist->voffset;

	for ( i = 0; i < npols; i++ ) {
		nv = sgetU2( &bp );
		flags = nv & 0xFC00;
		nv &= 0x03FF;

		pp->nverts = nv;
		pp->flags = flags;
		pp->type = type;
		if ( !pp->v ) {
			pp->v = pv;
		}
		for ( j = 0; j < nv; j++ )
			pp->v[ j ].index = sgetVX( &bp ) + ptoffset;

		pp++;
		pv += nv;
	}

	_pico_free( buf );
	return 1;

Fail:
	if ( buf ) {
		_pico_free( buf );
	}
	lwFreePolygons( plist );
	return 0;
}


/*
   ======================================================================
   lwGetPolyNormals()

   Calculate the polygon normals.  By convention, LW's polygon normals
   are found as the cross product of the first and last edges.  It's
   undefined for one- and two-point polygons.
   ====================================================================== */

void lwGetPolyNormals( lwPointList *point, lwPolygonList *polygon ){
	int i, j;
	float p1[ 3 ], p2[ 3 ], pn[ 3 ], v1[ 3 ], v2[ 3 ];

	for ( i = 0; i < polygon->count; i++ ) {
		if ( polygon->pol[ i ].nverts < 3 ) {
			continue;
		}
		for ( j = 0; j < 3; j++ ) {
			p1[ j ] = point->pt[ polygon->pol[ i ].v[ 0 ].index ].pos[ j ];
			p2[ j ] = point->pt[ polygon->pol[ i ].v[ 1 ].index ].pos[ j ];
			pn[ j ] = point->pt[ polygon->pol[ i ].v[
									 polygon->pol[ i ].nverts - 1 ].index ].pos[ j ];
		}

		for ( j = 0; j < 3; j++ ) {
			v1[ j ] = p2[ j ] - p1[ j ];
			v2[ j ] = pn[ j ] - p1[ j ];
		}

		cross( v1, v2, polygon->pol[ i ].norm );
		normalize( polygon->pol[ i ].norm );
	}
}


/*
   ======================================================================
   lwGetPointPolygons()

   For each point, fill in the indexes of the polygons that share the
   point.  Returns 0 if any of the memory allocations fail, otherwise
   returns 1.
   ====================================================================== */

int lwGetPointPolygons( lwPointList *point, lwPolygonList *polygon ){
	int i, j, k;

	/* count the number of polygons per point */

	for ( i = 0; i < polygon->count; i++ )
		for ( j = 0; j < polygon->pol[ i ].nverts; j++ )
			++point->pt[ polygon->pol[ i ].v[ j ].index ].npols;

	/* alloc per-point polygon arrays */

	for ( i = 0; i < point->count; i++ ) {
		if ( point->pt[ i ].npols == 0 ) {
			continue;
		}
		point->pt[ i ].pol = _pico_calloc( point->pt[ i ].npols, sizeof( int ) );
		if ( !point->pt[ i ].pol ) {
			return 0;
		}
		point->pt[ i ].npols = 0;
	}

	/* fill in polygon array for each point */

	for ( i = 0; i < polygon->count; i++ ) {
		for ( j = 0; j < polygon->pol[ i ].nverts; j++ ) {
			k = polygon->pol[ i ].v[ j ].index;
			point->pt[ k ].pol[ point->pt[ k ].npols ] = i;
			++point->pt[ k ].npols;
		}
	}

	return 1;
}


/*
   ======================================================================
   lwResolvePolySurfaces()

   Convert tag indexes into actual lwSurface pointers.  If any polygons
   point to tags for which no corresponding surface can be found, a
   default surface is created.
   ====================================================================== */

int lwResolvePolySurfaces( lwPolygonList *polygon, lwTagList *tlist,
						   lwSurface **surf, int *nsurfs ){
	lwSurface **s, *st;
	int i, index;

	if ( tlist->count == 0 ) {
		return 1;
	}

	s = _pico_calloc( tlist->count, sizeof( lwSurface * ) );
	if ( !s ) {
		return 0;
	}

	for ( i = 0; i < tlist->count; i++ ) {
		st = *surf;
		while ( st ) {
			if ( !strcmp( st->name, tlist->tag[ i ] ) ) {
				s[ i ] = st;
				break;
			}
			st = st->next;
		}
	}

	for ( i = 0; i < polygon->count; i++ ) {
		index = ( int ) ( (size_t)polygon->pol[ i ].surf );
		if ( index < 0 || index > tlist->count ) {
			return 0;
		}
		if ( !s[ index ] ) {
			s[ index ] = lwDefaultSurface();
			if ( !s[ index ] ) {
				return 0;
			}
			s[ index ]->name = _pico_alloc( strlen( tlist->tag[ index ] ) + 1 );
			if ( !s[ index ]->name ) {
				return 0;
			}
			strcpy( s[ index ]->name, tlist->tag[ index ] );
			lwListAdd( (void **) surf, s[ index ] );
			*nsurfs = *nsurfs + 1;
		}
		polygon->pol[ i ].surf = s[ index ];
	}

	_pico_free( s );
	return 1;
}


/*
   ======================================================================
   lwGetVertNormals()

   Calculate the vertex normals.  For each polygon vertex, sum the
   normals of the polygons that share the point.  If the normals of the
   current and adjacent polygons form an angle greater than the max
   smoothing angle for the current polygon's surface, the normal of the
   adjacent polygon is excluded from the sum.  It's also excluded if the
   polygons aren't in the same smoothing group.

   Assumes that lwGetPointPolygons(), lwGetPolyNormals() and
   lwResolvePolySurfaces() have already been called.
   ====================================================================== */

void lwGetVertNormals( lwPointList *point, lwPolygonList *polygon ){
	int j, k, n, g, h, p;
	float a;

	for ( j = 0; j < polygon->count; j++ ) {
		for ( n = 0; n < polygon->pol[ j ].nverts; n++ ) {
			for ( k = 0; k < 3; k++ )
				polygon->pol[ j ].v[ n ].norm[ k ] = polygon->pol[ j ].norm[ k ];

			if ( polygon->pol[ j ].surf->smooth <= 0 ) {
				continue;
			}

			p = polygon->pol[ j ].v[ n ].index;

			for ( g = 0; g < point->pt[ p ].npols; g++ ) {
				h = point->pt[ p ].pol[ g ];
				if ( h == j ) {
					continue;
				}

				if ( polygon->pol[ j ].smoothgrp != polygon->pol[ h ].smoothgrp ) {
					continue;
				}
				a = vecangle( polygon->pol[ j ].norm, polygon->pol[ h ].norm );
				if ( a > polygon->pol[ j ].surf->smooth ) {
					continue;
				}

				for ( k = 0; k < 3; k++ )
					polygon->pol[ j ].v[ n ].norm[ k ] += polygon->pol[ h ].norm[ k ];
			}

			normalize( polygon->pol[ j ].v[ n ].norm );
		}
	}
}


/*
   ======================================================================
   lwFreeTags()

   Free memory used by an lwTagList.
   ====================================================================== */

void lwFreeTags( lwTagList *tlist ){
	int i;

	if ( tlist ) {
		if ( tlist->tag ) {
			for ( i = 0; i < tlist->count; i++ )
				if ( tlist->tag[ i ] ) {
					_pico_free( tlist->tag[ i ] );
				}
			_pico_free( tlist->tag );
		}
		memset( tlist, 0, sizeof( lwTagList ) );
	}
}


/*
   ======================================================================
   lwGetTags()

   Read tag strings from a TAGS chunk in an LWO2 file.  The tags are
   added to the lwTagList array.
   ====================================================================== */

int lwGetTags( picoMemStream_t *fp, int cksize, lwTagList *tlist ){
	char *buf, *bp;
	int i, len, ntags;

	if ( cksize == 0 ) {
		return 1;
	}

	/* read the whole chunk */

	set_flen( 0 );
	buf = getbytes( fp, cksize );
	if ( !buf ) {
		return 0;
	}

	/* count the strings */

	ntags = 0;
	bp = buf;
	while ( bp < buf + cksize ) {
		len = strlen( bp ) + 1;
		len += len & 1;
		bp += len;
		++ntags;
	}

	/* expand the string array to hold the new tags */

	tlist->offset = tlist->count;
	tlist->count += ntags;
	if ( !_pico_realloc( (void *) &tlist->tag, ( tlist->count - ntags ) * sizeof( char * ), tlist->count * sizeof( char * ) ) ) {
		goto Fail;
	}
	memset( &tlist->tag[ tlist->offset ], 0, ntags * sizeof( char * ) );

	/* copy the new tags to the tag array */

	bp = buf;
	for ( i = 0; i < ntags; i++ )
		tlist->tag[ i + tlist->offset ] = sgetS0( &bp );

	_pico_free( buf );
	return 1;

Fail:
	if ( buf ) {
		_pico_free( buf );
	}
	return 0;
}


/*
   ======================================================================
   lwGetPolygonTags()

   Read polygon tags from a PTAG chunk in an LWO2 file.
   ====================================================================== */

int lwGetPolygonTags( picoMemStream_t *fp, int cksize, lwTagList *tlist,
					  lwPolygonList *plist ){
	unsigned int type;
	int rlen = 0, i, j;

	set_flen( 0 );
	type = getU4( fp );
	rlen = get_flen();
	if ( rlen < 0 ) {
		return 0;
	}

	if ( type != ID_SURF && type != ID_PART && type != ID_SMGP ) {
		_pico_memstream_seek( fp, cksize - 4, PICO_SEEK_CUR );
		return 1;
	}

	while ( rlen < cksize ) {
		i = getVX( fp ) + plist->offset;
		j = getVX( fp ) + tlist->offset;
		rlen = get_flen();
		if ( rlen < 0 || rlen > cksize ) {
			return 0;
		}

		switch ( type ) {
		case ID_SURF:  plist->pol[ i ].surf = ( lwSurface * ) ( (size_t)j );  break;
		case ID_PART:  plist->pol[ i ].part = j;  break;
		case ID_SMGP:  plist->pol[ i ].smoothgrp = j;  break;
		}
	}

	return 1;
}
