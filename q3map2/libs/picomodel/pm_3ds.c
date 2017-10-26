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
#define PM_3DS_C

/* dependencies */
#include "picointernal.h"

/* ydnar */
static picoColor_t white = { 255,255,255,255 };

/* remarks:
 * - 3ds file version is stored in pico special field 0 on load (ydnar: removed)
 * todo:
 * - sometimes there is one unnamed surface 0 having 0 verts as
 *   well as 0 faces. this error occurs since pm 0.6 (ydnar?)
 */
/* uncomment when debugging this module */
/* #define DEBUG_PM_3DS
 #define DEBUG_PM_3DS_EX */

/* structure holding persistent 3ds loader specific data used */
/* to store formerly static vars to keep the module reentrant */
/* safe. put everything that needs to be static in here. */
typedef struct S3dsLoaderPers
{
	picoModel_t    *model;          /* ptr to output model */
	picoSurface_t  *surface;        /* ptr to current surface */
	picoShader_t   *shader;         /* ptr to current shader */
	picoByte_t     *bufptr;         /* ptr to raw data */
	char           *basename;       /* ptr to model base name (eg. jeep) */
	int cofs;
	int maxofs;
}
T3dsLoaderPers;

/* 3ds chunk types that we use */
enum {
	/* primary chunk */
	CHUNK_MAIN              = 0x4D4D,

	/* main chunks */
	CHUNK_VERSION           = 0x0002,
	CHUNK_EDITOR_CONFIG     = 0x3D3E,
	CHUNK_EDITOR_DATA       = 0x3D3D,
	CHUNK_KEYFRAME_DATA     = 0xB000,

	/* editor data sub chunks */
	CHUNK_MATERIAL          = 0xAFFF,
	CHUNK_OBJECT            = 0x4000,

	/* material sub chunks */
	CHUNK_MATNAME           = 0xA000,
	CHUNK_MATDIFFUSE        = 0xA020,
	CHUNK_MATMAP            = 0xA200,
	CHUNK_MATMAPFILE        = 0xA300,

	/* lets us know we're reading a new object */
	CHUNK_OBJECT_MESH       = 0x4100,

	/* object mesh sub chunks */
	CHUNK_OBJECT_VERTICES   = 0x4110,
	CHUNK_OBJECT_FACES      = 0x4120,
	CHUNK_OBJECT_MATERIAL   = 0x4130,
	CHUNK_OBJECT_UV         = 0x4140,
};
#ifdef DEBUG_PM_3DS
static struct
{
	int id;
	char   *name;
}
debugChunkNames[] =
{
	{ CHUNK_MAIN, "CHUNK_MAIN"              },
	{ CHUNK_VERSION, "CHUNK_VERSION"           },
	{ CHUNK_EDITOR_CONFIG, "CHUNK_EDITOR_CONFIG"     },
	{ CHUNK_EDITOR_DATA, "CHUNK_EDITOR_DATA"       },
	{ CHUNK_KEYFRAME_DATA, "CHUNK_KEYFRAME_DATA"     },
	{ CHUNK_MATERIAL, "CHUNK_MATERIAL"          },
	{ CHUNK_OBJECT, "CHUNK_OBJECT"            },
	{ CHUNK_MATNAME, "CHUNK_MATNAME"           },
	{ CHUNK_MATDIFFUSE, "CHUNK_MATDIFFUSE"        },
	{ CHUNK_MATMAP, "CHUNK_MATMAP"            },
	{ CHUNK_MATMAPFILE, "CHUNK_MATMAPFILE"        },
	{ CHUNK_OBJECT_MESH, "CHUNK_OBJECT_MESH"       },
	{ CHUNK_OBJECT_VERTICES, "CHUNK_OBJECT_VERTICES"   },
	{ CHUNK_OBJECT_FACES, "CHUNK_OBJECT_FACES"      },
	{ CHUNK_OBJECT_MATERIAL, "CHUNK_OBJECT_MATERIAL"   },
	{ CHUNK_OBJECT_UV, "CHUNK_OBJECT_UV"         },
	{ 0,  NULL                     }
};
static char *DebugGetChunkName( int id ) {
	int i,max;  /* imax? ;) */
	max = sizeof( debugChunkNames ) / sizeof( debugChunkNames[0] );

	for ( i = 0; i < max; i++ )
	{
		if ( debugChunkNames[i].id == id ) {
			/* gaynux update -sea */
			return _pico_strlwr( debugChunkNames[i].name );
		}
	}
	return "chunk_unknown";
}
#endif /*DEBUG_PM_3DS*/

