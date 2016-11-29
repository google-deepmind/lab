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
#define SURFACE_C



/* dependencies */
#include "q3map2.h"



/*
   AllocDrawSurface()
   ydnar: gs mods: changed to force an explicit type when allocating
 */

mapDrawSurface_t *AllocDrawSurface( surfaceType_t type ){
	mapDrawSurface_t    *ds;


	/* ydnar: gs mods: only allocate valid types */
	if ( type <= SURFACE_BAD || type >= NUM_SURFACE_TYPES ) {
		Error( "AllocDrawSurface: Invalid surface type %d specified", type );
	}

	/* bounds check */
	if ( numMapDrawSurfs >= MAX_MAP_DRAW_SURFS ) {
		Error( "MAX_MAP_DRAW_SURFS (%d) exceeded", MAX_MAP_DRAW_SURFS );
	}
	ds = &mapDrawSurfs[ numMapDrawSurfs ];
	numMapDrawSurfs++;

	/* ydnar: do initial surface setup */
	memset( ds, 0, sizeof( mapDrawSurface_t ) );
	ds->type = type;
	ds->planeNum = -1;
	ds->fogNum = defaultFogNum;             /* ydnar 2003-02-12 */
	ds->outputNum = -1;                     /* ydnar 2002-08-13 */
	ds->surfaceNum = numMapDrawSurfs - 1;   /* ydnar 2003-02-16 */

	return ds;
}



/*
   FinishSurface()
   ydnar: general surface finish pass
 */

void FinishSurface( mapDrawSurface_t *ds ){
	mapDrawSurface_t    *ds2;


	/* dummy check */
	if ( ds->type <= SURFACE_BAD || ds->type >= NUM_SURFACE_TYPES || ds == NULL || ds->shaderInfo == NULL ) {
		return;
	}

	/* ydnar: rocking tek-fu celshading */
	if ( ds->celShader != NULL ) {
		MakeCelSurface( ds, ds->celShader );
	}

	/* backsides stop here */
	if ( ds->backSide ) {
		return;
	}

	/* ydnar: rocking surface cloning (fur baby yeah!) */
	if ( ds->shaderInfo->cloneShader != NULL && ds->shaderInfo->cloneShader[ 0 ] != '\0' ) {
		CloneSurface( ds, ShaderInfoForShader( ds->shaderInfo->cloneShader ) );
	}

	/* ydnar: q3map_backShader support */
	if ( ds->shaderInfo->backShader != NULL && ds->shaderInfo->backShader[ 0 ] != '\0' ) {
		ds2 = CloneSurface( ds, ShaderInfoForShader( ds->shaderInfo->backShader ) );
		ds2->backSide = qtrue;
	}
}



/*
   CloneSurface()
   clones a map drawsurface, using the specified shader
 */

mapDrawSurface_t *CloneSurface( mapDrawSurface_t *src, shaderInfo_t *si ){
	mapDrawSurface_t    *ds;


	/* dummy check */
	if ( src == NULL || si == NULL ) {
		return NULL;
	}

	/* allocate a new surface */
	ds = AllocDrawSurface( src->type );
	if ( ds == NULL ) {
		return NULL;
	}

	/* copy it */
	memcpy( ds, src, sizeof( *ds ) );

	/* destroy side reference */
	ds->sideRef = NULL;

	/* set shader */
	ds->shaderInfo = si;

	/* copy verts */
	if ( ds->numVerts > 0 ) {
		ds->verts = safe_malloc( ds->numVerts * sizeof( *ds->verts ) );
		memcpy( ds->verts, src->verts, ds->numVerts * sizeof( *ds->verts ) );
	}

	/* copy indexes */
	if ( ds->numIndexes <= 0 ) {
		return ds;
	}
	ds->indexes = safe_malloc( ds->numIndexes * sizeof( *ds->indexes ) );
	memcpy( ds->indexes, src->indexes, ds->numIndexes * sizeof( *ds->indexes ) );

	/* return the surface */
	return ds;
}



/*
   MakeCelSurface() - ydnar
   makes a copy of a surface, but specific to cel shading
 */

mapDrawSurface_t *MakeCelSurface( mapDrawSurface_t *src, shaderInfo_t *si ){
	mapDrawSurface_t    *ds;


	/* dummy check */
	if ( src == NULL || si == NULL ) {
		return NULL;
	}

	/* don't create cel surfaces for certain types of shaders */
	if ( ( src->shaderInfo->compileFlags & C_TRANSLUCENT ) ||
		 ( src->shaderInfo->compileFlags & C_SKY ) ) {
		return NULL;
	}

	/* make a copy */
	ds = CloneSurface( src, si );
	if ( ds == NULL ) {
		return NULL;
	}

	/* do some fixups for celshading */
	ds->planar = qfalse;
	ds->planeNum = -1;

	/* return the surface */
	return ds;
}



/*
   MakeSkyboxSurface() - ydnar
   generates a skybox surface, viewable from everywhere there is sky
 */

mapDrawSurface_t *MakeSkyboxSurface( mapDrawSurface_t *src ){
	int i;
	mapDrawSurface_t    *ds;


	/* dummy check */
	if ( src == NULL ) {
		return NULL;
	}

	/* make a copy */
	ds = CloneSurface( src, src->shaderInfo );
	if ( ds == NULL ) {
		return NULL;
	}

	/* set parent */
	ds->parent = src;

	/* scale the surface vertexes */
	for ( i = 0; i < ds->numVerts; i++ )
	{
		m4x4_transform_point( skyboxTransform, ds->verts[ i ].xyz );

		/* debug code */
		//%	bspDrawVerts[ bspDrawSurfaces[ ds->outputNum ].firstVert + i ].color[ 0 ][ 1 ] = 0;
		//%	bspDrawVerts[ bspDrawSurfaces[ ds->outputNum ].firstVert + i ].color[ 0 ][ 2 ] = 0;
	}

	/* so backface culling creep doesn't bork the surface */
	VectorClear( ds->lightmapVecs[ 2 ] );

	/* return the surface */
	return ds;
}



/*
   IsTriangleDegenerate
   returns qtrue if all three points are colinear, backwards, or the triangle is just plain bogus
 */

#define TINY_AREA   1.0f

qboolean IsTriangleDegenerate( bspDrawVert_t *points, int a, int b, int c ){
	vec3_t v1, v2, v3;
	float d;


	/* calcuate the area of the triangle */
	VectorSubtract( points[ b ].xyz, points[ a ].xyz, v1 );
	VectorSubtract( points[ c ].xyz, points[ a ].xyz, v2 );
	CrossProduct( v1, v2, v3 );
	d = VectorLength( v3 );

	/* assume all very small or backwards triangles will cause problems */
	if ( d < TINY_AREA ) {
		return qtrue;
	}

	/* must be a good triangle */
	return qfalse;
}



/*
   ClearSurface() - ydnar
   clears a surface and frees any allocated memory
 */

void ClearSurface( mapDrawSurface_t *ds ){
	ds->type = SURFACE_BAD;
	ds->planar = qfalse;
	ds->planeNum = -1;
	ds->numVerts = 0;
	if ( ds->verts != NULL ) {
		free( ds->verts );
	}
	ds->verts = NULL;
	ds->numIndexes = 0;
	if ( ds->indexes != NULL ) {
		free( ds->indexes );
	}
	ds->indexes = NULL;
	numClearedSurfaces++;
}



/*
   TidyEntitySurfaces() - ydnar
   deletes all empty or bad surfaces from the surface list
 */

void TidyEntitySurfaces( entity_t *e ){
	int i, j, deleted;
	mapDrawSurface_t    *out, *in;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- TidyEntitySurfaces ---\n" );

	/* walk the surface list */
	deleted = 0;
	for ( i = e->firstDrawSurf, j = e->firstDrawSurf; j < numMapDrawSurfs; i++, j++ )
	{
		/* get out surface */
		out = &mapDrawSurfs[ i ];

		/* walk the surface list again until a proper surface is found */
		for ( ; j < numMapDrawSurfs; j++ )
		{
			/* get in surface */
			in = &mapDrawSurfs[ j ];

			/* this surface ok? */
			if ( in->type == SURFACE_FLARE || in->type == SURFACE_SHADER ||
				 ( in->type != SURFACE_BAD && in->numVerts > 0 ) ) {
				break;
			}

			/* nuke it */
			ClearSurface( in );
			deleted++;
		}

		/* copy if necessary */
		if ( i != j ) {
			memcpy( out, in, sizeof( mapDrawSurface_t ) );
		}
	}

	/* set the new number of drawsurfs */
	numMapDrawSurfs = i;

	/* emit some stats */
	Sys_FPrintf( SYS_VRB, "%9d empty or malformed surfaces deleted\n", deleted );
}



/*
   CalcSurfaceTextureRange() - ydnar
   calculates the clamped texture range for a given surface, returns qtrue if it's within [-texRange,texRange]
 */

qboolean CalcSurfaceTextureRange( mapDrawSurface_t *ds ){
	int i, j, v, size[ 2 ];
	float mins[ 2 ], maxs[ 2 ];


	/* try to early out */
	if ( ds->numVerts <= 0 ) {
		return qtrue;
	}

	/* walk the verts and determine min/max st values */
	mins[ 0 ] = 999999;
	mins[ 1 ] = 999999;
	maxs[ 0 ] = -999999;
	maxs[ 1 ] = -999999;
	for ( i = 0; i < ds->numVerts; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			if ( ds->verts[ i ].st[ j ] < mins[ j ] ) {
				mins[ j ] = ds->verts[ i ].st[ j ];
			}
			if ( ds->verts[ i ].st[ j ] > maxs[ j ] ) {
				maxs[ j ] = ds->verts[ i ].st[ j ];
			}
		}
	}

	/* clamp to integer range and calculate surface bias values */
	for ( j = 0; j < 2; j++ )
		ds->bias[ j ] = -floor( 0.5f * ( mins[ j ] + maxs[ j ] ) );

	/* find biased texture coordinate mins/maxs */
	size[ 0 ] = ds->shaderInfo->shaderWidth;
	size[ 1 ] = ds->shaderInfo->shaderHeight;
	ds->texMins[ 0 ] = 999999;
	ds->texMins[ 1 ] = 999999;
	ds->texMaxs[ 0 ] = -999999;
	ds->texMaxs[ 1 ] = -999999;
	for ( i = 0; i < ds->numVerts; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			v = ( (float) ds->verts[ i ].st[ j ] + ds->bias[ j ] ) * size[ j ];
			if ( v < ds->texMins[ j ] ) {
				ds->texMins[ j ] = v;
			}
			if ( v > ds->texMaxs[ j ] ) {
				ds->texMaxs[ j ] = v;
			}
		}
	}

	/* calc ranges */
	for ( j = 0; j < 2; j++ )
		ds->texRange[ j ] = ( ds->texMaxs[ j ] - ds->texMins[ j ] );

	/* if range is zero, then assume unlimited precision */
	if ( texRange == 0 ) {
		return qtrue;
	}

	/* within range? */
	for ( j = 0; j < 2; j++ )
	{
		if ( ds->texMins[ j ] < -texRange || ds->texMaxs[ j ] > texRange ) {
			return qfalse;
		}
	}

	/* within range */
	return qtrue;
}



/*
   CalcLightmapAxis() - ydnar
   gives closed lightmap axis for a plane normal
 */

qboolean CalcLightmapAxis( vec3_t normal, vec3_t axis ){
	vec3_t absolute;


	/* test */
	if ( normal[ 0 ] == 0.0f && normal[ 1 ] == 0.0f && normal[ 2 ] == 0.0f ) {
		VectorClear( axis );
		return qfalse;
	}

	/* get absolute normal */
	absolute[ 0 ] = fabs( normal[ 0 ] );
	absolute[ 1 ] = fabs( normal[ 1 ] );
	absolute[ 2 ] = fabs( normal[ 2 ] );

	/* test and set */
	if ( absolute[ 2 ] > absolute[ 0 ] - 0.0001f && absolute[ 2 ] > absolute[ 1 ] - 0.0001f ) {
		if ( normal[ 2 ] > 0.0f ) {
			VectorSet( axis, 0.0f, 0.0f, 1.0f );
		}
		else{
			VectorSet( axis, 0.0f, 0.0f, -1.0f );
		}
	}
	else if ( absolute[ 0 ] > absolute[ 1 ] - 0.0001f && absolute[ 0 ] > absolute[ 2 ] - 0.0001f ) {
		if ( normal[ 0 ] > 0.0f ) {
			VectorSet( axis, 1.0f, 0.0f, 0.0f );
		}
		else{
			VectorSet( axis, -1.0f, 0.0f, 0.0f );
		}
	}
	else
	{
		if ( normal[ 1 ] > 0.0f ) {
			VectorSet( axis, 0.0f, 1.0f, 0.0f );
		}
		else{
			VectorSet( axis, 0.0f, -1.0f, 0.0f );
		}
	}

	/* return ok */
	return qtrue;
}



/*
   ClassifySurfaces() - ydnar
   fills out a bunch of info in the surfaces, including planar status, lightmap projection, and bounding box
 */

#define PLANAR_EPSILON  0.5f    //% 0.126f 0.25f

