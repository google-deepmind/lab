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
#define PM_MDC_C



/* dependencies */
#include "picointernal.h"

/* mdc model format */
#define MDC_MAGIC           "IDPC"
#define MDC_VERSION         2

/* mdc vertex scale */
#define MDC_SCALE           ( 1.0f / 64.0f )
#define MDC_MAX_OFS         127.0f
#define MDC_DIST_SCALE      0.05f

/* mdc decoding normal table */
const double mdcNormals[ 256 ][ 3 ] =
{
	{ 1.000000, 0.000000, 0.000000 },
	{ 0.980785, 0.195090, 0.000000 },
	{ 0.923880, 0.382683, 0.000000 },
	{ 0.831470, 0.555570, 0.000000 },
	{ 0.707107, 0.707107, 0.000000 },
	{ 0.555570, 0.831470, 0.000000 },
	{ 0.382683, 0.923880, 0.000000 },
	{ 0.195090, 0.980785, 0.000000 },
	{ -0.000000, 1.000000, 0.000000 },
	{ -0.195090, 0.980785, 0.000000 },
	{ -0.382683, 0.923880, 0.000000 },
	{ -0.555570, 0.831470, 0.000000 },
	{ -0.707107, 0.707107, 0.000000 },
	{ -0.831470, 0.555570, 0.000000 },
	{ -0.923880, 0.382683, 0.000000 },
	{ -0.980785, 0.195090, 0.000000 },
	{ -1.000000, -0.000000, 0.000000 },
	{ -0.980785, -0.195090, 0.000000 },
	{ -0.923880, -0.382683, 0.000000 },
	{ -0.831470, -0.555570, 0.000000 },
	{ -0.707107, -0.707107, 0.000000 },
	{ -0.555570, -0.831469, 0.000000 },
	{ -0.382684, -0.923880, 0.000000 },
	{ -0.195090, -0.980785, 0.000000 },
	{ 0.000000, -1.000000, 0.000000 },
	{ 0.195090, -0.980785, 0.000000 },
	{ 0.382684, -0.923879, 0.000000 },
	{ 0.555570, -0.831470, 0.000000 },
	{ 0.707107, -0.707107, 0.000000 },
	{ 0.831470, -0.555570, 0.000000 },
	{ 0.923880, -0.382683, 0.000000 },
	{ 0.980785, -0.195090, 0.000000 },
	{ 0.980785, 0.000000, -0.195090 },
	{ 0.956195, 0.218245, -0.195090 },
	{ 0.883657, 0.425547, -0.195090 },
	{ 0.766809, 0.611510, -0.195090 },
	{ 0.611510, 0.766809, -0.195090 },
	{ 0.425547, 0.883657, -0.195090 },
	{ 0.218245, 0.956195, -0.195090 },
	{ -0.000000, 0.980785, -0.195090 },
	{ -0.218245, 0.956195, -0.195090 },
	{ -0.425547, 0.883657, -0.195090 },
	{ -0.611510, 0.766809, -0.195090 },
	{ -0.766809, 0.611510, -0.195090 },
	{ -0.883657, 0.425547, -0.195090 },
	{ -0.956195, 0.218245, -0.195090 },
	{ -0.980785, -0.000000, -0.195090 },
	{ -0.956195, -0.218245, -0.195090 },
	{ -0.883657, -0.425547, -0.195090 },
	{ -0.766809, -0.611510, -0.195090 },
	{ -0.611510, -0.766809, -0.195090 },
	{ -0.425547, -0.883657, -0.195090 },
	{ -0.218245, -0.956195, -0.195090 },
	{ 0.000000, -0.980785, -0.195090 },
	{ 0.218245, -0.956195, -0.195090 },
	{ 0.425547, -0.883657, -0.195090 },
	{ 0.611510, -0.766809, -0.195090 },
	{ 0.766809, -0.611510, -0.195090 },
	{ 0.883657, -0.425547, -0.195090 },
	{ 0.956195, -0.218245, -0.195090 },
	{ 0.923880, 0.000000, -0.382683 },
	{ 0.892399, 0.239118, -0.382683 },
	{ 0.800103, 0.461940, -0.382683 },
	{ 0.653281, 0.653281, -0.382683 },
	{ 0.461940, 0.800103, -0.382683 },
	{ 0.239118, 0.892399, -0.382683 },
	{ -0.000000, 0.923880, -0.382683 },
	{ -0.239118, 0.892399, -0.382683 },
	{ -0.461940, 0.800103, -0.382683 },
	{ -0.653281, 0.653281, -0.382683 },
	{ -0.800103, 0.461940, -0.382683 },
	{ -0.892399, 0.239118, -0.382683 },
	{ -0.923880, -0.000000, -0.382683 },
	{ -0.892399, -0.239118, -0.382683 },
	{ -0.800103, -0.461940, -0.382683 },
	{ -0.653282, -0.653281, -0.382683 },
	{ -0.461940, -0.800103, -0.382683 },
	{ -0.239118, -0.892399, -0.382683 },
	{ 0.000000, -0.923880, -0.382683 },
	{ 0.239118, -0.892399, -0.382683 },
	{ 0.461940, -0.800103, -0.382683 },
	{ 0.653281, -0.653282, -0.382683 },
	{ 0.800103, -0.461940, -0.382683 },
	{ 0.892399, -0.239117, -0.382683 },
	{ 0.831470, 0.000000, -0.555570 },
	{ 0.790775, 0.256938, -0.555570 },
	{ 0.672673, 0.488726, -0.555570 },
	{ 0.488726, 0.672673, -0.555570 },
	{ 0.256938, 0.790775, -0.555570 },
	{ -0.000000, 0.831470, -0.555570 },
	{ -0.256938, 0.790775, -0.555570 },
	{ -0.488726, 0.672673, -0.555570 },
	{ -0.672673, 0.488726, -0.555570 },
	{ -0.790775, 0.256938, -0.555570 },
	{ -0.831470, -0.000000, -0.555570 },
	{ -0.790775, -0.256938, -0.555570 },
	{ -0.672673, -0.488726, -0.555570 },
	{ -0.488725, -0.672673, -0.555570 },
	{ -0.256938, -0.790775, -0.555570 },
	{ 0.000000, -0.831470, -0.555570 },
	{ 0.256938, -0.790775, -0.555570 },
	{ 0.488725, -0.672673, -0.555570 },
	{ 0.672673, -0.488726, -0.555570 },
	{ 0.790775, -0.256938, -0.555570 },
	{ 0.707107, 0.000000, -0.707107 },
	{ 0.653281, 0.270598, -0.707107 },
	{ 0.500000, 0.500000, -0.707107 },
	{ 0.270598, 0.653281, -0.707107 },
	{ -0.000000, 0.707107, -0.707107 },
	{ -0.270598, 0.653282, -0.707107 },
	{ -0.500000, 0.500000, -0.707107 },
	{ -0.653281, 0.270598, -0.707107 },
	{ -0.707107, -0.000000, -0.707107 },
	{ -0.653281, -0.270598, -0.707107 },
	{ -0.500000, -0.500000, -0.707107 },
	{ -0.270598, -0.653281, -0.707107 },
	{ 0.000000, -0.707107, -0.707107 },
	{ 0.270598, -0.653281, -0.707107 },
	{ 0.500000, -0.500000, -0.707107 },
	{ 0.653282, -0.270598, -0.707107 },
	{ 0.555570, 0.000000, -0.831470 },
	{ 0.481138, 0.277785, -0.831470 },
	{ 0.277785, 0.481138, -0.831470 },
	{ -0.000000, 0.555570, -0.831470 },
	{ -0.277785, 0.481138, -0.831470 },
	{ -0.481138, 0.277785, -0.831470 },
	{ -0.555570, -0.000000, -0.831470 },
	{ -0.481138, -0.277785, -0.831470 },
	{ -0.277785, -0.481138, -0.831470 },
	{ 0.000000, -0.555570, -0.831470 },
	{ 0.277785, -0.481138, -0.831470 },
	{ 0.481138, -0.277785, -0.831470 },
	{ 0.382683, 0.000000, -0.923880 },
	{ 0.270598, 0.270598, -0.923880 },
	{ -0.000000, 0.382683, -0.923880 },
	{ -0.270598, 0.270598, -0.923880 },
	{ -0.382683, -0.000000, -0.923880 },
	{ -0.270598, -0.270598, -0.923880 },
	{ 0.000000, -0.382683, -0.923880 },
	{ 0.270598, -0.270598, -0.923880 },
	{ 0.195090, 0.000000, -0.980785 },
	{ -0.000000, 0.195090, -0.980785 },
	{ -0.195090, -0.000000, -0.980785 },
	{ 0.000000, -0.195090, -0.980785 },
	{ 0.980785, 0.000000, 0.195090 },
	{ 0.956195, 0.218245, 0.195090 },
	{ 0.883657, 0.425547, 0.195090 },
	{ 0.766809, 0.611510, 0.195090 },
	{ 0.611510, 0.766809, 0.195090 },
	{ 0.425547, 0.883657, 0.195090 },
	{ 0.218245, 0.956195, 0.195090 },
	{ -0.000000, 0.980785, 0.195090 },
	{ -0.218245, 0.956195, 0.195090 },
	{ -0.425547, 0.883657, 0.195090 },
	{ -0.611510, 0.766809, 0.195090 },
	{ -0.766809, 0.611510, 0.195090 },
	{ -0.883657, 0.425547, 0.195090 },
	{ -0.956195, 0.218245, 0.195090 },
	{ -0.980785, -0.000000, 0.195090 },
	{ -0.956195, -0.218245, 0.195090 },
	{ -0.883657, -0.425547, 0.195090 },
	{ -0.766809, -0.611510, 0.195090 },
	{ -0.611510, -0.766809, 0.195090 },
	{ -0.425547, -0.883657, 0.195090 },
	{ -0.218245, -0.956195, 0.195090 },
	{ 0.000000, -0.980785, 0.195090 },
	{ 0.218245, -0.956195, 0.195090 },
	{ 0.425547, -0.883657, 0.195090 },
	{ 0.611510, -0.766809, 0.195090 },
	{ 0.766809, -0.611510, 0.195090 },
	{ 0.883657, -0.425547, 0.195090 },
	{ 0.956195, -0.218245, 0.195090 },
	{ 0.923880, 0.000000, 0.382683 },
	{ 0.892399, 0.239118, 0.382683 },
	{ 0.800103, 0.461940, 0.382683 },
	{ 0.653281, 0.653281, 0.382683 },
	{ 0.461940, 0.800103, 0.382683 },
	{ 0.239118, 0.892399, 0.382683 },
	{ -0.000000, 0.923880, 0.382683 },
	{ -0.239118, 0.892399, 0.382683 },
	{ -0.461940, 0.800103, 0.382683 },
	{ -0.653281, 0.653281, 0.382683 },
	{ -0.800103, 0.461940, 0.382683 },
	{ -0.892399, 0.239118, 0.382683 },
	{ -0.923880, -0.000000, 0.382683 },
	{ -0.892399, -0.239118, 0.382683 },
	{ -0.800103, -0.461940, 0.382683 },
	{ -0.653282, -0.653281, 0.382683 },
	{ -0.461940, -0.800103, 0.382683 },
	{ -0.239118, -0.892399, 0.382683 },
	{ 0.000000, -0.923880, 0.382683 },
	{ 0.239118, -0.892399, 0.382683 },
	{ 0.461940, -0.800103, 0.382683 },
	{ 0.653281, -0.653282, 0.382683 },
	{ 0.800103, -0.461940, 0.382683 },
	{ 0.892399, -0.239117, 0.382683 },
	{ 0.831470, 0.000000, 0.555570 },
	{ 0.790775, 0.256938, 0.555570 },
	{ 0.672673, 0.488726, 0.555570 },
	{ 0.488726, 0.672673, 0.555570 },
	{ 0.256938, 0.790775, 0.555570 },
	{ -0.000000, 0.831470, 0.555570 },
	{ -0.256938, 0.790775, 0.555570 },
	{ -0.488726, 0.672673, 0.555570 },
	{ -0.672673, 0.488726, 0.555570 },
	{ -0.790775, 0.256938, 0.555570 },
	{ -0.831470, -0.000000, 0.555570 },
	{ -0.790775, -0.256938, 0.555570 },
	{ -0.672673, -0.488726, 0.555570 },
	{ -0.488725, -0.672673, 0.555570 },
	{ -0.256938, -0.790775, 0.555570 },
	{ 0.000000, -0.831470, 0.555570 },
	{ 0.256938, -0.790775, 0.555570 },
	{ 0.488725, -0.672673, 0.555570 },
	{ 0.672673, -0.488726, 0.555570 },
	{ 0.790775, -0.256938, 0.555570 },
	{ 0.707107, 0.000000, 0.707107 },
	{ 0.653281, 0.270598, 0.707107 },
	{ 0.500000, 0.500000, 0.707107 },
	{ 0.270598, 0.653281, 0.707107 },
	{ -0.000000, 0.707107, 0.707107 },
	{ -0.270598, 0.653282, 0.707107 },
	{ -0.500000, 0.500000, 0.707107 },
	{ -0.653281, 0.270598, 0.707107 },
	{ -0.707107, -0.000000, 0.707107 },
	{ -0.653281, -0.270598, 0.707107 },
	{ -0.500000, -0.500000, 0.707107 },
	{ -0.270598, -0.653281, 0.707107 },
	{ 0.000000, -0.707107, 0.707107 },
	{ 0.270598, -0.653281, 0.707107 },
	{ 0.500000, -0.500000, 0.707107 },
	{ 0.653282, -0.270598, 0.707107 },
	{ 0.555570, 0.000000, 0.831470 },
	{ 0.481138, 0.277785, 0.831470 },
	{ 0.277785, 0.481138, 0.831470 },
	{ -0.000000, 0.555570, 0.831470 },
	{ -0.277785, 0.481138, 0.831470 },
	{ -0.481138, 0.277785, 0.831470 },
	{ -0.555570, -0.000000, 0.831470 },
	{ -0.481138, -0.277785, 0.831470 },
	{ -0.277785, -0.481138, 0.831470 },
	{ 0.000000, -0.555570, 0.831470 },
	{ 0.277785, -0.481138, 0.831470 },
	{ 0.481138, -0.277785, 0.831470 },
	{ 0.382683, 0.000000, 0.923880 },
	{ 0.270598, 0.270598, 0.923880 },
	{ -0.000000, 0.382683, 0.923880 },
	{ -0.270598, 0.270598, 0.923880 },
	{ -0.382683, -0.000000, 0.923880 },
	{ -0.270598, -0.270598, 0.923880 },
	{ 0.000000, -0.382683, 0.923880 },
	{ 0.270598, -0.270598, 0.923880 },
	{ 0.195090, 0.000000, 0.980785 },
	{ -0.000000, 0.195090, 0.980785 },
	{ -0.195090, -0.000000, 0.980785 },
	{ 0.000000, -0.195090, 0.980785 }
};

