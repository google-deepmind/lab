/* -----------------------------------------------------------------------------

   PicoModel Library

   Copyright (c) 2002, Randy Reddig & seaw0lf
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
#define PM_LWO_C

/* dependencies */
#include "picointernal.h"
#include "lwo/lwo2.h"

/* uncomment when debugging this module */
/*#define DEBUG_PM_LWO*/

#ifdef DEBUG_PM_LWO
#include "time.h"
#endif

/* helper functions */
static const char *lwo_lwIDToStr( unsigned int lwID ){
	static char lwIDStr[5];

	if ( !lwID ) {
		return "n/a";
	}

	lwIDStr[ 0 ] = (char)( ( lwID ) >> 24 );
	lwIDStr[ 1 ] = (char)( ( lwID ) >> 16 );
	lwIDStr[ 2 ] = (char)( ( lwID ) >> 8 );
	lwIDStr[ 3 ] = (char)( ( lwID ) );
	lwIDStr[ 4 ] = '\0';

	return lwIDStr;
}

/*
   _lwo_canload()
   validates a LightWave Object model file. btw, i use the
   preceding underscore cause it's a static func referenced
   by one structure only.
 */
static int _lwo_canload( PM_PARAMS_CANLOAD ){
	picoMemStream_t *s;
	unsigned int failID = 0;
	int failpos = -1;
	int ret;

	/* create a new pico memorystream */
	s = _pico_new_memstream( (picoByte_t *)buffer, bufSize );
	if ( s == NULL ) {
		return PICO_PMV_ERROR_MEMORY;
	}

	ret = lwValidateObject( fileName, s, &failID, &failpos );

	_pico_free_memstream( s );

	return ret;
}

/*
   _lwo_load()
   loads a LightWave Object model file.
 */