void ClassifySurfaces( int numSurfs, mapDrawSurface_t *ds ){
	int i, bestAxis;
	float dist;
	vec4_t plane;
	shaderInfo_t        *si;
	static vec3_t axii[ 6 ] =
	{
		{ 0, 0, -1 },
		{ 0, 0, 1 },
		{ -1, 0, 0 },
		{ 1, 0, 0 },
		{ 0, -1, 0 },
		{ 0, 1, 0 }
	};


	/* walk the list of surfaces */
	for ( ; numSurfs > 0; numSurfs--, ds++ )
	{
		/* ignore bogus (or flare) surfaces */
		if ( ds->type == SURFACE_BAD || ds->numVerts <= 0 ) {
			continue;
		}

		/* get shader */
		si = ds->shaderInfo;

		/* -----------------------------------------------------------------
		   force meta if vertex count is too high or shader requires it
		   ----------------------------------------------------------------- */

		if ( ds->type != SURFACE_PATCH && ds->type != SURFACE_FACE ) {
			if ( ds->numVerts > SHADER_MAX_VERTEXES ) {
				ds->type = SURFACE_FORCED_META;
			}
		}

		/* -----------------------------------------------------------------
		   plane and bounding box classification
		   ----------------------------------------------------------------- */

		/* set surface bounding box */
		ClearBounds( ds->mins, ds->maxs );
		for ( i = 0; i < ds->numVerts; i++ )
			AddPointToBounds( ds->verts[ i ].xyz, ds->mins, ds->maxs );

		/* try to get an existing plane */
		if ( ds->planeNum >= 0 ) {
			VectorCopy( mapplanes[ ds->planeNum ].normal, plane );
			plane[ 3 ] = mapplanes[ ds->planeNum ].dist;
		}

		/* construct one from the first vert with a valid normal */
		else
		{
			VectorClear( plane );
			plane[ 3 ] = 0.0f;
			for ( i = 0; i < ds->numVerts; i++ )
			{
				if ( ds->verts[ i ].normal[ 0 ] != 0.0f && ds->verts[ i ].normal[ 1 ] != 0.0f && ds->verts[ i ].normal[ 2 ] != 0.0f ) {
					VectorCopy( ds->verts[ i ].normal, plane );
					plane[ 3 ] = DotProduct( ds->verts[ i ].xyz, plane );
					break;
				}
			}
		}

		/* test for bogus plane */
		if ( VectorLength( plane ) <= 0.0f ) {
			ds->planar = qfalse;
			ds->planeNum = -1;
		}
		else
		{
			/* determine if surface is planar */
			ds->planar = qtrue;

			/* test each vert */
			for ( i = 0; i < ds->numVerts; i++ )
			{
				/* point-plane test */
				dist = DotProduct( ds->verts[ i ].xyz, plane ) - plane[ 3 ];
				if ( fabs( dist ) > PLANAR_EPSILON ) {
					//%	if( ds->planeNum >= 0 )
					//%	{
					//%		Sys_FPrintf( SYS_WRN, "WARNING: Planar surface marked unplanar (%f > %f)\n", fabs( dist ), PLANAR_EPSILON );
					//%		ds->verts[ i ].color[ 0 ][ 0 ] = ds->verts[ i ].color[ 0 ][ 2 ] = 0;
					//%	}
					ds->planar = qfalse;
					break;
				}
			}
		}

		/* find map plane if necessary */
		if ( ds->planar ) {
			if ( ds->planeNum < 0 ) {
				ds->planeNum = FindFloatPlane( plane, plane[ 3 ], 1, &ds->verts[ 0 ].xyz );
			}
			VectorCopy( plane, ds->lightmapVecs[ 2 ] );
		}
		else
		{
			ds->planeNum = -1;
			VectorClear( ds->lightmapVecs[ 2 ] );
			//% if( ds->type == SURF_META || ds->type == SURF_FACE )
			//%		Sys_FPrintf( SYS_WRN, "WARNING: Non-planar face (%d): %s\n", ds->planeNum, ds->shaderInfo->shader );
		}

		/* -----------------------------------------------------------------
		   lightmap bounds and axis projection
		   ----------------------------------------------------------------- */

		/* vertex lit surfaces don't need this information */
		if ( si->compileFlags & C_VERTEXLIT || ds->type == SURFACE_TRIANGLES ) {
			VectorClear( ds->lightmapAxis );
			//%	VectorClear( ds->lightmapVecs[ 2 ] );
			ds->sampleSize = 0;
			continue;
		}

		/* the shader can specify an explicit lightmap axis */
		if ( si->lightmapAxis[ 0 ] || si->lightmapAxis[ 1 ] || si->lightmapAxis[ 2 ] ) {
			VectorCopy( si->lightmapAxis, ds->lightmapAxis );
		}
		else if ( ds->type == SURFACE_FORCED_META ) {
			VectorClear( ds->lightmapAxis );
		}
		else if ( ds->planar ) {
			CalcLightmapAxis( plane, ds->lightmapAxis );
		}
		else
		{
			/* find best lightmap axis */
			for ( bestAxis = 0; bestAxis < 6; bestAxis++ )
			{
				for ( i = 0; i < ds->numVerts && bestAxis < 6; i++ )
				{
					//% Sys_Printf( "Comparing %1.3f %1.3f %1.3f to %1.3f %1.3f %1.3f\n",
					//%     ds->verts[ i ].normal[ 0 ], ds->verts[ i ].normal[ 1 ], ds->verts[ i ].normal[ 2 ],
					//%     axii[ bestAxis ][ 0 ], axii[ bestAxis ][ 1 ], axii[ bestAxis ][ 2 ] );
					if ( DotProduct( ds->verts[ i ].normal, axii[ bestAxis ] ) < 0.25f ) { /* fixme: adjust this tolerance to taste */
						break;
					}
				}

				if ( i == ds->numVerts ) {
					break;
				}
			}

			/* set axis if possible */
			if ( bestAxis < 6 ) {
				//% if( ds->type == SURFACE_PATCH )
				//%     Sys_Printf( "Mapped axis %d onto patch\n", bestAxis );
				VectorCopy( axii[ bestAxis ], ds->lightmapAxis );
			}

			/* debug code */
			//% if( ds->type == SURFACE_PATCH )
			//%     Sys_Printf( "Failed to map axis %d onto patch\n", bestAxis );
		}

		/* get lightmap sample size */
		if ( ds->sampleSize <= 0 ) {
			ds->sampleSize = sampleSize;
			if ( ds->shaderInfo->lightmapSampleSize ) {
				ds->sampleSize = ds->shaderInfo->lightmapSampleSize;
			}
			if ( ds->lightmapScale > 0 ) {
				ds->sampleSize *= ds->lightmapScale;
			}
			if ( ds->sampleSize <= 0 ) {
				ds->sampleSize = 1;
			}
			else if ( ds->sampleSize > 16384 ) { /* powers of 2 are preferred */
				ds->sampleSize = 16384;
			}
		}
	}
}



/*
   ClassifyEntitySurfaces() - ydnar
   classifies all surfaces in an entity
 */

void ClassifyEntitySurfaces( entity_t *e ){
	int i;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- ClassifyEntitySurfaces ---\n" );

	/* walk the surface list */
	for ( i = e->firstDrawSurf; i < numMapDrawSurfs; i++ )
	{
		FinishSurface( &mapDrawSurfs[ i ] );
		ClassifySurfaces( 1, &mapDrawSurfs[ i ] );
	}

	/* tidy things up */
	TidyEntitySurfaces( e );
}



/*
   GetShaderIndexForPoint() - ydnar
   for shader-indexed surfaces (terrain), find a matching index from the indexmap
 */

byte GetShaderIndexForPoint( indexMap_t *im, vec3_t eMins, vec3_t eMaxs, vec3_t point ){
	int i, x, y;
	float s, t;
	vec3_t mins, maxs, size;


	/* early out if no indexmap */
	if ( im == NULL ) {
		return 0;
	}

	/* this code is really broken */
	#if 0
	/* legacy precision fudges for terrain */
	for ( i = 0; i < 3; i++ )
	{
		mins[ i ] = floor( eMins[ i ] + 0.1 );
		maxs[ i ] = floor( eMaxs[ i ] + 0.1 );
		size[ i ] = maxs[ i ] - mins[ i ];
	}

	/* find st (fixme: support more than just z-axis projection) */
	s = floor( point[ 0 ] + 0.1f - mins[ 0 ] ) / size[ 0 ];
	t = floor( maxs[ 1 ] - point[ 1 ] + 0.1f ) / size[ 1 ];
	if ( s < 0.0f ) {
		s = 0.0f;
	}
	else if ( s > 1.0f ) {
		s = 1.0f;
	}
	if ( t < 0.0f ) {
		t = 0.0f;
	}
	else if ( t > 1.0f ) {
		t = 1.0f;
	}

	/* make xy */
	x = ( im->w - 1 ) * s;
	y = ( im->h - 1 ) * t;
	#else
	/* get size */
	for ( i = 0; i < 3; i++ )
	{
		mins[ i ] = eMins[ i ];
		maxs[ i ] = eMaxs[ i ];
		size[ i ] = maxs[ i ] - mins[ i ];
	}

	/* calc st */
	s = ( point[ 0 ] - mins[ 0 ] ) / size[ 0 ];
	t = ( maxs[ 1 ] - point[ 1 ] ) / size[ 1 ];

	/* calc xy */
	x = s * im->w;
	y = t * im->h;
	if ( x < 0 ) {
		x = 0;
	}
	else if ( x > ( im->w - 1 ) ) {
		x = ( im->w - 1 );
	}
	if ( y < 0 ) {
		y = 0;
	}
	else if ( y > ( im->h - 1 ) ) {
		y = ( im->h - 1 );
	}
	#endif

	/* return index */
	return im->pixels[ y * im->w + x ];
}



/*
   GetIndexedShader() - ydnar
   for a given set of indexes and an indexmap, get a shader and set the vertex alpha in-place
   this combines a couple different functions from terrain.c
 */

shaderInfo_t *GetIndexedShader( shaderInfo_t *parent, indexMap_t *im, int numPoints, byte *shaderIndexes ){
	int i;
	byte minShaderIndex, maxShaderIndex;
	char shader[ MAX_QPATH ];
	shaderInfo_t    *si;


	/* early out if bad data */
	if ( im == NULL || numPoints <= 0 || shaderIndexes == NULL ) {
		return ShaderInfoForShader( "default" );
	}

	/* determine min/max index */
	minShaderIndex = 255;
	maxShaderIndex = 0;
	for ( i = 0; i < numPoints; i++ )
	{
		if ( shaderIndexes[ i ] < minShaderIndex ) {
			minShaderIndex = shaderIndexes[ i ];
		}
		if ( shaderIndexes[ i ] > maxShaderIndex ) {
			maxShaderIndex = shaderIndexes[ i ];
		}
	}

	/* set alpha inline */
	for ( i = 0; i < numPoints; i++ )
	{
		/* straight rip from terrain.c */
		if ( shaderIndexes[ i ] < maxShaderIndex ) {
			shaderIndexes[ i ] = 0;
		}
		else{
			shaderIndexes[ i ] = 255;
		}
	}

	/* make a shader name */
	if ( minShaderIndex == maxShaderIndex ) {
		sprintf( shader, "textures/%s_%d", im->shader, maxShaderIndex );
	}
	else{
		sprintf( shader, "textures/%s_%dto%d", im->shader, minShaderIndex, maxShaderIndex );
	}

	/* get the shader */
	si = ShaderInfoForShader( shader );

	/* inherit a few things from parent shader */
	if ( parent->globalTexture ) {
		si->globalTexture = qtrue;
	}
	if ( parent->forceMeta ) {
		si->forceMeta = qtrue;
	}
	if ( parent->nonplanar ) {
		si->nonplanar = qtrue;
	}
	if ( si->shadeAngleDegrees == 0.0 ) {
		si->shadeAngleDegrees = parent->shadeAngleDegrees;
	}
	if ( parent->tcGen && si->tcGen == qfalse ) {
		/* set xy texture projection */
		si->tcGen = qtrue;
		VectorCopy( parent->vecs[ 0 ], si->vecs[ 0 ] );
		VectorCopy( parent->vecs[ 1 ], si->vecs[ 1 ] );
	}
	if ( VectorLength( parent->lightmapAxis ) > 0.0f && VectorLength( si->lightmapAxis ) <= 0.0f ) {
		/* set lightmap projection axis */
		VectorCopy( parent->lightmapAxis, si->lightmapAxis );
	}

	/* return the shader */
	return si;
}




/*
   DrawSurfaceForSide()
   creates a SURF_FACE drawsurface from a given brush side and winding
 */

#define SNAP_FLOAT_TO_INT   8
#define SNAP_INT_TO_FLOAT   ( 1.0 / SNAP_FLOAT_TO_INT )

mapDrawSurface_t *DrawSurfaceForSide( entity_t *e, brush_t *b, side_t *s, winding_t *w ){
	int i, j, k;
	mapDrawSurface_t    *ds;
	shaderInfo_t        *si, *parent;
	bspDrawVert_t       *dv;
	vec3_t texX, texY;
	vec_t x, y;
	vec3_t vTranslated;
	qboolean indexed;
	byte shaderIndexes[ 256 ];
	float offsets[ 256 ];
	char tempShader[ MAX_QPATH ];


	/* ydnar: don't make a drawsurf for culled sides */
	if ( s->culled ) {
		return NULL;
	}

	/* range check */
	if ( w->numpoints > MAX_POINTS_ON_WINDING ) {
		Error( "DrawSurfaceForSide: w->numpoints = %d (> %d)", w->numpoints, MAX_POINTS_ON_WINDING );
	}

	/* get shader */
	si = s->shaderInfo;

	/* ydnar: gs mods: check for indexed shader */
	if ( si->indexed && b->im != NULL ) {
		/* indexed */
		indexed = qtrue;

		/* get shader indexes for each point */
		for ( i = 0; i < w->numpoints; i++ )
		{
			shaderIndexes[ i ] = GetShaderIndexForPoint( b->im, b->eMins, b->eMaxs, w->p[ i ] );
			offsets[ i ] = b->im->offsets[ shaderIndexes[ i ] ];
			//%	Sys_Printf( "%f ", offsets[ i ] );
		}

		/* get matching shader and set alpha */
		parent = si;
		si = GetIndexedShader( parent, b->im, w->numpoints, shaderIndexes );
	}
	else{
		indexed = qfalse;
	}

	/* ydnar: sky hack/fix for GL_CLAMP borders on ati cards */
	if ( skyFixHack && si->skyParmsImageBase[ 0 ] != '\0' ) {
		//%	Sys_FPrintf( SYS_VRB, "Enabling sky hack for shader %s using env %s\n", si->shader, si->skyParmsImageBase );
		sprintf( tempShader, "%s_lf", si->skyParmsImageBase );
		DrawSurfaceForShader( tempShader );
		sprintf( tempShader, "%s_rt", si->skyParmsImageBase );
		DrawSurfaceForShader( tempShader );
		sprintf( tempShader, "%s_ft", si->skyParmsImageBase );
		DrawSurfaceForShader( tempShader );
		sprintf( tempShader, "%s_bk", si->skyParmsImageBase );
		DrawSurfaceForShader( tempShader );
		sprintf( tempShader, "%s_up", si->skyParmsImageBase );
		DrawSurfaceForShader( tempShader );
		sprintf( tempShader, "%s_dn", si->skyParmsImageBase );
		DrawSurfaceForShader( tempShader );
	}

	/* ydnar: gs mods */
	ds = AllocDrawSurface( SURFACE_FACE );
	ds->entityNum = b->entityNum;
	ds->castShadows = b->castShadows;
	ds->recvShadows = b->recvShadows;

	ds->planar = qtrue;
	ds->planeNum = s->planenum;
	VectorCopy( mapplanes[ s->planenum ].normal, ds->lightmapVecs[ 2 ] );

	ds->shaderInfo = si;
	ds->mapBrush = b;
	ds->sideRef = AllocSideRef( s, NULL );
	ds->fogNum = -1;
	ds->lightmapScale = b->lightmapScale;
	ds->numVerts = w->numpoints;
	ds->verts = safe_malloc( ds->numVerts * sizeof( *ds->verts ) );
	memset( ds->verts, 0, ds->numVerts * sizeof( *ds->verts ) );

	/* compute s/t coordinates from brush primitive texture matrix (compute axis base) */
	ComputeAxisBase( mapplanes[ s->planenum ].normal, texX, texY );

	/* create the vertexes */
	for ( j = 0; j < w->numpoints; j++ )
	{
		/* get the drawvert */
		dv = ds->verts + j;

		/* copy xyz and do potential z offset */
		VectorCopy( w->p[ j ], dv->xyz );
		if ( indexed ) {
			dv->xyz[ 2 ] += offsets[ j ];
		}

		/* round the xyz to a given precision and translate by origin */
		for ( i = 0 ; i < 3 ; i++ )
			dv->xyz[ i ] = SNAP_INT_TO_FLOAT * floor( dv->xyz[ i ] * SNAP_FLOAT_TO_INT + 0.5f );
		VectorAdd( dv->xyz, e->origin, vTranslated );

		/* ydnar: tek-fu celshading support for flat shaded shit */
		if ( flat ) {
			dv->st[ 0 ] = si->stFlat[ 0 ];
			dv->st[ 1 ] = si->stFlat[ 1 ];
		}

		/* ydnar: gs mods: added support for explicit shader texcoord generation */
		else if ( si->tcGen ) {
			dv->st[ 0 ] = DotProduct( si->vecs[ 0 ], vTranslated );
			dv->st[ 1 ] = DotProduct( si->vecs[ 1 ], vTranslated );
		}

		/* old quake-style texturing */
		else if ( g_bBrushPrimit == BPRIMIT_OLDBRUSHES ) {
			/* nearest-axial projection */
			dv->st[ 0 ] = s->vecs[ 0 ][ 3 ] + DotProduct( s->vecs[ 0 ], vTranslated );
			dv->st[ 1 ] = s->vecs[ 1 ][ 3 ] + DotProduct( s->vecs[ 1 ], vTranslated );
			dv->st[ 0 ] /= si->shaderWidth;
			dv->st[ 1 ] /= si->shaderHeight;
		}

		/* brush primitive texturing */
		else
		{
			/* calculate texture s/t from brush primitive texture matrix */
			x = DotProduct( vTranslated, texX );
			y = DotProduct( vTranslated, texY );
			dv->st[ 0 ] = s->texMat[ 0 ][ 0 ] * x + s->texMat[ 0 ][ 1 ] * y + s->texMat[ 0 ][ 2 ];
			dv->st[ 1 ] = s->texMat[ 1 ][ 0 ] * x + s->texMat[ 1 ][ 1 ] * y + s->texMat[ 1 ][ 2 ];
		}

		/* copy normal */
		VectorCopy( mapplanes[ s->planenum ].normal, dv->normal );

		/* ydnar: set color */
		for ( k = 0; k < MAX_LIGHTMAPS; k++ )
		{
			dv->color[ k ][ 0 ] = 255;
			dv->color[ k ][ 1 ] = 255;
			dv->color[ k ][ 2 ] = 255;

			/* ydnar: gs mods: handle indexed shader blending */
			dv->color[ k ][ 3 ] = ( indexed ? shaderIndexes[ j ] : 255 );
		}
	}

	/* set cel shader */
	ds->celShader = b->celShader;

	/* ydnar: gs mods: moved st biasing elsewhere */
	return ds;
}



