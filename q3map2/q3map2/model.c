/* -------------------------------------------------------------------------------

   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   ----------------------------------------------------------------------------------

   This code has been altered significantly from its original form, to support
   several games based on the Quake III Arena engine, in the form of "Q3Map2."

   ------------------------------------------------------------------------------- */



/* marker */
#define MODEL_C



/* dependencies */
#include "q3map2.h"



/*
   PicoPrintFunc()
   callback for picomodel.lib
 */

void PicoPrintFunc( int level, const char *str ){
	if ( str == NULL ) {
		return;
	}
	switch ( level )
	{
	case PICO_NORMAL:
		Sys_Printf( "%s\n", str );
		break;

	case PICO_VERBOSE:
		Sys_FPrintf( SYS_VRB, "%s\n", str );
		break;

	case PICO_WARNING:
		Sys_FPrintf( SYS_WRN, "WARNING: %s\n", str );
		break;

	case PICO_ERROR:
		Sys_FPrintf( SYS_ERR, "ERROR: %s\n", str );
		break;

	case PICO_FATAL:
		Error( "ERROR: %s\n", str );
		break;
	}
}



/*
   PicoLoadFileFunc()
   callback for picomodel.lib
 */

void PicoLoadFileFunc( char *name, byte **buffer, int *bufSize ){
	*bufSize = vfsLoadFile( (const char*) name, (void**) buffer, 0 );
}



/*
   FindModel() - ydnar
   finds an existing picoModel and returns a pointer to the picoModel_t struct or NULL if not found
 */

picoModel_t *FindModel( char *name, int frame ){
	int i;


	/* init */
	if ( numPicoModels <= 0 ) {
		memset( picoModels, 0, sizeof( picoModels ) );
	}

	/* dummy check */
	if ( name == NULL || name[ 0 ] == '\0' ) {
		return NULL;
	}

	/* search list */
	for ( i = 0; i < MAX_MODELS; i++ )
	{
		if ( picoModels[ i ] != NULL &&
			 !strcmp( PicoGetModelName( picoModels[ i ] ), name ) &&
			 PicoGetModelFrameNum( picoModels[ i ] ) == frame ) {
			return picoModels[ i ];
		}
	}

	/* no matching picoModel found */
	return NULL;
}



/*
   LoadModel() - ydnar
   loads a picoModel and returns a pointer to the picoModel_t struct or NULL if not found
 */

picoModel_t *LoadModel( char *name, int frame ){
	int i;
	picoModel_t     *model, **pm;


	/* init */
	if ( numPicoModels <= 0 ) {
		memset( picoModels, 0, sizeof( picoModels ) );
	}

	/* dummy check */
	if ( name == NULL || name[ 0 ] == '\0' ) {
		return NULL;
	}

	/* try to find existing picoModel */
	model = FindModel( name, frame );
	if ( model != NULL ) {
		return model;
	}

	/* none found, so find first non-null picoModel */
	pm = NULL;
	for ( i = 0; i < MAX_MODELS; i++ )
	{
		if ( picoModels[ i ] == NULL ) {
			pm = &picoModels[ i ];
			break;
		}
	}

	/* too many picoModels? */
	if ( pm == NULL ) {
		Error( "MAX_MODELS (%d) exceeded, there are too many model files referenced by the map.", MAX_MODELS );
	}

	/* attempt to parse model */
	*pm = PicoLoadModel( (char*) name, frame );

	/* if loading failed, make a bogus model to silence the rest of the warnings */
	if ( *pm == NULL ) {
		/* allocate a new model */
		*pm = PicoNewModel();
		if ( *pm == NULL ) {
			return NULL;
		}

		/* set data */
		PicoSetModelName( *pm, name );
		PicoSetModelFrameNum( *pm, frame );
	}

	/* debug code */
	#if 0
	{
		int numSurfaces, numVertexes;
		picoSurface_t   *ps;


		Sys_Printf( "Model %s\n", name );
		numSurfaces = PicoGetModelNumSurfaces( *pm );
		for ( i = 0; i < numSurfaces; i++ )
		{
			ps = PicoGetModelSurface( *pm, i );
			numVertexes = PicoGetSurfaceNumVertexes( ps );
			Sys_Printf( "Surface %d has %d vertexes\n", i, numVertexes );
		}
	}
	#endif

	/* set count */
	if ( *pm != NULL ) {
		numPicoModels++;
	}

	/* return the picoModel */
	return *pm;
}