/* mdc model frame information */
typedef struct mdcFrame_s
{
	float bounds[ 2 ][ 3 ];
	float localOrigin[ 3 ];
	float radius;
	char creator[ 16 ];
}
mdcFrame_t;

/* mdc model tag information */
typedef struct mdcTag_s
{
	short xyz[3];
	short angles[3];
}
mdcTag_t;

/* mdc surface mdc (one object mesh) */
typedef struct mdcSurface_s
{
	char magic[ 4 ];
	char name[ 64 ];                /* polyset name */
	int flags;
	int numCompFrames;              /* all surfaces in a model should have the same */
	int numBaseFrames;              /* ditto */
	int numShaders;                 /* all model surfaces should have the same */
	int numVerts;
	int numTriangles;
	int ofsTriangles;
	int ofsShaders;                 /* offset from start of mdcSurface_t */
	int ofsSt;                      /* texture coords are common for all frames */
	int ofsXyzNormals;              /* numVerts * numBaseFrames */
	int ofsXyzCompressed;           /* numVerts * numCompFrames */

	int ofsFrameBaseFrames;         /* numFrames */
	int ofsFrameCompFrames;         /* numFrames */
	int ofsEnd;                     /* next surface follows */
}
mdcSurface_t;

typedef struct mdcShader_s
{
	char name[ 64 ];
	int shaderIndex;            /* for ingame use */
}
mdcShader_t;