/*
   DrawSurfaceForMesh()
   moved here from patch.c
 */

#define YDNAR_NORMAL_EPSILON 0.50f

qboolean VectorCompareExt( vec3_t n1, vec3_t n2, float epsilon ){
	int i;


	/* test */
	for ( i = 0; i < 3; i++ )
		if ( fabs( n1[ i ] - n2[ i ] ) > epsilon ) {
			return qfalse;
		}
	return qtrue;
}

mapDrawSurface_t *DrawSurfaceForMesh( entity_t *e, parseMesh_t *p, mesh_t *mesh ){
	int i, k, numVerts;
	vec4_t plane;
	qboolean planar;
	float dist;
	mapDrawSurface_t    *ds;
	shaderInfo_t        *si, *parent;
	bspDrawVert_t       *dv;
	vec3_t vTranslated;
	mesh_t              *copy;
	qboolean indexed;
	byte shaderIndexes[ MAX_EXPANDED_AXIS * MAX_EXPANDED_AXIS ];
	float offsets[ MAX_EXPANDED_AXIS * MAX_EXPANDED_AXIS ];


	/* get mesh and shader shader */
	if ( mesh == NULL ) {
		mesh = &p->mesh;
	}
	si = p->shaderInfo;
	if ( mesh == NULL || si == NULL ) {
		return NULL;
	}

	/* get vertex count */
	numVerts = mesh->width * mesh->height;

	/* to make valid normals for patches with degenerate edges,
	   we need to make a copy of the mesh and put the aproximating
	   points onto the curve */

	/* create a copy of the mesh */
	copy = CopyMesh( mesh );

	/* store off the original (potentially bad) normals */
	MakeMeshNormals( *copy );
	for ( i = 0; i < numVerts; i++ )
		VectorCopy( copy->verts[ i ].normal, mesh->verts[ i ].normal );

	/* put the mesh on the curve */
	PutMeshOnCurve( *copy );

	/* find new normals (to take into account degenerate/flipped edges */
	MakeMeshNormals( *copy );
	for ( i = 0; i < numVerts; i++ )
	{
		/* ydnar: only copy normals that are significantly different from the originals */
		if ( DotProduct( copy->verts[ i ].normal, mesh->verts[ i ].normal ) < 0.75f ) {
			VectorCopy( copy->verts[ i ].normal, mesh->verts[ i ].normal );
		}
	}

	/* free the old mesh */
	FreeMesh( copy );

	/* ydnar: gs mods: check for indexed shader */
	if ( si->indexed && p->im != NULL ) {
		/* indexed */
		indexed = qtrue;

		/* get shader indexes for each point */
		for ( i = 0; i < numVerts; i++ )
		{
			shaderIndexes[ i ] = GetShaderIndexForPoint( p->im, p->eMins, p->eMaxs, mesh->verts[ i ].xyz );
			offsets[ i ] = p->im->offsets[ shaderIndexes[ i ] ];
		}

		/* get matching shader and set alpha */
		parent = si;
		si = GetIndexedShader( parent, p->im, numVerts, shaderIndexes );
	}
	else{
		indexed = qfalse;
	}


	/* ydnar: gs mods */
	ds = AllocDrawSurface( SURFACE_PATCH );
	ds->entityNum = p->entityNum;
	ds->castShadows = p->castShadows;
	ds->recvShadows = p->recvShadows;

	ds->shaderInfo = si;
	ds->mapMesh = p;
	ds->lightmapScale = p->lightmapScale;   /* ydnar */
	ds->patchWidth = mesh->width;
	ds->patchHeight = mesh->height;
	ds->numVerts = ds->patchWidth * ds->patchHeight;
	ds->verts = safe_malloc( ds->numVerts * sizeof( *ds->verts ) );
	memcpy( ds->verts, mesh->verts, ds->numVerts * sizeof( *ds->verts ) );

	ds->fogNum = -1;
	ds->planeNum = -1;

	ds->longestCurve = p->longestCurve;
	ds->maxIterations = p->maxIterations;

	/* construct a plane from the first vert */
	VectorCopy( mesh->verts[ 0 ].normal, plane );
	plane[ 3 ] = DotProduct( mesh->verts[ 0 ].xyz, plane );
	planar = qtrue;

	/* spew forth errors */
	if ( VectorLength( plane ) < 0.001f ) {
		Sys_Printf( "DrawSurfaceForMesh: bogus plane\n" );
	}

	/* test each vert */
	for ( i = 1; i < ds->numVerts && planar; i++ )
	{
		/* normal test */
		if ( VectorCompare( plane, mesh->verts[ i ].normal ) == qfalse ) {
			planar = qfalse;
		}

		/* point-plane test */
		dist = DotProduct( mesh->verts[ i ].xyz, plane ) - plane[ 3 ];
		if ( fabs( dist ) > EQUAL_EPSILON ) {
			planar = qfalse;
		}
	}

	/* add a map plane */
	if ( planar ) {
		/* make a map plane */
		ds->planeNum = FindFloatPlane( plane, plane[ 3 ], 1, &mesh->verts[ 0 ].xyz );
		VectorCopy( plane, ds->lightmapVecs[ 2 ] );

		/* push this normal to all verts (ydnar 2003-02-14: bad idea, small patches get screwed up) */
		for ( i = 0; i < ds->numVerts; i++ )
			VectorCopy( plane, ds->verts[ i ].normal );
	}

	/* walk the verts to do special stuff */
	for ( i = 0; i < ds->numVerts; i++ )
	{
		/* get the drawvert */
		dv = &ds->verts[ i ];

		/* ydnar: tek-fu celshading support for flat shaded shit */
		if ( flat ) {
			dv->st[ 0 ] = si->stFlat[ 0 ];
			dv->st[ 1 ] = si->stFlat[ 1 ];
		}

		/* ydnar: gs mods: added support for explicit shader texcoord generation */
		else if ( si->tcGen ) {
			/* translate by origin and project the texture */
			VectorAdd( dv->xyz, e->origin, vTranslated );
			dv->st[ 0 ] = DotProduct( si->vecs[ 0 ], vTranslated );
			dv->st[ 1 ] = DotProduct( si->vecs[ 1 ], vTranslated );
		}

		/* ydnar: set color */
		for ( k = 0; k < MAX_LIGHTMAPS; k++ )
		{
			dv->color[ k ][ 0 ] = 255;
			dv->color[ k ][ 1 ] = 255;
			dv->color[ k ][ 2 ] = 255;

			/* ydnar: gs mods: handle indexed shader blending */
			dv->color[ k ][ 3 ] = ( indexed ? shaderIndexes[ i ] : 255 );
		}

		/* ydnar: offset */
		if ( indexed ) {
			dv->xyz[ 2 ] += offsets[ i ];
		}
	}

	/* set cel shader */
	ds->celShader = p->celShader;

	/* return the drawsurface */
	return ds;
}



/*
   DrawSurfaceForFlare() - ydnar
   creates a flare draw surface
 */

mapDrawSurface_t *DrawSurfaceForFlare( int entNum, vec3_t origin, vec3_t normal, vec3_t color, char *flareShader, int lightStyle ){
	mapDrawSurface_t    *ds;


	/* emit flares? */
	if ( emitFlares == qfalse ) {
		return NULL;
	}

	/* allocate drawsurface */
	ds = AllocDrawSurface( SURFACE_FLARE );
	ds->entityNum = entNum;

	/* set it up */
	if ( flareShader != NULL && flareShader[ 0 ] != '\0' ) {
		ds->shaderInfo = ShaderInfoForShader( flareShader );
	}
	else{
		ds->shaderInfo = ShaderInfoForShader( game->flareShader );
	}
	if ( origin != NULL ) {
		VectorCopy( origin, ds->lightmapOrigin );
	}
	if ( normal != NULL ) {
		VectorCopy( normal, ds->lightmapVecs[ 2 ] );
	}
	if ( color != NULL ) {
		VectorCopy( color, ds->lightmapVecs[ 0 ] );
	}

	/* store light style */
	ds->lightStyle = lightStyle;
	if ( ds->lightStyle < 0 || ds->lightStyle >= LS_NONE ) {
		ds->lightStyle = LS_NORMAL;
	}

	/* fixme: fog */

	/* return to sender */
	return ds;
}



/*
   DrawSurfaceForShader() - ydnar
   creates a bogus surface to forcing the game to load a shader
 */

mapDrawSurface_t *DrawSurfaceForShader( char *shader ){
	int i;
	shaderInfo_t        *si;
	mapDrawSurface_t    *ds;


	/* get shader */
	si = ShaderInfoForShader( shader );

	/* find existing surface */
	for ( i = 0; i < numMapDrawSurfs; i++ )
	{
		/* get surface */
		ds = &mapDrawSurfs[ i ];

		/* check it */
		if ( ds->shaderInfo == si ) {
			return ds;
		}
	}

	/* create a new surface */
	ds = AllocDrawSurface( SURFACE_SHADER );
	ds->entityNum = 0;
	ds->shaderInfo = ShaderInfoForShader( shader );

	/* return to sender */
	return ds;
}



/*
   AddSurfaceFlare() - ydnar
   creates flares (coronas) centered on surfaces
 */

static void AddSurfaceFlare( mapDrawSurface_t *ds, vec3_t entityOrigin ){
	vec3_t origin;
	int i;


	/* find centroid */
	VectorClear( origin );
	for ( i = 0; i < ds->numVerts; i++ )
		VectorAdd( origin, ds->verts[ i ].xyz, origin );
	VectorScale( origin, ( 1.0f / ds->numVerts ), origin );
	if ( entityOrigin != NULL ) {
		VectorAdd( origin, entityOrigin, origin );
	}

	/* push origin off surface a bit */
	VectorMA( origin, 2.0f,  ds->lightmapVecs[ 2 ], origin );

	/* create the drawsurface */
	DrawSurfaceForFlare( ds->entityNum, origin, ds->lightmapVecs[ 2 ], ds->shaderInfo->color, ds->shaderInfo->flareShader, ds->shaderInfo->lightStyle );
}



/*
   SubdivideFace()
   subdivides a face surface until it is smaller than the specified size (subdivisions)
 */

static void SubdivideFace_r( entity_t *e, brush_t *brush, side_t *side, winding_t *w, int fogNum, float subdivisions ){
	int i;
	int axis;
	vec3_t bounds[ 2 ];
	const float epsilon = 0.1;
	int subFloor, subCeil;
	winding_t           *frontWinding, *backWinding;
	mapDrawSurface_t    *ds;


	/* dummy check */
	if ( w == NULL ) {
		return;
	}
	if ( w->numpoints < 3 ) {
		Error( "SubdivideFace_r: Bad w->numpoints (%d < 3)", w->numpoints );
	}

	/* determine surface bounds */
	ClearBounds( bounds[ 0 ], bounds[ 1 ] );
	for ( i = 0; i < w->numpoints; i++ )
		AddPointToBounds( w->p[ i ], bounds[ 0 ], bounds[ 1 ] );

	/* split the face */
	for ( axis = 0; axis < 3; axis++ )
	{
		vec3_t planePoint = { 0, 0, 0 };
		vec3_t planeNormal = { 0, 0, 0 };
		float d;


		/* create an axial clipping plane */
		subFloor = floor( bounds[ 0 ][ axis ] / subdivisions ) * subdivisions;
		subCeil = ceil( bounds[ 1 ][ axis ] / subdivisions ) * subdivisions;
		planePoint[ axis ] = subFloor + subdivisions;
		planeNormal[ axis ] = -1;
		d = DotProduct( planePoint, planeNormal );

		/* subdivide if necessary */
		if ( ( subCeil - subFloor ) > subdivisions ) {
			/* clip the winding */
			ClipWindingEpsilon( w, planeNormal, d, epsilon, &frontWinding, &backWinding );

			/* the clip may not produce two polygons if it was epsilon close */
			if ( frontWinding == NULL ) {
				w = backWinding;
			}
			else if ( backWinding == NULL ) {
				w = frontWinding;
			}
			else
			{
				SubdivideFace_r( e, brush, side, frontWinding, fogNum, subdivisions );
				SubdivideFace_r( e, brush, side, backWinding, fogNum, subdivisions );
				return;
			}
		}
	}

	/* create a face surface */
	ds = DrawSurfaceForSide( e, brush, side, w );

	/* set correct fog num */
	ds->fogNum = fogNum;
}



/*
   SubdivideFaceSurfaces()
   chop up brush face surfaces that have subdivision attributes
   ydnar: and subdivide surfaces that exceed specified texture coordinate range
 */