/*
   InsertModel() - ydnar
   adds a picomodel into the bsp
 */

void InsertModel( char *name, int frame, m4x4_t transform, remap_t *remap, shaderInfo_t *celShader, int eNum, int castShadows, int recvShadows, int spawnFlags, float lightmapScale ){
	int i, j, k, s, numSurfaces;
	m4x4_t identity, nTransform;
	picoModel_t         *model;
	picoShader_t        *shader;
	picoSurface_t       *surface;
	shaderInfo_t        *si;
	mapDrawSurface_t    *ds;
	bspDrawVert_t       *dv;
	char                *picoShaderName;
	char shaderName[ MAX_QPATH ];
	picoVec_t           *xyz, *normal, *st;
	byte                *color;
	picoIndex_t         *indexes;
	remap_t             *rm, *glob;


	/* get model */
	model = LoadModel( name, frame );
	if ( model == NULL ) {
		return;
	}

	/* handle null matrix */
	if ( transform == NULL ) {
		m4x4_identity( identity );
		transform = identity;
	}

	/* hack: Stable-1_2 and trunk have differing row/column major matrix order
	   this transpose is necessary with Stable-1_2
	   uncomment the following line with old m4x4_t (non 1.3/spog_branch) code */
	//%	m4x4_transpose( transform );

	/* create transform matrix for normals */
	memcpy( nTransform, transform, sizeof( m4x4_t ) );
	if ( m4x4_invert( nTransform ) ) {
		Sys_FPrintf( SYS_VRB, "WARNING: Can't invert model transform matrix, using transpose instead\n" );
	}
	m4x4_transpose( nTransform );

	/* fix bogus lightmap scale */
	if ( lightmapScale <= 0.0f ) {
		lightmapScale = 1.0f;
	}

	/* each surface on the model will become a new map drawsurface */
	numSurfaces = PicoGetModelNumSurfaces( model );
	//%	Sys_FPrintf( SYS_VRB, "Model %s has %d surfaces\n", name, numSurfaces );
	for ( s = 0; s < numSurfaces; s++ )
	{
		/* get surface */
		surface = PicoGetModelSurface( model, s );
		if ( surface == NULL ) {
			continue;
		}

		/* only handle triangle surfaces initially (fixme: support patches) */
		if ( PicoGetSurfaceType( surface ) != PICO_TRIANGLES ) {
			continue;
		}

		/* fix the surface's normals */
		PicoFixSurfaceNormals( surface );

		/* allocate a surface (ydnar: gs mods) */
		ds = AllocDrawSurface( SURFACE_TRIANGLES );
		ds->entityNum = eNum;
		ds->castShadows = castShadows;
		ds->recvShadows = recvShadows;

		/* get shader name */
		shader = PicoGetSurfaceShader( surface );
		if ( shader == NULL ) {
			picoShaderName = "";
		}
		else{
			picoShaderName = PicoGetShaderName( shader );
		}

		/* handle shader remapping */
		glob = NULL;
		for ( rm = remap; rm != NULL; rm = rm->next )
		{
			if ( rm->from[ 0 ] == '*' && rm->from[ 1 ] == '\0' ) {
				glob = rm;
			}
			else if ( !Q_stricmp( picoShaderName, rm->from ) ) {
				Sys_FPrintf( SYS_VRB, "Remapping %s to %s\n", picoShaderName, rm->to );
				picoShaderName = rm->to;
				glob = NULL;
				break;
			}
		}

		if ( glob != NULL ) {
			Sys_FPrintf( SYS_VRB, "Globbing %s to %s\n", picoShaderName, glob->to );
			picoShaderName = glob->to;
		}

		/* shader renaming for sof2 */
		if ( renameModelShaders ) {
			strcpy( shaderName, picoShaderName );
			StripExtension( shaderName );
			if ( spawnFlags & 1 ) {
				strcat( shaderName, "_RMG_BSP" );
			}
			else{
				strcat( shaderName, "_BSP" );
			}
			si = ShaderInfoForShader( shaderName );
		}
		else{
			si = ShaderInfoForShader( picoShaderName );
		}

		/* set shader */
		ds->shaderInfo = si;

		/* set lightmap scale */
		ds->lightmapScale = lightmapScale;

		/* force to meta? */
		if ( ( si != NULL && si->forceMeta ) || ( spawnFlags & 4 ) ) { /* 3rd bit */
			ds->type = SURFACE_FORCED_META;
		}

		/* set particulars */
		ds->numVerts = PicoGetSurfaceNumVertexes( surface );
		ds->verts = safe_malloc( ds->numVerts * sizeof( ds->verts[ 0 ] ) );
		memset( ds->verts, 0, ds->numVerts * sizeof( ds->verts[ 0 ] ) );

		ds->numIndexes = PicoGetSurfaceNumIndexes( surface );
		ds->indexes = safe_malloc( ds->numIndexes * sizeof( ds->indexes[ 0 ] ) );
		memset( ds->indexes, 0, ds->numIndexes * sizeof( ds->indexes[ 0 ] ) );

		/* copy vertexes */
		for ( i = 0; i < ds->numVerts; i++ )
		{
			/* get vertex */
			dv = &ds->verts[ i ];

			/* xyz and normal */
			xyz = PicoGetSurfaceXYZ( surface, i );
			VectorCopy( xyz, dv->xyz );
			m4x4_transform_point( transform, dv->xyz );

			normal = PicoGetSurfaceNormal( surface, i );
			VectorCopy( normal, dv->normal );
			m4x4_transform_normal( nTransform, dv->normal );
			VectorNormalize( dv->normal, dv->normal );

			/* ydnar: tek-fu celshading support for flat shaded shit */
			if ( flat ) {
				dv->st[ 0 ] = si->stFlat[ 0 ];
				dv->st[ 1 ] = si->stFlat[ 1 ];
			}

			/* ydnar: gs mods: added support for explicit shader texcoord generation */
			else if ( si->tcGen ) {
				/* project the texture */
				dv->st[ 0 ] = DotProduct( si->vecs[ 0 ], dv->xyz );
				dv->st[ 1 ] = DotProduct( si->vecs[ 1 ], dv->xyz );
			}

			/* normal texture coordinates */
			else
			{
				st = PicoGetSurfaceST( surface, 0, i );
				dv->st[ 0 ] = st[ 0 ];
				dv->st[ 1 ] = st[ 1 ];
			}

			/* set lightmap/color bits */
			color = PicoGetSurfaceColor( surface, 0, i );
			for ( j = 0; j < MAX_LIGHTMAPS; j++ )
			{
				dv->lightmap[ j ][ 0 ] = 0.0f;
				dv->lightmap[ j ][ 1 ] = 0.0f;
				dv->color[ j ][ 0 ] = color[ 0 ];
				dv->color[ j ][ 1 ] = color[ 1 ];
				dv->color[ j ][ 2 ] = color[ 2 ];
				dv->color[ j ][ 3 ] = color[ 3 ];
			}
		}

		/* copy indexes */
		indexes = PicoGetSurfaceIndexes( surface, 0 );
		for ( i = 0; i < ds->numIndexes; i++ )
			ds->indexes[ i ] = indexes[ i ];

		/* set cel shader */
		ds->celShader = celShader;

		/* ydnar: giant hack land: generate clipping brushes for model triangles */
		if ( si->clipModel || ( spawnFlags & 2 ) ) { /* 2nd bit */
			vec3_t points[ 3 ], backs[ 3 ];
			vec4_t plane, reverse, pa, pb, pc;
			vec3_t nadir;


			/* temp hack */
			if ( !si->clipModel &&
				 ( ( si->compileFlags & C_TRANSLUCENT ) || !( si->compileFlags & C_SOLID ) ) ) {
				continue;
			}

			/* overflow check */
			if ( ( nummapplanes + 64 ) >= ( MAX_MAP_PLANES >> 1 ) ) {
				continue;
			}

			/* walk triangle list */
			for ( i = 0; i < ds->numIndexes; i += 3 )
			{
				/* overflow hack */
				if ( ( nummapplanes + 64 ) >= ( MAX_MAP_PLANES >> 1 ) ) {
					Sys_FPrintf( SYS_WRN, "WARNING: MAX_MAP_PLANES (%d) hit generating clip brushes for model %s.\n",
								MAX_MAP_PLANES, name );
					break;
				}

				/* make points and back points */
				for ( j = 0; j < 3; j++ )
				{
					/* get vertex */
					dv = &ds->verts[ ds->indexes[ i + j ] ];

					/* copy xyz */
					VectorCopy( dv->xyz, points[ j ] );
#if !Q3MAP2_EXPERIMENTAL_MODEL_CLIPPING_FIX
					// The code below is totally unneeded regardless of Q3MAP2_EXPERIMENTAL_MODEL_CLIPPING_FIX.
					// backs is reinitialized further below.  However, the extra code does not hurt anything.
					VectorCopy( dv->xyz, backs[ j ] );

					/* find nearest axial to normal and push back points opposite */
					/* note: this doesn't work as well as simply using the plane of the triangle, below */
					for ( k = 0; k < 3; k++ )
					{
						if ( fabs( dv->normal[ k ] ) > fabs( dv->normal[ ( k + 1 ) % 3 ] ) &&
							 fabs( dv->normal[ k ] ) > fabs( dv->normal[ ( k + 2 ) % 3 ] ) ) {
							backs[ j ][ k ] += dv->normal[ k ] < 0.0f ? 64.0f : -64.0f;
							break;
						}
					}
#endif
				}

				/* make plane for triangle */
				if ( PlaneFromPoints( plane, points[ 0 ], points[ 1 ], points[ 2 ] ) ) {
					/* regenerate back points */
					for ( j = 0; j < 3; j++ )
					{
						/* get vertex */
						dv = &ds->verts[ ds->indexes[ i + j ] ];

						/* copy xyz */
						VectorCopy( dv->xyz, backs[ j ] );

						/* find nearest axial to plane normal and push back points opposite */
						for ( k = 0; k < 3; k++ )
						{
#if Q3MAP2_EXPERIMENTAL_MODEL_CLIPPING_FIX
							if ( fabs( plane[ k ] ) >= fabs( plane[ ( k + 1 ) % 3 ] ) &&
								 fabs( plane[ k ] ) >= fabs( plane[ ( k + 2 ) % 3 ] ) )
#else
							// This code is broken for 45 degree angles where there
							// is no clear winner.
							if ( fabs( plane[ k ] ) > fabs( plane[ ( k + 1 ) % 3 ] ) &&
								 fabs( plane[ k ] ) > fabs( plane[ ( k + 2 ) % 3 ] ) )
#endif
							{
								backs[ j ][ k ] += plane[ k ] < 0.0f ? 64.0f : -64.0f;
								break;
							}
						}
					}

					/* make back plane */
					VectorScale( plane, -1.0f, reverse );
					reverse[ 3 ] = -( plane[ 3 ] - 1 );

					/* make back pyramid point */
					VectorCopy( points[ 0 ], nadir );
					VectorAdd( nadir, points[ 1 ], nadir );
					VectorAdd( nadir, points[ 2 ], nadir );
					VectorScale( nadir, 0.3333333333333f, nadir );
					VectorMA( nadir, -2.0f, plane, nadir );

					/* make 3 more planes */
					//%	if( PlaneFromPoints( pa, points[ 2 ], points[ 1 ], nadir ) &&
					//%		PlaneFromPoints( pb, points[ 1 ], points[ 0 ], nadir ) &&
					//%		PlaneFromPoints( pc, points[ 0 ], points[ 2 ], nadir ) )
					if ( PlaneFromPoints( pa, points[ 2 ], points[ 1 ], backs[ 1 ] ) &&
						 PlaneFromPoints( pb, points[ 1 ], points[ 0 ], backs[ 0 ] ) &&
						 PlaneFromPoints( pc, points[ 0 ], points[ 2 ], backs[ 2 ] ) ) {
						/* build a brush */
						buildBrush = AllocBrush( 48 );

						buildBrush->entityNum = mapEntityNum;
						buildBrush->original = buildBrush;
						buildBrush->contentShader = si;
						buildBrush->compileFlags = si->compileFlags;
						buildBrush->contentFlags = si->contentFlags;
						buildBrush->detail = qtrue;

						/* set up brush sides */
						buildBrush->numsides = 5;
						for ( j = 0; j < buildBrush->numsides; j++ )
							buildBrush->sides[ j ].shaderInfo = si;
						buildBrush->sides[ 0 ].planenum = FindFloatPlane( plane, plane[ 3 ], 3, points );
						buildBrush->sides[ 1 ].planenum = FindFloatPlane( pa, pa[ 3 ], 1, &points[ 2 ] );
						buildBrush->sides[ 2 ].planenum = FindFloatPlane( pb, pb[ 3 ], 1, &points[ 1 ] );
						buildBrush->sides[ 3 ].planenum = FindFloatPlane( pc, pc[ 3 ], 1, &points[ 0 ] );
						buildBrush->sides[ 4 ].planenum = FindFloatPlane( reverse, reverse[ 3 ], 3, points );

						/* add to entity */
						if ( CreateBrushWindings( buildBrush ) ) {
							AddBrushBevels();
							//%	EmitBrushes( buildBrush, NULL, NULL );
							buildBrush->next = entities[ mapEntityNum ].brushes;
							entities[ mapEntityNum ].brushes = buildBrush;
							entities[ mapEntityNum ].numBrushes++;
						}
						else{
							free( buildBrush );
						}
					}
				}
			}
		}
	}
}



