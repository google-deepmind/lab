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
#define PM_MS3D_C

/* dependencies */
#include "picointernal.h"

/* disable warnings */
#ifdef _WIN32
#pragma warning( disable:4100 )		/* unref param */
#endif

/* remarks:
 * - loader seems stable
 * todo:
 * - fix uv coordinate problem
 * - check for buffer overflows ('bufptr' accesses)
 */
/* uncomment when debugging this module */
 #define DEBUG_PM_MS3D
 #define DEBUG_PM_MS3D_EX

/* plain white */
static picoColor_t white = { 255,255,255,255 };

/* ms3d limits */
#define MS3D_MAX_VERTS      8192
#define MS3D_MAX_TRIS       16384
#define MS3D_MAX_GROUPS     128
#define MS3D_MAX_MATERIALS  128
#define MS3D_MAX_JOINTS     128
#define MS3D_MAX_KEYFRAMES  216

/* ms3d flags */
#define MS3D_SELECTED       1
#define MS3D_HIDDEN         2
#define MS3D_SELECTED2      4
#define MS3D_DIRTY          8

/* this freaky loader needs byte alignment */
#pragma pack(push, 1)

/* ms3d header */
typedef struct SMsHeader
{
	char magic[10];
	int version;
}
TMsHeader;

/* ms3d vertex */
typedef struct SMsVertex
{
	unsigned char flags;                /* sel, sel2, or hidden */
	float xyz[3];
	char boneID;                        /* -1 means 'no bone' */
	unsigned char refCount;
}
TMsVertex;

/* ms3d triangle */
typedef struct SMsTriangle
{
	unsigned short flags;               /* sel, sel2, or hidden */
	unsigned short vertexIndices[3];
	float vertexNormals[3][3];
	float s[3];
	float t[3];
	unsigned char smoothingGroup;       /* 1 - 32 */
	unsigned char groupIndex;
}
TMsTriangle;

/* ms3d material */
typedef struct SMsMaterial
{
	char name[32];
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emissive[4];
	float shininess;                    /* range 0..128 */
	float transparency;                 /* range 0..1 */
	unsigned char mode;
	char texture [128];                 /* texture.bmp */
	char alphamap[128];                 /* alpha.bmp */
}
TMsMaterial;

// ms3d group (static part)
// followed by a variable size block (see below)
typedef struct SMsGroup
{
	unsigned char flags;                // sel, hidden
	char name[32];
	unsigned short numTriangles;
/*
    unsigned short	triangleIndices[ numTriangles ];
    char			materialIndex;		// -1 means 'no material'
 */
}
TMsGroup;

// ms3d joint
typedef struct SMsJoint
{
	unsigned char flags;
	char name[32];
	char parentName[32];
	float rotation[3];
	float translation[3];
	unsigned short numRotationKeyframes;
	unsigned short numTranslationKeyframes;
}
TMsJoint;

// ms3d keyframe
typedef struct SMsKeyframe
{
	float time;
	float parameter[3];
}
TMsKeyframe;

/* restore previous data alignment */
#pragma pack(pop)

/* _ms3d_canload:
 *	validates a milkshape3d model file.
 */
static int _ms3d_canload( PM_PARAMS_CANLOAD ){
	TMsHeader *hdr;


	/* to keep the compiler happy */
	*fileName = *fileName;

	/* sanity check */
	if ( bufSize < sizeof( TMsHeader ) ) {
		return PICO_PMV_ERROR_SIZE;
	}

	/* get ms3d header */
	hdr = (TMsHeader *)buffer;

	/* check ms3d magic */
	if ( strncmp( hdr->magic,"MS3D000000",10 ) != 0 ) {
		return PICO_PMV_ERROR_IDENT;
	}

	/* check ms3d version */
	if ( _pico_little_long( hdr->version ) < 3 ||
		 _pico_little_long( hdr->version ) > 4 ) {
		_pico_printf( PICO_ERROR,"MS3D file ignored. Only MS3D 1.3 and 1.4 is supported." );
		return PICO_PMV_ERROR_VERSION;
	}
	/* file seems to be a valid ms3d */
	return PICO_PMV_OK;
}

static unsigned char *GetWord( unsigned char *bufptr, int *out ){
	if ( bufptr == NULL ) {
		return NULL;
	}
	*out = _pico_little_short( *(unsigned short *)bufptr );
	return( bufptr + 2 );
}

/* _ms3d_load:
 *	loads a milkshape3d model file.
 */