void SubdivideFaceSurfaces( entity_t *e, tree_t *tree ){
	int i, j, numBaseDrawSurfs, fogNum;
	mapDrawSurface_t    *ds;
	brush_t             *brush;
	side_t              *side;
	shaderInfo_t        *si;
	winding_t           *w;
	float range, size, subdivisions, s2;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- SubdivideFaceSurfaces ---\n" );

	/* walk the list of surfaces */
	numBaseDrawSurfs = numMapDrawSurfs;
	for ( i = e->firstDrawSurf; i < numBaseDrawSurfs; i++ )
	{
		/* get surface */
		ds = &mapDrawSurfs[ i ];

		/* only subdivide brush sides */
		if ( ds->type != SURFACE_FACE || ds->mapBrush == NULL || ds->sideRef == NULL || ds->sideRef->side == NULL ) {
			continue;
		}

		/* get bits */
		brush = ds->mapBrush;
		side = ds->sideRef->side;

		/* check subdivision for shader */
		si = side->shaderInfo;
		if ( si == NULL ) {
			continue;
		}

		/* ydnar: don't subdivide sky surfaces */
		if ( si->compileFlags & C_SKY ) {
			continue;
		}

		/* do texture coordinate range check */
		ClassifySurfaces( 1, ds );
		if ( CalcSurfaceTextureRange( ds ) == qfalse ) {
			/* calculate subdivisions texture range (this code is shit) */
			range = ( ds->texRange[ 0 ] > ds->texRange[ 1 ] ? ds->texRange[ 0 ] : ds->texRange[ 1 ] );
			size = ds->maxs[ 0 ] - ds->mins[ 0 ];
			for ( j = 1; j < 3; j++ )
				if ( ( ds->maxs[ j ] - ds->mins[ j ] ) > size ) {
					size = ds->maxs[ j ] - ds->mins[ j ];
				}
			subdivisions = ( size / range ) * texRange;
			subdivisions = ceil( subdivisions / 2 ) * 2;
			for ( j = 1; j < 8; j++ )
			{
				s2 = ceil( (float) texRange / j );
				if ( fabs( subdivisions - s2 ) <= 4.0 ) {
					subdivisions = s2;
					break;
				}
			}
		}
		else{
			subdivisions = si->subdivisions;
		}

		/* get subdivisions from shader */
		if ( si->subdivisions > 0 && si->subdivisions < subdivisions ) {
			subdivisions = si->subdivisions;
		}
		if ( subdivisions < 1.0f ) {
			continue;
		}

		/* preserve fog num */
		fogNum = ds->fogNum;

		/* make a winding and free the surface */
		w = WindingFromDrawSurf( ds );
		ClearSurface( ds );

		/* subdivide it */
		SubdivideFace_r( e, brush, side, w, fogNum, subdivisions );
	}
}



/*
   ====================
   ClipSideIntoTree_r

   Adds non-opaque leaf fragments to the convex hull
   ====================
 */

void ClipSideIntoTree_r( winding_t *w, side_t *side, node_t *node ){
	plane_t         *plane;
	winding_t       *front, *back;

	if ( !w ) {
		return;
	}

	if ( node->planenum != PLANENUM_LEAF ) {
		if ( side->planenum == node->planenum ) {
			ClipSideIntoTree_r( w, side, node->children[0] );
			return;
		}
		if ( side->planenum == ( node->planenum ^ 1 ) ) {
			ClipSideIntoTree_r( w, side, node->children[1] );
			return;
		}

		plane = &mapplanes[ node->planenum ];
		ClipWindingEpsilon( w, plane->normal, plane->dist,
							ON_EPSILON, &front, &back );
		FreeWinding( w );

		ClipSideIntoTree_r( front, side, node->children[0] );
		ClipSideIntoTree_r( back, side, node->children[1] );

		return;
	}

	// if opaque leaf, don't add
	if ( !node->opaque ) {
		AddWindingToConvexHull( w, &side->visibleHull, mapplanes[ side->planenum ].normal );
	}

	FreeWinding( w );
	return;
}





static int g_numHiddenFaces, g_numCoinFaces;



/*
   CullVectorCompare() - ydnar
   compares two vectors with an epsilon
 */

#define CULL_EPSILON 0.1f

qboolean CullVectorCompare( const vec3_t v1, const vec3_t v2 ){
	int i;


	for ( i = 0; i < 3; i++ )
		if ( fabs( v1[ i ] - v2[ i ] ) > CULL_EPSILON ) {
			return qfalse;
		}
	return qtrue;
}



/*
   SideInBrush() - ydnar
   determines if a brushside lies inside another brush
 */

qboolean SideInBrush( side_t *side, brush_t *b ){
	int i, s;
	plane_t     *plane;


	/* ignore sides w/o windings or shaders */
	if ( side->winding == NULL || side->shaderInfo == NULL ) {
		return qtrue;
	}

	/* ignore culled sides and translucent brushes */
	if ( side->culled == qtrue || ( b->compileFlags & C_TRANSLUCENT ) ) {
		return qfalse;
	}

	/* side iterator */
	for ( i = 0; i < b->numsides; i++ )
	{
		/* fail if any sides are caulk */
		if ( b->sides[ i ].compileFlags & C_NODRAW ) {
			return qfalse;
		}

		/* check if side's winding is on or behind the plane */
		plane = &mapplanes[ b->sides[ i ].planenum ];
		s = WindingOnPlaneSide( side->winding, plane->normal, plane->dist );
		if ( s == SIDE_FRONT || s == SIDE_CROSS ) {
			return qfalse;
		}
	}

	/* don't cull autosprite or polygonoffset surfaces */
	if ( side->shaderInfo ) {
		if ( side->shaderInfo->autosprite || side->shaderInfo->polygonOffset ) {
			return qfalse;
		}
	}

	/* inside */
	side->culled = qtrue;
	g_numHiddenFaces++;
	return qtrue;
}


/*
   CullSides() - ydnar
   culls obscured or buried brushsides from the map
 */

void CullSides( entity_t *e ){
	int numPoints;
	int i, j, k, l, first, second, dir;
	winding_t   *w1, *w2;
	brush_t *b1, *b2;
	side_t      *side1, *side2;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- CullSides ---\n" );

	g_numHiddenFaces = 0;
	g_numCoinFaces = 0;

	/* brush interator 1 */
	for ( b1 = e->brushes; b1; b1 = b1->next )
	{
		/* sides check */
		if ( b1->numsides < 1 ) {
			continue;
		}

		/* brush iterator 2 */
		for ( b2 = b1->next; b2; b2 = b2->next )
		{
			/* sides check */
			if ( b2->numsides < 1 ) {
				continue;
			}

			/* original check */
			if ( b1->original == b2->original && b1->original != NULL ) {
				continue;
			}

			/* bbox check */
			j = 0;
			for ( i = 0; i < 3; i++ )
				if ( b1->mins[ i ] > b2->maxs[ i ] || b1->maxs[ i ] < b2->mins[ i ] ) {
					j++;
				}
			if ( j ) {
				continue;
			}

			/* cull inside sides */
			for ( i = 0; i < b1->numsides; i++ )
				SideInBrush( &b1->sides[ i ], b2 );
			for ( i = 0; i < b2->numsides; i++ )
				SideInBrush( &b2->sides[ i ], b1 );

			/* side iterator 1 */
			for ( i = 0; i < b1->numsides; i++ )
			{
				/* winding check */
				side1 = &b1->sides[ i ];
				w1 = side1->winding;
				if ( w1 == NULL ) {
					continue;
				}
				numPoints = w1->numpoints;
				if ( side1->shaderInfo == NULL ) {
					continue;
				}

				/* side iterator 2 */
				for ( j = 0; j < b2->numsides; j++ )
				{
					/* winding check */
					side2 = &b2->sides[ j ];
					w2 = side2->winding;
					if ( w2 == NULL ) {
						continue;
					}
					if ( side2->shaderInfo == NULL ) {
						continue;
					}
					if ( w1->numpoints != w2->numpoints ) {
						continue;
					}
					if ( side1->culled == qtrue && side2->culled == qtrue ) {
						continue;
					}

					/* compare planes */
					if ( ( side1->planenum & ~0x00000001 ) != ( side2->planenum & ~0x00000001 ) ) {
						continue;
					}

					/* get autosprite and polygonoffset status */
					if ( side1->shaderInfo &&
						 ( side1->shaderInfo->autosprite || side1->shaderInfo->polygonOffset ) ) {
						continue;
					}
					if ( side2->shaderInfo &&
						 ( side2->shaderInfo->autosprite || side2->shaderInfo->polygonOffset ) ) {
						continue;
					}

					/* find first common point */
					first = -1;
					for ( k = 0; k < numPoints; k++ )
					{
						if ( VectorCompare( w1->p[ 0 ], w2->p[ k ] ) ) {
							first = k;
							k = numPoints;
						}
					}
					if ( first == -1 ) {
						continue;
					}

					/* find second common point (regardless of winding order) */
					second = -1;
					dir = 0;
					if ( ( first + 1 ) < numPoints ) {
						second = first + 1;
					}
					else{
						second = 0;
					}
					if ( CullVectorCompare( w1->p[ 1 ], w2->p[ second ] ) ) {
						dir = 1;
					}
					else
					{
						if ( first > 0 ) {
							second = first - 1;
						}
						else{
							second = numPoints - 1;
						}
						if ( CullVectorCompare( w1->p[ 1 ], w2->p[ second ] ) ) {
							dir = -1;
						}
					}
					if ( dir == 0 ) {
						continue;
					}

					/* compare the rest of the points */
					l = first;
					for ( k = 0; k < numPoints; k++ )
					{
						if ( !CullVectorCompare( w1->p[ k ], w2->p[ l ] ) ) {
							k = 100000;
						}

						l += dir;
						if ( l < 0 ) {
							l = numPoints - 1;
						}
						else if ( l >= numPoints ) {
							l = 0;
						}
					}
					if ( k >= 100000 ) {
						continue;
					}

					/* cull face 1 */
					if ( !side2->culled && !( side2->compileFlags & C_TRANSLUCENT ) && !( side2->compileFlags & C_NODRAW ) ) {
						side1->culled = qtrue;
						g_numCoinFaces++;
					}

					if ( side1->planenum == side2->planenum && side1->culled == qtrue ) {
						continue;
					}

					/* cull face 2 */
					if ( !side1->culled && !( side1->compileFlags & C_TRANSLUCENT ) && !( side1->compileFlags & C_NODRAW ) ) {
						side2->culled = qtrue;
						g_numCoinFaces++;
					}
				}
			}
		}
	}

	/* emit some stats */
	Sys_FPrintf( SYS_VRB, "%9d hidden faces culled\n", g_numHiddenFaces );
	Sys_FPrintf( SYS_VRB, "%9d coincident faces culled\n", g_numCoinFaces );
}




/*
   ClipSidesIntoTree()

   creates side->visibleHull for all visible sides

   the drawsurf for a side will consist of the convex hull of
   all points in non-opaque clusters, which allows overlaps
   to be trimmed off automatically.
 */

void ClipSidesIntoTree( entity_t *e, tree_t *tree ){
	brush_t     *b;
	int i;
	winding_t       *w;
	side_t          *side, *newSide;
	shaderInfo_t    *si;


	/* ydnar: cull brush sides */
	CullSides( e );

	/* note it */
	Sys_FPrintf( SYS_VRB, "--- ClipSidesIntoTree ---\n" );

	/* walk the brush list */
	for ( b = e->brushes; b; b = b->next )
	{
		/* walk the brush sides */
		for ( i = 0; i < b->numsides; i++ )
		{
			/* get side */
			side = &b->sides[ i ];
			if ( side->winding == NULL ) {
				continue;
			}

			/* copy the winding */
			w = CopyWinding( side->winding );
			side->visibleHull = NULL;
			ClipSideIntoTree_r( w, side, tree->headnode );

			/* anything left? */
			w = side->visibleHull;
			if ( w == NULL ) {
				continue;
			}

			/* shader? */
			si = side->shaderInfo;
			if ( si == NULL ) {
				continue;
			}

			/* don't create faces for non-visible sides */
			/* ydnar: except indexed shaders, like common/terrain and nodraw fog surfaces */
			if ( ( si->compileFlags & C_NODRAW ) && si->indexed == qfalse && !( si->compileFlags & C_FOG ) ) {
				continue;
			}

			/* always use the original winding for autosprites and noclip faces */
			if ( si->autosprite || si->noClip ) {
				w = side->winding;
			}

			/* save this winding as a visible surface */
			DrawSurfaceForSide( e, b, side, w );

			/* make a back side for fog */
			if ( !( si->compileFlags & C_FOG ) ) {
				continue;
			}

			/* duplicate the up-facing side */
			w = ReverseWinding( w );
			newSide = safe_malloc( sizeof( *side ) );
			*newSide = *side;
			newSide->visibleHull = w;
			newSide->planenum ^= 1;

			/* save this winding as a visible surface */
			DrawSurfaceForSide( e, b, newSide, w );
		}
	}
}



/*

   this section deals with filtering drawsurfaces into the bsp tree,
   adding references to each leaf a surface touches

 */

/*
   AddReferenceToLeaf() - ydnar
   adds a reference to surface ds in the bsp leaf node
 */

int AddReferenceToLeaf( mapDrawSurface_t *ds, node_t *node ){
	drawSurfRef_t   *dsr;


	/* dummy check */
	if ( node->planenum != PLANENUM_LEAF || node->opaque ) {
		return 0;
	}

	/* try to find an existing reference */
	for ( dsr = node->drawSurfReferences; dsr; dsr = dsr->nextRef )
	{
		if ( dsr->outputNum == numBSPDrawSurfaces ) {
			return 0;
		}
	}

	/* add a new reference */
	dsr = safe_malloc( sizeof( *dsr ) );
	dsr->outputNum = numBSPDrawSurfaces;
	dsr->nextRef = node->drawSurfReferences;
	node->drawSurfReferences = dsr;

	/* ydnar: sky/skybox surfaces */
	if ( node->skybox ) {
		ds->skybox = qtrue;
	}
	if ( ds->shaderInfo->compileFlags & C_SKY ) {
		node->sky = qtrue;
	}

	/* return */
	return 1;
}



/*
   AddReferenceToTree_r() - ydnar
   adds a reference to the specified drawsurface to every leaf in the tree
 */

int AddReferenceToTree_r( mapDrawSurface_t *ds, node_t *node, qboolean skybox ){
	int i, refs = 0;


	/* dummy check */
	if ( node == NULL ) {
		return 0;
	}

	/* is this a decision node? */
	if ( node->planenum != PLANENUM_LEAF ) {
		/* add to child nodes and return */
		refs += AddReferenceToTree_r( ds, node->children[ 0 ], skybox );
		refs += AddReferenceToTree_r( ds, node->children[ 1 ], skybox );
		return refs;
	}

	/* ydnar */
	if ( skybox ) {
		/* skybox surfaces only get added to sky leaves */
		if ( !node->sky ) {
			return 0;
		}

		/* increase the leaf bounds */
		for ( i = 0; i < ds->numVerts; i++ )
			AddPointToBounds( ds->verts[ i ].xyz, node->mins, node->maxs );
	}

	/* add a reference */
	return AddReferenceToLeaf( ds, node );
}



