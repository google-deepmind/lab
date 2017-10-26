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
#ifndef PICOMODEL_H
#define PICOMODEL_H

#ifdef __cplusplus
extern "C"
{
#endif



/* version */
#define PICOMODEL_VERSION       "0.8.20"


/* constants */
#define PICO_GROW_SHADERS       16
#define PICO_GROW_SURFACES      16
#define PICO_GROW_VERTEXES      1024
#define PICO_GROW_INDEXES       1024
#define PICO_GROW_ARRAYS        8
#define PICO_GROW_FACES         256
#define PICO_MAX_SPECIAL        8
#define PICO_MAX_DEFAULT_EXTS   4       /* max default extensions per module */


/* types */
typedef unsigned char picoByte_t;
typedef float picoVec_t;
typedef float picoVec2_t[ 2 ];
typedef float picoVec3_t[ 3 ];
typedef float picoVec4_t[ 4 ];
typedef picoByte_t picoColor_t[ 4 ];
typedef int picoIndex_t;

typedef enum
{
	PICO_BAD,
	PICO_TRIANGLES,
	PICO_PATCH
}
picoSurfaceType_t;

typedef enum
{
	PICO_NORMAL,
	PICO_VERBOSE,
	PICO_WARNING,
	PICO_ERROR,
	PICO_FATAL
}
picoPrintLevel_t;

typedef struct picoSurface_s picoSurface_t;
typedef struct picoShader_s picoShader_t;
typedef struct picoModel_s picoModel_t;
typedef struct picoModule_s picoModule_t;

struct picoSurface_s
{
	void                        *data;

	picoModel_t                 *model;     /* owner model */

	picoSurfaceType_t type;
	char                        *name;      /* sea: surface name */
	picoShader_t                *shader;    /* ydnar: changed to ptr */

	int numVertexes, maxVertexes;
	picoVec3_t                  *xyz;
	picoVec3_t                  *normal;
	picoIndex_t					*smoothingGroup;

	int numSTArrays, maxSTArrays;
	picoVec2_t                  **st;

	int numColorArrays, maxColorArrays;
	picoColor_t                 **color;

	int numIndexes, maxIndexes;
	picoIndex_t                 *index;

	int numFaceNormals, maxFaceNormals;
	picoVec3_t                  *faceNormal;

	int special[ PICO_MAX_SPECIAL ];
};


/* seaw0lf */
struct picoShader_s
{
	picoModel_t                 *model;         /* owner model */

	char                        *name;          /* shader name */
	char                        *mapName;       /* shader file name (name of diffuse texturemap) */
	picoColor_t ambientColor;                   /* ambient color of mesh (rgba) */
	picoColor_t diffuseColor;                   /* diffuse color of mesh (rgba) */
	picoColor_t specularColor;                  /* specular color of mesh (rgba) */
	float transparency;                         /* transparency (0..1; 1 = 100% transparent) */
	float shininess;                            /* shininess (0..128; 128 = 100% shiny) */
};

struct picoModel_s
{
	void                        *data;
	char                        *name;          /* model name */
	char                        *fileName;      /* sea: model file name */
	int frameNum;                               /* sea: renamed to frameNum */
	int numFrames;                              /* sea: number of frames */
	picoVec3_t mins;
	picoVec3_t maxs;

	int numShaders, maxShaders;
	picoShader_t                **shader;

	int numSurfaces, maxSurfaces;
	picoSurface_t               **surface;

	const picoModule_t          *module;        /* sea */
};


/* seaw0lf */
/* return codes used by the validation callbacks; pmv is short */
/* for 'pico module validation'. everything >PICO_PMV_OK means */
/* that there was an error. */
enum
{
	PICO_PMV_OK,            /* file valid */
	PICO_PMV_ERROR,         /* file not valid */
	PICO_PMV_ERROR_IDENT,   /* unknown file magic (aka ident) */
	PICO_PMV_ERROR_VERSION, /* unsupported file version */
	PICO_PMV_ERROR_SIZE,    /* file size error */
	PICO_PMV_ERROR_MEMORY,  /* out of memory error */
};

/* convenience (makes it easy to add new params to the callbacks) */
#define PM_PARAMS_CANLOAD \
	char *fileName, const void *buffer, int bufSize

#define PM_PARAMS_LOAD \
	char *fileName, int frameNum, const void *buffer, int bufSize

#define PM_PARAMS_CANSAVE \
	void

#define PM_PARAMS_SAVE \
	char *fileName, picoModel_t * model

/* pico file format module structure */
struct picoModule_s
{
	char                    *version;                               /* internal module version (e.g. '1.5-b2') */

	char                    *displayName;                           /* string used to display in guis, etc. */
	char                    *authorName;                            /* author name (eg. 'My Real Name') */
	char                    *copyright;                             /* copyright year and holder (eg. '2002 My Company') */

	char                    *defaultExts[ PICO_MAX_DEFAULT_EXTS ];  /* default file extensions used by this file type */
	int ( *canload )( PM_PARAMS_CANLOAD );                          /* checks whether module can load given file (returns PMVR_*) */
	picoModel_t             *( *load )( PM_PARAMS_LOAD );             /* parses model file data */
	int ( *cansave )( PM_PARAMS_CANSAVE );                          /* checks whether module can save (returns 1 or 0 and might spit out a message) */
	int ( *save )( PM_PARAMS_SAVE );                                /* saves a pico model in module's native model format */
};



/* general functions */
int                         PicoInit( void );
void                        PicoShutdown( void );
int                         PicoError( void );

void                        PicoSetMallocFunc( void *( *func )( size_t ) );
void                        PicoSetFreeFunc( void ( *func )( void* ) );
void                        PicoSetLoadFileFunc( void ( *func )( char*, unsigned char**, int* ) );
void                        PicoSetFreeFileFunc( void ( *func )( void* ) );
void                        PicoSetPrintFunc( void ( *func )( int, const char* ) );

const picoModule_t          **PicoModuleList( int *numModules );

picoModel_t                 *PicoLoadModel( char *name, int frameNum );

typedef size_t(*PicoInputStreamReadFunc)(void* inputStream, unsigned char* buffer, size_t length);
picoModel_t* PicoModuleLoadModelStream(const picoModule_t* module, void* inputStream, PicoInputStreamReadFunc inputStreamRead, size_t streamLength, int frameNum);


/* model functions */
picoModel_t                 *PicoNewModel( void );
void                        PicoFreeModel( picoModel_t *model );
int                         PicoAdjustModel( picoModel_t *model, int numShaders, int numSurfaces );


/* shader functions */
picoShader_t                *PicoNewShader( picoModel_t *model );
void                        PicoFreeShader( picoShader_t *shader );
picoShader_t                *PicoFindShader( picoModel_t *model, char *name, int caseSensitive );


/* surface functions */
picoSurface_t               *PicoNewSurface( picoModel_t *model );
void                        PicoFreeSurface( picoSurface_t *surface );
picoSurface_t               *PicoFindSurface( picoModel_t *model, char *name, int caseSensitive );
int                         PicoAdjustSurface( picoSurface_t *surface, int numVertexes, int numSTArrays, int numColorArrays, int numIndexes, int numFaceNormals );


/* setter functions */
void                        PicoSetModelName( picoModel_t *model, char *name );
void                        PicoSetModelFileName( picoModel_t *model, char *fileName );
void                        PicoSetModelFrameNum( picoModel_t *model, int frameNum );
void                        PicoSetModelNumFrames( picoModel_t *model, int numFrames );
void                        PicoSetModelData( picoModel_t *model, void *data );

void                        PicoSetShaderName( picoShader_t *shader, char *name );
void                        PicoSetShaderMapName( picoShader_t *shader, char *mapName );
void                        PicoSetShaderAmbientColor( picoShader_t *shader, picoColor_t color );
void                        PicoSetShaderDiffuseColor( picoShader_t *shader, picoColor_t color );
void                        PicoSetShaderSpecularColor( picoShader_t *shader, picoColor_t color );
void                        PicoSetShaderTransparency( picoShader_t *shader, float value );
void                        PicoSetShaderShininess( picoShader_t *shader, float value );

void                        PicoSetSurfaceData( picoSurface_t *surface, void *data );
void                        PicoSetSurfaceType( picoSurface_t *surface, picoSurfaceType_t type );
void                        PicoSetSurfaceName( picoSurface_t *surface, char *name );
void                        PicoSetSurfaceShader( picoSurface_t *surface, picoShader_t *shader );
void                        PicoSetSurfaceXYZ( picoSurface_t *surface, int num, picoVec3_t xyz );
void                        PicoSetSurfaceNormal( picoSurface_t *surface, int num, picoVec3_t normal );
void                        PicoSetSurfaceST( picoSurface_t *surface, int array, int num, picoVec2_t st );
void                        PicoSetSurfaceColor( picoSurface_t *surface, int array, int num, picoColor_t color );
void                        PicoSetSurfaceIndex( picoSurface_t *surface, int num, picoIndex_t index );
void                        PicoSetSurfaceIndexes( picoSurface_t *surface, int num, picoIndex_t *index, int count );
void                        PicoSetFaceNormal( picoSurface_t *surface, int num, picoVec3_t normal );
void                        PicoSetSurfaceSpecial( picoSurface_t *surface, int num, int special );
void                        PicoSetSurfaceSmoothingGroup( picoSurface_t *surface, int num, picoIndex_t smoothingGroup );


/* getter functions */
char                        *PicoGetModelName( picoModel_t *model );
char                        *PicoGetModelFileName( picoModel_t *model );
int                         PicoGetModelFrameNum( picoModel_t *model );
int                         PicoGetModelNumFrames( picoModel_t *model );
void                        *PicoGetModelData( picoModel_t *model );
int                         PicoGetModelNumShaders( picoModel_t *model );
picoShader_t                *PicoGetModelShader( picoModel_t *model, int num ); /* sea */
int                         PicoGetModelNumSurfaces( picoModel_t *model );
picoSurface_t               *PicoGetModelSurface( picoModel_t *model, int num );
int                         PicoGetModelTotalVertexes( picoModel_t *model );
int                         PicoGetModelTotalIndexes( picoModel_t *model );

char                        *PicoGetShaderName( picoShader_t *shader );
char                        *PicoGetShaderMapName( picoShader_t *shader );
picoByte_t                  *PicoGetShaderAmbientColor( picoShader_t *shader );
picoByte_t                  *PicoGetShaderDiffuseColor( picoShader_t *shader );
picoByte_t                  *PicoGetShaderSpecularColor( picoShader_t *shader );
float                       PicoGetShaderTransparency( picoShader_t *shader );
float                       PicoGetShaderShininess( picoShader_t *shader );

void                        *PicoGetSurfaceData( picoSurface_t *surface );
char                        *PicoGetSurfaceName( picoSurface_t *surface );      /* sea */
picoSurfaceType_t           PicoGetSurfaceType( picoSurface_t *surface );
char                        *PicoGetSurfaceName( picoSurface_t *surface );
picoShader_t                *PicoGetSurfaceShader( picoSurface_t *surface );    /* sea */

int                         PicoGetSurfaceNumVertexes( picoSurface_t *surface );
picoVec_t                   *PicoGetSurfaceXYZ( picoSurface_t *surface, int num );
picoVec_t                   *PicoGetSurfaceNormal( picoSurface_t *surface, int num );
picoVec_t                   *PicoGetSurfaceST( picoSurface_t *surface, int array, int num );
picoByte_t                  *PicoGetSurfaceColor( picoSurface_t *surface, int array, int num );
int                         PicoGetSurfaceNumIndexes( picoSurface_t *surface );
picoIndex_t                 PicoGetSurfaceIndex( picoSurface_t *surface, int num );
picoIndex_t                 *PicoGetSurfaceIndexes( picoSurface_t *surface, int num );
picoVec_t                   *PicoGetFaceNormal( picoSurface_t *surface, int num );
int                         PicoGetSurfaceSpecial( picoSurface_t *surface, int num );


/* hashtable related functions */
typedef struct picoVertexCombinationData_s
{
	picoVec3_t xyz, normal;
	picoVec2_t st;
	picoColor_t color;
} picoVertexCombinationData_t;

typedef struct picoVertexCombinationHash_s
{
	picoVertexCombinationData_t vcd;
	picoIndex_t index;

	void                        *data;

	struct picoVertexCombinationHash_s  *next;
} picoVertexCombinationHash_t;

int                         PicoGetHashTableSize( void );
unsigned int                PicoVertexCoordGenerateHash( picoVec3_t xyz );
picoVertexCombinationHash_t **PicoNewVertexCombinationHashTable( void );
void                        PicoFreeVertexCombinationHashTable( picoVertexCombinationHash_t **hashTable );
picoVertexCombinationHash_t *PicoFindVertexCombinationInHashTable( picoVertexCombinationHash_t **hashTable, picoVec3_t xyz, picoVec3_t normal, picoVec3_t st, picoColor_t color );
picoVertexCombinationHash_t *PicoAddVertexCombinationToHashTable( picoVertexCombinationHash_t **hashTable, picoVec3_t xyz, picoVec3_t normal, picoVec3_t st, picoColor_t color, picoIndex_t index );

/* specialized functions */
int                         PicoFindSurfaceVertexNum( picoSurface_t *surface, picoVec3_t xyz, picoVec3_t normal, int numSTs, picoVec2_t *st, int numColors, picoColor_t *color, picoIndex_t smoothingGroup );
void                        PicoFixSurfaceNormals( picoSurface_t *surface );
int                         PicoRemapModel( picoModel_t *model, char *remapFile );


void PicoAddTriangleToModel( picoModel_t *model, picoVec3_t** xyz, picoVec3_t** normals, int numSTs, picoVec2_t **st, int numColors, picoColor_t **colors, picoShader_t* shader, picoIndex_t* smoothingGroup );

/* end marker */
#ifdef __cplusplus
}
#endif

#endif