static picoModel_t *_ms3d_load( PM_PARAMS_LOAD ){
	picoModel_t    *model;
	unsigned char  *bufptr;
	int shaderRefs[ MS3D_MAX_GROUPS ];
	int numGroups;
	int numMaterials;
//	unsigned char  *ptrToGroups;
	int numVerts;
	unsigned char  *ptrToVerts;
	int numTris;
	unsigned char  *ptrToTris;
	int i,k,m;

	/* create new pico model */
	model = PicoNewModel();
	if ( model == NULL ) {
		return NULL;
	}

	/* do model setup */
	PicoSetModelFrameNum( model, frameNum );
	PicoSetModelName( model, fileName );
	PicoSetModelFileName( model, fileName );

	/* skip header */
	bufptr = (unsigned char *)buffer + sizeof( TMsHeader );

	/* get number of vertices */
	bufptr = GetWord( bufptr,&numVerts );
	ptrToVerts = bufptr;

#ifdef DEBUG_PM_MS3D
	printf( "NumVertices: %d\n",numVerts );
#endif
	/* swap verts */
	for ( i = 0; i < numVerts; i++ )
	{
		TMsVertex *vertex;
		vertex = (TMsVertex *)bufptr;
		bufptr += sizeof( TMsVertex );

		vertex->xyz[ 0 ] = _pico_little_float( vertex->xyz[ 0 ] );
		vertex->xyz[ 1 ] = _pico_little_float( vertex->xyz[ 1 ] );
		vertex->xyz[ 2 ] = _pico_little_float( vertex->xyz[ 2 ] );

#ifdef DEBUG_PM_MS3D_EX_
		printf( "Vertex: x: %f y: %f z: %f\n",
				msvd[i]->vertex[0],
				msvd[i]->vertex[1],
				msvd[i]->vertex[2] );
#endif
	}
	/* get number of triangles */
	bufptr = GetWord( bufptr,&numTris );
	ptrToTris = bufptr;

#ifdef DEBUG_PM_MS3D
	printf( "NumTriangles: %d\n",numTris );
#endif
	/* swap tris */
	for ( i = 0; i < numTris; i++ )
	{
		TMsTriangle *triangle;
		triangle = (TMsTriangle *)bufptr;
		bufptr += sizeof( TMsTriangle );

		triangle->flags = _pico_little_short( triangle->flags );

		/* run through all tri verts */
		for ( k = 0; k < 3; k++ )
		{
			/* swap tex coords */
			triangle->s[ k ] = _pico_little_float( triangle->s[ k ] );
			triangle->t[ k ] = _pico_little_float( triangle->t[ k ] );

			/* swap fields */
			triangle->vertexIndices[ k ]      = _pico_little_short( triangle->vertexIndices[ k ] );
			triangle->vertexNormals[ 0 ][ k ] = _pico_little_float( triangle->vertexNormals[ 0 ][ k ] );
			triangle->vertexNormals[ 1 ][ k ] = _pico_little_float( triangle->vertexNormals[ 1 ][ k ] );
			triangle->vertexNormals[ 2 ][ k ] = _pico_little_float( triangle->vertexNormals[ 2 ][ k ] );

			/* check for out of range indices */
			if ( triangle->vertexIndices[ k ] >= numVerts ) {
				_pico_printf( PICO_ERROR,"Vertex %d index %d out of range (%d, max %d)",i,k,triangle->vertexIndices[k],numVerts - 1 );
				PicoFreeModel( model );
				return NULL; /* yuck */
			}
		}
	}
	/* get number of groups */
	bufptr = GetWord( bufptr,&numGroups );
//	ptrToGroups = bufptr;

#ifdef DEBUG_PM_MS3D
	printf( "NumGroups: %d\n",numGroups );
#endif
	/* run through all groups in model */
	for ( i = 0; i < numGroups && i < MS3D_MAX_GROUPS; i++ )
	{
		picoSurface_t *surface;
		TMsGroup      *group;

		group = (TMsGroup *)bufptr;
		bufptr += sizeof( TMsGroup );

		/* we ignore hidden groups */
		if ( group->flags & MS3D_HIDDEN ) {
			bufptr += ( group->numTriangles * 2 ) + 1;
			continue;
		}
		/* forced null term of group name */
		group->name[ 31 ] = '\0';

		/* create new pico surface */
		surface = PicoNewSurface( model );
		if ( surface == NULL ) {
			PicoFreeModel( model );
			return NULL;
		}
		/* do surface setup */
		PicoSetSurfaceType( surface,PICO_TRIANGLES );
		PicoSetSurfaceName( surface,group->name );

		/* process triangle indices */
		for ( k = 0; k < group->numTriangles; k++ )
		{
			TMsTriangle *triangle;
			unsigned int triangleIndex;

			/* get triangle index */
			bufptr = GetWord( bufptr,(int *)&triangleIndex );

			/* get ptr to triangle data */
			triangle = (TMsTriangle *)( ptrToTris + ( sizeof( TMsTriangle ) * triangleIndex ) );

			/* run through triangle vertices */
			for ( m = 0; m < 3; m++ )
			{
				TMsVertex   *vertex;
				unsigned int vertexIndex;
				picoVec2_t texCoord;

				/* get ptr to vertex data */
				vertexIndex = triangle->vertexIndices[ m ];
				vertex = (TMsVertex *)( ptrToVerts + ( sizeof( TMsVertex ) * vertexIndex ) );

				/* store vertex origin */
				PicoSetSurfaceXYZ( surface,vertexIndex,vertex->xyz );

				/* store vertex color */
				PicoSetSurfaceColor( surface,0,vertexIndex,white );

				/* store vertex normal */
				PicoSetSurfaceNormal( surface,vertexIndex,triangle->vertexNormals[ m ] );

				/* store current face vertex index */
				PicoSetSurfaceIndex( surface,( k * 3 + ( 2 - m ) ),(picoIndex_t)vertexIndex );

				/* get texture vertex coord */
				texCoord[ 0 ] = triangle->s[ m ];
				texCoord[ 1 ] = -triangle->t[ m ];  /* flip t */

				/* store texture vertex coord */
				PicoSetSurfaceST( surface,0,vertexIndex,texCoord );
			}
		}
		/* store material */
		shaderRefs[ i ] = *bufptr++;

#ifdef DEBUG_PM_MS3D
		printf( "Group %d: '%s' (%d tris)\n",i,group->name,group->numTriangles );
#endif
	}
	/* get number of materials */
	bufptr = GetWord( bufptr,&numMaterials );

#ifdef DEBUG_PM_MS3D
	printf( "NumMaterials: %d\n",numMaterials );
#endif
	/* run through all materials in model */
	for ( i = 0; i < numMaterials; i++ )
	{
		picoShader_t *shader;
		picoColor_t ambient,diffuse,specular;
		TMsMaterial  *material;
		int k;

		material = (TMsMaterial *)bufptr;
		bufptr += sizeof( TMsMaterial );

		/* null term strings */
		material->name    [  31 ] = '\0';
		material->texture [ 127 ] = '\0';
		material->alphamap[ 127 ] = '\0';

		/* ltrim strings */
		_pico_strltrim( material->name );
		_pico_strltrim( material->texture );
		_pico_strltrim( material->alphamap );

		/* rtrim strings */
		_pico_strrtrim( material->name );
		_pico_strrtrim( material->texture );
		_pico_strrtrim( material->alphamap );

		/* create new pico shader */
		shader = PicoNewShader( model );
		if ( shader == NULL ) {
			PicoFreeModel( model );
			return NULL;
		}
		/* scale shader colors */
		for ( k = 0; k < 4; k++ )
		{
			ambient [ k ] = (picoByte_t) ( material->ambient[ k ] * 255 );
			diffuse [ k ] = (picoByte_t) ( material->diffuse[ k ] * 255 );
			specular[ k ] = (picoByte_t) ( material->specular[ k ] * 255 );
		}
		/* set shader colors */
		PicoSetShaderAmbientColor( shader,ambient );
		PicoSetShaderDiffuseColor( shader,diffuse );
		PicoSetShaderSpecularColor( shader,specular );

		/* set shader transparency */
		PicoSetShaderTransparency( shader,material->transparency );

		/* set shader shininess (0..127) */
		PicoSetShaderShininess( shader,material->shininess );

		/* set shader name */
		PicoSetShaderName( shader,material->name );

		/* set shader texture map name */
		PicoSetShaderMapName( shader,material->texture );

#ifdef DEBUG_PM_MS3D
		printf( "Material %d: '%s' ('%s','%s')\n",i,material->name,material->texture,material->alphamap );
#endif
	}
	/* assign shaders to surfaces */
	for ( i = 0; i < numGroups && i < MS3D_MAX_GROUPS; i++ )
	{
		picoSurface_t *surface;
		picoShader_t  *shader;

		/* sanity check */
		if ( shaderRefs[ i ] >= MS3D_MAX_MATERIALS ||
			 shaderRefs[ i ] < 0 ) {
			continue;
		}

		/* get surface */
		surface = PicoGetModelSurface( model,i );
		if ( surface == NULL ) {
			continue;
		}

		/* get shader */
		shader = PicoGetModelShader( model,shaderRefs[ i ] );
		if ( shader == NULL ) {
			continue;
		}

		/* assign shader */
		PicoSetSurfaceShader( surface,shader );

#ifdef DEBUG_PM_MS3D
		printf( "Mapped: %d ('%s') to %d (%s)\n",
				shaderRefs[i],shader->name,i,surface->name );
#endif
	}
	/* return allocated pico model */
	return model;
//	return NULL;
}

/* pico file format module definition */
const picoModule_t picoModuleMS3D =
{
	"0.4-a",                    /* module version string */
	"Milkshape 3D",             /* module display name */
	"seaw0lf",                  /* author's name */
	"2002 seaw0lf",             /* module copyright */
	{
		"ms3d",NULL,NULL,NULL   /* default extensions to use */
	},
	_ms3d_canload,              /* validation routine */
	_ms3d_load,                 /* load routine */
	NULL,                       /* save validation routine */
	NULL                        /* save routine */
};
