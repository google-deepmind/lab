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
#define PM_MD3_C



/* dependencies */
#include "picointernal.h"



/* md3 model format */
#define MD3_MAGIC           "IDP3"
#define MD3_VERSION         15

/* md3 vertex scale */
#define MD3_SCALE         ( 1.0f / 64.0f )

/* md3 model frame information */
typedef struct md3Frame_s
{
	float bounds[ 2 ][ 3 ];
	float localOrigin[ 3 ];
	float radius;
	char creator[ 16 ];
}
md3Frame_t;

/* md3 model tag information */
typedef struct md3Tag_s
{
	char name[ 64 ];
	float origin[ 3 ];
	float axis[ 3 ][ 3 ];
}
md3Tag_t;

/* md3 surface md3 (one object mesh) */
typedef struct md3Surface_s
{
	char magic[ 4 ];
	char name[ 64 ];            /* polyset name */
	int flags;
	int numFrames;              /* all model surfaces should have the same */
	int numShaders;             /* all model surfaces should have the same */
	int numVerts;
	int numTriangles;
	int ofsTriangles;
	int ofsShaders;             /* offset from start of md3Surface_t */
	int ofsSt;                  /* texture coords are common for all frames */
	int ofsVertexes;            /* numVerts * numFrames */
	int ofsEnd;                 /* next surface follows */
}
md3Surface_t;

typedef struct md3Shader_s
{
	char name[ 64 ];
	int shaderIndex;            /* for ingame use */
}
md3Shader_t;

typedef struct md3Triangle_s
{
	int indexes[ 3 ];
}
md3Triangle_t;

typedef struct md3TexCoord_s
{
	float st[ 2 ];
}
md3TexCoord_t;

typedef struct md3Vertex_s
{
	short xyz[ 3 ];
	short normal;
}
md3Vertex_t;


/* md3 model file md3 structure */
typedef struct md3_s
{
	char magic[ 4 ];            /* MD3_MAGIC */
	int version;
	char name[ 64 ];            /* model name */
	int flags;
	int numFrames;
	int numTags;
	int numSurfaces;
	int numSkins;               /* number of skins for the mesh */
	int ofsFrames;              /* offset for first frame */
	int ofsTags;                /* numFrames * numTags */
	int ofsSurfaces;            /* first surface, others follow */
	int ofsEnd;                 /* end of file */
}
md3_t;




/*
   _md3_canload()
   validates a quake3 arena md3 model file. btw, i use the
   preceding underscore cause it's a static func referenced
   by one structure only.
 */

static int _md3_canload( PM_PARAMS_CANLOAD ){
	md3_t   *md3;


	/* to keep the compiler happy */
	*fileName = *fileName;

	/* sanity check */
	if ( bufSize < ( sizeof( *md3 ) * 2 ) ) {
		return PICO_PMV_ERROR_SIZE;
	}

	/* set as md3 */
	md3 = (md3_t*) buffer;

	/* check md3 magic */
	if ( *( (int*) md3->magic ) != *( (int*) MD3_MAGIC ) ) {
		return PICO_PMV_ERROR_IDENT;
	}

	/* check md3 version */
	if ( _pico_little_long( md3->version ) != MD3_VERSION ) {
		return PICO_PMV_ERROR_VERSION;
	}

	/* file seems to be a valid md3 */
	return PICO_PMV_OK;
}



/*
   _md3_load()
   loads a quake3 arena md3 model file.
 */

