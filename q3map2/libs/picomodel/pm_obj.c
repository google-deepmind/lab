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
#define PM_OBJ_C

/* dependencies */
#include "picointernal.h"

/* disable warnings */
#ifdef _WIN32
#pragma warning( disable:4100 )		/* unref param */
#endif

/* todo:
 * - '_obj_load' code crashes in a weird way after
 *   '_obj_mtl_load' for a few .mtl files
 * - process 'mtllib' rather than using <model>.mtl
 */
/* uncomment when debugging this module */
/* #define DEBUG_PM_OBJ */
/* #define DEBUG_PM_OBJ_EX */

/* this holds temporary vertex data read by parser */
typedef struct SObjVertexData
{
	picoVec3_t v;           /* geometric vertices */
	picoVec2_t vt;          /* texture vertices */
	picoVec3_t vn;          /* vertex normals (optional) */
}
TObjVertexData;

/* _obj_canload:
 *  validates a wavefront obj model file.
 */
static int _obj_canload( PM_PARAMS_CANLOAD ){
	picoParser_t *p;

	/* check data length */
	if ( bufSize < 30 ) {
		return PICO_PMV_ERROR_SIZE;
	}

	/* first check file extension. we have to do this for objs */
	/* cause there is no good way to identify the contents */
	if ( _pico_stristr( fileName,".obj" ) != NULL ||
		 _pico_stristr( fileName,".wf" ) != NULL ) {
		return PICO_PMV_OK;
	}
	/* if the extension check failed we parse through the first */
	/* few lines in file and look for common keywords often */
	/* appearing at the beginning of wavefront objects */

	/* alllocate a new pico parser */
	p = _pico_new_parser( (picoByte_t *)buffer,bufSize );
	if ( p == NULL ) {
		return PICO_PMV_ERROR_MEMORY;
	}

	/* parse obj head line by line for type check */
	while ( 1 )
	{
		/* get first token on line */
		if ( _pico_parse_first( p ) == NULL ) {
			break;
		}

		/* we only parse the first few lines, say 80 */
		if ( p->curLine > 80 ) {
			break;
		}

		/* skip empty lines */
		if ( p->token == NULL || !strlen( p->token ) ) {
			continue;
		}

		/* material library keywords are teh good */
		if ( !_pico_stricmp( p->token,"usemtl" ) ||
			 !_pico_stricmp( p->token,"mtllib" ) ||
			 !_pico_stricmp( p->token,"g" ) ||
			 !_pico_stricmp( p->token,"v" ) ) { /* v,g bit fishy, but uh... */
			/* free the pico parser thing */
			_pico_free_parser( p );

			/* seems to be a valid wavefront obj */
			return PICO_PMV_OK;
		}
		/* skip rest of line */
		_pico_parse_skip_rest( p );
	}
	/* free the pico parser thing */
	_pico_free_parser( p );

	/* doesn't really look like an obj to us */
	return PICO_PMV_ERROR;
}

/* SizeObjVertexData:
 *   This pretty piece of 'alloc ahead' code dynamically
 *   allocates - and reallocates as soon as required -
 *   my vertex data array in even steps.
 */
#define SIZE_OBJ_STEP  4096

