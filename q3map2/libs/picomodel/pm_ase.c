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
   other aseMaterialList provided with the distribution.

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
#define PM_ASE_C

/* uncomment when debugging this module */
//#define DEBUG_PM_ASE
//#define DEBUG_PM_ASE_EX


/* dependencies */
#include "picointernal.h"

#ifdef DEBUG_PM_ASE
#include "time.h"
#endif

/* plain white */
static picoColor_t white = { 255, 255, 255, 255 };

/* jhefty - multi-subobject material support */

/* Material/SubMaterial management */
/* A material should have 1..n submaterials assigned to it */

typedef struct aseSubMaterial_s
{
	struct aseSubMaterial_s* next;
	int subMtlId;
	picoShader_t* shader;

} aseSubMaterial_t;

typedef struct aseMaterial_s
{
	struct aseMaterial_s* next;
	struct aseSubMaterial_s* subMtls;
	int mtlId;
} aseMaterial_t;

/* Material/SubMaterial management functions */
static aseMaterial_t* _ase_get_material( aseMaterial_t* list, int mtlIdParent ){
	aseMaterial_t* mtl = list;

	while ( mtl )
	{
		if ( mtlIdParent == mtl->mtlId ) {
			break;
		}
		mtl = mtl->next;
	}
	return mtl;
}

static aseSubMaterial_t* _ase_get_submaterial( aseMaterial_t* list, int mtlIdParent, int subMtlId ){
	aseMaterial_t* parent = _ase_get_material( list, mtlIdParent );
	aseSubMaterial_t* subMtl = NULL;

	if ( !parent ) {
		_pico_printf( PICO_ERROR, "No ASE material exists with id %i\n", mtlIdParent );
		return NULL;
	}

	subMtl = parent->subMtls;
	while ( subMtl )
	{
		if ( subMtlId == subMtl->subMtlId ) {
			break;
		}
		subMtl = subMtl->next;
	}
	return subMtl;
}

static aseMaterial_t* _ase_add_material( aseMaterial_t **list, int mtlIdParent ){
	aseMaterial_t *mtl = _pico_calloc( 1, sizeof( aseMaterial_t ) );
	mtl->mtlId = mtlIdParent;
	mtl->subMtls = NULL;
	mtl->next = *list;
	*list = mtl;

	return mtl;
}

static aseSubMaterial_t* _ase_add_submaterial( aseMaterial_t **list, int mtlIdParent, int subMtlId, picoShader_t* shader ){
	aseMaterial_t *parent = _ase_get_material( *list,  mtlIdParent );
	aseSubMaterial_t *subMtl = _pico_calloc( 1, sizeof( aseSubMaterial_t ) );

	if ( !parent ) {
		parent = _ase_add_material( list, mtlIdParent );
	}

	subMtl->shader = shader;
	subMtl->subMtlId = subMtlId;
	subMtl->next = parent->subMtls;
	parent->subMtls = subMtl;

	return subMtl;
}

static void _ase_free_materials( aseMaterial_t **list ){
	aseMaterial_t* mtl = *list;
	aseSubMaterial_t* subMtl = NULL;

	aseMaterial_t* mtlTemp = NULL;
	aseSubMaterial_t* subMtlTemp = NULL;

	while ( mtl )
	{
		subMtl = mtl->subMtls;
		while ( subMtl )
		{
			subMtlTemp = subMtl->next;
			_pico_free( subMtl );
			subMtl = subMtlTemp;
		}
		mtlTemp = mtl->next;
		_pico_free( mtl );
		mtl = mtlTemp;
	}
	( *list ) = NULL;
}

#ifdef DEBUG_PM_ASE
static void _ase_print_materials( aseMaterial_t *list ){
	aseMaterial_t* mtl = list;
	aseSubMaterial_t* subMtl = NULL;

	while ( mtl )
	{
		_pico_printf( PICO_NORMAL,  "ASE Material %i", mtl->mtlId );
		subMtl = mtl->subMtls;
		while ( subMtl )
		{
			_pico_printf( PICO_NORMAL,  " -- ASE SubMaterial %i - %s\n", subMtl->subMtlId, subMtl->shader->name );
			subMtl = subMtl->next;
		}
		mtl = mtl->next;
	}
}
#endif //DEBUG_PM_ASE