static picoModel_t *_md3_load( PM_PARAMS_LOAD ){
	int i, j;
	picoByte_t      *bb;
	md3_t           *md3;
	md3Surface_t    *surface;
	md3Shader_t     *shader;
	md3TexCoord_t   *texCoord;
	md3Frame_t      *frame;
	md3Triangle_t   *triangle;
	md3Vertex_t     *vertex;
	double lat, lng;

	picoModel_t     *picoModel;
	picoSurface_t   *picoSurface;
	picoShader_t    *picoShader;
	picoVec3_t xyz, normal;
	picoVec2_t st;
	picoColor_t color;


	/* -------------------------------------------------
	   md3 loading
	   ------------------------------------------------- */


	/* set as md3 */
	bb = (picoByte_t*) buffer;
	md3 = (md3_t*) buffer;

	/* check ident and version */
	if ( *( (int*) md3->magic ) != *( (int*) MD3_MAGIC ) || _pico_little_long( md3->version ) != MD3_VERSION ) {
		/* not an md3 file (todo: set error) */
		return NULL;
	}

	/* swap md3; sea: swaps fixed */
	md3->version = _pico_little_long( md3->version );
	md3->numFrames = _pico_little_long( md3->numFrames );
	md3->numTags = _pico_little_long( md3->numTags );
	md3->numSurfaces = _pico_little_long( md3->numSurfaces );
	md3->numSkins = _pico_little_long( md3->numSkins );
	md3->ofsFrames = _pico_little_long( md3->ofsFrames );
	md3->ofsTags = _pico_little_long( md3->ofsTags );
	md3->ofsSurfaces = _pico_little_long( md3->ofsSurfaces );
	md3->ofsEnd = _pico_little_long( md3->ofsEnd );

	/* do frame check */
	if ( md3->numFrames < 1 ) {
		_pico_printf( PICO_ERROR, "MD3 with 0 frames" );
		return NULL;
	}

	if ( frameNum < 0 || frameNum >= md3->numFrames ) {
		_pico_printf( PICO_ERROR, "Invalid or out-of-range MD3 frame specified" );
		return NULL;
	}

	/* swap frames */
	frame = (md3Frame_t*) ( bb + md3->ofsFrames );
	for ( i = 0; i < md3->numFrames; i++, frame++ )
	{
		frame->radius = _pico_little_float( frame->radius );
		for ( j = 0; j < 3; j++ )
		{
			frame->bounds[ 0 ][ j ] = _pico_little_float( frame->bounds[ 0 ][ j ] );
			frame->bounds[ 1 ][ j ] = _pico_little_float( frame->bounds[ 1 ][ j ] );
			frame->localOrigin[ j ] = _pico_little_float( frame->localOrigin[ j ] );
		}
	}

	/* swap surfaces */
	surface = (md3Surface_t*) ( bb + md3->ofsSurfaces );
	for ( i = 0; i < md3->numSurfaces; i++ )
	{
		/* swap surface md3; sea: swaps fixed */
		surface->flags = _pico_little_long( surface->flags );
		surface->numFrames = _pico_little_long( surface->numFrames );
		surface->numShaders = _pico_little_long( surface->numShaders );
		surface->numTriangles = _pico_little_long( surface->numTriangles );
		surface->ofsTriangles = _pico_little_long( surface->ofsTriangles );
		surface->numVerts = _pico_little_long( surface->numVerts );
		surface->ofsShaders = _pico_little_long( surface->ofsShaders );
		surface->ofsSt = _pico_little_long( surface->ofsSt );
		surface->ofsVertexes = _pico_little_long( surface->ofsVertexes );
		surface->ofsEnd = _pico_little_long( surface->ofsEnd );

		/* swap triangles */
		triangle = (md3Triangle_t*) ( (picoByte_t*) surface + surface->ofsTriangles );
		for ( j = 0; j < surface->numTriangles; j++, triangle++ )
		{
			/* sea: swaps fixed */
			triangle->indexes[ 0 ] = _pico_little_long( triangle->indexes[ 0 ] );
			triangle->indexes[ 1 ] = _pico_little_long( triangle->indexes[ 1 ] );
			triangle->indexes[ 2 ] = _pico_little_long( triangle->indexes[ 2 ] );
		}

		/* swap st coords */
		texCoord = (md3TexCoord_t*) ( (picoByte_t*) surface + surface->ofsSt );
		for ( j = 0; j < surface->numVerts; j++, texCoord++ )
		{
			texCoord->st[ 0 ] = _pico_little_float( texCoord->st[ 0 ] );
			texCoord->st[ 1 ] = _pico_little_float( texCoord->st[ 1 ] );
		}

		/* swap xyz/normals */
		vertex = (md3Vertex_t*) ( (picoByte_t*) surface + surface->ofsVertexes );
		for ( j = 0; j < ( surface->numVerts * surface->numFrames ); j++, vertex++ )
		{
			vertex->xyz[ 0 ] = _pico_little_short( vertex->xyz[ 0 ] );
			vertex->xyz[ 1 ] = _pico_little_short( vertex->xyz[ 1 ] );
			vertex->xyz[ 2 ] = _pico_little_short( vertex->xyz[ 2 ] );
			vertex->normal   = _pico_little_short( vertex->normal );
		}

		/* get next surface */
		surface = (md3Surface_t*) ( (picoByte_t*) surface + surface->ofsEnd );
	}

	/* -------------------------------------------------
	   pico model creation
	   ------------------------------------------------- */

	/* create new pico model */
	picoModel = PicoNewModel();
	if ( picoModel == NULL ) {
		_pico_printf( PICO_ERROR, "Unable to allocate a new model" );
		return NULL;
	}

	/* do model setup */
	PicoSetModelFrameNum( picoModel, frameNum );
	PicoSetModelNumFrames( picoModel, md3->numFrames ); /* sea */
	PicoSetModelName( picoModel, fileName );
	PicoSetModelFileName( picoModel, fileName );

	/* md3 surfaces become picomodel surfaces */
	surface = (md3Surface_t*) ( bb + md3->ofsSurfaces );

	/* run through md3 surfaces */
	for ( i = 0; i < md3->numSurfaces; i++ )
	{
		/* allocate new pico surface */
		picoSurface = PicoNewSurface( picoModel );
		if ( picoSurface == NULL ) {
			_pico_printf( PICO_ERROR, "Unable to allocate a new model surface" );
			PicoFreeModel( picoModel ); /* sea */
			return NULL;
		}

		/* md3 model surfaces are all triangle meshes */
		PicoSetSurfaceType( picoSurface, PICO_TRIANGLES );

		/* set surface name */
		PicoSetSurfaceName( picoSurface, surface->name );

		/* create new pico shader -sea */
		picoShader = PicoNewShader( picoModel );
		if ( picoShader == NULL ) {
			_pico_printf( PICO_ERROR, "Unable to allocate a new model shader" );
			PicoFreeModel( picoModel );
			return NULL;
		}

		/* detox and set shader name */
		shader = (md3Shader_t*) ( (picoByte_t*) surface + surface->ofsShaders );
		_pico_setfext( shader->name, "" );
		_pico_unixify( shader->name );
		PicoSetShaderName( picoShader, shader->name );

		/* associate current surface with newly created shader */
		PicoSetSurfaceShader( picoSurface, picoShader );

		/* copy indexes */
		triangle = (md3Triangle_t *) ( (picoByte_t*) surface + surface->ofsTriangles );

		for ( j = 0; j < surface->numTriangles; j++, triangle++ )
		{
			PicoSetSurfaceIndex( picoSurface, ( j * 3 + 0 ), (picoIndex_t) triangle->indexes[ 0 ] );
			PicoSetSurfaceIndex( picoSurface, ( j * 3 + 1 ), (picoIndex_t) triangle->indexes[ 1 ] );
			PicoSetSurfaceIndex( picoSurface, ( j * 3 + 2 ), (picoIndex_t) triangle->indexes[ 2 ] );
		}

		/* copy vertexes */
		texCoord = (md3TexCoord_t*) ( (picoByte_t *) surface + surface->ofsSt );
		vertex = (md3Vertex_t*) ( (picoByte_t*) surface + surface->ofsVertexes + surface->numVerts * frameNum * sizeof( md3Vertex_t ) );
		_pico_set_color( color, 255, 255, 255, 255 );

		for ( j = 0; j < surface->numVerts; j++, texCoord++, vertex++ )
		{
			/* set vertex origin */
			xyz[ 0 ] = MD3_SCALE * vertex->xyz[ 0 ];
			xyz[ 1 ] = MD3_SCALE * vertex->xyz[ 1 ];
			xyz[ 2 ] = MD3_SCALE * vertex->xyz[ 2 ];
			PicoSetSurfaceXYZ( picoSurface, j, xyz );

			/* decode lat/lng normal to 3 float normal */
			lat = (float) ( ( vertex->normal >> 8 ) & 0xff );
			lng = (float) ( vertex->normal & 0xff );
			lat *= PICO_PI / 128;
			lng *= PICO_PI / 128;
			normal[ 0 ] = (picoVec_t) cos( lat ) * (picoVec_t) sin( lng );
			normal[ 1 ] = (picoVec_t) sin( lat ) * (picoVec_t) sin( lng );
			normal[ 2 ] = (picoVec_t) cos( lng );
			PicoSetSurfaceNormal( picoSurface, j, normal );

			/* set st coords */
			st[ 0 ] = texCoord->st[ 0 ];
			st[ 1 ] = texCoord->st[ 1 ];
			PicoSetSurfaceST( picoSurface, 0, j, st );

			/* set color */
			PicoSetSurfaceColor( picoSurface, 0, j, color );
		}

		/* get next surface */
		surface = (md3Surface_t*) ( (picoByte_t*) surface + surface->ofsEnd );
	}

	/* return the new pico model */
	return picoModel;
}



/* pico file format module definition */
const picoModule_t picoModuleMD3 =
{
	"1.3",                      /* module version string */
	"Quake 3 Arena",            /* module display name */
	"Randy Reddig",             /* author's name */
	"2002 Randy Reddig",        /* module copyright */
	{
		"md3", NULL, NULL, NULL /* default extensions to use */
	},
	_md3_canload,               /* validation routine */
	_md3_load,                  /* load routine */
	NULL,                       /* save validation routine */
	NULL                        /* save routine */
};