static TObjVertexData *SizeObjVertexData(
	TObjVertexData *vertexData, int reqEntries,
	int *entries, int *allocated ){
	int newAllocated;

	/* sanity checks */
	if ( reqEntries < 1 ) {
		return NULL;
	}
	if ( entries == NULL || allocated == NULL ) {
		return NULL; /* must have */

	}
	/* no need to grow yet */
	if ( vertexData && ( reqEntries < *allocated ) ) {
		*entries = reqEntries;
		return vertexData;
	}
	/* given vertex data ptr not allocated yet */
	if ( vertexData == NULL ) {
		/* how many entries to allocate */
		newAllocated = ( reqEntries > SIZE_OBJ_STEP ) ?
					   reqEntries : SIZE_OBJ_STEP;

		/* throw out an extended debug message */
#ifdef DEBUG_PM_OBJ_EX
		printf( "SizeObjVertexData: allocate (%d entries)\n",
				newAllocated );
#endif
		/* first time allocation */
		vertexData = (TObjVertexData *)
					 _pico_alloc( sizeof( TObjVertexData ) * newAllocated );

		/* allocation failed */
		if ( vertexData == NULL ) {
			return NULL;
		}

		/* allocation succeeded */
		*allocated = newAllocated;
		*entries   = reqEntries;
		return vertexData;
	}
	/* given vertex data ptr needs to be resized */
	if ( reqEntries == *allocated ) {
		newAllocated = ( *allocated + SIZE_OBJ_STEP );

		/* throw out an extended debug message */
#ifdef DEBUG_PM_OBJ_EX
		printf( "SizeObjVertexData: reallocate (%d entries)\n",
				newAllocated );
#endif
		/* try to reallocate */
		vertexData = (TObjVertexData *)
					 _pico_realloc( (void *)&vertexData,
									sizeof( TObjVertexData ) * ( *allocated ),
									sizeof( TObjVertexData ) * ( newAllocated ) );

		/* reallocation failed */
		if ( vertexData == NULL ) {
			return NULL;
		}

		/* reallocation succeeded */
		*allocated = newAllocated;
		*entries   = reqEntries;
		return vertexData;
	}
	/* we're b0rked when we reach this */
	return NULL;
}

static void FreeObjVertexData( TObjVertexData *vertexData ){
	if ( vertexData != NULL ) {
		free( (TObjVertexData *)vertexData );
	}
}