/* this funky loader needs byte alignment */
#pragma pack(push, 1)

typedef struct S3dsIndices
{
	unsigned short a,b,c;
	unsigned short visible;
}
T3dsIndices;

typedef struct S3dsChunk
{
	unsigned short id;
	unsigned int len;
}
T3dsChunk;

/* restore previous data alignment */
#pragma pack(pop)

/* _3ds_canload:
 *  validates an autodesk 3ds model file.
 */
static int _3ds_canload( PM_PARAMS_CANLOAD ){
	T3dsChunk *chunk;

	/* to keep the compiler happy */
	*fileName = *fileName;

	/* sanity check */
	if ( bufSize < sizeof( T3dsChunk ) ) {
		return PICO_PMV_ERROR_SIZE;
	}

	/* get pointer to 3ds header chunk */
	chunk = (T3dsChunk *)buffer;

	/* check data length */
	if ( bufSize < _pico_little_long( chunk->len ) ) {
		return PICO_PMV_ERROR_SIZE;
	}

	/* check 3ds magic */
	if ( _pico_little_short( chunk->id ) != CHUNK_MAIN ) {
		return PICO_PMV_ERROR_IDENT;
	}

	/* file seems to be a valid 3ds */
	return PICO_PMV_OK;
}

static T3dsChunk *GetChunk( T3dsLoaderPers *pers ){
	T3dsChunk *chunk;

	/* sanity check */
	if ( pers->cofs > pers->maxofs ) {
		return 0;
	}

#ifdef DEBUG_PM_3DS
/*	printf("GetChunk: pers->cofs %x\n",pers->cofs); */
#endif
	/* fill in pointer to chunk */
	chunk = (T3dsChunk *)&pers->bufptr[ pers->cofs ];
	if ( !chunk ) {
		return NULL;
	}

	chunk->id  = _pico_little_short( chunk->id );
	chunk->len = _pico_little_long( chunk->len );

	/* advance in buffer */
	pers->cofs += sizeof( T3dsChunk );

	/* this means yay */
	return chunk;
}

static int GetASCIIZ( T3dsLoaderPers *pers, char *dest, int max ){
	int pos = 0;
	int ch;

	for (;; )
	{
		ch = pers->bufptr[ pers->cofs++ ];
		if ( ch == '\0' ) {
			break;
		}
		if ( pers->cofs >= pers->maxofs ) {
			dest[ pos ] = '\0';
			return 0;
		}
		dest[ pos++ ] = ch;
		if ( pos >= max ) {
			break;
		}
	}
	dest[ pos ] = '\0';
	return 1;
}

static picoByte_t GetByte( T3dsLoaderPers *pers ){
	picoByte_t *value;

	/* sanity check */
	if ( pers->cofs > pers->maxofs ) {
		return 0;
	}

	/* get and return value */
	value = (picoByte_t *)( pers->bufptr + pers->cofs );
	pers->cofs += 1;
	return *value;
}

static int GetWord( T3dsLoaderPers *pers ){
	unsigned short *value;

	/* sanity check */
	if ( pers->cofs > pers->maxofs ) {
		return 0;
	}

	/* get and return value */
	value = (unsigned short *)( pers->bufptr + pers->cofs );
	pers->cofs += 2;
	return _pico_little_short( *value );
}

static float GetFloat( T3dsLoaderPers *pers ){
	float *value;

	/* sanity check */
	if ( pers->cofs > pers->maxofs ) {
		return 0;
	}

	/* get and return value */
	value = (float *)( pers->bufptr + pers->cofs );
	pers->cofs += 4;
	return _pico_little_float( *value );
}