/*
   FilterPointIntoTree_r() - ydnar
   filters a single point from a surface into the tree
 */

int FilterPointIntoTree_r( vec3_t point, mapDrawSurface_t *ds, node_t *node ){
	float d;
	plane_t         *plane;
	int refs = 0;


	/* is this a decision node? */
	if ( node->planenum != PLANENUM_LEAF ) {
		/* classify the point in relation to the plane */
		plane = &mapplanes[ node->planenum ];
		d = DotProduct( point, plane->normal ) - plane->dist;

		/* filter by this plane */
		refs = 0;
		if ( d >= -ON_EPSILON ) {
			refs += FilterPointIntoTree_r( point, ds, node->children[ 0 ] );
		}
		if ( d <= ON_EPSILON ) {
			refs += FilterPointIntoTree_r( point, ds, node->children[ 1 ] );
		}

		/* return */
		return refs;
	}

	/* add a reference */
	return AddReferenceToLeaf( ds, node );
}



/*
   FilterWindingIntoTree_r() - ydnar
   filters a winding from a drawsurface into the tree
 */

int FilterWindingIntoTree_r( winding_t *w, mapDrawSurface_t *ds, node_t *node ){
	int i, refs = 0;
	plane_t         *p1, *p2;
	vec4_t plane1, plane2, reverse;
	winding_t       *fat, *front, *back;
	shaderInfo_t    *si;


	/* get shaderinfo */
	si = ds->shaderInfo;

	/* ydnar: is this the head node? */
	if ( node->parent == NULL && si != NULL &&
		 ( si->mins[ 0 ] != 0.0f || si->maxs[ 0 ] != 0.0f ||
		   si->mins[ 1 ] != 0.0f || si->maxs[ 1 ] != 0.0f ||
		   si->mins[ 2 ] != 0.0f || si->maxs[ 2 ] != 0.0f ) ) {
		/* 'fatten' the winding by the shader mins/maxs (parsed from vertexDeform move) */
		/* note this winding is completely invalid (concave, nonplanar, etc) */
		fat = AllocWinding( w->numpoints * 3 );
		fat->numpoints = w->numpoints * 3;
		for ( i = 0; i < w->numpoints; i++ )
		{
			VectorCopy( w->p[ i ], fat->p[ i ] );
			VectorAdd( w->p[ i ], si->mins, fat->p[ i * 2 ] );
			VectorAdd( w->p[ i ], si->maxs, fat->p[ i * 3 ] );
		}

		FreeWinding( w );
		w = fat;
	}

	/* is this a decision node? */
	if ( node->planenum != PLANENUM_LEAF ) {
		/* get node plane */
		p1 = &mapplanes[ node->planenum ];
		VectorCopy( p1->normal, plane1 );
		plane1[ 3 ] = p1->dist;

		/* check if surface is planar */
		if ( ds->planeNum >= 0 ) {
			/* get surface plane */
			p2 = &mapplanes[ ds->planeNum ];
			VectorCopy( p2->normal, plane2 );
			plane2[ 3 ] = p2->dist;

			#if 1
			/* invert surface plane */
			VectorSubtract( vec3_origin, plane2, reverse );
			reverse[ 3 ] = -plane2[ 3 ];

			/* compare planes */
			if ( DotProduct( plane1, plane2 ) > 0.999f && fabs( plane1[ 3 ] - plane2[ 3 ] ) < 0.001f ) {
				return FilterWindingIntoTree_r( w, ds, node->children[ 0 ] );
			}
			if ( DotProduct( plane1, reverse ) > 0.999f && fabs( plane1[ 3 ] - reverse[ 3 ] ) < 0.001f ) {
				return FilterWindingIntoTree_r( w, ds, node->children[ 1 ] );
			}
			#else
			/* the drawsurf might have an associated plane, if so, force a filter here */
			if ( ds->planeNum == node->planenum ) {
				return FilterWindingIntoTree_r( w, ds, node->children[ 0 ] );
			}
			if ( ds->planeNum == ( node->planenum ^ 1 ) ) {
				return FilterWindingIntoTree_r( w, ds, node->children[ 1 ] );
			}
			#endif
		}

		/* clip the winding by this plane */
		ClipWindingEpsilon( w, plane1, plane1[ 3 ], ON_EPSILON, &front, &back );

		/* filter by this plane */
		refs = 0;
		if ( front != NULL ) {
			refs += FilterWindingIntoTree_r( front, ds, node->children[ 0 ] );
		}
		if ( back != NULL ) {
			refs += FilterWindingIntoTree_r( back, ds, node->children[ 1 ] );
		}
		FreeWinding( w );

		/* return */
		return refs;
	}

	/* add a reference */
	return AddReferenceToLeaf( ds, node );
}



/*
   FilterFaceIntoTree()
   filters a planar winding face drawsurface into the bsp tree
 */

int FilterFaceIntoTree( mapDrawSurface_t *ds, tree_t *tree ){
	winding_t   *w;
	int refs = 0;


	/* make a winding and filter it into the tree */
	w = WindingFromDrawSurf( ds );
	refs = FilterWindingIntoTree_r( w, ds, tree->headnode );

	/* return */
	return refs;
}



/*
   FilterPatchIntoTree()
   subdivides a patch into an approximate curve and filters it into the tree
 */

#define FILTER_SUBDIVISION      8

static int FilterPatchIntoTree( mapDrawSurface_t *ds, tree_t *tree ){
	int i, x, y, refs;
	mesh_t src, *mesh;
	winding_t           *w;


	/* subdivide the surface */
	src.width = ds->patchWidth;
	src.height = ds->patchHeight;
	src.verts = ds->verts;
	mesh = SubdivideMesh( src, FILTER_SUBDIVISION, 32 );


	/* filter each quad into the tree (fixme: use new patch x-triangulation code?) */
	refs = 0;
	for ( y = 0; y < ( mesh->height - 1 ); y++ )
	{
		for ( x = 0; x < ( mesh->width - 1 ); x++ )
		{
			/* triangle 1 */
			w = AllocWinding( 3 );
			w->numpoints = 3;
			VectorCopy( mesh->verts[ y * mesh->width + x ].xyz, w->p[ 0 ] );
			VectorCopy( mesh->verts[ y * mesh->width + x + 1 ].xyz, w->p[ 1 ] );
			VectorCopy( mesh->verts[ ( y + 1 ) * mesh->width + x ].xyz, w->p[ 2 ] );
			refs += FilterWindingIntoTree_r( w, ds, tree->headnode );

			/* triangle 2 */
			w = AllocWinding( 3 );
			w->numpoints = 3;
			VectorCopy( mesh->verts[ y * mesh->width + x + 1 ].xyz, w->p[ 0 ] );
			VectorCopy( mesh->verts[ ( y + 1 ) * mesh->width + x + 1 ].xyz, w->p[ 1 ] );
			VectorCopy( mesh->verts[ ( y + 1 ) * mesh->width + x ].xyz, w->p[ 2 ] );
			refs += FilterWindingIntoTree_r( w, ds, tree->headnode );
		}
	}

	/* use point filtering as well */
	for ( i = 0; i < ( mesh->width * mesh->height ); i++ )
		refs += FilterPointIntoTree_r( mesh->verts[ i ].xyz, ds, tree->headnode );

	/* free the subdivided mesh and return */
	FreeMesh( mesh );
	return refs;
}



/*
   FilterTrianglesIntoTree()
   filters a triangle surface (meta, model) into the bsp
 */

static int FilterTrianglesIntoTree( mapDrawSurface_t *ds, tree_t *tree ){
	int i, refs;
	winding_t   *w;


	/* ydnar: gs mods: this was creating bogus triangles before */
	refs = 0;
	for ( i = 0; i < ds->numIndexes; i += 3 )
	{
		/* error check */
		if ( ds->indexes[ i ] >= ds->numVerts ||
			 ds->indexes[ i + 1 ] >= ds->numVerts ||
			 ds->indexes[ i + 2 ] >= ds->numVerts ) {
			Error( "Index %d greater than vertex count %d", ds->indexes[ i ], ds->numVerts );
		}

		/* make a triangle winding and filter it into the tree */
		w = AllocWinding( 3 );
		w->numpoints = 3;
		VectorCopy( ds->verts[ ds->indexes[ i ] ].xyz, w->p[ 0 ] );
		VectorCopy( ds->verts[ ds->indexes[ i + 1 ] ].xyz, w->p[ 1 ] );
		VectorCopy( ds->verts[ ds->indexes[ i + 2 ] ].xyz, w->p[ 2 ] );
		refs += FilterWindingIntoTree_r( w, ds, tree->headnode );
	}

	/* use point filtering as well */
	for ( i = 0; i < ds->numVerts; i++ )
		refs += FilterPointIntoTree_r( ds->verts[ i ].xyz, ds, tree->headnode );

	return refs;
}



/*
   FilterFoliageIntoTree()
   filters a foliage surface (wolf et/splash damage)
 */

static int FilterFoliageIntoTree( mapDrawSurface_t *ds, tree_t *tree ){
	int f, i, refs;
	bspDrawVert_t   *instance;
	vec3_t xyz;
	winding_t       *w;


	/* walk origin list */
	refs = 0;
	for ( f = 0; f < ds->numFoliageInstances; f++ )
	{
		/* get instance */
		instance = ds->verts + ds->patchHeight + f;

		/* walk triangle list */
		for ( i = 0; i < ds->numIndexes; i += 3 )
		{
			/* error check */
			if ( ds->indexes[ i ] >= ds->numVerts ||
				 ds->indexes[ i + 1 ] >= ds->numVerts ||
				 ds->indexes[ i + 2 ] >= ds->numVerts ) {
				Error( "Index %d greater than vertex count %d", ds->indexes[ i ], ds->numVerts );
			}

			/* make a triangle winding and filter it into the tree */
			w = AllocWinding( 3 );
			w->numpoints = 3;
			VectorAdd( instance->xyz, ds->verts[ ds->indexes[ i ] ].xyz, w->p[ 0 ] );
			VectorAdd( instance->xyz, ds->verts[ ds->indexes[ i + 1 ] ].xyz, w->p[ 1 ] );
			VectorAdd( instance->xyz, ds->verts[ ds->indexes[ i + 2 ] ].xyz, w->p[ 2 ] );
			refs += FilterWindingIntoTree_r( w, ds, tree->headnode );
		}

		/* use point filtering as well */
		for ( i = 0; i < ( ds->numVerts - ds->numFoliageInstances ); i++ )
		{
			VectorAdd( instance->xyz, ds->verts[ i ].xyz, xyz );
			refs += FilterPointIntoTree_r( xyz, ds, tree->headnode );
		}
	}

	return refs;
}



/*
   FilterFlareIntoTree()
   simple point filtering for flare surfaces
 */
static int FilterFlareSurfIntoTree( mapDrawSurface_t *ds, tree_t *tree ){
	return FilterPointIntoTree_r( ds->lightmapOrigin, ds, tree->headnode );
}



/*
   EmitDrawVerts() - ydnar
   emits bsp drawverts from a map drawsurface
 */

void EmitDrawVerts( mapDrawSurface_t *ds, bspDrawSurface_t *out ){
	int i, k;
	bspDrawVert_t   *dv;
	shaderInfo_t    *si;
	float offset;


	/* get stuff */
	si = ds->shaderInfo;
	offset = si->offset;

	/* copy the verts */
	out->firstVert = numBSPDrawVerts;
	out->numVerts = ds->numVerts;
	for ( i = 0; i < ds->numVerts; i++ )
	{
		/* allocate a new vert */
		if ( numBSPDrawVerts == MAX_MAP_DRAW_VERTS ) {
			Error( "MAX_MAP_DRAW_VERTS" );
		}
		IncDrawVerts();
		dv = &bspDrawVerts[ numBSPDrawVerts - 1 ];

		/* copy it */
		memcpy( dv, &ds->verts[ i ], sizeof( *dv ) );

		/* offset? */
		if ( offset != 0.0f ) {
			VectorMA( dv->xyz, offset, dv->normal, dv->xyz );
		}

		/* expand model bounds
		   necessary because of misc_model surfaces on entities
		   note: does not happen on worldspawn as its bounds is only used for determining lightgrid bounds */
		if ( numBSPModels > 0 ) {
			AddPointToBounds( dv->xyz, bspModels[ numBSPModels ].mins, bspModels[ numBSPModels ].maxs );
		}

		/* debug color? */
		if ( debugSurfaces ) {
			for ( k = 0; k < MAX_LIGHTMAPS; k++ )
				VectorCopy( debugColors[ ( ds - mapDrawSurfs ) % 12 ], dv->color[ k ] );
		}
	}
}



/*
   FindDrawIndexes() - ydnar
   this attempts to find a run of indexes in the bsp that match the given indexes
   this tends to reduce the size of the bsp index pool by 1/3 or more
   returns numIndexes + 1 if the search failed
 */

int FindDrawIndexes( int numIndexes, int *indexes ){
	int i, j, numTestIndexes;


	/* dummy check */
	if ( numIndexes < 3 || numBSPDrawIndexes < numIndexes || indexes == NULL ) {
		return numBSPDrawIndexes;
	}

	/* set limit */
	numTestIndexes = 1 + numBSPDrawIndexes - numIndexes;

	/* handle 3 indexes as a special case for performance */
	if ( numIndexes == 3 ) {
		/* run through all indexes */
		for ( i = 0; i < numTestIndexes; i++ )
		{
			/* test 3 indexes */
			if ( indexes[ 0 ] == bspDrawIndexes[ i ] &&
				 indexes[ 1 ] == bspDrawIndexes[ i + 1 ] &&
				 indexes[ 2 ] == bspDrawIndexes[ i + 2 ] ) {
				numRedundantIndexes += numIndexes;
				return i;
			}
		}

		/* failed */
		return numBSPDrawIndexes;
	}

	/* handle 4 or more indexes */
	for ( i = 0; i < numTestIndexes; i++ )
	{
		/* test first 4 indexes */
		if ( indexes[ 0 ] == bspDrawIndexes[ i ] &&
			 indexes[ 1 ] == bspDrawIndexes[ i + 1 ] &&
			 indexes[ 2 ] == bspDrawIndexes[ i + 2 ] &&
			 indexes[ 3 ] == bspDrawIndexes[ i + 3 ] ) {
			/* handle 4 indexes */
			if ( numIndexes == 4 ) {
				return i;
			}

			/* test the remainder */
			for ( j = 4; j < numIndexes; j++ )
			{
				if ( indexes[ j ] != bspDrawIndexes[ i + j ] ) {
					break;
				}
				else if ( j == ( numIndexes - 1 ) ) {
					numRedundantIndexes += numIndexes;
					return i;
				}
			}
		}
	}

	/* failed */
	return numBSPDrawIndexes;
}



/*
   EmitDrawIndexes() - ydnar
   attempts to find an existing run of drawindexes before adding new ones
 */