#if 0
static int _obj_mtl_load( picoModel_t *model ){
	//picoShader_t *curShader = NULL;
	picoParser_t *p;
	picoByte_t   *mtlBuffer;
	int mtlBufSize;
	char         *fileName;

	/* sanity checks */
	if ( model == NULL || model->fileName == NULL ) {
		return 0;
	}

	/* skip if we have a zero length model file name */
	if ( !strlen( model->fileName ) ) {
		return 0;
	}

	/* helper */
	#define _obj_mtl_error_return \
	{ \
		_pico_free_parser( p );	\
		_pico_free_file( mtlBuffer ); \
		_pico_free( fileName );	\
		return 0; \
	}
	/* alloc copy of model file name */
	fileName = _pico_clone_alloc( model->fileName );
	if ( fileName == NULL ) {
		return 0;
	}

	/* change extension of model file to .mtl */
	_pico_setfext( fileName, "mtl" );

	/* load .mtl file contents */
	_pico_load_file( fileName,&mtlBuffer,&mtlBufSize );

	/* check result */
	if ( mtlBufSize == 0 ) {
		return 1;                       /* file is empty: no error */
	}
	if ( mtlBufSize  < 0 ) {
		return 0;                       /* load failed: error */

	}
	/* create a new pico parser */
	p = _pico_new_parser( mtlBuffer, mtlBufSize );
	if ( p == NULL ) {
		_obj_mtl_error_return;
	}

	/* doo teh .mtl parse */
	while ( 1 )
	{
		/* get next token in material file */
		if ( _pico_parse( p,1 ) == NULL ) {
			break;
		}
#if 0

		/* skip empty lines */
		if ( p->token == NULL || !strlen( p->token ) ) {
			continue;
		}

		/* skip comment lines */
		if ( p->token[0] == '#' ) {
			_pico_parse_skip_rest( p );
			continue;
		}
		/* new material */
		if ( !_pico_stricmp( p->token,"newmtl" ) ) {
			picoShader_t *shader;
			char *name;

			/* get material name */
			name = _pico_parse( p,0 );

			/* validate material name */
			if ( name == NULL || !strlen( name ) ) {
				_pico_printf( PICO_ERROR,"Missing material name in MTL, line %d.",p->curLine );
				_obj_mtl_error_return;
			}
			/* create a new pico shader */
			shader = PicoNewShader( model );
			if ( shader == NULL ) {
				_obj_mtl_error_return;
			}

			/* set shader name */
			PicoSetShaderName( shader,name );

			/* assign pointer to current shader */
			curShader = shader;
		}
		/* diffuse map name */
		else if ( !_pico_stricmp( p->token,"map_kd" ) ) {
			char *mapName;

			/* pointer to current shader must be valid */
			if ( curShader == NULL ) {
				_obj_mtl_error_return;
			}

			/* get material's diffuse map name */
			mapName = _pico_parse( p,0 );

			/* validate map name */
			if ( mapName == NULL || !strlen( mapName ) ) {
				_pico_printf( PICO_ERROR,"Missing material map name in MTL, line %d.",p->curLine );
				_obj_mtl_error_return;
			}
			/* set shader map name */
			PicoSetShaderMapName( shader,mapName );
		}
		/* dissolve factor (pseudo transparency 0..1) */
		/* where 0 means 100% transparent and 1 means opaque */
		else if ( !_pico_stricmp( p->token,"d" ) ) {
			picoByte_t *diffuse;
			float value;


			/* get dissolve factor */
			if ( !_pico_parse_float( p,&value ) ) {
				_obj_mtl_error_return;
			}

			/* set shader transparency */
			PicoSetShaderTransparency( curShader,value );

			/* get shader's diffuse color */
			diffuse = PicoGetShaderDiffuseColor( curShader );

			/* set diffuse alpha to transparency */
			diffuse[ 3 ] = (picoByte_t)( value * 255.0 );

			/* set shader's new diffuse color */
			PicoSetShaderDiffuseColor( curShader,diffuse );
		}
		/* shininess (phong specular component) */
		else if ( !_pico_stricmp( p->token,"ns" ) ) {
			/* remark:
			 * - well, this is some major obj spec fuckup once again. some
			 *   apps store this in 0..1 range, others use 0..100 range,
			 *   even others use 0..2048 range, and again others use the
			 *   range 0..128, some even use 0..1000, 0..200, 400..700,
			 *   honestly, what's up with the 3d app coders? happens when
			 *   you smoke too much weed i guess. -sea
			 */
			float value;

			/* pointer to current shader must be valid */
			if ( curShader == NULL ) {
				_obj_mtl_error_return;
			}

			/* get totally screwed up shininess (a random value in fact ;) */
			if ( !_pico_parse_float( p,&value ) ) {
				_obj_mtl_error_return;
			}

			/* okay, there is no way to set this correctly, so we simply */
			/* try to guess a few ranges (most common ones i have seen) */

			/* assume 0..2048 range */
			if ( value > 1000 ) {
				value = 128.0 * ( value / 2048.0 );
			}
			/* assume 0..1000 range */
			else if ( value > 200 ) {
				value = 128.0 * ( value / 1000.0 );
			}
			/* assume 0..200 range */
			else if ( value > 100 ) {
				value = 128.0 * ( value / 200.0 );
			}
			/* assume 0..100 range */
			else if ( value > 1 ) {
				value = 128.0 * ( value / 100.0 );
			}
			/* assume 0..1 range */
			else {
				value *= 128.0;
			}
			/* negative shininess is bad (yes, i have seen it...) */
			if ( value < 0.0 ) {
				value = 0.0;
			}

			/* set the pico shininess value in range 0..127 */
			/* geez, .obj is such a mess... */
			PicoSetShaderShininess( curShader,value );
		}
		/* kol0r ambient (wut teh fuk does "ka" stand for?) */
		else if ( !_pico_stricmp( p->token,"ka" ) ) {
			picoColor_t color;
			picoVec3_t v;

			/* pointer to current shader must be valid */
			if ( curShader == NULL ) {
				_obj_mtl_error_return;
			}

			/* get color vector */
			if ( !_pico_parse_vec( p,v ) ) {
				_obj_mtl_error_return;
			}

			/* scale to byte range */
			color[ 0 ] = (picoByte_t)( v[ 0 ] * 255 );
			color[ 1 ] = (picoByte_t)( v[ 1 ] * 255 );
			color[ 2 ] = (picoByte_t)( v[ 2 ] * 255 );
			color[ 3 ] = (picoByte_t)( 255 );

			/* set ambient color */
			PicoSetShaderAmbientColor( curShader,color );
		}
		/* kol0r diffuse */
		else if ( !_pico_stricmp( p->token,"kd" ) ) {
			picoColor_t color;
			picoVec3_t v;

			/* pointer to current shader must be valid */
			if ( curShader == NULL ) {
				_obj_mtl_error_return;
			}

			/* get color vector */
			if ( !_pico_parse_vec( p,v ) ) {
				_obj_mtl_error_return;
			}

			/* scale to byte range */
			color[ 0 ] = (picoByte_t)( v[ 0 ] * 255 );
			color[ 1 ] = (picoByte_t)( v[ 1 ] * 255 );
			color[ 2 ] = (picoByte_t)( v[ 2 ] * 255 );
			color[ 3 ] = (picoByte_t)( 255 );

			/* set diffuse color */
			PicoSetShaderDiffuseColor( curShader,color );
		}
		/* kol0r specular */
		else if ( !_pico_stricmp( p->token,"ks" ) ) {
			picoColor_t color;
			picoVec3_t v;

			/* pointer to current shader must be valid */
			if ( curShader == NULL ) {
				_obj_mtl_error_return;
			}

			/* get color vector */
			if ( !_pico_parse_vec( p,v ) ) {
				_obj_mtl_error_return;
			}

			/* scale to byte range */
			color[ 0 ] = (picoByte_t)( v[ 0 ] * 255 );
			color[ 1 ] = (picoByte_t)( v[ 1 ] * 255 );
			color[ 2 ] = (picoByte_t)( v[ 2 ] * 255 );
			color[ 3 ] = (picoByte_t)( 255 );

			/* set specular color */
			PicoSetShaderSpecularColor( curShader,color );
		}
#endif
		/* skip rest of line */
		_pico_parse_skip_rest( p );
	}

	/* free parser, file buffer, and file name */
	_pico_free_parser( p );
	_pico_free_file( mtlBuffer );
	_pico_free( fileName );

	/* return with success */
	return 1;
}
#endif