static int GetMeshVertices( T3dsLoaderPers *pers ){
	int numVerts;
	int i;

	/* get number of verts for this surface */
	numVerts = GetWord( pers );

#ifdef DEBUG_PM_3DS
	printf( "GetMeshVertices: numverts %d\n",numVerts );
#endif
	/* read in vertices for current surface */
	for ( i = 0; i < numVerts; i++ )
	{
		picoVec3_t v;
		v[0] = GetFloat( pers );
		v[1] = GetFloat( pers );    /* ydnar: unflipped */
		v[2] = GetFloat( pers );    /* ydnar: unflipped and negated */

		/* add current vertex */
		PicoSetSurfaceXYZ( pers->surface,i,v );
		PicoSetSurfaceColor( pers->surface,0,i,white ); /* ydnar */

#ifdef DEBUG_PM_3DS_EX
		printf( "Vertex: x: %f y: %f z: %f\n",v[0],v[1],v[2] );
#endif
	}
	/* success (no errors occured) */
	return 1;
}

static int GetMeshFaces( T3dsLoaderPers *pers ){
	int numFaces;
	int i;

	/* get number of faces for this surface */
	numFaces = GetWord( pers );

#ifdef DEBUG_PM_3DS
	printf( "GetMeshFaces: numfaces %d\n",numFaces );
#endif
	/* read in vertex indices for current surface */
	for ( i = 0; i < numFaces; i++ )
	{
		/* remember, we only need 3 of 4 values read in for each */
		/* face. the 4th value is a vis flag for 3dsmax which is */
		/* being ignored by us here */
		T3dsIndices face;
		face.a       = GetWord( pers );
		face.c       = GetWord( pers );   /* ydnar: flipped order */
		face.b       = GetWord( pers );   /* ydnar: flipped order */
		face.visible = GetWord( pers );

		/* copy indexes */
		PicoSetSurfaceIndex( pers->surface, ( i * 3 + 0 ), (picoIndex_t)face.a );
		PicoSetSurfaceIndex( pers->surface, ( i * 3 + 1 ), (picoIndex_t)face.b );
		PicoSetSurfaceIndex( pers->surface, ( i * 3 + 2 ), (picoIndex_t)face.c );

#ifdef DEBUG_PM_3DS_EX
		printf( "Face: a: %d b: %d c: %d (%d)\n",face.a,face.b,face.c,face.visible );
#endif
	}
	/* success (no errors occured) */
	return 1;
}

static int GetMeshTexCoords( T3dsLoaderPers *pers ){
	int numTexCoords;
	int i;

	/* get number of uv coords for this surface */
	numTexCoords = GetWord( pers );

#ifdef DEBUG_PM_3DS
	printf( "GetMeshTexCoords: numcoords %d\n",numTexCoords );
#endif
	/* read in uv coords for current surface */
	for ( i = 0; i < numTexCoords; i++ )
	{
		picoVec2_t uv;
		uv[0] =  GetFloat( pers );
		uv[1] = -GetFloat( pers );  /* ydnar: we use origin at bottom */

		/* to make sure we don't mess up memory */
		if ( pers->surface == NULL ) {
			continue;
		}

		/* add current uv */
		PicoSetSurfaceST( pers->surface,0,i,uv );

#ifdef DEBUG_PM_3DS_EX
		printf( "u: %f v: %f\n",uv[0],uv[1] );
#endif
	}
	/* success (no errors occured) */
	return 1;
}