static picoModel_t *_lwo_load( PM_PARAMS_LOAD ){
	picoMemStream_t *s;
	unsigned int failID = 0;
	int failpos = -1;
	lwObject        *obj;
	lwSurface       *surface;
	lwLayer         *layer;
	lwPoint         *pt;
	lwPolygon       *pol;
	lwPolVert       *v;
	lwVMapPt        *vm;
	char name[ 256 ];
	int i, j, k, numverts;

	picoModel_t     *picoModel;
	picoSurface_t   *picoSurface;
	picoShader_t    *picoShader;
	picoVec3_t xyz, normal;
	picoVec2_t st;
	picoColor_t color;

	int defaultSTAxis[ 2 ];
	picoVec2_t defaultXYZtoSTScale;

	picoVertexCombinationHash_t **hashTable;
	picoVertexCombinationHash_t *vertexCombinationHash;

#ifdef DEBUG_PM_LWO
	clock_t load_start, load_finish, convert_start, convert_finish;
	double load_elapsed, convert_elapsed;

	load_start = clock();
#endif

	/* do frame check */
	if ( frameNum < 0 || frameNum >= 1 ) {
		_pico_printf( PICO_ERROR, "Invalid or out-of-range LWO frame specified" );
		return NULL;
	}

	/* create a new pico memorystream */
	s = _pico_new_memstream( (picoByte_t *)buffer, bufSize );
	if ( s == NULL ) {
		return NULL;
	}

	obj = lwGetObject( fileName, s, &failID, &failpos );

	_pico_free_memstream( s );

	if ( !obj ) {
		_pico_printf( PICO_ERROR, "Couldn't load LWO file, failed on ID '%s', position %d", lwo_lwIDToStr( failID ), failpos );
		return NULL;
	}

#ifdef DEBUG_PM_LWO
	convert_start = load_finish = clock();
	load_elapsed = (double)( load_finish - load_start ) / CLOCKS_PER_SEC;
#endif

	/* -------------------------------------------------
	   pico model creation
	   ------------------------------------------------- */

	/* create a new pico model */
	picoModel = PicoNewModel();
	if ( picoModel == NULL ) {
		_pico_printf( PICO_ERROR, "Unable to allocate a new model" );
		return NULL;
	}

	/* do model setup */
	PicoSetModelFrameNum( picoModel, frameNum );
	PicoSetModelNumFrames( picoModel, 1 );
	PicoSetModelName( picoModel, fileName );
	PicoSetModelFileName( picoModel, fileName );

	/* create all polygons from layer[ 0 ] that belong to this surface */
	layer = &obj->layer[0];

	/* warn the user that other layers are discarded */
	if ( obj->nlayers > 1 ) {
		_pico_printf( PICO_WARNING, "LWO loader discards any geometry data not in Layer 1 (%d layers found)", obj->nlayers );
	}

	/* initialize dummy normal */
	normal[ 0 ] = normal[ 1 ] = normal[ 2 ] = 0.f;

	/* setup default st map */
	st[ 0 ] = st[ 1 ] = 0.f;    /* st[0] holds max, st[1] holds max par one */
	defaultSTAxis[ 0 ] = 0;
	defaultSTAxis[ 1 ] = 1;
	for ( i = 0; i < 3; i++ )
	{
		float min = layer->bbox[ i ];
		float max = layer->bbox[ i + 3 ];
		float size = max - min;

		if ( size > st[ 0 ] ) {
			defaultSTAxis[ 1 ] = defaultSTAxis[ 0 ];
			defaultSTAxis[ 0 ] = i;

			st[ 1 ] = st[ 0 ];
			st[ 0 ] = size;
		}
		else if ( size > st[ 1 ] ) {
			defaultSTAxis[ 1 ] = i;
			st[ 1 ] = size;
		}
	}
	defaultXYZtoSTScale[ 0 ] = 4.f / st[ 0 ];
	defaultXYZtoSTScale[ 1 ] = 4.f / st[ 1 ];

	/* LWO surfaces become pico surfaces */
	surface = obj->surf;
	while ( surface )
	{
		/* allocate new pico surface */
		picoSurface = PicoNewSurface( picoModel );
		if ( picoSurface == NULL ) {
			_pico_printf( PICO_ERROR, "Unable to allocate a new model surface" );
			PicoFreeModel( picoModel );
			lwFreeObject( obj );
			return NULL;
		}

		/* LWO model surfaces are all triangle meshes */
		PicoSetSurfaceType( picoSurface, PICO_TRIANGLES );

		/* set surface name */
		PicoSetSurfaceName( picoSurface, surface->name );

		/* create new pico shader */
		picoShader = PicoNewShader( picoModel );
		if ( picoShader == NULL ) {
			_pico_printf( PICO_ERROR, "Unable to allocate a new model shader" );
			PicoFreeModel( picoModel );
			lwFreeObject( obj );
			return NULL;
		}

		/* detox and set shader name */
		strncpy( name, surface->name, sizeof( name ) );
		_pico_first_token( name );
		_pico_setfext( name, "" );
		_pico_unixify( name );
		PicoSetShaderName( picoShader, name );

		/* associate current surface with newly created shader */
		PicoSetSurfaceShader( picoSurface, picoShader );

		/* copy indices and vertex data */
		numverts = 0;

		hashTable = PicoNewVertexCombinationHashTable();

		if ( hashTable == NULL ) {
			_pico_printf( PICO_ERROR, "Unable to allocate hash table" );
			PicoFreeModel( picoModel );
			lwFreeObject( obj );
			return NULL;
		}

		for ( i = 0, pol = layer->polygon.pol; i < layer->polygon.count; i++, pol++ )
		{
			/* does this polygon belong to this surface? */
			if ( pol->surf != surface ) {
				continue;
			}

			/* we only support polygons of the FACE type */
			if ( pol->type != ID_FACE ) {
				_pico_printf( PICO_WARNING, "LWO loader discarded a polygon because it's type != FACE (%s)", lwo_lwIDToStr( pol->type ) );
				continue;
			}

			/* NOTE: LWO has support for non-convex polygons, do we want to store them as well? */
			if ( pol->nverts != 3 ) {
				_pico_printf( PICO_WARNING, "LWO loader discarded a polygon because it has != 3 verts (%d)", pol->nverts );
				continue;
			}

			for ( j = 0, v = pol->v; j < 3; j++, v++ )
			{
				pt = &layer->point.pt[ v->index ];

				/* setup data */
				xyz[ 0 ] = pt->pos[ 0 ];
				xyz[ 1 ] = pt->pos[ 2 ];
				xyz[ 2 ] = pt->pos[ 1 ];

///* doom3 lwo data doesn't seem to have smoothing-angle information */
//#if 0
				if ( surface->smooth <= 0 ) {
					/* use face normals */
				normal[ 0 ] = v->norm[ 0 ];
				normal[ 1 ] = v->norm[ 2 ];
				normal[ 2 ] = v->norm[ 1 ];
				}
				else
//#endif
				{
					/* smooth normals later */
					normal[ 0 ] = 0;
					normal[ 1 ] = 0;
					normal[ 2 ] = 0;
				}

				st[ 0 ] = xyz[ defaultSTAxis[ 0 ] ] * defaultXYZtoSTScale[ 0 ];
				st[ 1 ] = xyz[ defaultSTAxis[ 1 ] ] * defaultXYZtoSTScale[ 1 ];

				color[ 0 ] = (picoByte_t)( surface->color.rgb[ 0 ] * surface->diffuse.val * 0xFF );
				color[ 1 ] = (picoByte_t)( surface->color.rgb[ 1 ] * surface->diffuse.val * 0xFF );
				color[ 2 ] = (picoByte_t)( surface->color.rgb[ 2 ] * surface->diffuse.val * 0xFF );
				color[ 3 ] = 0xFF;

				/* set from points */
				for ( k = 0, vm = pt->vm; k < pt->nvmaps; k++, vm++ )
				{
					if ( vm->vmap->type == LWID_( 'T','X','U','V' ) ) {
						/* set st coords */
						st[ 0 ] = vm->vmap->val[ vm->index ][ 0 ];
						st[ 1 ] = 1.f - vm->vmap->val[ vm->index ][ 1 ];
					}
					else if ( vm->vmap->type == LWID_( 'R','G','B','A' ) ) {
						/* set rgba */
						color[ 0 ] = (picoByte_t)( vm->vmap->val[ vm->index ][ 0 ] * surface->color.rgb[ 0 ] * surface->diffuse.val * 0xFF );
						color[ 1 ] = (picoByte_t)( vm->vmap->val[ vm->index ][ 1 ] * surface->color.rgb[ 1 ] * surface->diffuse.val * 0xFF );
						color[ 2 ] = (picoByte_t)( vm->vmap->val[ vm->index ][ 2 ] * surface->color.rgb[ 2 ] * surface->diffuse.val * 0xFF );
						color[ 3 ] = (picoByte_t)( vm->vmap->val[ vm->index ][ 3 ] * 0xFF );
					}
				}

				/* override with polygon data */
				for ( k = 0, vm = v->vm; k < v->nvmaps; k++, vm++ )
				{
					if ( vm->vmap->type == LWID_( 'T','X','U','V' ) ) {
						/* set st coords */
						st[ 0 ] = vm->vmap->val[ vm->index ][ 0 ];
						st[ 1 ] = 1.f - vm->vmap->val[ vm->index ][ 1 ];
					}
					else if ( vm->vmap->type == LWID_( 'R','G','B','A' ) ) {
						/* set rgba */
						color[ 0 ] = (picoByte_t)( vm->vmap->val[ vm->index ][ 0 ] * surface->color.rgb[ 0 ] * surface->diffuse.val * 0xFF );
						color[ 1 ] = (picoByte_t)( vm->vmap->val[ vm->index ][ 1 ] * surface->color.rgb[ 1 ] * surface->diffuse.val * 0xFF );
						color[ 2 ] = (picoByte_t)( vm->vmap->val[ vm->index ][ 2 ] * surface->color.rgb[ 2 ] * surface->diffuse.val * 0xFF );
						color[ 3 ] = (picoByte_t)( vm->vmap->val[ vm->index ][ 3 ] * 0xFF );
					}
				}

				/* find vertex in this surface and if we can't find it there create it */
				vertexCombinationHash = PicoFindVertexCombinationInHashTable( hashTable, xyz, normal, st, color );

				if ( vertexCombinationHash ) {
					/* found an existing one */
					PicoSetSurfaceIndex( picoSurface, ( i * 3 + j ), vertexCombinationHash->index );
				}
				else
				{
					/* it is a new one */
					vertexCombinationHash = PicoAddVertexCombinationToHashTable( hashTable, xyz, normal, st, color, (picoIndex_t) numverts );

					if ( vertexCombinationHash == NULL ) {
						_pico_printf( PICO_ERROR, "Unable to allocate hash bucket entry table" );
						PicoFreeVertexCombinationHashTable( hashTable );
						PicoFreeModel( picoModel );
						lwFreeObject( obj );
						return NULL;
					}

					/* add the vertex to this surface */
					PicoSetSurfaceXYZ( picoSurface, numverts, xyz );

					/* set dummy normal */
					PicoSetSurfaceNormal( picoSurface, numverts, normal );

					/* set color */
					PicoSetSurfaceColor( picoSurface, 0, numverts, color );

					/* set st coords */
					PicoSetSurfaceST( picoSurface, 0, numverts, st );

					/* set index */
					PicoSetSurfaceIndex( picoSurface, ( i * 3 + j ), (picoIndex_t) numverts );

					numverts++;
				}
			}
		}

		/* free the hashtable */
		PicoFreeVertexCombinationHashTable( hashTable );

		/* get next surface */
		surface = surface->next;
	}

#ifdef DEBUG_PM_LWO
	load_start = convert_finish = clock();
#endif

	lwFreeObject( obj );

#ifdef DEBUG_PM_LWO
	load_finish = clock();
	load_elapsed += (double)( load_finish - load_start ) / CLOCKS_PER_SEC;
	convert_elapsed = (double)( convert_finish - convert_start ) / CLOCKS_PER_SEC;
	_pico_printf( PICO_NORMAL, "Loaded model in in %-.2f second(s) (loading: %-.2fs converting: %-.2fs)\n", load_elapsed + convert_elapsed, load_elapsed, convert_elapsed  );
#endif

	/* return the new pico model */
	return picoModel;
}

/* pico file format module definition */
const picoModule_t picoModuleLWO =
{
	"1.0",                      /* module version string */
	"LightWave Object",         /* module display name */
	"Arnout van Meer",          /* author's name */
	"2003 Arnout van Meer, 2000 Ernie Wright",      /* module copyright */
	{
		"lwo", NULL, NULL, NULL /* default extensions to use */
	},
	_lwo_canload,               /* validation routine */
	_lwo_load,                  /* load routine */
	NULL,                       /* save validation routine */
	NULL                        /* save routine */
};