/*
   AddTriangleModels()
   adds misc_model surfaces to the bsp
 */

void AddTriangleModels( entity_t *e ){
	int num, frame, castShadows, recvShadows, spawnFlags;
	entity_t        *e2;
	const char      *targetName;
	const char      *target, *model, *value;
	char shader[ MAX_QPATH ];
	shaderInfo_t    *celShader;
	float temp, baseLightmapScale, lightmapScale;
	vec3_t origin, scale, angles;
	m4x4_t transform;
	epair_t         *ep;
	remap_t         *remap, *remap2;
	char            *split;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- AddTriangleModels ---\n" );

	/* get current brush entity targetname */
	if ( e == entities ) {
		targetName = "";
	}
	else
	{
		targetName = ValueForKey( e, "targetname" );

		/* misc_model entities target non-worldspawn brush model entities */
		if ( targetName[ 0 ] == '\0' ) {
			return;
		}
	}

	/* get lightmap scale */
	baseLightmapScale = FloatForKey( e, "_lightmapscale" );
	if ( baseLightmapScale <= 0.0f ) {
		baseLightmapScale = 0.0f;
	}

	/* walk the entity list */
	for ( num = 1; num < numEntities; num++ )
	{
		/* get e2 */
		e2 = &entities[ num ];

		/* convert misc_models into raw geometry */
		if ( Q_stricmp( "misc_model", ValueForKey( e2, "classname" ) ) ) {
			continue;
		}

		/* ydnar: added support for md3 models on non-worldspawn models */
		target = ValueForKey( e2, "target" );
		if ( strcmp( target, targetName ) ) {
			continue;
		}

		/* get model name */
		model = ValueForKey( e2, "model" );
		if ( model[ 0 ] == '\0' ) {
			Sys_FPrintf( SYS_WRN, "WARNING: misc_model at %i %i %i without a model key\n",
						(int) origin[ 0 ], (int) origin[ 1 ], (int) origin[ 2 ] );
			continue;
		}

		/* get model frame */
		frame = IntForKey( e2, "_frame" );

		/* worldspawn (and func_groups) default to cast/recv shadows in worldspawn group */
		if ( e == entities ) {
			castShadows = WORLDSPAWN_CAST_SHADOWS;
			recvShadows = WORLDSPAWN_RECV_SHADOWS;
		}

		/* other entities don't cast any shadows, but recv worldspawn shadows */
		else
		{
			castShadows = ENTITY_CAST_SHADOWS;
			recvShadows = ENTITY_RECV_SHADOWS;
		}

		/* get explicit shadow flags */
		GetEntityShadowFlags( e2, e, &castShadows, &recvShadows );

		/* get spawnflags */
		spawnFlags = IntForKey( e2, "spawnflags" );

		/* get origin */
		GetVectorForKey( e2, "origin", origin );
		VectorSubtract( origin, e->origin, origin );    /* offset by parent */

		/* get scale */
		scale[ 0 ] = scale[ 1 ] = scale[ 2 ] = 1.0f;
		temp = FloatForKey( e2, "modelscale" );
		if ( temp != 0.0f ) {
			scale[ 0 ] = scale[ 1 ] = scale[ 2 ] = temp;
		}
		value = ValueForKey( e2, "modelscale_vec" );
		if ( value[ 0 ] != '\0' ) {
			sscanf( value, "%f %f %f", &scale[ 0 ], &scale[ 1 ], &scale[ 2 ] );
		}

		/* get "angle" (yaw) or "angles" (pitch yaw roll) */
		angles[ 0 ] = angles[ 1 ] = angles[ 2 ] = 0.0f;
		angles[ 2 ] = FloatForKey( e2, "angle" );
		value = ValueForKey( e2, "angles" );
		if ( value[ 0 ] != '\0' ) {
			sscanf( value, "%f %f %f", &angles[ 1 ], &angles[ 2 ], &angles[ 0 ] );
		}

		/* set transform matrix (thanks spog) */
		m4x4_identity( transform );
		m4x4_pivoted_transform_by_vec3( transform, origin, angles, eXYZ, scale, vec3_origin );

		/* get shader remappings */
		remap = NULL;
		for ( ep = e2->epairs; ep != NULL; ep = ep->next )
		{
			/* look for keys prefixed with "_remap" */
			if ( ep->key != NULL && ep->value != NULL &&
				 ep->key[ 0 ] != '\0' && ep->value[ 0 ] != '\0' &&
				 !Q_strncasecmp( ep->key, "_remap", 6 ) ) {
				/* create new remapping */
				remap2 = remap;
				remap = safe_malloc( sizeof( *remap ) );
				remap->next = remap2;
				strcpy( remap->from, ep->value );

				/* split the string */
				split = strchr( remap->from, ';' );
				if ( split == NULL ) {
					Sys_FPrintf( SYS_WRN, "WARNING: Shader _remap key found in misc_model without a ; character\n" );
					free( remap );
					remap = remap2;
					continue;
				}

				/* store the split */
				*split = '\0';
				strcpy( remap->to, ( split + 1 ) );

				/* note it */
				//%	Sys_FPrintf( SYS_VRB, "Remapping %s to %s\n", remap->from, remap->to );
			}
		}

		/* ydnar: cel shader support */
		value = ValueForKey( e2, "_celshader" );
		if ( value[ 0 ] == '\0' ) {
			value = ValueForKey( &entities[ 0 ], "_celshader" );
		}
		if ( value[ 0 ] != '\0' ) {
			sprintf( shader, "textures/%s", value );
			celShader = ShaderInfoForShader( shader );
		}
		else{
			celShader = NULL;
		}

		/* get lightmap scale */
		lightmapScale = FloatForKey( e2, "_lightmapscale" );
		if ( lightmapScale <= 0.0f ) {
			lightmapScale = baseLightmapScale;
		}

		/* insert the model */
		InsertModel( (char*) model, frame, transform, remap, celShader, mapEntityNum, castShadows, recvShadows, spawnFlags, lightmapScale );

		/* free shader remappings */
		while ( remap != NULL )
		{
			remap2 = remap->next;
			free( remap );
			remap = remap2;
		}
	}
}