typedef struct mdcTriangle_s
{
	int indexes[ 3 ];
}
mdcTriangle_t;

typedef struct mdcTexCoord_s
{
	float st[ 2 ];
}
mdcTexCoord_t;

typedef struct mdcVertex_s
{
	short xyz[ 3 ];
	short normal;
}
mdcVertex_t;

typedef struct mdcXyzCompressed_s
{
	unsigned int ofsVec;        /* offset direction from the last base frame */
}
mdcXyzCompressed_t;


/* mdc model file mdc structure */
typedef struct mdc_s
{
	char magic[ 4 ];            /* MDC_MAGIC */
	int version;
	char name[ 64 ];            /* model name */
	int flags;
	int numFrames;
	int numTags;
	int numSurfaces;
	int numSkins;               /* number of skins for the mesh */
	int ofsFrames;              /* offset for first frame */
	int ofsTagNames;            /* numTags */
	int ofsTags;                /* numFrames * numTags */
	int ofsSurfaces;            /* first surface, others follow */
	int ofsEnd;                 /* end of file */
}
mdc_t;




/*
   _mdc_canload()
   validates a Return to Castle Wolfenstein model file. btw, i use the
   preceding underscore cause it's a static func referenced
   by one structure only.
 */

static int _mdc_canload( PM_PARAMS_CANLOAD ){
	mdc_t   *mdc;


	/* to keep the compiler happy */
	*fileName = *fileName;

	/* sanity check */
	if ( bufSize < ( sizeof( *mdc ) * 2 ) ) {
		return PICO_PMV_ERROR_SIZE;
	}

	/* set as mdc */
	mdc = (mdc_t*) buffer;

	/* check mdc magic */
	if ( *( (int*) mdc->magic ) != *( (int*) MDC_MAGIC ) ) {
		return PICO_PMV_ERROR_IDENT;
	}

	/* check mdc version */
	if ( _pico_little_long( mdc->version ) != MDC_VERSION ) {
		return PICO_PMV_ERROR_VERSION;
	}

	/* file seems to be a valid mdc */
	return PICO_PMV_OK;
}