void EmitDrawIndexes( mapDrawSurface_t *ds, bspDrawSurface_t *out ){
	int i;


	/* attempt to use redundant indexing */
	out->firstIndex = FindDrawIndexes( ds->numIndexes, ds->indexes );
	out->numIndexes = ds->numIndexes;
	if ( out->firstIndex == numBSPDrawIndexes ) {
		/* copy new unique indexes */
		for ( i = 0; i < ds->numIndexes; i++ )
		{
			if ( numBSPDrawIndexes == MAX_MAP_DRAW_INDEXES ) {
				Error( "MAX_MAP_DRAW_INDEXES" );
			}
			bspDrawIndexes[ numBSPDrawIndexes ] = ds->indexes[ i ];

			/* validate the index */
			if ( ds->type != SURFACE_PATCH ) {
				if ( bspDrawIndexes[ numBSPDrawIndexes ] < 0 || bspDrawIndexes[ numBSPDrawIndexes ] >= ds->numVerts ) {
					Sys_FPrintf( SYS_WRN, "WARNING: %d %s has invalid index %d (%d)\n",
								numBSPDrawSurfaces,
								ds->shaderInfo->shader,
								bspDrawIndexes[ numBSPDrawIndexes ],
								i );
					bspDrawIndexes[ numBSPDrawIndexes ] = 0;
				}
			}

			/* increment index count */
			numBSPDrawIndexes++;
		}
	}
}




/*
   EmitFlareSurface()
   emits a bsp flare drawsurface
 */

void EmitFlareSurface( mapDrawSurface_t *ds ){
	int i;
	bspDrawSurface_t        *out;


	/* ydnar: nuking useless flare drawsurfaces */
	if ( emitFlares == qfalse && ds->type != SURFACE_SHADER ) {
		return;
	}

	/* limit check */
	if ( numBSPDrawSurfaces == MAX_MAP_DRAW_SURFS ) {
		Error( "MAX_MAP_DRAW_SURFS" );
	}

	/* allocate a new surface */
	if ( numBSPDrawSurfaces == MAX_MAP_DRAW_SURFS ) {
		Error( "MAX_MAP_DRAW_SURFS" );
	}
	out = &bspDrawSurfaces[ numBSPDrawSurfaces ];
	ds->outputNum = numBSPDrawSurfaces;
	numBSPDrawSurfaces++;
	memset( out, 0, sizeof( *out ) );

	/* set it up */
	out->surfaceType = MST_FLARE;
	out->shaderNum = EmitShader( ds->shaderInfo->shader, &ds->shaderInfo->contentFlags, &ds->shaderInfo->surfaceFlags );
	out->fogNum = ds->fogNum;

	/* RBSP */
	for ( i = 0; i < MAX_LIGHTMAPS; i++ )
	{
		out->lightmapNum[ i ] = -3;
		out->lightmapStyles[ i ] = LS_NONE;
		out->vertexStyles[ i ] = LS_NONE;
	}
	out->lightmapStyles[ 0 ] = ds->lightStyle;
	out->vertexStyles[ 0 ] = ds->lightStyle;

	VectorCopy( ds->lightmapOrigin, out->lightmapOrigin );          /* origin */
	VectorCopy( ds->lightmapVecs[ 0 ], out->lightmapVecs[ 0 ] );    /* color */
	VectorCopy( ds->lightmapVecs[ 1 ], out->lightmapVecs[ 1 ] );
	VectorCopy( ds->lightmapVecs[ 2 ], out->lightmapVecs[ 2 ] );    /* normal */

	/* add to count */
	numSurfacesByType[ ds->type ]++;
}



/*
   EmitPatchSurface()
   emits a bsp patch drawsurface
 */

void EmitPatchSurface( mapDrawSurface_t *ds ){
	int i, j;
	bspDrawSurface_t    *out;
	int surfaceFlags, contentFlags;


	/* invert the surface if necessary */
	if ( ds->backSide || ds->shaderInfo->invert ) {
		bspDrawVert_t   *dv1, *dv2, temp;


		/* walk the verts, flip the normal */
		for ( i = 0; i < ds->numVerts; i++ )
			VectorScale( ds->verts[ i ].normal, -1.0f, ds->verts[ i ].normal );

		/* walk the verts again, but this time reverse their order */
		for ( j = 0; j < ds->patchHeight; j++ )
		{
			for ( i = 0; i < ( ds->patchWidth / 2 ); i++ )
			{
				dv1 = &ds->verts[ j * ds->patchWidth + i ];
				dv2 = &ds->verts[ j * ds->patchWidth + ( ds->patchWidth - i - 1 ) ];
				memcpy( &temp, dv1, sizeof( bspDrawVert_t ) );
				memcpy( dv1, dv2, sizeof( bspDrawVert_t ) );
				memcpy( dv2, &temp, sizeof( bspDrawVert_t ) );
			}
		}

		/* invert facing */
		VectorScale( ds->lightmapVecs[ 2 ], -1.0f, ds->lightmapVecs[ 2 ] );
	}

	/* allocate a new surface */
	if ( numBSPDrawSurfaces == MAX_MAP_DRAW_SURFS ) {
		Error( "MAX_MAP_DRAW_SURFS" );
	}
	out = &bspDrawSurfaces[ numBSPDrawSurfaces ];
	ds->outputNum = numBSPDrawSurfaces;
	numBSPDrawSurfaces++;
	memset( out, 0, sizeof( *out ) );

	/* set it up */
	out->surfaceType = MST_PATCH;
	if ( debugSurfaces ) {
		out->shaderNum = EmitShader( "debugsurfaces", NULL, NULL );
	}
	else if ( patchMeta ) {
		/* patch meta requires that we have nodraw patches for collision */
		surfaceFlags = ds->shaderInfo->surfaceFlags;
		contentFlags = ds->shaderInfo->contentFlags;
		ApplySurfaceParm( "nodraw", &contentFlags, &surfaceFlags, NULL );
		ApplySurfaceParm( "pointlight", &contentFlags, &surfaceFlags, NULL );

		/* we don't want this patch getting lightmapped */
		VectorClear( ds->lightmapVecs[ 2 ] );
		VectorClear( ds->lightmapAxis );
		ds->sampleSize = 0;

		/* emit the new fake shader */
		out->shaderNum = EmitShader( ds->shaderInfo->shader, &contentFlags, &surfaceFlags );
	}
	else{
		out->shaderNum = EmitShader( ds->shaderInfo->shader, &ds->shaderInfo->contentFlags, &ds->shaderInfo->surfaceFlags );
	}
	out->patchWidth = ds->patchWidth;
	out->patchHeight = ds->patchHeight;
	out->fogNum = ds->fogNum;

	/* RBSP */
	for ( i = 0; i < MAX_LIGHTMAPS; i++ )
	{
		out->lightmapNum[ i ] = -3;
		out->lightmapStyles[ i ] = LS_NONE;
		out->vertexStyles[ i ] = LS_NONE;
	}
	out->lightmapStyles[ 0 ] = LS_NORMAL;
	out->vertexStyles[ 0 ] = LS_NORMAL;

	/* ydnar: gs mods: previously, the lod bounds were stored in lightmapVecs[ 0 ] and [ 1 ], moved to bounds[ 0 ] and [ 1 ] */
	VectorCopy( ds->lightmapOrigin, out->lightmapOrigin );
	VectorCopy( ds->bounds[ 0 ], out->lightmapVecs[ 0 ] );
	VectorCopy( ds->bounds[ 1 ], out->lightmapVecs[ 1 ] );
	VectorCopy( ds->lightmapVecs[ 2 ], out->lightmapVecs[ 2 ] );

	/* ydnar: gs mods: clear out the plane normal */
	if ( ds->planar == qfalse ) {
		VectorClear( out->lightmapVecs[ 2 ] );
	}

	/* emit the verts and indexes */
	EmitDrawVerts( ds, out );
	EmitDrawIndexes( ds, out );

	/* add to count */
	numSurfacesByType[ ds->type ]++;
}



/*
   OptimizeTriangleSurface() - ydnar
   optimizes the vertex/index data in a triangle surface
 */

#define VERTEX_CACHE_SIZE   16

static void OptimizeTriangleSurface( mapDrawSurface_t *ds ){
	int i, j, k, temp, first, best, bestScore, score;
	int vertexCache[ VERTEX_CACHE_SIZE + 1 ];       /* one more for optimizing insert */
	int     *indexes;


	/* certain surfaces don't get optimized */
	if ( ds->numIndexes <= VERTEX_CACHE_SIZE ||
		 ds->shaderInfo->autosprite ) {
		return;
	}

	/* create index scratch pad */
	indexes = safe_malloc( ds->numIndexes * sizeof( *indexes ) );
	memcpy( indexes, ds->indexes, ds->numIndexes * sizeof( *indexes ) );

	/* setup */
	for ( i = 0; i <= VERTEX_CACHE_SIZE && i < ds->numIndexes; i++ )
		vertexCache[ i ] = indexes[ i ];

	/* add triangles in a vertex cache-aware order */
	for ( i = 0; i < ds->numIndexes; i += 3 )
	{
		/* find best triangle given the current vertex cache */
		first = -1;
		best = -1;
		bestScore = -1;
		for ( j = 0; j < ds->numIndexes; j += 3 )
		{
			/* valid triangle? */
			if ( indexes[ j ] != -1 ) {
				/* set first if necessary */
				if ( first < 0 ) {
					first = j;
				}

				/* score the triangle */
				score = 0;
				for ( k = 0; k < VERTEX_CACHE_SIZE; k++ )
				{
					if ( indexes[ j ] == vertexCache[ k ] || indexes[ j + 1 ] == vertexCache[ k ] || indexes[ j + 2 ] == vertexCache[ k ] ) {
						score++;
					}
				}

				/* better triangle? */
				if ( score > bestScore ) {
					bestScore = score;
					best = j;
				}

				/* a perfect score of 3 means this triangle's verts are already present in the vertex cache */
				if ( score == 3 ) {
					break;
				}
			}
		}

		/* check if no decent triangle was found, and use first available */
		if ( best < 0 ) {
			best = first;
		}

		/* valid triangle? */
		if ( best >= 0 ) {
			/* add triangle to vertex cache */
			for ( j = 0; j < 3; j++ )
			{
				for ( k = 0; k < VERTEX_CACHE_SIZE; k++ )
				{
					if ( indexes[ best + j ] == vertexCache[ k ] ) {
						break;
					}
				}

				if ( k >= VERTEX_CACHE_SIZE ) {
					/* pop off top of vertex cache */
					for ( k = VERTEX_CACHE_SIZE; k > 0; k-- )
						vertexCache[ k ] = vertexCache[ k - 1 ];

					/* add vertex */
					vertexCache[ 0 ] = indexes[ best + j ];
				}
			}

			/* add triangle to surface */
			ds->indexes[ i ] = indexes[ best ];
			ds->indexes[ i + 1 ] = indexes[ best + 1 ];
			ds->indexes[ i + 2 ] = indexes[ best + 2 ];

			/* clear from input pool */
			indexes[ best ] = -1;
			indexes[ best + 1 ] = -1;
			indexes[ best + 2 ] = -1;

			/* sort triangle windings (312 -> 123) */
			while ( ds->indexes[ i ] > ds->indexes[ i + 1 ] || ds->indexes[ i ] > ds->indexes[ i + 2 ] )
			{
				temp = ds->indexes[ i ];
				ds->indexes[ i ] = ds->indexes[ i + 1 ];
				ds->indexes[ i + 1 ] = ds->indexes[ i + 2 ];
				ds->indexes[ i + 2 ] = temp;
			}
		}
	}

	/* clean up */
	free( indexes );
}



/*
   EmitTriangleSurface()
   creates a bsp drawsurface from arbitrary triangle surfaces
 */

static void EmitTriangleSurface( mapDrawSurface_t *ds ){
	int i, temp;
	bspDrawSurface_t        *out;


	/* invert the surface if necessary */
	if ( ds->backSide || ds->shaderInfo->invert ) {
		/* walk the indexes, reverse the triangle order */
		for ( i = 0; i < ds->numIndexes; i += 3 )
		{
			temp = ds->indexes[ i ];
			ds->indexes[ i ] = ds->indexes[ i + 1 ];
			ds->indexes[ i + 1 ] = temp;
		}

		/* walk the verts, flip the normal */
		for ( i = 0; i < ds->numVerts; i++ )
			VectorScale( ds->verts[ i ].normal, -1.0f, ds->verts[ i ].normal );

		/* invert facing */
		VectorScale( ds->lightmapVecs[ 2 ], -1.0f, ds->lightmapVecs[ 2 ] );
	}

	/* allocate a new surface */
	if ( numBSPDrawSurfaces == MAX_MAP_DRAW_SURFS ) {
		Error( "MAX_MAP_DRAW_SURFS" );
	}
	out = &bspDrawSurfaces[ numBSPDrawSurfaces ];
	ds->outputNum = numBSPDrawSurfaces;
	numBSPDrawSurfaces++;
	memset( out, 0, sizeof( *out ) );

	/* ydnar/sd: handle wolf et foliage surfaces */
	if ( ds->type == SURFACE_FOLIAGE ) {
		out->surfaceType = MST_FOLIAGE;
	}

	/* ydnar: gs mods: handle lightmapped terrain (force to planar type) */
	//%	else if( VectorLength( ds->lightmapAxis ) <= 0.0f || ds->type == SURFACE_TRIANGLES || ds->type == SURFACE_FOGHULL || debugSurfaces )
	else if ( ( VectorLength( ds->lightmapAxis ) <= 0.0f && ds->planar == qfalse ) ||
			  ds->type == SURFACE_TRIANGLES ||
			  ds->type == SURFACE_FOGHULL ||
			  ds->numVerts > maxLMSurfaceVerts ||
			  debugSurfaces ) {
		out->surfaceType = MST_TRIANGLE_SOUP;
	}

	/* set to a planar face */
	else{
		out->surfaceType = MST_PLANAR;
	}

	/* set it up */
	if ( debugSurfaces ) {
		out->shaderNum = EmitShader( "debugsurfaces", NULL, NULL );
	}
	else{
		out->shaderNum = EmitShader( ds->shaderInfo->shader, &ds->shaderInfo->contentFlags, &ds->shaderInfo->surfaceFlags );
	}
	out->patchWidth = ds->patchWidth;
	out->patchHeight = ds->patchHeight;
	out->fogNum = ds->fogNum;

	/* debug inset (push each triangle vertex towards the center of each triangle it is on */
	if ( debugInset ) {
		bspDrawVert_t   *a, *b, *c;
		vec3_t cent, dir;


		/* walk triangle list */
		for ( i = 0; i < ds->numIndexes; i += 3 )
		{
			/* get verts */
			a = &ds->verts[ ds->indexes[ i ] ];
			b = &ds->verts[ ds->indexes[ i + 1 ] ];
			c = &ds->verts[ ds->indexes[ i + 2 ] ];

			/* calculate centroid */
			VectorCopy( a->xyz, cent );
			VectorAdd( cent, b->xyz, cent );
			VectorAdd( cent, c->xyz, cent );
			VectorScale( cent, 1.0f / 3.0f, cent );

			/* offset each vertex */
			VectorSubtract( cent, a->xyz, dir );
			VectorNormalize( dir, dir );
			VectorAdd( a->xyz, dir, a->xyz );
			VectorSubtract( cent, b->xyz, dir );
			VectorNormalize( dir, dir );
			VectorAdd( b->xyz, dir, b->xyz );
			VectorSubtract( cent, c->xyz, dir );
			VectorNormalize( dir, dir );
			VectorAdd( c->xyz, dir, c->xyz );
		}
	}

	/* RBSP */
	for ( i = 0; i < MAX_LIGHTMAPS; i++ )
	{
		out->lightmapNum[ i ] = -3;
		out->lightmapStyles[ i ] = LS_NONE;
		out->vertexStyles[ i ] = LS_NONE;
	}
	out->lightmapStyles[ 0 ] = LS_NORMAL;
	out->vertexStyles[ 0 ] = LS_NORMAL;

	/* lightmap vectors (lod bounds for patches */
	VectorCopy( ds->lightmapOrigin, out->lightmapOrigin );
	VectorCopy( ds->lightmapVecs[ 0 ], out->lightmapVecs[ 0 ] );
	VectorCopy( ds->lightmapVecs[ 1 ], out->lightmapVecs[ 1 ] );
	VectorCopy( ds->lightmapVecs[ 2 ], out->lightmapVecs[ 2 ] );

	/* ydnar: gs mods: clear out the plane normal */
	if ( ds->planar == qfalse ) {
		VectorClear( out->lightmapVecs[ 2 ] );
	}

	/* optimize the surface's triangles */
	OptimizeTriangleSurface( ds );

	/* emit the verts and indexes */
	EmitDrawVerts( ds, out );
	EmitDrawIndexes( ds, out );

	/* add to count */
	numSurfacesByType[ ds->type ]++;
}