/* ASE Face management */
/* These are used to keep an association between a submaterial and a face definition */
/* They are kept in parallel with the current picoSurface, */
/* and are used by _ase_submit_triangles to lookup the proper material/submaterial IDs */
typedef struct aseFace_s
{
	struct aseFace_s* next;
	int mtlId;
	int subMtlId;
	int index[9];
} aseFace_t;

/* ASE Face management functions */
void _ase_add_face( aseFace_t **list, aseFace_t **tail, aseFace_t *newFace ){
	/* insert as head of list */
	if ( !( *list ) ) {
		*list = newFace;
	}
	else
	{
		( *tail )->next = newFace;
	}

	*tail = newFace;
	newFace->next = NULL;

	//tag the color indices so we can detect them and apply the default color to them
	newFace->index[6] = -1;
	newFace->index[7] = -1;
	newFace->index[8] = -1;
}

aseFace_t* _ase_get_face_for_index( aseFace_t *list, int index ){
	int counter = 0;
	aseFace_t* face = list;

	while ( counter < index )
	{
		face = face->next;
		counter++;
	}
	return face;
}
static void _ase_free_faces( aseFace_t** list, aseFace_t** tail ){
	aseFace_t* face = *list;
	aseFace_t* tempFace = NULL;

	while ( face )
	{
		tempFace = face->next;
		_pico_free( face );
		face = tempFace;
	}

	( *list ) = NULL;
	( *tail ) = NULL;
}

/* todo:
 * - apply material specific uv offsets to uv coordinates
 */

/* _ase_canload:
 *  validates a 3dsmax ase model file.
 */
static int _ase_canload( PM_PARAMS_CANLOAD ){
	picoParser_t *p;


	/* quick data length validation */
	if ( bufSize < 80 ) {
		return PICO_PMV_ERROR_SIZE;
	}

	/* keep the friggin compiler happy */
	*fileName = *fileName;

	/* create pico parser */
	p = _pico_new_parser( (picoByte_t*) buffer, bufSize );
	if ( p == NULL ) {
		return PICO_PMV_ERROR_MEMORY;
	}

	/* get first token */
	if ( _pico_parse_first( p ) == NULL ) {
		return PICO_PMV_ERROR_IDENT;
	}

	/* check first token */
	if ( _pico_stricmp( p->token, "*3dsmax_asciiexport" ) ) {
		_pico_free_parser( p );
		return PICO_PMV_ERROR_IDENT;
	}

	/* free the pico parser object */
	_pico_free_parser( p );

	/* file seems to be a valid ase file */
	return PICO_PMV_OK;
}



/* _ase_submit_triangles - jhefty
   use the surface and the current face list to look up material/submaterial IDs
   and submit them to the model for proper processing

   The following still holds from ydnar's _ase_make_surface:
   indexes 0 1 2 = vert indexes
   indexes 3 4 5 = st indexes
   indexes 6 7 8 = color indexes (new)
 */

static void _ase_submit_triangles( picoSurface_t* surface, picoModel_t* model, aseMaterial_t* materials, aseFace_t* faces ){
	aseFace_t* face;
	aseSubMaterial_t* subMtl;
	picoVec3_t* xyz[3];
	picoVec3_t* normal[3];
	picoVec2_t* st[3];
	picoColor_t* color[3];
	int i;

	face = faces;
	while ( face != NULL )
	{
		/* look up the shader for the material/submaterial pair */
		subMtl = _ase_get_submaterial( materials, face->mtlId, face->subMtlId );
		if ( subMtl == NULL ) {
			/* ydnar: trying default submaterial */
			subMtl = _ase_get_submaterial( materials, face->mtlId, 0 );
			if ( subMtl == NULL ) {
				_pico_printf( PICO_ERROR, "Could not find material/submaterial for id %d/%d\n", face->mtlId, face->subMtlId );
				return;
			}
		}

		/* we pull the data from the surface using the facelist data */
		for ( i = 0 ; i < 3 ; i++ )
		{
			xyz[i]    = (picoVec3_t*) PicoGetSurfaceXYZ( surface, face->index[ i ] );
			normal[i] = (picoVec3_t*) PicoGetSurfaceNormal( surface, face->index[ i ] );
			st[i]     = (picoVec2_t*) PicoGetSurfaceST( surface, 0, face->index[ i + 3 ] );

			if ( face->index [ i + 6] >= 0 ) {
				color[i]  = (picoColor_t*)PicoGetSurfaceColor( surface, 0, face->index[ i + 6 ] );
			}
			else
			{
				color[i] = &white;
			}

		}

		/* submit the triangle to the model */
		PicoAddTriangleToModel( model, xyz, normal, 1, st, 1, color, subMtl->shader );

		/* advance to the next face */
		face = face->next;
	}
}