/*
   _mdc_load()
   loads a Return to Castle Wolfenstein mdc model file.
 */

static picoModel_t *_mdc_load( PM_PARAMS_LOAD ){
	int i, j;
	picoByte_t          *bb;
	mdc_t               *mdc;
	mdcSurface_t        *surface;
	mdcShader_t         *shader;
	mdcTexCoord_t       *texCoord;
	mdcFrame_t          *frame;
	mdcTriangle_t       *triangle;
	mdcVertex_t         *vertex;
	mdcXyzCompressed_t  *vertexComp;
	short               *mdcShort, *mdcCompVert;
	double lat, lng;

	picoModel_t         *picoModel;
	picoSurface_t       *picoSurface;
	picoShader_t        *picoShader;
	picoVec3_t xyz, normal;
	picoVec2_t st;
	picoColor_t color;


	/* -------------------------------------------------
	   mdc loading
	   ------------------------------------------------- */


	/* set as mdc */
	bb = (picoByte_t*) buffer;
	mdc = (mdc_t*) buffer;

	/* check ident and version */
	if ( *( (int*) mdc->magic ) != *( (int*) MDC_MAGIC ) || _pico_little_long( mdc->version ) != MDC_VERSION ) {
		/* not an mdc file (todo: set error) */
		return NULL;
	}

	/* swap mdc */
	mdc->version = _pico_little_long( mdc->version );
	mdc->numFrames = _pico_little_long( mdc->numFrames );
	mdc->numTags = _pico_little_long( mdc->numTags );
	mdc->numSurfaces = _pico_little_long( mdc->numSurfaces );
	mdc->numSkins = _pico_little_long( mdc->numSkins );
	mdc->ofsFrames = _pico_little_long( mdc->ofsFrames );
	mdc->ofsTags = _pico_little_long( mdc->ofsTags );
	mdc->ofsTagNames = _pico_little_long( mdc->ofsTagNames );
	mdc->ofsSurfaces = _pico_little_long( mdc->ofsSurfaces );
	mdc->ofsEnd = _pico_little_long( mdc->ofsEnd );

	/* do frame check */
	if ( mdc->numFrames < 1 ) {
		_pico_printf( PICO_ERROR, "MDC with 0 frames" );
		return NULL;
	}

	if ( frameNum < 0 || frameNum >= mdc->numFrames ) {
		_pico_printf( PICO_ERROR, "Invalid or out-of-range MDC frame specified" );
		return NULL;
	}

	/* swap frames */
	frame = (mdcFrame_t*) ( bb + mdc->ofsFrames );
	for ( i = 0; i < mdc->numFrames; i++, frame++ )
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
	surface = (mdcSurface_t*) ( bb + mdc->ofsSurfaces );
	for ( i = 0; i < mdc->numSurfaces; i++ )
	{
		/* swap surface mdc */
		surface->flags = _pico_little_long( surface->flags );
		surface->numBaseFrames = _pico_little_long( surface->numBaseFrames );
		surface->numCompFrames = _pico_little_long( surface->numCompFrames );
		surface->numShaders = _pico_little_long( surface->numShaders );
		surface->numTriangles = _pico_little_long( surface->numTriangles );
		surface->ofsTriangles = _pico_little_long( surface->ofsTriangles );
		surface->numVerts = _pico_little_long( surface->numVerts );
		surface->ofsShaders = _pico_little_long( surface->ofsShaders );
		surface->ofsSt = _pico_little_long( surface->ofsSt );
		surface->ofsXyzNormals = _pico_little_long( surface->ofsXyzNormals );
		surface->ofsXyzCompressed = _pico_little_long( surface->ofsXyzCompressed );
		surface->ofsFrameBaseFrames = _pico_little_long( surface->ofsFrameBaseFrames );
		surface->ofsFrameCompFrames = _pico_little_long( surface->ofsFrameCompFrames );
		surface->ofsEnd = _pico_little_long( surface->ofsEnd );

		/* swap triangles */
		triangle = (mdcTriangle_t*) ( (picoByte_t*) surface + surface->ofsTriangles );
		for ( j = 0; j < surface->numTriangles; j++, triangle++ )
		{
			/* sea: swaps fixed */
			triangle->indexes[ 0 ] = _pico_little_long( triangle->indexes[ 0 ] );
			triangle->indexes[ 1 ] = _pico_little_long( triangle->indexes[ 1 ] );
			triangle->indexes[ 2 ] = _pico_little_long( triangle->indexes[ 2 ] );
		}

		/* swap st coords */
		texCoord = (mdcTexCoord_t*) ( (picoByte_t*) surface + surface->ofsSt );
		for ( j = 0; j < surface->numVerts; j++, texCoord++ )
		{
			texCoord->st[ 0 ] = _pico_little_float( texCoord->st[ 0 ] );
			texCoord->st[ 1 ] = _pico_little_float( texCoord->st[ 1 ] );
		}

		/* swap xyz/normals */
		vertex = (mdcVertex_t*) ( (picoByte_t*) surface + surface->ofsXyzNormals );
		for ( j = 0; j < ( surface->numVerts * surface->numBaseFrames ); j++, vertex++ )
		{
			vertex->xyz[ 0 ] = _pico_little_short( vertex->xyz[ 0 ] );
			vertex->xyz[ 1 ] = _pico_little_short( vertex->xyz[ 1 ] );
			vertex->xyz[ 2 ] = _pico_little_short( vertex->xyz[ 2 ] );
			vertex->normal   = _pico_little_short( vertex->normal );
		}

		/* swap xyz/compressed */
		vertexComp = (mdcXyzCompressed_t*) ( (picoByte_t*) surface + surface->ofsXyzCompressed );
		for ( j = 0; j < ( surface->numVerts * surface->numCompFrames ); j++, vertexComp++ )
		{
			vertexComp->ofsVec  = _pico_little_long( vertexComp->ofsVec );
		}

		/* swap base frames */
		mdcShort = (short *) ( (picoByte_t*) surface + surface->ofsFrameBaseFrames );
		for ( j = 0; j < mdc->numFrames; j++, mdcShort++ )
		{
			*mdcShort   = _pico_little_short( *mdcShort );
		}

		/* swap compressed frames */
		mdcShort = (short *) ( (picoByte_t*) surface + surface->ofsFrameCompFrames );
		for ( j = 0; j < mdc->numFrames; j++, mdcShort++ )
		{
			*mdcShort   = _pico_little_short( *mdcShort );
		}

		/* get next surface */
		surface = (mdcSurface_t*) ( (picoByte_t*) surface + surface->ofsEnd );
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
	PicoSetModelNumFrames( picoModel, mdc->numFrames ); /* sea */
	PicoSetModelName( picoModel, fileName );
	PicoSetModelFileName( picoModel, fileName );

	/* mdc surfaces become picomodel surfaces */
	surface = (mdcSurface_t*) ( bb + mdc->ofsSurfaces );

	/* run through mdc surfaces */
	for ( i = 0; i < mdc->numSurfaces; i++ )
	{
		/* allocate new pico surface */
		picoSurface = PicoNewSurface( picoModel );
		if ( picoSurface == NULL ) {
			_pico_printf( PICO_ERROR, "Unable to allocate a new model surface" );
			PicoFreeModel( picoModel ); /* sea */
			return NULL;
		}

		/* mdc model surfaces are all triangle meshes */
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
		shader = (mdcShader_t*) ( (picoByte_t*) surface + surface->ofsShaders );
		_pico_setfext( shader->name, "" );
		_pico_unixify( shader->name );
		PicoSetShaderName( picoShader, shader->name );

		/* associate current surface with newly created shader */
		PicoSetSurfaceShader( picoSurface, picoShader );

		/* copy indexes */
		triangle = (mdcTriangle_t *) ( (picoByte_t*) surface + surface->ofsTriangles );

		for ( j = 0; j < surface->numTriangles; j++, triangle++ )
		{
			PicoSetSurfaceIndex( picoSurface, ( j * 3 + 0 ), (picoIndex_t) triangle->indexes[ 0 ] );
			PicoSetSurfaceIndex( picoSurface, ( j * 3 + 1 ), (picoIndex_t) triangle->indexes[ 1 ] );
			PicoSetSurfaceIndex( picoSurface, ( j * 3 + 2 ), (picoIndex_t) triangle->indexes[ 2 ] );
		}

		/* copy vertexes */
		texCoord = (mdcTexCoord_t*) ( (picoByte_t *) surface + surface->ofsSt );
		mdcShort = (short *) ( (picoByte_t *) surface + surface->ofsXyzNormals ) + ( (int)*( (short *) ( (picoByte_t *) surface + surface->ofsFrameBaseFrames ) + frameNum ) * surface->numVerts * 4 );
		if ( surface->numCompFrames > 0 ) {
			mdcCompVert = (short *) ( (picoByte_t *) surface + surface->ofsFrameCompFrames ) + frameNum;
			if ( *mdcCompVert >= 0 ) {
				vertexComp = (mdcXyzCompressed_t *) ( (picoByte_t *) surface + surface->ofsXyzCompressed ) + ( *mdcCompVert * surface->numVerts );
			}
		}
		_pico_set_color( color, 255, 255, 255, 255 );

		for ( j = 0; j < surface->numVerts; j++, texCoord++, mdcShort += 4 )
		{
			/* set vertex origin */
			xyz[ 0 ] = MDC_SCALE * mdcShort[ 0 ];
			xyz[ 1 ] = MDC_SCALE * mdcShort[ 1 ];
			xyz[ 2 ] = MDC_SCALE * mdcShort[ 2 ];

			/* add compressed ofsVec */
			if ( surface->numCompFrames > 0 && *mdcCompVert >= 0 ) {
				xyz[ 0 ] += ( (float) ( ( vertexComp->ofsVec ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE;
				xyz[ 1 ] += ( (float) ( ( vertexComp->ofsVec >> 8 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE;
				xyz[ 2 ] += ( (float) ( ( vertexComp->ofsVec >> 16 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE;
				PicoSetSurfaceXYZ( picoSurface, j, xyz );

				normal[ 0 ] = (float) mdcNormals[ ( vertexComp->ofsVec >> 24 ) ][ 0 ];
				normal[ 1 ] = (float) mdcNormals[ ( vertexComp->ofsVec >> 24 ) ][ 1 ];
				normal[ 2 ] = (float) mdcNormals[ ( vertexComp->ofsVec >> 24 ) ][ 2 ];
				PicoSetSurfaceNormal( picoSurface, j, normal );

				vertexComp++;
			}
			else
			{
				PicoSetSurfaceXYZ( picoSurface, j, xyz );

				/* decode lat/lng normal to 3 float normal */
				lat = (float) ( ( *( mdcShort + 3 ) >> 8 ) & 0xff );
				lng = (float) ( *( mdcShort + 3 ) & 0xff );
				lat *= PICO_PI / 128;
				lng *= PICO_PI / 128;
				normal[ 0 ] = (picoVec_t) cos( lat ) * (picoVec_t) sin( lng );
				normal[ 1 ] = (picoVec_t) sin( lat ) * (picoVec_t) sin( lng );
				normal[ 2 ] = (picoVec_t) cos( lng );
				PicoSetSurfaceNormal( picoSurface, j, normal );
			}

			/* set st coords */
			st[ 0 ] = texCoord->st[ 0 ];
			st[ 1 ] = texCoord->st[ 1 ];
			PicoSetSurfaceST( picoSurface, 0, j, st );

			/* set color */
			PicoSetSurfaceColor( picoSurface, 0, j, color );
		}

		/* get next surface */
		surface = (mdcSurface_t*) ( (picoByte_t*) surface + surface->ofsEnd );
	}

	/* return the new pico model */
	return picoModel;
}



/* pico file format module definition */
const picoModule_t picoModuleMDC =
{
	"1.3",                          /* module version string */
	"RtCW MDC",                     /* module display name */
	"Arnout van Meer",              /* author's name */
	"2002 Arnout van Meer",         /* module copyright */
	{
		"mdc", NULL, NULL, NULL     /* default extensions to use */
	},
	_mdc_canload,                   /* validation routine */
	_mdc_load,                      /* load routine */
	NULL,                           /* save validation routine */
	NULL                            /* save routine */
};