/*
   EmitFaceSurface()
   emits a bsp planar winding (brush face) drawsurface
 */

static void EmitFaceSurface( mapDrawSurface_t *ds ){
	/* strip/fan finding was moved elsewhere */
	StripFaceSurface( ds );
	EmitTriangleSurface( ds );
}



/*
   MakeDebugPortalSurfs_r() - ydnar
   generates drawsurfaces for passable portals in the bsp
 */

static void MakeDebugPortalSurfs_r( node_t *node, shaderInfo_t *si ){
	int i, k, c, s;
	portal_t            *p;
	winding_t           *w;
	mapDrawSurface_t    *ds;
	bspDrawVert_t       *dv;


	/* recurse if decision node */
	if ( node->planenum != PLANENUM_LEAF ) {
		MakeDebugPortalSurfs_r( node->children[ 0 ], si );
		MakeDebugPortalSurfs_r( node->children[ 1 ], si );
		return;
	}

	/* don't bother with opaque leaves */
	if ( node->opaque ) {
		return;
	}

	/* walk the list of portals */
	for ( c = 0, p = node->portals; p != NULL; c++, p = p->next[ s ] )
	{
		/* get winding and side even/odd */
		w = p->winding;
		s = ( p->nodes[ 1 ] == node );

		/* is this a valid portal for this leaf? */
		if ( w && p->nodes[ 0 ] == node ) {
			/* is this portal passable? */
			if ( PortalPassable( p ) == qfalse ) {
				continue;
			}

			/* check max points */
			if ( w->numpoints > 64 ) {
				Error( "MakePortalSurfs_r: w->numpoints = %d", w->numpoints );
			}

			/* allocate a drawsurface */
			ds = AllocDrawSurface( SURFACE_FACE );
			ds->shaderInfo = si;
			ds->planar = qtrue;
			ds->sideRef = AllocSideRef( p->side, NULL );
			ds->planeNum = FindFloatPlane( p->plane.normal, p->plane.dist, 0, NULL );
			VectorCopy( p->plane.normal, ds->lightmapVecs[ 2 ] );
			ds->fogNum = -1;
			ds->numVerts = w->numpoints;
			ds->verts = safe_malloc( ds->numVerts * sizeof( *ds->verts ) );
			memset( ds->verts, 0, ds->numVerts * sizeof( *ds->verts ) );

			/* walk the winding */
			for ( i = 0; i < ds->numVerts; i++ )
			{
				/* get vert */
				dv = ds->verts + i;

				/* set it */
				VectorCopy( w->p[ i ], dv->xyz );
				VectorCopy( p->plane.normal, dv->normal );
				dv->st[ 0 ] = 0;
				dv->st[ 1 ] = 0;
				for ( k = 0; k < MAX_LIGHTMAPS; k++ )
				{
					VectorCopy( debugColors[ c % 12 ], dv->color[ k ] );
					dv->color[ k ][ 3 ] = 32;
				}
			}
		}
	}
}



/*
   MakeDebugPortalSurfs() - ydnar
   generates drawsurfaces for passable portals in the bsp
 */

void MakeDebugPortalSurfs( tree_t *tree ){
	shaderInfo_t    *si;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- MakeDebugPortalSurfs ---\n" );

	/* get portal debug shader */
	si = ShaderInfoForShader( "debugportals" );

	/* walk the tree */
	MakeDebugPortalSurfs_r( tree->headnode, si );
}



/*
   MakeFogHullSurfs()
   generates drawsurfaces for a foghull (this MUST use a sky shader)
 */

void MakeFogHullSurfs( entity_t *e, tree_t *tree, char *shader ){
	shaderInfo_t        *si;
	mapDrawSurface_t    *ds;
	vec3_t fogMins, fogMaxs;
	int i, indexes[] =
	{
		0, 1, 2, 0, 2, 3,
		4, 7, 5, 5, 7, 6,
		1, 5, 6, 1, 6, 2,
		0, 4, 5, 0, 5, 1,
		2, 6, 7, 2, 7, 3,
		3, 7, 4, 3, 4, 0
	};


	/* dummy check */
	if ( shader == NULL || shader[ 0 ] == '\0' ) {
		return;
	}

	/* note it */
	Sys_FPrintf( SYS_VRB, "--- MakeFogHullSurfs ---\n" );

	/* get hull bounds */
	VectorCopy( mapMins, fogMins );
	VectorCopy( mapMaxs, fogMaxs );
	for ( i = 0; i < 3; i++ )
	{
		fogMins[ i ] -= 128;
		fogMaxs[ i ] += 128;
	}

	/* get foghull shader */
	si = ShaderInfoForShader( shader );

	/* allocate a drawsurface */
	ds = AllocDrawSurface( SURFACE_FOGHULL );
	ds->shaderInfo = si;
	ds->fogNum = -1;
	ds->numVerts = 8;
	ds->verts = safe_malloc( ds->numVerts * sizeof( *ds->verts ) );
	memset( ds->verts, 0, ds->numVerts * sizeof( *ds->verts ) );
	ds->numIndexes = 36;
	ds->indexes = safe_malloc( ds->numIndexes * sizeof( *ds->indexes ) );
	memset( ds->indexes, 0, ds->numIndexes * sizeof( *ds->indexes ) );

	/* set verts */
	VectorSet( ds->verts[ 0 ].xyz, fogMins[ 0 ], fogMins[ 1 ], fogMins[ 2 ] );
	VectorSet( ds->verts[ 1 ].xyz, fogMins[ 0 ], fogMaxs[ 1 ], fogMins[ 2 ] );
	VectorSet( ds->verts[ 2 ].xyz, fogMaxs[ 0 ], fogMaxs[ 1 ], fogMins[ 2 ] );
	VectorSet( ds->verts[ 3 ].xyz, fogMaxs[ 0 ], fogMins[ 1 ], fogMins[ 2 ] );

	VectorSet( ds->verts[ 4 ].xyz, fogMins[ 0 ], fogMins[ 1 ], fogMaxs[ 2 ] );
	VectorSet( ds->verts[ 5 ].xyz, fogMins[ 0 ], fogMaxs[ 1 ], fogMaxs[ 2 ] );
	VectorSet( ds->verts[ 6 ].xyz, fogMaxs[ 0 ], fogMaxs[ 1 ], fogMaxs[ 2 ] );
	VectorSet( ds->verts[ 7 ].xyz, fogMaxs[ 0 ], fogMins[ 1 ], fogMaxs[ 2 ] );

	/* set indexes */
	memcpy( ds->indexes, indexes, ds->numIndexes * sizeof( *ds->indexes ) );
}



/*
   BiasSurfaceTextures()
   biases a surface's texcoords as close to 0 as possible
 */

void BiasSurfaceTextures( mapDrawSurface_t *ds ){
	int i;


	/* calculate the surface texture bias */
	CalcSurfaceTextureRange( ds );

	/* don't bias globaltextured shaders */
	if ( ds->shaderInfo->globalTexture ) {
		return;
	}

	/* bias the texture coordinates */
	for ( i = 0; i < ds->numVerts; i++ )
	{
		ds->verts[ i ].st[ 0 ] += ds->bias[ 0 ];
		ds->verts[ i ].st[ 1 ] += ds->bias[ 1 ];
	}
}



/*
   AddSurfaceModelsToTriangle_r()
   adds models to a specified triangle, returns the number of models added
 */

int AddSurfaceModelsToTriangle_r( mapDrawSurface_t *ds, surfaceModel_t *model, bspDrawVert_t **tri ){
	bspDrawVert_t mid, *tri2[ 3 ];
	int max, n, localNumSurfaceModels;


	/* init */
	localNumSurfaceModels = 0;

	/* subdivide calc */
	{
		int i;
		float       *a, *b, dx, dy, dz, dist, maxDist;


		/* find the longest edge and split it */
		max = -1;
		maxDist = 0.0f;
		for ( i = 0; i < 3; i++ )
		{
			/* get verts */
			a = tri[ i ]->xyz;
			b = tri[ ( i + 1 ) % 3 ]->xyz;

			/* get dists */
			dx = a[ 0 ] - b[ 0 ];
			dy = a[ 1 ] - b[ 1 ];
			dz = a[ 2 ] - b[ 2 ];
			dist = ( dx * dx ) + ( dy * dy ) + ( dz * dz );

			/* longer? */
			if ( dist > maxDist ) {
				maxDist = dist;
				max = i;
			}
		}

		/* is the triangle small enough? */
		if ( max < 0 || maxDist <= ( model->density * model->density ) ) {
			float odds, r, angle;
			vec3_t origin, normal, scale, axis[ 3 ], angles;
			m4x4_t transform, temp;


			/* roll the dice (model's odds scaled by vertex alpha) */
			odds = model->odds * ( tri[ 0 ]->color[ 0 ][ 3 ] + tri[ 0 ]->color[ 0 ][ 3 ] + tri[ 0 ]->color[ 0 ][ 3 ] ) / 765.0f;
			r = Random();
			if ( r > model->odds ) {
				return 0;
			}

			/* calculate scale */
			r = model->minScale + Random() * ( model->maxScale - model->minScale );
			VectorSet( scale, r, r, r );

			/* calculate angle */
			angle = model->minAngle + Random() * ( model->maxAngle - model->minAngle );

			/* calculate average origin */
			VectorCopy( tri[ 0 ]->xyz, origin );
			VectorAdd( origin, tri[ 1 ]->xyz, origin );
			VectorAdd( origin, tri[ 2 ]->xyz, origin );
			VectorScale( origin, ( 1.0f / 3.0f ), origin );

			/* clear transform matrix */
			m4x4_identity( transform );

			/* handle oriented models */
			if ( model->oriented ) {
				/* set angles */
				VectorSet( angles, 0.0f, 0.0f, angle );

				/* calculate average normal */
				VectorCopy( tri[ 0 ]->normal, normal );
				VectorAdd( normal, tri[ 1 ]->normal, normal );
				VectorAdd( normal, tri[ 2 ]->normal, normal );
				if ( VectorNormalize( normal, axis[ 2 ] ) == 0.0f ) {
					VectorCopy( tri[ 0 ]->normal, axis[ 2 ] );
				}

				/* make perpendicular vectors */
				MakeNormalVectors( axis[ 2 ], axis[ 1 ], axis[ 0 ] );

				/* copy to matrix */
				m4x4_identity( temp );
				temp[ 0 ] = axis[ 0 ][ 0 ]; temp[ 1 ] = axis[ 0 ][ 1 ]; temp[ 2 ] = axis[ 0 ][ 2 ];
				temp[ 4 ] = axis[ 1 ][ 0 ]; temp[ 5 ] = axis[ 1 ][ 1 ]; temp[ 6 ] = axis[ 1 ][ 2 ];
				temp[ 8 ] = axis[ 2 ][ 0 ]; temp[ 9 ] = axis[ 2 ][ 1 ]; temp[ 10 ] = axis[ 2 ][ 2 ];

				/* scale */
				m4x4_scale_by_vec3( temp, scale );

				/* rotate around z axis */
				m4x4_rotate_by_vec3( temp, angles, eXYZ );

				/* translate */
				m4x4_translate_by_vec3( transform, origin );

				/* tranform into axis space */
				m4x4_multiply_by_m4x4( transform, temp );
			}

			/* handle z-up models */
			else
			{
				/* set angles */
				VectorSet( angles, 0.0f, 0.0f, angle );

				/* set matrix */
				m4x4_pivoted_transform_by_vec3( transform, origin, angles, eXYZ, scale, vec3_origin );
			}

			/* insert the model */
			InsertModel( (char *) model->model, 0, transform, NULL, ds->celShader, ds->entityNum, ds->castShadows, ds->recvShadows, 0, ds->lightmapScale );

			/* return to sender */
			return 1;
		}
	}

	/* split the longest edge and map it */
	LerpDrawVert( tri[ max ], tri[ ( max + 1 ) % 3 ], &mid );

	/* recurse to first triangle */
	VectorCopy( tri, tri2 );
	tri2[ max ] = &mid;
	n = AddSurfaceModelsToTriangle_r( ds, model, tri2 );
	if ( n < 0 ) {
		return n;
	}
	localNumSurfaceModels += n;

	/* recurse to second triangle */
	VectorCopy( tri, tri2 );
	tri2[ ( max + 1 ) % 3 ] = &mid;
	n = AddSurfaceModelsToTriangle_r( ds, model, tri2 );
	if ( n < 0 ) {
		return n;
	}
	localNumSurfaceModels += n;

	/* return count */
	return localNumSurfaceModels;
}



/*
   AddSurfaceModels()
   adds a surface's shader models to the surface
 */