static int GetMeshShader( T3dsLoaderPers *pers ){
	char shaderName[255] = { 0 };
	picoShader_t  *shader;
	int numSharedVerts;
	int setShaderName = 0;
	int i;

	/* the shader is either the color or the texture map of the */
	/* object. it can also hold other information like the brightness, */
	/* shine, etc. stuff we don't really care about. we just want the */
	/* color, or the texture map file name really */

	/* get in the shader name */
	if ( !GetASCIIZ( pers,shaderName,sizeof( shaderName ) ) ) {
		return 0;
	}

	/* ydnar: trim to first whitespace */
	_pico_first_token( shaderName );
	
	/* now that we have the shader name we need to go through all of */
	/* the shaders and check the name against each shader. when we */
	/* find a shader in our shader list that matches this name we */
	/* just read in, then we assign the shader's id of the object to */
	/* that shader */

	/* get shader id for shader name */
	shader = PicoFindShader( pers->model, shaderName, 1 );

	/* we've found a matching shader */
	if ( ( shader != NULL ) && pers->surface ) {
		char mapName[1024 + 1];
		char *mapNamePtr;
		memset( mapName,0,sizeof( mapName ) );

		/* get ptr to shader's map name */
		mapNamePtr = PicoGetShaderMapName( shader );

		/* we have a valid map name ptr */
		if ( mapNamePtr != NULL ) {
			char temp[128];
			const char *name;

			/* copy map name to local buffer */
			strcpy( mapName,mapNamePtr );

			/* extract file name */
			name = _pico_nopath( mapName );
			strncpy( temp, name, sizeof( temp ) );

			/* remove file extension */
			/* name = _pico_setfext( name,"" ); */

			/* assign default name if no name available */
			if ( strlen( temp ) < 1 ) {
				strcpy( temp,pers->basename );
			}

			/* build shader name */
			_pico_strlwr( temp ); /* gaynux update -sea */
			sprintf( mapName,"models/mapobjects/%s/%s",pers->basename,temp );

			/* set shader name */
			/* PicoSetShaderName( shader,mapName ); */	/* ydnar: this will screw up the named shader */

			/* set surface's shader index */
			PicoSetSurfaceShader( pers->surface, shader );

			setShaderName = 1;
		}
	}
	/* we didn't set a shader name; throw out warning */
	if ( !setShaderName ) {
		_pico_printf( PICO_WARNING,"3DS mesh is missing shader name" );
	}
	/* we don't process the list of shared vertices here; there is a */
	/* short int that gives the number of faces of the mesh concerned */
	/* by this shader, then there is the list itself of these faces. */
	/* 0000 means the first face of the (4120) face list */

	/* get number of shared verts */
	numSharedVerts = GetWord( pers );

#ifdef DEBUG_PM_3DS
	printf( "GetMeshShader: uses shader '%s' (nsv %d)\n",shaderName,numSharedVerts );
#endif
	/* skip list of shared verts */
	for ( i = 0; i < numSharedVerts; i++ )
	{
		GetWord( pers );
	}
	/* success (no errors occured) */
	return 1;
}

static int GetDiffuseColor( T3dsLoaderPers *pers ){
	/* todo: support all 3ds specific color formats; */
	/* that means: rgb,tru,trug,rgbg */

	/* get rgb color (range 0..255; 3 bytes) */
	picoColor_t color;

	color[0] = GetByte( pers );
	color[1] = GetByte( pers );
	color[2] = GetByte( pers );
	color[3] = 255;

	/* store this as the current shader's diffuse color */
	if ( pers->shader ) {
		PicoSetShaderDiffuseColor( pers->shader,color );
	}
#ifdef DEBUG_PM_3DS
	printf( "GetDiffuseColor: %d %d %d\n",color[0],color[1],color[2] );
#endif
	/* success (no errors occured) */
	return 1;
}

