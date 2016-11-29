/*
   ======================================================================
   vmap.c

   Vertex map functions for an LWO2 reader.

   Ernie Wright  17 Sep 00
   ====================================================================== */

#include "../picointernal.h"
#include "lwo2.h"


/*
   ======================================================================
   lwFreeVMap()

   Free memory used by an lwVMap.
   ====================================================================== */

void lwFreeVMap( lwVMap *vmap ){
	if ( vmap ) {
		if ( vmap->name ) {
			_pico_free( vmap->name );
		}
		if ( vmap->vindex ) {
			_pico_free( vmap->vindex );
		}
		if ( vmap->pindex ) {
			_pico_free( vmap->pindex );
		}
		if ( vmap->val ) {
			if ( vmap->val[ 0 ] ) {
				_pico_free( vmap->val[ 0 ] );
			}
			_pico_free( vmap->val );
		}
		_pico_free( vmap );
	}
}


/*
   ======================================================================
   lwGetVMap()

   Read an lwVMap from a VMAP or VMAD chunk in an LWO2.
   ====================================================================== */

lwVMap *lwGetVMap( picoMemStream_t *fp, int cksize, int ptoffset, int poloffset,
				   int perpoly ){
	unsigned char *buf, *bp;
	lwVMap *vmap;
	float *f;
	int i, j, npts, rlen;


	/* read the whole chunk */

	set_flen( 0 );
	buf = getbytes( fp, cksize );
	if ( !buf ) {
		return NULL;
	}

	vmap = _pico_calloc( 1, sizeof( lwVMap ) );
	if ( !vmap ) {
		_pico_free( buf );
		return NULL;
	}

	/* initialize the vmap */

	vmap->perpoly = perpoly;

	bp = buf;
	set_flen( 0 );
	vmap->type = sgetU4( &bp );
	vmap->dim  = sgetU2( &bp );
	vmap->name = sgetS0( &bp );
	rlen = get_flen();

	/* count the vmap records */

	npts = 0;
	while ( bp < buf + cksize ) {
		i = sgetVX( &bp );
		if ( perpoly ) {
			i = sgetVX( &bp );
		}
		bp += vmap->dim * sizeof( float );
		++npts;
	}

	/* allocate the vmap */

	vmap->nverts = npts;
	vmap->vindex = _pico_calloc( npts, sizeof( int ) );
	if ( !vmap->vindex ) {
		goto Fail;
	}
	if ( perpoly ) {
		vmap->pindex = _pico_calloc( npts, sizeof( int ) );
		if ( !vmap->pindex ) {
			goto Fail;
		}
	}

	if ( vmap->dim > 0 ) {
		vmap->val = _pico_calloc( npts, sizeof( float * ) );
		if ( !vmap->val ) {
			goto Fail;
		}
		f = _pico_alloc( npts * vmap->dim * sizeof( float ) );
		if ( !f ) {
			goto Fail;
		}
		for ( i = 0; i < npts; i++ )
			vmap->val[ i ] = f + i * vmap->dim;
	}

	/* fill in the vmap values */

	bp = buf + rlen;
	for ( i = 0; i < npts; i++ ) {
		vmap->vindex[ i ] = sgetVX( &bp );
		if ( perpoly ) {
			vmap->pindex[ i ] = sgetVX( &bp );
		}
		for ( j = 0; j < vmap->dim; j++ )
			vmap->val[ i ][ j ] = sgetF4( &bp );
	}

	_pico_free( buf );
	return vmap;

Fail:
	if ( buf ) {
		_pico_free( buf );
	}
	lwFreeVMap( vmap );
	return NULL;
}


/*
   ======================================================================
   lwGetPointVMaps()

   Fill in the lwVMapPt structure for each point.
   ====================================================================== */

int lwGetPointVMaps( lwPointList *point, lwVMap *vmap ){
	lwVMap *vm;
	int i, j, n;

	/* count the number of vmap values for each point */

	vm = vmap;
	while ( vm ) {
		if ( !vm->perpoly ) {
			for ( i = 0; i < vm->nverts; i++ )
				++point->pt[ vm->vindex[ i ]].nvmaps;
		}
		vm = vm->next;
	}

	/* allocate vmap references for each mapped point */

	for ( i = 0; i < point->count; i++ ) {
		if ( point->pt[ i ].nvmaps ) {
			point->pt[ i ].vm = _pico_calloc( point->pt[ i ].nvmaps, sizeof( lwVMapPt ) );
			if ( !point->pt[ i ].vm ) {
				return 0;
			}
			point->pt[ i ].nvmaps = 0;
		}
	}

	/* fill in vmap references for each mapped point */

	vm = vmap;
	while ( vm ) {
		if ( !vm->perpoly ) {
			for ( i = 0; i < vm->nverts; i++ ) {
				j = vm->vindex[ i ];
				n = point->pt[ j ].nvmaps;
				point->pt[ j ].vm[ n ].vmap = vm;
				point->pt[ j ].vm[ n ].index = i;
				++point->pt[ j ].nvmaps;
			}
		}
		vm = vm->next;
	}

	return 1;
}


/*
   ======================================================================
   lwGetPolyVMaps()

   Fill in the lwVMapPt structure for each polygon vertex.
   ====================================================================== */

int lwGetPolyVMaps( lwPolygonList *polygon, lwVMap *vmap ){
	lwVMap *vm;
	lwPolVert *pv;
	int i, j;

	/* count the number of vmap values for each polygon vertex */

	vm = vmap;
	while ( vm ) {
		if ( vm->perpoly ) {
			for ( i = 0; i < vm->nverts; i++ ) {
				for ( j = 0; j < polygon->pol[ vm->pindex[ i ]].nverts; j++ ) {
					pv = &polygon->pol[ vm->pindex[ i ]].v[ j ];
					if ( vm->vindex[ i ] == pv->index ) {
						++pv->nvmaps;
						break;
					}
				}
			}
		}
		vm = vm->next;
	}

	/* allocate vmap references for each mapped vertex */

	for ( i = 0; i < polygon->count; i++ ) {
		for ( j = 0; j < polygon->pol[ i ].nverts; j++ ) {
			pv = &polygon->pol[ i ].v[ j ];
			if ( pv->nvmaps ) {
				pv->vm = _pico_calloc( pv->nvmaps, sizeof( lwVMapPt ) );
				if ( !pv->vm ) {
					return 0;
				}
				pv->nvmaps = 0;
			}
		}
	}

	/* fill in vmap references for each mapped point */

	vm = vmap;
	while ( vm ) {
		if ( vm->perpoly ) {
			for ( i = 0; i < vm->nverts; i++ ) {
				for ( j = 0; j < polygon->pol[ vm->pindex[ i ]].nverts; j++ ) {
					pv = &polygon->pol[ vm->pindex[ i ]].v[ j ];
					if ( vm->vindex[ i ] == pv->index ) {
						pv->vm[ pv->nvmaps ].vmap = vm;
						pv->vm[ pv->nvmaps ].index = i;
						++pv->nvmaps;
						break;
					}
				}
			}
		}
		vm = vm->next;
	}

	return 1;
}