int AddSurfaceModels( mapDrawSurface_t *ds ){
	surfaceModel_t  *model;
	int i, x, y, n, pw[ 5 ], r, localNumSurfaceModels, iterations;
	mesh_t src, *mesh, *subdivided;
	bspDrawVert_t centroid, *tri[ 3 ];
	float alpha;


	/* dummy check */
	if ( ds == NULL || ds->shaderInfo == NULL || ds->shaderInfo->surfaceModel == NULL ) {
		return 0;
	}

	/* init */
	localNumSurfaceModels = 0;

	/* walk the model list */
	for ( model = ds->shaderInfo->surfaceModel; model != NULL; model = model->next )
	{
		/* switch on type */
		switch ( ds->type )
		{
		/* handle brush faces and decals */
		case SURFACE_FACE:
		case SURFACE_DECAL:
			/* calculate centroid */
			memset( &centroid, 0, sizeof( centroid ) );
			alpha = 0.0f;

			/* walk verts */
			for ( i = 0; i < ds->numVerts; i++ )
			{
				VectorAdd( centroid.xyz, ds->verts[ i ].xyz, centroid.xyz );
				VectorAdd( centroid.normal, ds->verts[ i ].normal, centroid.normal );
				centroid.st[ 0 ] += ds->verts[ i ].st[ 0 ];
				centroid.st[ 1 ] += ds->verts[ i ].st[ 1 ];
				alpha += ds->verts[ i ].color[ 0 ][ 3 ];
			}

			/* average */
			centroid.xyz[ 0 ] /= ds->numVerts;
			centroid.xyz[ 1 ] /= ds->numVerts;
			centroid.xyz[ 2 ] /= ds->numVerts;
			if ( VectorNormalize( centroid.normal, centroid.normal ) == 0.0f ) {
				VectorCopy( ds->verts[ 0 ].normal, centroid.normal );
			}
			centroid.st[ 0 ]  /= ds->numVerts;
			centroid.st[ 1 ]  /= ds->numVerts;
			alpha /= ds->numVerts;
			centroid.color[ 0 ][ 0 ] = 0xFF;
			centroid.color[ 0 ][ 1 ] = 0xFF;
			centroid.color[ 0 ][ 2 ] = 0xFF;
			centroid.color[ 0 ][ 2 ] = ( alpha > 255.0f ? 0xFF : alpha );

			/* head vert is centroid */
			tri[ 0 ] = &centroid;

			/* walk fanned triangles */
			for ( i = 0; i < ds->numVerts; i++ )
			{
				/* set triangle */
				tri[ 1 ] = &ds->verts[ i ];
				tri[ 2 ] = &ds->verts[ ( i + 1 ) % ds->numVerts ];

				/* create models */
				n = AddSurfaceModelsToTriangle_r( ds, model, tri );
				if ( n < 0 ) {
					return n;
				}
				localNumSurfaceModels += n;
			}
			break;

		/* handle patches */
		case SURFACE_PATCH:
			/* subdivide the surface */
			src.width = ds->patchWidth;
			src.height = ds->patchHeight;
			src.verts = ds->verts;
			//%	subdivided = SubdivideMesh( src, 8.0f, 512 );
			iterations = IterationsForCurve( ds->longestCurve, patchSubdivisions );
			subdivided = SubdivideMesh2( src, iterations );

			/* fit it to the curve and remove colinear verts on rows/columns */
			PutMeshOnCurve( *subdivided );
			mesh = RemoveLinearMeshColumnsRows( subdivided );
			FreeMesh( subdivided );

			/* subdivide each quad to place the models */
			for ( y = 0; y < ( mesh->height - 1 ); y++ )
			{
				for ( x = 0; x < ( mesh->width - 1 ); x++ )
				{
					/* set indexes */
					pw[ 0 ] = x + ( y * mesh->width );
					pw[ 1 ] = x + ( ( y + 1 ) * mesh->width );
					pw[ 2 ] = x + 1 + ( ( y + 1 ) * mesh->width );
					pw[ 3 ] = x + 1 + ( y * mesh->width );
					pw[ 4 ] = x + ( y * mesh->width );      /* same as pw[ 0 ] */

					/* set radix */
					r = ( x + y ) & 1;

					/* triangle 1 */
					tri[ 0 ] = &mesh->verts[ pw[ r + 0 ] ];
					tri[ 1 ] = &mesh->verts[ pw[ r + 1 ] ];
					tri[ 2 ] = &mesh->verts[ pw[ r + 2 ] ];
					n = AddSurfaceModelsToTriangle_r( ds, model, tri );
					if ( n < 0 ) {
						return n;
					}
					localNumSurfaceModels += n;

					/* triangle 2 */
					tri[ 0 ] = &mesh->verts[ pw[ r + 0 ] ];
					tri[ 1 ] = &mesh->verts[ pw[ r + 2 ] ];
					tri[ 2 ] = &mesh->verts[ pw[ r + 3 ] ];
					n = AddSurfaceModelsToTriangle_r( ds, model, tri );
					if ( n < 0 ) {
						return n;
					}
					localNumSurfaceModels += n;
				}
			}

			/* free the subdivided mesh */
			FreeMesh( mesh );
			break;

		/* handle triangle surfaces */
		case SURFACE_TRIANGLES:
		case SURFACE_FORCED_META:
		case SURFACE_META:
			/* walk the triangle list */
			for ( i = 0; i < ds->numIndexes; i += 3 )
			{
				tri[ 0 ] = &ds->verts[ ds->indexes[ i ] ];
				tri[ 1 ] = &ds->verts[ ds->indexes[ i + 1 ] ];
				tri[ 2 ] = &ds->verts[ ds->indexes[ i + 2 ] ];
				n = AddSurfaceModelsToTriangle_r( ds, model, tri );
				if ( n < 0 ) {
					return n;
				}
				localNumSurfaceModels += n;
			}
			break;

		/* no support for flares, foghull, etc */
		default:
			break;
		}
	}

	/* return count */
	return localNumSurfaceModels;
}



/*
   AddEntitySurfaceModels() - ydnar
   adds surfacemodels to an entity's surfaces
 */

void AddEntitySurfaceModels( entity_t *e ){
	int i;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- AddEntitySurfaceModels ---\n" );

	/* walk the surface list */
	for ( i = e->firstDrawSurf; i < numMapDrawSurfs; i++ )
		numSurfaceModels += AddSurfaceModels( &mapDrawSurfs[ i ] );
}



/*
   VolumeColorMods() - ydnar
   applies brush/volumetric color/alpha modulation to vertexes
 */

static void VolumeColorMods( entity_t *e, mapDrawSurface_t *ds ){
	int i, j;
	float d;
	brush_t     *b;
	plane_t     *plane;


	/* early out */
	if ( e->colorModBrushes == NULL ) {
		return;
	}

	/* iterate brushes */
	for ( b = e->colorModBrushes; b != NULL; b = b->nextColorModBrush )
	{
		/* worldspawn alpha brushes affect all, grouped ones only affect original entity */
		if ( b->entityNum != 0 && b->entityNum != ds->entityNum ) {
			continue;
		}

		/* test bbox */
		if ( b->mins[ 0 ] > ds->maxs[ 0 ] || b->maxs[ 0 ] < ds->mins[ 0 ] ||
			 b->mins[ 1 ] > ds->maxs[ 1 ] || b->maxs[ 1 ] < ds->mins[ 1 ] ||
			 b->mins[ 2 ] > ds->maxs[ 2 ] || b->maxs[ 2 ] < ds->mins[ 2 ] ) {
			continue;
		}

		/* iterate verts */
		for ( i = 0; i < ds->numVerts; i++ )
		{
			/* iterate planes */
			for ( j = 0; j < b->numsides; j++ )
			{
				/* point-plane test */
				plane = &mapplanes[ b->sides[ j ].planenum ];
				d = DotProduct( ds->verts[ i ].xyz, plane->normal ) - plane->dist;
				if ( d > 1.0f ) {
					break;
				}
			}

			/* apply colormods */
			if ( j == b->numsides ) {
				ColorMod( b->contentShader->colorMod, 1, &ds->verts[ i ] );
			}
		}
	}
}



/*
   FilterDrawsurfsIntoTree()
   upon completion, all drawsurfs that actually generate a reference
   will have been emited to the bspfile arrays, and the references
   will have valid final indexes
 */

void FilterDrawsurfsIntoTree( entity_t *e, tree_t *tree ){
	int i, j;
	mapDrawSurface_t    *ds;
	shaderInfo_t        *si;
	vec3_t origin, mins, maxs;
	int refs;
	int numSurfs, numRefs, numSkyboxSurfaces;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- FilterDrawsurfsIntoTree ---\n" );

	/* filter surfaces into the tree */
	numSurfs = 0;
	numRefs = 0;
	numSkyboxSurfaces = 0;
	for ( i = e->firstDrawSurf; i < numMapDrawSurfs; i++ )
	{
		/* get surface and try to early out */
		ds = &mapDrawSurfs[ i ];
		if ( ds->numVerts == 0 && ds->type != SURFACE_FLARE && ds->type != SURFACE_SHADER ) {
			continue;
		}

		/* get shader */
		si = ds->shaderInfo;

		/* ydnar: skybox surfaces are special */
		if ( ds->skybox ) {
			refs = AddReferenceToTree_r( ds, tree->headnode, qtrue );
			ds->skybox = qfalse;
		}
		else
		{
			/* refs initially zero */
			refs = 0;

			/* apply texture coordinate mods */
			for ( j = 0; j < ds->numVerts; j++ )
				TCMod( si->mod, ds->verts[ j ].st );

			/* ydnar: apply shader colormod */
			ColorMod( ds->shaderInfo->colorMod, ds->numVerts, ds->verts );

			/* ydnar: apply brush colormod */
			VolumeColorMods( e, ds );

			/* ydnar: make fur surfaces */
			if ( si->furNumLayers > 0 ) {
				Fur( ds );
			}

			/* ydnar/sd: make foliage surfaces */
			if ( si->foliage != NULL ) {
				Foliage( ds );
			}

			/* create a flare surface if necessary */
			if ( si->flareShader != NULL && si->flareShader[ 0 ] ) {
				AddSurfaceFlare( ds, e->origin );
			}

			/* ydnar: don't emit nodraw surfaces (like nodraw fog) */
			if ( si != NULL && ( si->compileFlags & C_NODRAW ) && ds->type != SURFACE_PATCH ) {
				continue;
			}

			/* ydnar: bias the surface textures */
			BiasSurfaceTextures( ds );

			/* ydnar: globalizing of fog volume handling (eek a hack) */
			if ( e != entities && si->noFog == qfalse ) {
				/* find surface origin and offset by entity origin */
				VectorAdd( ds->mins, ds->maxs, origin );
				VectorScale( origin, 0.5f, origin );
				VectorAdd( origin, e->origin, origin );

				VectorAdd( ds->mins, e->origin, mins );
				VectorAdd( ds->maxs, e->origin, maxs );

				/* set the fog number for this surface */
				ds->fogNum = FogForBounds( mins, maxs, 1.0f );  //%	FogForPoint( origin, 0.0f );
			}
		}

		/* ydnar: remap shader */
		if ( ds->shaderInfo->remapShader && ds->shaderInfo->remapShader[ 0 ] ) {
			ds->shaderInfo = ShaderInfoForShader( ds->shaderInfo->remapShader );
		}

		/* ydnar: gs mods: handle the various types of surfaces */
		switch ( ds->type )
		{
		/* handle brush faces */
		case SURFACE_FACE:
		case SURFACE_DECAL:
			if ( refs == 0 ) {
				refs = FilterFaceIntoTree( ds, tree );
			}
			if ( refs > 0 ) {
				EmitFaceSurface( ds );
			}
			break;

		/* handle patches */
		case SURFACE_PATCH:
			if ( refs == 0 ) {
				refs = FilterPatchIntoTree( ds, tree );
			}
			if ( refs > 0 ) {
				EmitPatchSurface( ds );
			}
			break;

		/* handle triangle surfaces */
		case SURFACE_TRIANGLES:
		case SURFACE_FORCED_META:
		case SURFACE_META:
			//%	Sys_FPrintf( SYS_VRB, "Surface %4d: [%1d] %4d verts %s\n", numSurfs, ds->planar, ds->numVerts, si->shader );
			if ( refs == 0 ) {
				refs = FilterTrianglesIntoTree( ds, tree );
			}
			if ( refs > 0 ) {
				EmitTriangleSurface( ds );
			}
			break;

		/* handle foliage surfaces (splash damage/wolf et) */
		case SURFACE_FOLIAGE:
			//%	Sys_FPrintf( SYS_VRB, "Surface %4d: [%d] %4d verts %s\n", numSurfs, ds->numFoliageInstances, ds->numVerts, si->shader );
			if ( refs == 0 ) {
				refs = FilterFoliageIntoTree( ds, tree );
			}
			if ( refs > 0 ) {
				EmitTriangleSurface( ds );
			}
			break;

		/* handle foghull surfaces */
		case SURFACE_FOGHULL:
			if ( refs == 0 ) {
				refs = AddReferenceToTree_r( ds, tree->headnode, qfalse );
			}
			if ( refs > 0 ) {
				EmitTriangleSurface( ds );
			}
			break;

		/* handle flares */
		case SURFACE_FLARE:
			if ( refs == 0 ) {
				refs = FilterFlareSurfIntoTree( ds, tree );
			}
			if ( refs > 0 ) {
				EmitFlareSurface( ds );
			}
			break;

		/* handle shader-only surfaces */
		case SURFACE_SHADER:
			refs = 1;
			EmitFlareSurface( ds );
			break;

		/* no references */
		default:
			refs = 0;
			break;
		}

		/* tot up the references */
		if ( refs > 0 ) {
			/* tot up counts */
			numSurfs++;
			numRefs += refs;

			/* emit extra surface data */
			SetSurfaceExtra( ds, numBSPDrawSurfaces - 1 );
			//%	Sys_FPrintf( SYS_VRB, "%d verts %d indexes\n", ds->numVerts, ds->numIndexes );

			/* one last sanity check */
			{
				bspDrawSurface_t    *out;
				out = &bspDrawSurfaces[ numBSPDrawSurfaces - 1 ];
				if ( out->numVerts == 3 && out->numIndexes > 3 ) {
					Sys_FPrintf( SYS_WRN, "WARNING: Potentially bad %s surface (%d: %d, %d)\n     %s\n",
								surfaceTypes[ ds->type ],
								numBSPDrawSurfaces - 1, out->numVerts, out->numIndexes, si->shader );
				}
			}

			/* ydnar: handle skybox surfaces */
			if ( ds->skybox ) {
				MakeSkyboxSurface( ds );
				numSkyboxSurfaces++;
			}
		}
	}

	/* emit some statistics */
	Sys_FPrintf( SYS_VRB, "%9d references\n", numRefs );
	Sys_FPrintf( SYS_VRB, "%9d (%d) emitted drawsurfs\n", numSurfs, numBSPDrawSurfaces );
	Sys_FPrintf( SYS_VRB, "%9d stripped face surfaces\n", numStripSurfaces );
	Sys_FPrintf( SYS_VRB, "%9d fanned face surfaces\n", numFanSurfaces );
	Sys_FPrintf( SYS_VRB, "%9d surface models generated\n", numSurfaceModels );
	Sys_FPrintf( SYS_VRB, "%9d skybox surfaces generated\n", numSkyboxSurfaces );
	for ( i = 0; i < NUM_SURFACE_TYPES; i++ )
		Sys_FPrintf( SYS_VRB, "%9d %s surfaces\n", numSurfacesByType[ i ], surfaceTypes[ i ] );

	Sys_FPrintf( SYS_VRB, "%9d redundant indexes supressed, saving %d Kbytes\n", numRedundantIndexes, ( numRedundantIndexes * 4 / 1024 ) );
}