/* _obj_load:
 *  loads a wavefront obj model file.
 */
static picoModel_t *_obj_load( PM_PARAMS_LOAD ){
	TObjVertexData *vertexData  = NULL;
	picoModel_t    *model;
	picoSurface_t  *curSurface  = NULL;
	picoParser_t   *p;
	int allocated;
	int entries;
	int numVerts    = 0;
	int numNormals  = 0;
	int numUVs      = 0;
	int curVertex   = 0;
	int curFace     = 0;

	/* helper */
	#define _obj_error_return( m ) \
	{ \
		_pico_printf( PICO_ERROR,"%s in OBJ, line %d.",m,p->curLine ); \
		_pico_free_parser( p );	\
		FreeObjVertexData( vertexData ); \
		PicoFreeModel( model );	\
		return NULL; \
	}
	/* alllocate a new pico parser */
	p = _pico_new_parser( (picoByte_t *)buffer,bufSize );
	if ( p == NULL ) {
		return NULL;
	}

	/* create a new pico model */
	model = PicoNewModel();
	if ( model == NULL ) {
		_pico_free_parser( p );
		return NULL;
	}
	/* do model setup */
	PicoSetModelFrameNum( model,frameNum );
	PicoSetModelName( model,fileName );
	PicoSetModelFileName( model,fileName );

	/* try loading the materials; we don't handle the result */
#if 0
	_obj_mtl_load( model );
#endif

	/* parse obj line by line */
	while ( 1 )
	{
		/* get first token on line */
		if ( _pico_parse_first( p ) == NULL ) {
			break;
		}

		/* skip empty lines */
		if ( p->token == NULL || !strlen( p->token ) ) {
			continue;
		}

		/* skip comment lines */
		if ( p->token[0] == '#' ) {
			_pico_parse_skip_rest( p );
			continue;
		}
		/* vertex */
		if ( !_pico_stricmp( p->token,"v" ) ) {
			TObjVertexData *data;
			picoVec3_t v;

			vertexData = SizeObjVertexData( vertexData,numVerts + 1,&entries,&allocated );
			if ( vertexData == NULL ) {
				_obj_error_return( "Realloc of vertex data failed (1)" );
			}

			data = &vertexData[ numVerts++ ];

			/* get and copy vertex */
			if ( !_pico_parse_vec( p,v ) ) {
				_obj_error_return( "Vertex parse error" );
			}

			_pico_copy_vec( v,data->v );

#ifdef DEBUG_PM_OBJ_EX
			printf( "Vertex: x: %f y: %f z: %f\n",v[0],v[1],v[2] );
#endif
		}
		/* uv coord */
		else if ( !_pico_stricmp( p->token,"vt" ) ) {
			TObjVertexData *data;
			picoVec2_t coord;

			vertexData = SizeObjVertexData( vertexData,numUVs + 1,&entries,&allocated );
			if ( vertexData == NULL ) {
				_obj_error_return( "Realloc of vertex data failed (2)" );
			}

			data = &vertexData[ numUVs++ ];

			/* get and copy tex coord */
			if ( !_pico_parse_vec2( p,coord ) ) {
				_obj_error_return( "UV coord parse error" );
			}

			_pico_copy_vec2( coord,data->vt );

#ifdef DEBUG_PM_OBJ_EX
			printf( "TexCoord: u: %f v: %f\n",coord[0],coord[1] );
#endif
		}
		/* vertex normal */
		else if ( !_pico_stricmp( p->token,"vn" ) ) {
			TObjVertexData *data;
			picoVec3_t n;

			vertexData = SizeObjVertexData( vertexData,numNormals + 1,&entries,&allocated );
			if ( vertexData == NULL ) {
				_obj_error_return( "Realloc of vertex data failed (3)" );
			}

			data = &vertexData[ numNormals++ ];

			/* get and copy vertex normal */
			if ( !_pico_parse_vec( p,n ) ) {
				_obj_error_return( "Vertex normal parse error" );
			}

			_pico_copy_vec( n,data->vn );

#ifdef DEBUG_PM_OBJ_EX
			printf( "Normal: x: %f y: %f z: %f\n",n[0],n[1],n[2] );
#endif
		}
		/* new group (for us this means a new surface) */
		else if ( !_pico_stricmp( p->token,"g" ) ) {
			picoSurface_t *newSurface;
			char *groupName;

			/* get first group name (ignore 2nd,3rd,etc.) */
			groupName = _pico_parse( p,0 );
			if ( groupName == NULL || !strlen( groupName ) ) {
				/* some obj exporters feel like they don't need to */
				/* supply a group name. so we gotta handle it here */
#if 1
				strcpy( p->token,"default" );
				groupName = p->token;
#else
				_obj_error_return( "Invalid or missing group name" );
#endif
			}
			/* allocate a pico surface */
			newSurface = PicoNewSurface( model );
			if ( newSurface == NULL ) {
				_obj_error_return( "Error allocating surface" );
			}

			/* reset face index for surface */
			curFace = 0;

			/* set ptr to current surface */
			curSurface = newSurface;

			/* we use triangle meshes */
			PicoSetSurfaceType( newSurface,PICO_TRIANGLES );

			/* set surface name */
			PicoSetSurfaceName( newSurface,groupName );

#ifdef DEBUG_PM_OBJ_EX
			printf( "Group: '%s'\n",groupName );
#endif
		}
		/* face (oh jesus, hopefully this will do the job right ;) */
		else if ( !_pico_stricmp( p->token,"f" ) ) {
			/* okay, this is a mess. some 3d apps seem to try being unique, */
			/* hello cinema4d & 3d exploration, feel good today?, and save */
			/* this crap in tons of different formats. gah, those screwed */
			/* coders. tho the wavefront obj standard defines exactly two */
			/* ways of storing face information. so, i really won't support */
			/* such stupid extravaganza here! */

			picoVec3_t verts  [ 4 ];
			picoVec3_t normals[ 4 ];
			picoVec2_t coords [ 4 ];

			int iv [ 4 ], has_v;
			int ivt[ 4 ], has_vt = 0;
			int ivn[ 4 ], has_vn = 0;
			int have_quad = 0;
			int slashcount;
			int doubleslash;
			int i;

			/* group defs *must* come before faces */
			if ( curSurface == NULL ) {
				_obj_error_return( "No group defined for faces" );
			}

#ifdef DEBUG_PM_OBJ_EX
			printf( "Face: " );
#endif
			/* read vertex/uv/normal indices for the first three face */
			/* vertices (cause we only support triangles) into 'i*[]' */
			/* store the actual vertex/uv/normal data in three arrays */
			/* called 'verts','coords' and 'normals'. */
			for ( i = 0; i < 4; i++ )
			{
				char *str;

				/* get next vertex index string (different */
				/* formats are handled below) */
				str = _pico_parse( p,0 );
				if ( str == NULL ) {
					/* just break for quads */
					if ( i == 3 ) {
						break;
					}

					/* error otherwise */
					_obj_error_return( "Face parse error" );
				}
				/* if this is the fourth index string we're */
				/* parsing we assume that we have a quad */
				if ( i == 3 ) {
					have_quad = 1;
				}

				/* get slash count once */
				if ( i == 0 ) {
					slashcount  = _pico_strchcount( str,'/' );
					doubleslash =  strstr( str,"//" ) != NULL;
				}
				/* handle format 'v//vn' */
				if ( doubleslash && ( slashcount == 2 ) ) {
					has_v = has_vn = 1;
					sscanf( str,"%d//%d",&iv[ i ],&ivn[ i ] );
				}
				/* handle format 'v/vt/vn' */
				else if ( !doubleslash && ( slashcount == 2 ) ) {
					has_v = has_vt = has_vn = 1;
					sscanf( str,"%d/%d/%d",&iv[ i ],&ivt[ i ],&ivn[ i ] );
				}
				/* handle format 'v/vt' (non-standard fuckage) */
				else if ( !doubleslash && ( slashcount == 1 ) ) {
					has_v = has_vt = 1;
					sscanf( str,"%d/%d",&iv[ i ],&ivt[ i ] );
				}
				/* else assume face format 'v' */
				/* (must have been invented by some bored granny) */
				else {
					/* get single vertex index */
					has_v = 1;
					iv[ i ] = atoi( str );

					/* either invalid face format or out of range */
					if ( iv[ i ] == 0 ) {
						_obj_error_return( "Invalid face format" );
					}
				}
				/* fix useless back references */
				/* todo: check if this works as it is supposed to */

				/* assign new indices */
				if ( iv [ i ] < 0 ) {
					iv [ i ] = ( numVerts   - iv [ i ] );
				}
				if ( ivt[ i ] < 0 ) {
					ivt[ i ] = ( numUVs     - ivt[ i ] );
				}
				if ( ivn[ i ] < 0 ) {
					ivn[ i ] = ( numNormals - ivn[ i ] );
				}

				/* validate indices */
				/* - commented out. index range checks will trigger
				   if (iv [ i ] < 1) iv [ i ] = 1;
				   if (ivt[ i ] < 1) ivt[ i ] = 1;
				   if (ivn[ i ] < 1) ivn[ i ] = 1;
				 */
				/* set vertex origin */
				if ( has_v ) {
					/* check vertex index range */
					if ( iv[ i ] < 1 || iv[ i ] > numVerts ) {
						_obj_error_return( "Vertex index out of range" );
					}

					/* get vertex data */
					verts[ i ][ 0 ] = vertexData[ iv[ i ] - 1 ].v[ 0 ];
					verts[ i ][ 1 ] = vertexData[ iv[ i ] - 1 ].v[ 1 ];
					verts[ i ][ 2 ] = vertexData[ iv[ i ] - 1 ].v[ 2 ];
				}
				/* set vertex normal */
				if ( has_vn ) {
					/* check normal index range */
					if ( ivn[ i ] < 1 || ivn[ i ] > numNormals ) {
						_obj_error_return( "Normal index out of range" );
					}

					/* get normal data */
					normals[ i ][ 0 ] = vertexData[ ivn[ i ] - 1 ].vn[ 0 ];
					normals[ i ][ 1 ] = vertexData[ ivn[ i ] - 1 ].vn[ 1 ];
					normals[ i ][ 2 ] = vertexData[ ivn[ i ] - 1 ].vn[ 2 ];
				}
				/* set texture coordinate */
				if ( has_vt ) {
					/* check uv index range */
					if ( ivt[ i ] < 1 || ivt[ i ] > numUVs ) {
						_obj_error_return( "UV coord index out of range" );
					}

					/* get uv coord data */
					coords[ i ][ 0 ] = vertexData[ ivt[ i ] - 1 ].vt[ 0 ];
					coords[ i ][ 1 ] = vertexData[ ivt[ i ] - 1 ].vt[ 1 ];
					coords[ i ][ 1 ] = -coords[ i ][ 1 ];
				}
#ifdef DEBUG_PM_OBJ_EX
				printf( "(%4d",iv[ i ] );
				if ( has_vt ) {
					printf( " %4d",ivt[ i ] );
				}
				if ( has_vn ) {
					printf( " %4d",ivn[ i ] );
				}
				printf( ") " );
#endif
			}
#ifdef DEBUG_PM_OBJ_EX
			printf( "\n" );
#endif
			/* now that we have extracted all the indices and have */
			/* read the actual data we need to assign all the crap */
			/* to our current pico surface */
			if ( has_v ) {
				int max = 3;
				if ( have_quad ) {
					max = 4;
				}

				/* assign all surface information */
				for ( i = 0; i < max; i++ )
				{
					/*if( has_v  )*/ PicoSetSurfaceXYZ( curSurface,  ( curVertex + i ), verts  [ i ] );
					/*if( has_vt )*/ PicoSetSurfaceST( curSurface,0,( curVertex + i ), coords [ i ] );
					/*if( has_vn )*/ PicoSetSurfaceNormal( curSurface,  ( curVertex + i ), normals[ i ] );
				}
				/* add our triangle (A B C) */
				PicoSetSurfaceIndex( curSurface,( curFace * 3 + 2 ),(picoIndex_t)( curVertex + 0 ) );
				PicoSetSurfaceIndex( curSurface,( curFace * 3 + 1 ),(picoIndex_t)( curVertex + 1 ) );
				PicoSetSurfaceIndex( curSurface,( curFace * 3 + 0 ),(picoIndex_t)( curVertex + 2 ) );
				curFace++;

				/* if we don't have a simple triangle, but a quad... */
				if ( have_quad ) {
					/* we have to add another triangle (2nd half of quad which is A C D) */
					PicoSetSurfaceIndex( curSurface,( curFace * 3 + 2 ),(picoIndex_t)( curVertex + 0 ) );
					PicoSetSurfaceIndex( curSurface,( curFace * 3 + 1 ),(picoIndex_t)( curVertex + 2 ) );
					PicoSetSurfaceIndex( curSurface,( curFace * 3 + 0 ),(picoIndex_t)( curVertex + 3 ) );
					curFace++;
				}
				/* increase vertex count */
				curVertex += max;
			}

		}
		else if ( !_pico_stricmp( p->token, "usemtl" ) ) {
			char *materialName;
			materialName = _pico_parse( p, 0 );
			if( materialName && strlen( materialName ) ) {
				picoShader_t *shader;
				shader = PicoFindShader( model, materialName, 0 );
				if( !shader ) {
					shader = PicoNewShader( model );
					if( shader ) {
						PicoSetShaderMapName( shader, materialName );
						PicoSetShaderName( shader, materialName );
					}
				}
				if( shader && curSurface ) {
					PicoSetSurfaceShader( curSurface, shader );
				}
			}
		}
		/* skip unparsed rest of line and continue */
		_pico_parse_skip_rest( p );
	}
	/* free memory used by temporary vertexdata */
	FreeObjVertexData( vertexData );

	/* return allocated pico model */
	return model;
//	return NULL;
}

/* pico file format module definition */
const picoModule_t picoModuleOBJ =
{
	"0.6-b",                    /* module version string */
	"Wavefront ASCII",          /* module display name */
	"seaw0lf",                  /* author's name */
	"2002 seaw0lf",             /* module copyright */
	{
		"obj",NULL,NULL,NULL    /* default extensions to use */
	},
	_obj_canload,               /* validation routine */
	_obj_load,                  /* load routine */
	NULL,                       /* save validation routine */
	NULL                        /* save routine */
};