static int DoNextEditorDataChunk( T3dsLoaderPers *pers, long endofs ){
	T3dsChunk *chunk;

#ifdef DEBUG_PM_3DS_EX
	printf( "DoNextEditorDataChunk: endofs %d\n",endofs );
#endif
	while ( pers->cofs < endofs )
	{
		long nextofs = pers->cofs;
		if ( ( chunk = GetChunk( pers ) ) == NULL ) {
			return 0;
		}
		if ( !chunk->len ) {
			return 0;
		}
		nextofs += chunk->len;

#ifdef DEBUG_PM_3DS_EX
		printf( "Chunk %04x (%s), len %d pers->cofs %x\n",chunk->id,DebugGetChunkName( chunk->id ),chunk->len,pers->cofs );
#endif
		/*** meshes ***/
		if ( chunk->id == CHUNK_OBJECT ) {
			picoSurface_t *surface;
			char surfaceName[ 0xff ] = { 0 };

			/* read in surface name */
			if ( !GetASCIIZ( pers,surfaceName,sizeof( surfaceName ) ) ) {
				return 0; /* this is bad */
			}
//PicoGetSurfaceName
			/* ignore NULL name surfaces */
//			if( surfaceName

			/* allocate a pico surface */
			surface = PicoNewSurface( pers->model );
			if ( surface == NULL ) {
				pers->surface = NULL;
				return 0; /* this is bad too */
			}
			/* assign ptr to current surface */
			pers->surface = surface;

			/* 3ds models surfaces are all triangle meshes */
			PicoSetSurfaceType( pers->surface,PICO_TRIANGLES );

			/* set surface name */
			PicoSetSurfaceName( pers->surface,surfaceName );

			/* continue mess with object's sub chunks */
			DoNextEditorDataChunk( pers,nextofs );
			continue;
		}
		if ( chunk->id == CHUNK_OBJECT_MESH ) {
			/* continue mess with mesh's sub chunks */
			if ( !DoNextEditorDataChunk( pers,nextofs ) ) {
				return 0;
			}
			continue;
		}
		if ( chunk->id == CHUNK_OBJECT_VERTICES ) {
			if ( !GetMeshVertices( pers ) ) {
				return 0;
			}
			continue;
		}
		if ( chunk->id == CHUNK_OBJECT_FACES ) {
			if ( !GetMeshFaces( pers ) ) {
				return 0;
			}
			continue;
		}
		if ( chunk->id == CHUNK_OBJECT_UV ) {
			if ( !GetMeshTexCoords( pers ) ) {
				return 0;
			}
			continue;
		}
		if ( chunk->id == CHUNK_OBJECT_MATERIAL ) {
			if ( !GetMeshShader( pers ) ) {
				return 0;
			}
			continue;
		}
		/*** materials ***/
		if ( chunk->id == CHUNK_MATERIAL ) {
			/* new shader specific things should be */
			/* initialized right here */
			picoShader_t *shader;

			/* allocate a pico shader */
			shader = PicoNewShader( pers->model );  /* ydnar */
			if ( shader == NULL ) {
				pers->shader = NULL;
				return 0; /* this is bad too */
			}

			/* assign ptr to current shader */
			pers->shader = shader;

			/* continue and process the material's sub chunks */
			DoNextEditorDataChunk( pers,nextofs );
			continue;
		}
		if ( chunk->id == CHUNK_MATNAME ) {
			/* new material's names should be stored here. note that */
			/* GetMeshMaterial returns the name of the material that */
			/* is used by the mesh. new material names are set HERE. */
			/* but for now we skip the new material's name ... */
			if ( pers->shader ) {
				char *name = (char *)( pers->bufptr + pers->cofs );
				char *cleanedName = _pico_clone_alloc( name );
				_pico_first_token( cleanedName );
				PicoSetShaderName( pers->shader, cleanedName );
#ifdef DEBUG_PM_3DS
				printf( "NewShader: '%s'\n", cleanedName );
#endif
				_pico_free( cleanedName );
			}
		}
		if ( chunk->id == CHUNK_MATDIFFUSE ) {
			/* todo: color for last inserted new material should be */
			/* stored somewhere by GetDiffuseColor */
			if ( !GetDiffuseColor( pers ) ) {
				return 0;
			}

			/* rest of chunk is skipped here */
		}
		if ( chunk->id == CHUNK_MATMAP ) {
			/* continue and process the material map sub chunks */
			DoNextEditorDataChunk( pers,nextofs );
			continue;
		}
		if ( chunk->id == CHUNK_MATMAPFILE ) {
			/* map file name for last inserted new material should */
			/* be stored here. but for now we skip this too ... */
			if ( pers->shader ) {
				char *name = (char *)( pers->bufptr + pers->cofs );
				PicoSetShaderMapName( pers->shader,name );
#ifdef DEBUG_PM_3DS
				printf( "NewShaderMapfile: '%s'\n",name );
#endif
			}
		}
		/*** keyframes ***/
		if ( chunk->id == CHUNK_KEYFRAME_DATA ) {
			/* well umm, this is a bit too much since we don't really */
			/* need model animation sequences right now. we skip this */
#ifdef DEBUG_PM_3DS
			printf( "KeyframeData: len %d\n",chunk->len );
#endif
		}
		/* skip unknown chunk */
		pers->cofs = nextofs;
		if ( pers->cofs >= pers->maxofs ) {
			break;
		}
	}
	return 1;
}