/* _ase_load:
 *  loads a 3dsmax ase model file.
 */
static picoModel_t *_ase_load( PM_PARAMS_LOAD ){
	picoModel_t    *model;
	picoSurface_t  *surface = NULL;
	picoParser_t   *p;
	char lastNodeName[ 1024 ];

	aseFace_t* faces = NULL;
	aseFace_t* facesTail = NULL;
	aseMaterial_t* materials = NULL;

#ifdef DEBUG_PM_ASE
	clock_t start, finish;
	double elapsed;
	start = clock();
#endif

	/* helper */
	#define _ase_error_return( m ) \
	{ \
		_pico_printf( PICO_ERROR,"%s in ASE, line %d.",m,p->curLine ); \
		_pico_free_parser( p );	\
		PicoFreeModel( model );	\
		return NULL; \
	}
	/* create a new pico parser */
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
	PicoSetModelFrameNum( model, frameNum );
	PicoSetModelName( model, fileName );
	PicoSetModelFileName( model, fileName );

	/* initialize some stuff */
	memset( lastNodeName,0,sizeof( lastNodeName ) );

	/* parse ase model file */
	while ( 1 )
	{
		/* get first token on line */
		if ( _pico_parse_first( p ) == NULL ) {
			break;
		}

		/* we just skip empty lines */
		if ( p->token == NULL || !strlen( p->token ) ) {
			continue;
		}

		/* we skip invalid ase statements */
		if ( p->token[0] != '*' && p->token[0] != '{' && p->token[0] != '}' ) {
			_pico_parse_skip_rest( p );
			continue;
		}
		/* remember node name */
		if ( !_pico_stricmp( p->token,"*node_name" ) ) {
			/* read node name */
			char *ptr = _pico_parse( p,0 );
			if ( ptr == NULL ) {
				_ase_error_return( "Node name parse error" );
			}

			/* remember node name */
			strncpy( lastNodeName,ptr,sizeof( lastNodeName ) );
		}
		/* model mesh (originally contained within geomobject) */
		else if ( !_pico_stricmp( p->token,"*mesh" ) ) {
			/* finish existing surface */
			//_ase_make_surface( model, &surface );
			_ase_submit_triangles( surface, model,materials,faces );
			_ase_free_faces( &faces,&facesTail );

			/* allocate new pico surface */
			surface = PicoNewSurface( NULL );
			if ( surface == NULL ) {
				PicoFreeModel( model );
				return NULL;
			}
		}
		/* mesh material reference. this usually comes at the end of */
		/* geomobjects after the mesh blocks. we must assume that the */
		/* new mesh was already created so all we can do here is assign */
		/* the material reference id (shader index) now. */
		else if ( !_pico_stricmp( p->token,"*material_ref" ) ) {
			int mtlId;
			aseFace_t* face;

			/* we must have a valid surface */
			if ( surface == NULL ) {
				_ase_error_return( "Missing mesh for material reference" );
			}

			/* get the material ref (0..n) */
			if ( !_pico_parse_int( p,&mtlId ) ) {
				_ase_error_return( "Missing material reference ID" );
			}

			/* fix up all of the aseFaceList in the surface to point to the parent material */
			/* we've already saved off their subMtl */
			face = faces;
			while ( face != NULL )
			{
				face->mtlId = mtlId;
				face = face->next;
			}
		}
		/* model mesh vertex */
		else if ( !_pico_stricmp( p->token,"*mesh_vertex" ) ) {
			picoVec3_t v;
			int index;

			/* we must have a valid surface */
			if ( surface == NULL ) {
				continue;
			}

			/* get vertex data (orig: index +y -x +z) */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Vertex parse error" );
			}
			if ( !_pico_parse_vec( p,v ) ) {
				_ase_error_return( "Vertex parse error" );
			}

			/* set vertex */
			PicoSetSurfaceXYZ( surface,index,v );
		}
		/* model mesh vertex normal */
		else if ( !_pico_stricmp( p->token,"*mesh_vertexnormal" ) ) {
			picoVec3_t v;
			int index;

			/* we must have a valid surface */
			if ( surface == NULL ) {
				continue;
			}

			/* get vertex data (orig: index +y -x +z) */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Vertex parse error" );
			}
			if ( !_pico_parse_vec( p,v ) ) {
				_ase_error_return( "Vertex parse error" );
			}

			/* set vertex */
			PicoSetSurfaceNormal( surface,index,v );
		}
		/* model mesh face */
		else if ( !_pico_stricmp( p->token,"*mesh_face" ) ) {
			picoIndex_t indexes[3];
			int index;

			/* we must have a valid surface */
			if ( surface == NULL ) {
				continue;
			}

			/* get face index */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 1st vertex index */
			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indexes[0] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 2nd vertex index */
			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indexes[1] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 3rd vertex index */
			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indexes[2] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* set face indexes (note interleaved offset!) */
			PicoSetSurfaceIndex( surface, ( index * 9 + 0 ), indexes[2] );
			PicoSetSurfaceIndex( surface, ( index * 9 + 1 ), indexes[1] );
			PicoSetSurfaceIndex( surface, ( index * 9 + 2 ), indexes[0] );

			/* parse to the subMaterial ID */
			while ( 1 )
			{
				_pico_parse( p,0 );
				if ( !_pico_stricmp( p->token,"*MESH_MTLID" ) ) {
					aseFace_t* newFace;
					int subMtlId;

					_pico_parse_int( p, &subMtlId );
					newFace = _pico_calloc( 1, sizeof( aseFace_t ) );

					/* we fix up the mtlId later when we parse the material_ref */
					newFace->mtlId = 0;
					newFace->subMtlId = subMtlId;
					newFace->index[0] = indexes[2];
					newFace->index[1] = indexes[1];
					newFace->index[2] = indexes[0];

					_ase_add_face( &faces,&facesTail,newFace );
					break;
				}
			}

		}
		/* model texture vertex */
		else if ( !_pico_stricmp( p->token,"*mesh_tvert" ) ) {
			picoVec2_t uv;
			int index;

			/* we must have a valid surface */
			if ( surface == NULL ) {
				continue;
			}

			/* get uv vertex index */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "UV vertex parse error" );
			}

			/* get uv vertex s */
			if ( !_pico_parse_float( p,&uv[0] ) ) {
				_ase_error_return( "UV vertex parse error" );
			}

			/* get uv vertex t */
			if ( !_pico_parse_float( p,&uv[1] ) ) {
				_ase_error_return( "UV vertex parse error" );
			}

			/* ydnar: invert t */
			uv[ 1 ] = 1.0f - uv[ 1 ];

			/* set texture vertex */
			PicoSetSurfaceST( surface,0,index,uv );
		}
		/* ydnar: model mesh texture face */
		else if ( !_pico_stricmp( p->token, "*mesh_tface" ) ) {
			picoIndex_t indexes[3];
			int index;
			aseFace_t* face;

			/* we must have a valid surface */
			if ( surface == NULL ) {
				continue;
			}

			/* get face index */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Texture face parse error" );
			}

			/* get 1st vertex index */
			if ( !_pico_parse_int( p,&indexes[0] ) ) {
				_ase_error_return( "Texture face parse error" );
			}

			/* get 2nd vertex index */
			if ( !_pico_parse_int( p,&indexes[1] ) ) {
				_ase_error_return( "Texture face parse error" );
			}

			/* get 3rd vertex index */
			if ( !_pico_parse_int( p,&indexes[2] ) ) {
				_ase_error_return( "Texture face parse error" );
			}

			/* set face indexes (note interleaved offset!) */
			PicoSetSurfaceIndex( surface, ( index * 9 + 3 ), indexes[2] );
			PicoSetSurfaceIndex( surface, ( index * 9 + 4 ), indexes[1] );
			PicoSetSurfaceIndex( surface, ( index * 9 + 5 ), indexes[0] );

			face = _ase_get_face_for_index( faces,index );
			face->index[3] = indexes[2];
			face->index[4] = indexes[1];
			face->index[5] = indexes[0];
		}
		/* model color vertex */
		else if ( !_pico_stricmp( p->token,"*mesh_vertcol" ) ) {
			picoColor_t color;
			int index;
			float colorInput;

			/* we must have a valid surface */
			if ( surface == NULL ) {
				continue;
			}

			/* get color vertex index */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "UV vertex parse error" );
			}

			/* get R component */
			if ( !_pico_parse_float( p,&colorInput ) ) {
				_ase_error_return( "color vertex parse error" );
			}
			color[0] = (picoByte_t)( colorInput * 255 );

			/* get G component */
			if ( !_pico_parse_float( p,&colorInput ) ) {
				_ase_error_return( "color vertex parse error" );
			}
			color[1] = (picoByte_t)( colorInput * 255 );

			/* get B component */
			if ( !_pico_parse_float( p,&colorInput ) ) {
				_ase_error_return( "color vertex parse error" );
			}
			color[2] = (picoByte_t)( colorInput * 255 );

			/* leave alpha alone since we don't get any data from the ASE format */
			color[3] = 255;

			/* set texture vertex */
			PicoSetSurfaceColor( surface,0,index,color );
		}
		/* model color face */
		else if ( !_pico_stricmp( p->token,"*mesh_cface" ) ) {
			picoIndex_t indexes[3];
			int index;
			aseFace_t*  face;

			/* we must have a valid surface */
			if ( surface == NULL ) {
				continue;
			}

			/* get face index */
			if ( !_pico_parse_int( p,&index ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 1st cvertex index */
			//			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indexes[0] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 2nd cvertex index */
			//			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indexes[1] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* get 3rd cvertex index */
			//			_pico_parse( p,0 );
			if ( !_pico_parse_int( p,&indexes[2] ) ) {
				_ase_error_return( "Face parse error" );
			}

			/* set face indexes (note interleaved offset!) */
			PicoSetSurfaceIndex( surface, ( index * 9 + 6 ), indexes[2] );
			PicoSetSurfaceIndex( surface, ( index * 9 + 7 ), indexes[1] );
			PicoSetSurfaceIndex( surface, ( index * 9 + 8 ), indexes[0] );

			face = _ase_get_face_for_index( faces,index );
			face->index[6] = indexes[2];
			face->index[7] = indexes[1];
			face->index[8] = indexes[0];
		}
		/* model material */
		else if ( !_pico_stricmp( p->token, "*material" ) ) {
			aseSubMaterial_t*   subMaterial = NULL;
			picoShader_t        *shader = NULL;
			int level = 1, index;
			char materialName[ 1024 ];
			float transValue = 0.0f, shineValue = 1.0f;
			picoColor_t ambientColor, diffuseColor, specularColor;
			char                *mapname = NULL;
			int subMtlId, subMaterialLevel = -1;


			/* get material index */
			_pico_parse_int( p,&index );

			/* check brace */
			if ( !_pico_parse_check( p,1,"{" ) ) {
				_ase_error_return( "Material missing opening brace" );
			}

			/* parse material block */
			while ( 1 )
			{
				/* get next token */
				if ( _pico_parse( p,1 ) == NULL ) {
					break;
				}
				if ( !strlen( p->token ) ) {
					continue;
				}

				/* handle levels */
				if ( p->token[0] == '{' ) {
					level++;
				}
				if ( p->token[0] == '}' ) {
					level--;
				}
				if ( !level ) {
					break;
				}

				if ( level == subMaterialLevel ) {
					/* set material name */
					PicoSetShaderName( shader, materialName );

					/* set shader's transparency */
					PicoSetShaderTransparency( shader,transValue );

					/* set shader's ambient color */
					PicoSetShaderAmbientColor( shader,ambientColor );

					/* set diffuse alpha to transparency */
					diffuseColor[3] = (picoByte_t)( transValue * 255.0 );

					/* set shader's diffuse color */
					PicoSetShaderDiffuseColor( shader,diffuseColor );

					/* set shader's specular color */
					PicoSetShaderSpecularColor( shader,specularColor );

					/* set shader's shininess */
					PicoSetShaderShininess( shader,shineValue );

					/* set material map name */
					PicoSetShaderMapName( shader, mapname );

					subMaterial = _ase_add_submaterial( &materials, index, subMtlId, shader );
					subMaterialLevel = -1;
				}

				/* parse submaterial index */
				if ( !_pico_stricmp( p->token,"*submaterial" ) ) {
					/* allocate new pico shader */
					_pico_parse_int( p, &subMtlId );

					shader = PicoNewShader( model );
					if ( shader == NULL ) {
						PicoFreeModel( model );
						return NULL;
					}
					subMaterialLevel = level;
				}
				/* parse material name */
				else if ( !_pico_stricmp( p->token,"*material_name" ) ) {
					char* name = _pico_parse( p,0 );
					if ( name == NULL ) {
						_ase_error_return( "Missing material name" );
					}

					strcpy( materialName, name );
					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse material transparency */
				else if ( !_pico_stricmp( p->token,"*material_transparency" ) ) {
					/* get transparency value from ase */
					if ( !_pico_parse_float( p,&transValue ) ) {
						_ase_error_return( "Material transparency parse error" );
					}

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse material shininess */
				else if ( !_pico_stricmp( p->token,"*material_shine" ) ) {
					/* remark:
					 * - not sure but instead of '*material_shine' i might
					 *   need to use '*material_shinestrength' */

					/* get shine value from ase */
					if ( !_pico_parse_float( p,&shineValue ) ) {
						_ase_error_return( "Material shine parse error" );
					}

					/* scale ase shine range 0..1 to pico range 0..127 */
					shineValue *= 128.0;

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse ambient material color */
				else if ( !_pico_stricmp( p->token,"*material_ambient" ) ) {
					picoVec3_t vec;
					/* get r,g,b float values from ase */
					if ( !_pico_parse_vec( p,vec ) ) {
						_ase_error_return( "Material color parse error" );
					}

					/* setup 0..255 range color values */
					ambientColor[ 0 ] = (int)( vec[ 0 ] * 255.0 );
					ambientColor[ 1 ] = (int)( vec[ 1 ] * 255.0 );
					ambientColor[ 2 ] = (int)( vec[ 2 ] * 255.0 );
					ambientColor[ 3 ] = (int)( 255 );

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse diffuse material color */
				else if ( !_pico_stricmp( p->token,"*material_diffuse" ) ) {
					picoVec3_t vec;

					/* get r,g,b float values from ase */
					if ( !_pico_parse_vec( p,vec ) ) {
						_ase_error_return( "Material color parse error" );
					}

					/* setup 0..255 range color */
					diffuseColor[ 0 ] = (int)( vec[ 0 ] * 255.0 );
					diffuseColor[ 1 ] = (int)( vec[ 1 ] * 255.0 );
					diffuseColor[ 2 ] = (int)( vec[ 2 ] * 255.0 );
					diffuseColor[ 3 ] = (int)( 255 );

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* parse specular material color */
				else if ( !_pico_stricmp( p->token,"*material_specular" ) ) {
					picoVec3_t vec;

					/* get r,g,b float values from ase */
					if ( !_pico_parse_vec( p,vec ) ) {
						_ase_error_return( "Material color parse error" );
					}

					/* setup 0..255 range color */
					specularColor[ 0 ] = (int)( vec[ 0 ] * 255 );
					specularColor[ 1 ] = (int)( vec[ 1 ] * 255 );
					specularColor[ 2 ] = (int)( vec[ 2 ] * 255 );
					specularColor[ 3 ] = (int)( 255 );

					/* skip rest and continue with next token */
					_pico_parse_skip_rest( p );
					continue;
				}
				/* material diffuse map */
				else if ( !_pico_stricmp( p->token,"*map_diffuse" ) ) {
					int sublevel = 0;

					/* parse material block */
					while ( 1 )
					{
						/* get next token */
						if ( _pico_parse( p,1 ) == NULL ) {
							break;
						}
						if ( !strlen( p->token ) ) {
							continue;
						}

						/* handle levels */
						if ( p->token[0] == '{' ) {
							sublevel++;
						}
						if ( p->token[0] == '}' ) {
							sublevel--;
						}
						if ( !sublevel ) {
							break;
						}

						/* parse diffuse map bitmap */
						if ( !_pico_stricmp( p->token,"*bitmap" ) ) {
							char* name = _pico_parse( p,0 );
							if ( name == NULL ) {
								_ase_error_return( "Missing material map bitmap name" );
							}
							mapname = _pico_alloc( strlen( name ) + 1 );
							strcpy( mapname, name );
							/* skip rest and continue with next token */
							_pico_parse_skip_rest( p );
							continue;
						}
					}
				}
				/* end map_diffuse block */
			}
			/* end material block */

			if ( subMaterial == NULL ) {
				/* allocate new pico shader */
				shader = PicoNewShader( model );
				if ( shader == NULL ) {
					PicoFreeModel( model );
					return NULL;
				}

				/* set material name */
				PicoSetShaderName( shader,materialName );

				/* set shader's transparency */
				PicoSetShaderTransparency( shader,transValue );

				/* set shader's ambient color */
				PicoSetShaderAmbientColor( shader,ambientColor );

				/* set diffuse alpha to transparency */
				diffuseColor[3] = (picoByte_t)( transValue * 255.0 );

				/* set shader's diffuse color */
				PicoSetShaderDiffuseColor( shader,diffuseColor );

				/* set shader's specular color */
				PicoSetShaderSpecularColor( shader,specularColor );

				/* set shader's shininess */
				PicoSetShaderShininess( shader,shineValue );

				/* set material map name */
				PicoSetShaderMapName( shader, mapname );

				/* this is just a material with 1 submaterial */
				subMaterial = _ase_add_submaterial( &materials, index, 0, shader );
			}

			/* ydnar: free mapname */
			if ( mapname != NULL ) {
				_pico_free( mapname );
			}
		}   // !_pico_stricmp ( "*material" )

		/* skip unparsed rest of line and continue */
		_pico_parse_skip_rest( p );
	}

	/* ydnar: finish existing surface */
//	_ase_make_surface( model, &surface );
	_ase_submit_triangles( surface, model,materials,faces );
	_ase_free_faces( &faces,&facesTail );

#ifdef DEBUG_PM_ASE
	_ase_print_materials( materials );
	finish = clock();
	elapsed = (double)( finish - start ) / CLOCKS_PER_SEC;
	_pico_printf( PICO_NORMAL, "Loaded model in in %-.2f second(s)\n", elapsed );
#endif //DEBUG_PM_ASE

	_ase_free_materials( &materials );

	/* return allocated pico model */
	return model;
}

/* pico file format module definition */
const picoModule_t picoModuleASE =
{
	"1.0",                  /* module version string */
	"Autodesk 3DSMAX ASCII",    /* module display name */
	"Jared Hefty, seaw0lf",                 /* author's name */
	"2003 Jared Hefty, 2002 seaw0lf",               /* module copyright */
	{
		"ase",NULL,NULL,NULL    /* default extensions to use */
	},
	_ase_canload,               /* validation routine */
	_ase_load,                  /* load routine */
	NULL,                       /* save validation routine */
	NULL                        /* save routine */
};