static int DoNextChunk( T3dsLoaderPers *pers, int endofs ){
	T3dsChunk *chunk;

#ifdef DEBUG_PM_3DS
	printf( "DoNextChunk: endofs %d\n",endofs );
#endif
	while ( pers->cofs < endofs )
	{
		long nextofs = pers->cofs;
		if ( ( chunk = GetChunk( pers ) ) == NULL ) {
			return 0;
		}
		if ( !chunk->len ) {
			return 0;
		}
		nextofs += chunk->len;

#ifdef DEBUG_PM_3DS_EX
		printf( "Chunk %04x (%s), len %d pers->cofs %x\n",chunk->id,DebugGetChunkName( chunk->id ),chunk->len,pers->cofs );
#endif
		/*** version ***/
		if ( chunk->id == CHUNK_VERSION ) {
			/* at this point i get the 3ds file version. since there */
			/* might be new additions to the 3ds file format in 4.0 */
			/* it might be a good idea to store the version somewhere */
			/* for later handling or message displaying */

			/* get the version */
			int version;
			version = GetWord( pers );
			GetWord( pers );
#ifdef DEBUG_PM_3DS
			printf( "FileVersion: %d\n",version );
#endif

			/* throw out a warning for version 4 models */
			if ( version == 4 ) {
				_pico_printf( PICO_WARNING,
							  "3DS version is 4. Model might load incorrectly." );
			}
			/* store the 3ds file version in pico special field 0 */
			/* PicoSetSurfaceSpecial(pers->surface,0,version); */		/* ydnar: this was causing a crash accessing uninitialized surface */

			/* rest of chunk is skipped here */
		}
		/*** editor data ***/
		if ( chunk->id == CHUNK_EDITOR_DATA ) {
			if ( !DoNextEditorDataChunk( pers,nextofs ) ) {
				return 0;
			}
			continue;
		}
		/* skip unknown chunk */
		pers->cofs = nextofs;
		if ( pers->cofs >= pers->maxofs ) {
			break;
		}
	}
	return 1;
}

/* _3ds_load:
 *  loads an autodesk 3ds model file.
 */
static picoModel_t *_3ds_load( PM_PARAMS_LOAD ){
	T3dsLoaderPers pers;
	picoModel_t    *model;
	char basename[128];

	/* create a new pico model */
	model = PicoNewModel();
	if ( model == NULL ) {
		/* user must have some serious ram problems ;) */
		return NULL;
	}
	/* get model's base name (eg. jeep from c:\models\jeep.3ds) */
	memset( basename,0,sizeof( basename ) );
	strncpy( basename,_pico_nopath( fileName ),sizeof( basename ) );
	_pico_setfext( basename,"" );

	/* initialize persistant vars (formerly static) */
	pers.model    =  model;
	pers.bufptr   = (picoByte_t *)buffer;
	pers.basename = (char *)basename;
	pers.maxofs   =  bufSize;
	pers.cofs     =  0L;

	/* do model setup */
	PicoSetModelFrameNum( model,frameNum );
	PicoSetModelName( model,fileName );
	PicoSetModelFileName( model,fileName );

	/* skip first chunk in file (magic) */
	GetChunk( &pers );

	/* process chunks */
	if ( !DoNextChunk( &pers,pers.maxofs ) ) {
		/* well, bleh i guess */
		PicoFreeModel( model );
		return NULL;
	}
	/* return allocated pico model */
	return model;
}

/* pico file format module definition */
const picoModule_t picoModule3DS =
{
	"0.86-b",                   /* module version string */
	"Autodesk 3Dstudio",        /* module display name */
	"seaw0lf",                  /* author's name */
	"2002 seaw0lf",             /* module copyright */
	{
		"3ds",NULL,NULL,NULL    /* default extensions to use */
	},
	_3ds_canload,               /* validation routine */
	_3ds_load,                  /* load routine */
	NULL,                       /* save validation routine */
	NULL                        /* save routine */
};
