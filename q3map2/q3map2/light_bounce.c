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
#define LIGHT_BOUNCE_C



/* dependencies */
#include "q3map2.h"



/* functions */

/*
   RadFreeLights()
   deletes any existing lights, freeing up memory for the next bounce
 */

void RadFreeLights( void ){
	light_t     *light, *next;


	/* delete lights */
	for ( light = lights; light; light = next )
	{
		next = light->next;
		if ( light->w != NULL ) {
			FreeWinding( light->w );
		}
		free( light );
	}
	numLights = 0;
	lights = NULL;
}



/*
   RadClipWindingEpsilon()
   clips a rad winding by a plane
   based off the regular clip winding code
 */

static void RadClipWindingEpsilon( radWinding_t *in, vec3_t normal, vec_t dist,
								   vec_t epsilon, radWinding_t *front, radWinding_t *back, clipWork_t *cw ){
	vec_t           *dists;
	int             *sides;
	int counts[ 3 ];
	vec_t dot;                  /* ydnar: changed from static b/c of threading */ /* VC 4.2 optimizer bug if not static? */
	int i, j, k;
	radVert_t       *v1, *v2, mid;
	int maxPoints;


	/* crutch */
	dists = cw->dists;
	sides = cw->sides;

	/* clear counts */
	counts[ 0 ] = counts[ 1 ] = counts[ 2 ] = 0;

	/* determine sides for each point */
	for ( i = 0; i < in->numVerts; i++ )
	{
		dot = DotProduct( in->verts[ i ].xyz, normal );
		dot -= dist;
		dists[ i ] = dot;
		if ( dot > epsilon ) {
			sides[ i ] = SIDE_FRONT;
		}
		else if ( dot < -epsilon ) {
			sides[ i ] = SIDE_BACK;
		}
		else{
			sides[ i ] = SIDE_ON;
		}
		counts[ sides[ i ] ]++;
	}
	sides[ i ] = sides[ 0 ];
	dists[ i ] = dists[ 0 ];

	/* clear front and back */
	front->numVerts = back->numVerts = 0;

	/* handle all on one side cases */
	if ( counts[ 0 ] == 0 ) {
		memcpy( back, in, sizeof( radWinding_t ) );
		return;
	}
	if ( counts[ 1 ] == 0 ) {
		memcpy( front, in, sizeof( radWinding_t ) );
		return;
	}

	/* setup windings */
	maxPoints = in->numVerts + 4;

	/* do individual verts */
	for ( i = 0; i < in->numVerts; i++ )
	{
		/* do simple vertex copies first */
		v1 = &in->verts[ i ];

		if ( sides[ i ] == SIDE_ON ) {
			memcpy( &front->verts[ front->numVerts++ ], v1, sizeof( radVert_t ) );
			memcpy( &back->verts[ back->numVerts++ ], v1, sizeof( radVert_t ) );
			continue;
		}

		if ( sides[ i ] == SIDE_FRONT ) {
			memcpy( &front->verts[ front->numVerts++ ], v1, sizeof( radVert_t ) );
		}

		if ( sides[ i ] == SIDE_BACK ) {
			memcpy( &back->verts[ back->numVerts++ ], v1, sizeof( radVert_t ) );
		}

		if ( sides[ i + 1 ] == SIDE_ON || sides[ i + 1 ] == sides[ i ] ) {
			continue;
		}

		/* generate a split vertex */
		v2 = &in->verts[ ( i + 1 ) % in->numVerts ];

		dot = dists[ i ] / ( dists[ i ] - dists[ i + 1 ] );

		/* average vertex values */
		for ( j = 0; j < 4; j++ )
		{
			/* color */
			if ( j < 4 ) {
				for ( k = 0; k < MAX_LIGHTMAPS; k++ )
					mid.color[ k ][ j ] = v1->color[ k ][ j ] + dot * ( v2->color[ k ][ j ] - v1->color[ k ][ j ] );
			}

			/* xyz, normal */
			if ( j < 3 ) {
				mid.xyz[ j ] = v1->xyz[ j ] + dot * ( v2->xyz[ j ] - v1->xyz[ j ] );
				mid.normal[ j ] = v1->normal[ j ] + dot * ( v2->normal[ j ] - v1->normal[ j ] );
			}

			/* st, lightmap */
			if ( j < 2 ) {
				mid.st[ j ] = v1->st[ j ] + dot * ( v2->st[ j ] - v1->st[ j ] );
				for ( k = 0; k < MAX_LIGHTMAPS; k++ )
					mid.lightmap[ k ][ j ] = v1->lightmap[ k ][ j ] + dot * ( v2->lightmap[ k ][ j ] - v1->lightmap[ k ][ j ] );
			}
		}

		/* normalize the averaged normal */
		VectorNormalize( mid.normal, mid.normal );

		/* copy the midpoint to both windings */
		memcpy( &front->verts[ front->numVerts++ ], &mid, sizeof( radVert_t ) );
		memcpy( &back->verts[ back->numVerts++ ], &mid, sizeof( radVert_t ) );
	}

	/* error check */
	if ( front->numVerts > maxPoints ) {
		Error( "RadClipWindingEpsilon: points exceeded estimate" );
	}
	if ( front->numVerts > MAX_POINTS_ON_WINDING ) {
		Error( "RadClipWindingEpsilon: MAX_POINTS_ON_WINDING" );
	}
}





/*
   RadSampleImage()
   samples a texture image for a given color
   returns qfalse if pixels are bad
 */

qboolean RadSampleImage( byte *pixels, int width, int height, float st[ 2 ], float color[ 4 ] ){
	float sto[ 2 ];
	int x, y;


	/* clear color first */
	color[ 0 ] = color[ 1 ] = color[ 2 ] = color[ 3 ] = 255;

	/* dummy check */
	if ( pixels == NULL || width < 1 || height < 1 ) {
		return qfalse;
	}

	/* bias st */
	sto[ 0 ] = st[ 0 ];
	while ( sto[ 0 ] < 0.0f )
		sto[ 0 ] += 1.0f;
	sto[ 1 ] = st[ 1 ];
	while ( sto[ 1 ] < 0.0f )
		sto[ 1 ] += 1.0f;

	/* get offsets */
	x = ( (float) width * sto[ 0 ] ) + 0.5f;
	x %= width;
	y = ( (float) height * sto[ 1 ] )  + 0.5f;
	y %= height;

	/* get pixel */
	pixels += ( y * width * 4 ) + ( x * 4 );
	VectorCopy( pixels, color );
	color[ 3 ] = pixels[ 3 ];
	return qtrue;
}



/*
   RadSample()
   samples a fragment's lightmap or vertex color and returns an
   average color and a color gradient for the sample
 */

#define MAX_SAMPLES         150
#define SAMPLE_GRANULARITY  6

static void RadSample( int lightmapNum, bspDrawSurface_t *ds, rawLightmap_t *lm, shaderInfo_t *si, radWinding_t *rw, vec3_t average, vec3_t gradient, int *style ){
	int i, j, k, l, v, x, y, samples;
	vec3_t color, mins, maxs;
	vec4_t textureColor;
	float alpha, alphaI, bf;
	vec3_t blend;
	float st[ 2 ], lightmap[ 2 ], *radLuxel;
	radVert_t   *rv[ 3 ];


	/* initial setup */
	ClearBounds( mins, maxs );
	VectorClear( average );
	VectorClear( gradient );
	alpha = 0;

	/* dummy check */
	if ( rw == NULL || rw->numVerts < 3 ) {
		return;
	}

	/* start sampling */
	samples = 0;

	/* sample vertex colors if no lightmap or this is the initial pass */
	if ( lm == NULL || lm->radLuxels[ lightmapNum ] == NULL || bouncing == qfalse ) {
		for ( samples = 0; samples < rw->numVerts; samples++ )
		{
			/* multiply by texture color */
			if ( !RadSampleImage( si->lightImage->pixels, si->lightImage->width, si->lightImage->height, rw->verts[ samples ].st, textureColor ) ) {
				VectorCopy( si->averageColor, textureColor );
				textureColor[ 3 ] = 255.0f;
			}
			for ( i = 0; i < 3; i++ )
				color[ i ] = ( textureColor[ i ] / 255 ) * ( rw->verts[ samples ].color[ lightmapNum ][ i ] / 255.0f );

			AddPointToBounds( color, mins, maxs );
			VectorAdd( average, color, average );

			/* get alpha */
			alpha += ( textureColor[ 3 ] / 255.0f ) * ( rw->verts[ samples ].color[ lightmapNum ][ 3 ] / 255.0f );
		}

		/* set style */
		*style = ds->vertexStyles[ lightmapNum ];
	}

	/* sample lightmap */
	else
	{
		/* fracture the winding into a fan (including degenerate tris) */
		for ( v = 1; v < ( rw->numVerts - 1 ) && samples < MAX_SAMPLES; v++ )
		{
			/* get a triangle */
			rv[ 0 ] = &rw->verts[ 0 ];
			rv[ 1 ] = &rw->verts[ v ];
			rv[ 2 ] = &rw->verts[ v + 1 ];

			/* this code is embarassing (really should just rasterize the triangle) */
			for ( i = 1; i < SAMPLE_GRANULARITY && samples < MAX_SAMPLES; i++ )
			{
				for ( j = 1; j < SAMPLE_GRANULARITY && samples < MAX_SAMPLES; j++ )
				{
					for ( k = 1; k < SAMPLE_GRANULARITY && samples < MAX_SAMPLES; k++ )
					{
						/* create a blend vector (barycentric coordinates) */
						blend[ 0 ] = i;
						blend[ 1 ] = j;
						blend[ 2 ] = k;
						bf = ( 1.0 / ( blend[ 0 ] + blend[ 1 ] + blend[ 2 ] ) );
						VectorScale( blend, bf, blend );

						/* create a blended sample */
						st[ 0 ] = st[ 1 ] = 0.0f;
						lightmap[ 0 ] = lightmap[ 1 ] = 0.0f;
						alphaI = 0.0f;
						for ( l = 0; l < 3; l++ )
						{
							st[ 0 ] += ( rv[ l ]->st[ 0 ] * blend[ l ] );
							st[ 1 ] += ( rv[ l ]->st[ 1 ] * blend[ l ] );
							lightmap[ 0 ] += ( rv[ l ]->lightmap[ lightmapNum ][ 0 ] * blend[ l ] );
							lightmap[ 1 ] += ( rv[ l ]->lightmap[ lightmapNum ][ 1 ] * blend[ l ] );
							alphaI += ( rv[ l ]->color[ lightmapNum ][ 3 ] * blend[ l ] );
						}

						/* get lightmap xy coords */
						x = lightmap[ 0 ] / (float) superSample;
						y = lightmap[ 1 ] / (float) superSample;
						if ( x < 0 ) {
							x = 0;
						}
						else if ( x >= lm->w ) {
							x = lm->w - 1;
						}
						if ( y < 0 ) {
							y = 0;
						}
						else if ( y >= lm->h ) {
							y = lm->h - 1;
						}

						/* get radiosity luxel */
						radLuxel = RAD_LUXEL( lightmapNum, x, y );

						/* ignore unlit/unused luxels */
						if ( radLuxel[ 0 ] < 0.0f ) {
							continue;
						}

						/* inc samples */
						samples++;

						/* multiply by texture color */
						if ( !RadSampleImage( si->lightImage->pixels, si->lightImage->width, si->lightImage->height, st, textureColor ) ) {
							VectorCopy( si->averageColor, textureColor );
							textureColor[ 3 ] = 255;
						}
						for ( i = 0; i < 3; i++ )
							color[ i ] = ( textureColor[ i ] / 255 ) * ( radLuxel[ i ] / 255 );

						AddPointToBounds( color, mins, maxs );
						VectorAdd( average, color, average );

						/* get alpha */
						alpha += ( textureColor[ 3 ] / 255 ) * ( alphaI / 255 );
					}
				}
			}
		}

		/* set style */
		*style = ds->lightmapStyles[ lightmapNum ];
	}

	/* any samples? */
	if ( samples <= 0 ) {
		return;
	}

	/* average the color */
	VectorScale( average, ( 1.0 / samples ), average );

	/* create the color gradient */
	//%	VectorSubtract( maxs, mins, delta );

	/* new: color gradient will always be 0-1.0, expressed as the range of light relative to overall light */
	//%	gradient[ 0 ] = maxs[ 0 ] > 0.0f ? (maxs[ 0 ] - mins[ 0 ]) / maxs[ 0 ] : 0.0f;
	//%	gradient[ 1 ] = maxs[ 1 ] > 0.0f ? (maxs[ 1 ] - mins[ 1 ]) / maxs[ 1 ] : 0.0f;
	//%	gradient[ 2 ] = maxs[ 2 ] > 0.0f ? (maxs[ 2 ] - mins[ 2 ]) / maxs[ 2 ] : 0.0f;

	/* newer: another contrast function */
	for ( i = 0; i < 3; i++ )
		gradient[ i ] = ( maxs[ i ] - mins[ i ] ) * maxs[ i ];
}



/*
   RadSubdivideDiffuseLight()
   subdivides a radiosity winding until it is smaller than subdivide, then generates an area light
 */

#define RADIOSITY_MAX_GRADIENT      0.75f   //%	0.25f
#define RADIOSITY_VALUE             500.0f
#define RADIOSITY_MIN               0.0001f
#define RADIOSITY_CLIP_EPSILON      0.125f

static void RadSubdivideDiffuseLight( int lightmapNum, bspDrawSurface_t *ds, rawLightmap_t *lm, shaderInfo_t *si,
									  float scale, float subdivide, qboolean original, radWinding_t *rw, clipWork_t *cw ){
	int i, style;
	float dist, area, value;
	vec3_t mins, maxs, normal, d1, d2, cross, color, gradient;
	light_t         *light, *splash;
	winding_t       *w;


	/* dummy check */
	if ( rw == NULL || rw->numVerts < 3 ) {
		return;
	}

	/* get bounds for winding */
	ClearBounds( mins, maxs );
	for ( i = 0; i < rw->numVerts; i++ )
		AddPointToBounds( rw->verts[ i ].xyz, mins, maxs );

	/* subdivide if necessary */
	for ( i = 0; i < 3; i++ )
	{
		if ( maxs[ i ] - mins[ i ] > subdivide ) {
			radWinding_t front, back;


			/* make axial plane */
			VectorClear( normal );
			normal[ i ] = 1;
			dist = ( maxs[ i ] + mins[ i ] ) * 0.5f;

			/* clip the winding */
			RadClipWindingEpsilon( rw, normal, dist, RADIOSITY_CLIP_EPSILON, &front, &back, cw );

			/* recurse */
			RadSubdivideDiffuseLight( lightmapNum, ds, lm, si, scale, subdivide, qfalse, &front, cw );
			RadSubdivideDiffuseLight( lightmapNum, ds, lm, si, scale, subdivide, qfalse, &back, cw );
			return;
		}
	}

	/* check area */
	area = 0.0f;
	for ( i = 2; i < rw->numVerts; i++ )
	{
		VectorSubtract( rw->verts[ i - 1 ].xyz, rw->verts[ 0 ].xyz, d1 );
		VectorSubtract( rw->verts[ i ].xyz, rw->verts[ 0 ].xyz, d2 );
		CrossProduct( d1, d2, cross );
		area += 0.5f * VectorLength( cross );
	}
	if ( area < 1.0f || area > 20000000.0f ) {
		return;
	}

	/* more subdivision may be necessary */
	if ( bouncing ) {
		/* get color sample for the surface fragment */
		RadSample( lightmapNum, ds, lm, si, rw, color, gradient, &style );

		/* if color gradient is too high, subdivide again */
		if ( subdivide > minDiffuseSubdivide &&
			 ( gradient[ 0 ] > RADIOSITY_MAX_GRADIENT || gradient[ 1 ] > RADIOSITY_MAX_GRADIENT || gradient[ 2 ] > RADIOSITY_MAX_GRADIENT ) ) {
			RadSubdivideDiffuseLight( lightmapNum, ds, lm, si, scale, ( subdivide / 2.0f ), qfalse, rw, cw );
			return;
		}
	}

	/* create a regular winding and an average normal */
	w = AllocWinding( rw->numVerts );
	w->numpoints = rw->numVerts;
	VectorClear( normal );
	for ( i = 0; i < rw->numVerts; i++ )
	{
		VectorCopy( rw->verts[ i ].xyz, w->p[ i ] );
		VectorAdd( normal, rw->verts[ i ].normal, normal );
	}
	VectorScale( normal, ( 1.0f / rw->numVerts ), normal );
	if ( VectorNormalize( normal, normal ) == 0.0f ) {
		return;
	}

	/* early out? */
	if ( bouncing && VectorLength( color ) < RADIOSITY_MIN ) {
		return;
	}

	/* debug code */
	//%	Sys_Printf( "Size: %d %d %d\n", (int) (maxs[ 0 ] - mins[ 0 ]), (int) (maxs[ 1 ] - mins[ 1 ]), (int) (maxs[ 2 ] - mins[ 2 ]) );
	//%	Sys_Printf( "Grad: %f %f %f\n", gradient[ 0 ], gradient[ 1 ], gradient[ 2 ] );

	/* increment counts */
	pthread_mutex_lock( &master_mutex );
	numDiffuseLights++;
	pthread_mutex_unlock( &master_mutex );
	switch ( ds->surfaceType )
	{
	case MST_PLANAR:
		pthread_mutex_lock( &master_mutex );
		numBrushDiffuseLights++;
		pthread_mutex_unlock( &master_mutex );
		break;

	case MST_TRIANGLE_SOUP:
		pthread_mutex_lock( &master_mutex );
		numTriangleDiffuseLights++;
		pthread_mutex_unlock( &master_mutex );
		break;

	case MST_PATCH:
		pthread_mutex_lock( &master_mutex );
		numPatchDiffuseLights++;
		pthread_mutex_unlock( &master_mutex );
		break;
	}

	/* create a light */
	light = safe_malloc( sizeof( *light ) );
	memset( light, 0, sizeof( *light ) );

	/* attach it */
	ThreadLock();
	light->next = lights;
	lights = light;
	ThreadUnlock();

	/* initialize the light */
	light->flags = LIGHT_AREA_DEFAULT;
	light->type = EMIT_AREA;
	light->si = si;
	light->fade = 1.0f;
	light->w = w;

	/* set falloff threshold */
	light->falloffTolerance = falloffTolerance;

	/* bouncing light? */
	if ( bouncing == qfalse ) {
		/* handle first-pass lights in normal q3a style */
		value = si->value;
		light->photons = value * area * areaScale;
		light->add = value * formFactorValueScale * areaScale;
		VectorCopy( si->color, light->color );
		VectorScale( light->color, light->add, light->emitColor );
		light->style = noStyles ? LS_NORMAL : si->lightStyle;
		if ( light->style < LS_NORMAL || light->style >= LS_NONE ) {
			light->style = LS_NORMAL;
		}

		/* set origin */
		VectorAdd( mins, maxs, light->origin );
		VectorScale( light->origin, 0.5f, light->origin );

		/* nudge it off the plane a bit */
		VectorCopy( normal, light->normal );
		VectorMA( light->origin, 1.0f, light->normal, light->origin );
		light->dist = DotProduct( light->origin, normal );

		/* optionally create a point splashsplash light for first pass */
		if ( original && si->backsplashFraction > 0 ) {
			/* allocate a new point light */
			splash = safe_malloc( sizeof( *splash ) );
			memset( splash, 0, sizeof( *splash ) );
			splash->next = lights;
			lights = splash;

			/* set it up */
			splash->flags = LIGHT_Q3A_DEFAULT;
			splash->type = EMIT_POINT;
			splash->photons = light->photons * si->backsplashFraction;
			splash->fade = 1.0f;
			splash->si = si;
			VectorMA( light->origin, si->backsplashDistance, normal, splash->origin );
			VectorCopy( si->color, splash->color );
			splash->falloffTolerance = falloffTolerance;
			splash->style = noStyles ? LS_NORMAL : light->style;

			/* add to counts */
			numPointLights++;
		}
	}
	else
	{
		/* handle bounced light (radiosity) a little differently */
		value = RADIOSITY_VALUE * si->bounceScale * 0.375f;
		light->photons = value * area * bounceScale;
		light->add = value * formFactorValueScale * bounceScale;
		VectorCopy( color, light->color );
		VectorScale( light->color, light->add, light->emitColor );
		light->style = noStyles ? LS_NORMAL : style;
		if ( light->style < LS_NORMAL || light->style >= LS_NONE ) {
			light->style = LS_NORMAL;
		}

		/* set origin */
		WindingCenter( w, light->origin );

		/* nudge it off the plane a bit */
		VectorCopy( normal, light->normal );
		VectorMA( light->origin, 1.0f, light->normal, light->origin );
		light->dist = DotProduct( light->origin, normal );
	}

	/* emit light from both sides? */
	if ( si->compileFlags & C_FOG || si->twoSided ) {
		light->flags |= LIGHT_TWOSIDED;
	}

	//%	Sys_Printf( "\nAL: C: (%6f, %6f, %6f) [%6f] N: (%6f, %6f, %6f) %s\n",
	//%		light->color[ 0 ], light->color[ 1 ], light->color[ 2 ], light->add,
	//%		light->normal[ 0 ], light->normal[ 1 ], light->normal[ 2 ],
	//%		light->si->shader );
}



/*
   RadLightForTriangles()
   creates unbounced diffuse lights for triangle soup (misc_models, etc)
 */

void RadLightForTriangles( int num, int lightmapNum, rawLightmap_t *lm, shaderInfo_t *si, float scale, float subdivide, clipWork_t *cw ){
	int i, j, k, v;
	bspDrawSurface_t    *ds;
	surfaceInfo_t       *info;
	float               *radVertexLuxel;
	radWinding_t rw;


	/* get surface */
	ds = &bspDrawSurfaces[ num ];
	info = &surfaceInfos[ num ];

	/* each triangle is a potential emitter */
	rw.numVerts = 3;
	for ( i = 0; i < ds->numIndexes; i += 3 )
	{
		/* copy each vert */
		for ( j = 0; j < 3; j++ )
		{
			/* get vertex index and rad vertex luxel */
			v = ds->firstVert + bspDrawIndexes[ ds->firstIndex + i + j ];

			/* get most everything */
			memcpy( &rw.verts[ j ], &yDrawVerts[ v ], sizeof( bspDrawVert_t ) );

			/* fix colors */
			for ( k = 0; k < MAX_LIGHTMAPS; k++ )
			{
				radVertexLuxel = RAD_VERTEX_LUXEL( k, ds->firstVert + bspDrawIndexes[ ds->firstIndex + i + j ] );
				VectorCopy( radVertexLuxel, rw.verts[ j ].color[ k ] );
				rw.verts[ j ].color[ k ][ 3 ] = yDrawVerts[ v ].color[ k ][ 3 ];
			}
		}

		/* subdivide into area lights */
		RadSubdivideDiffuseLight( lightmapNum, ds, lm, si, scale, subdivide, qtrue, &rw, cw );
	}
}



/*
   RadLightForPatch()
   creates unbounced diffuse lights for patches
 */

#define PLANAR_EPSILON  0.1f

void RadLightForPatch( int num, int lightmapNum, rawLightmap_t *lm, shaderInfo_t *si, float scale, float subdivide, clipWork_t *cw ){
	int i, x, y, v, t, pw[ 5 ], r;
	bspDrawSurface_t    *ds;
	surfaceInfo_t       *info;
	bspDrawVert_t       *bogus;
	bspDrawVert_t       *dv[ 4 ];
	mesh_t src, *subdivided, *mesh;
	float               *radVertexLuxel;
	float dist;
	vec4_t plane;
	qboolean planar;
	radWinding_t rw;


	/* get surface */
	ds = &bspDrawSurfaces[ num ];
	info = &surfaceInfos[ num ];

	/* construct a bogus vert list with color index stuffed into color[ 0 ] */
	bogus = safe_malloc( ds->numVerts * sizeof( bspDrawVert_t ) );
	memcpy( bogus, &yDrawVerts[ ds->firstVert ], ds->numVerts * sizeof( bspDrawVert_t ) );
	for ( i = 0; i < ds->numVerts; i++ )
		bogus[ i ].color[ 0 ][ 0 ] = i;

	/* build a subdivided mesh identical to shadow facets for this patch */
	/* this MUST MATCH FacetsForPatch() identically! */
	src.width = ds->patchWidth;
	src.height = ds->patchHeight;
	src.verts = bogus;
	//%	subdivided = SubdivideMesh( src, 8, 512 );
	subdivided = SubdivideMesh2( src, info->patchIterations );
	PutMeshOnCurve( *subdivided );
	//%	MakeMeshNormals( *subdivided );
	mesh = RemoveLinearMeshColumnsRows( subdivided );
	FreeMesh( subdivided );
	free( bogus );

	/* FIXME: build interpolation table into color[ 1 ] */

	/* fix up color indexes */
	for ( i = 0; i < ( mesh->width * mesh->height ); i++ )
	{
		dv[ 0 ] = &mesh->verts[ i ];
		if ( dv[ 0 ]->color[ 0 ][ 0 ] >= ds->numVerts ) {
			dv[ 0 ]->color[ 0 ][ 0 ] = ds->numVerts - 1;
		}
	}

	/* iterate through the mesh quads */
	for ( y = 0; y < ( mesh->height - 1 ); y++ )
	{
		for ( x = 0; x < ( mesh->width - 1 ); x++ )
		{
			/* set indexes */
			pw[ 0 ] = x + ( y * mesh->width );
			pw[ 1 ] = x + ( ( y + 1 ) * mesh->width );
			pw[ 2 ] = x + 1 + ( ( y + 1 ) * mesh->width );
			pw[ 3 ] = x + 1 + ( y * mesh->width );
			pw[ 4 ] = x + ( y * mesh->width );    /* same as pw[ 0 ] */

			/* set radix */
			r = ( x + y ) & 1;

			/* get drawverts */
			dv[ 0 ] = &mesh->verts[ pw[ r + 0 ] ];
			dv[ 1 ] = &mesh->verts[ pw[ r + 1 ] ];
			dv[ 2 ] = &mesh->verts[ pw[ r + 2 ] ];
			dv[ 3 ] = &mesh->verts[ pw[ r + 3 ] ];

			/* planar? */
			planar = PlaneFromPoints( plane, dv[ 0 ]->xyz, dv[ 1 ]->xyz, dv[ 2 ]->xyz );
			if ( planar ) {
				dist = DotProduct( dv[ 1 ]->xyz, plane ) - plane[ 3 ];
				if ( fabs( dist ) > PLANAR_EPSILON ) {
					planar = qfalse;
				}
			}

			/* generate a quad */
			if ( planar ) {
				rw.numVerts = 4;
				for ( v = 0; v < 4; v++ )
				{
					/* get most everything */
					memcpy( &rw.verts[ v ], dv[ v ], sizeof( bspDrawVert_t ) );

					/* fix colors */
					for ( i = 0; i < MAX_LIGHTMAPS; i++ )
					{
						radVertexLuxel = RAD_VERTEX_LUXEL( i, ds->firstVert + dv[ v ]->color[ 0 ][ 0 ] );
						VectorCopy( radVertexLuxel, rw.verts[ v ].color[ i ] );
						rw.verts[ v ].color[ i ][ 3 ] = dv[ v ]->color[ i ][ 3 ];
					}
				}

				/* subdivide into area lights */
				RadSubdivideDiffuseLight( lightmapNum, ds, lm, si, scale, subdivide, qtrue, &rw, cw );
			}

			/* generate 2 tris */
			else
			{
				rw.numVerts = 3;
				for ( t = 0; t < 2; t++ )
				{
					for ( v = 0; v < 3 + t; v++ )
					{
						/* get "other" triangle (stupid hacky logic, but whatevah) */
						if ( v == 1 && t == 1 ) {
							v++;
						}

						/* get most everything */
						memcpy( &rw.verts[ v ], dv[ v ], sizeof( bspDrawVert_t ) );

						/* fix colors */
						for ( i = 0; i < MAX_LIGHTMAPS; i++ )
						{
							radVertexLuxel = RAD_VERTEX_LUXEL( i, ds->firstVert + dv[ v ]->color[ 0 ][ 0 ] );
							VectorCopy( radVertexLuxel, rw.verts[ v ].color[ i ] );
							rw.verts[ v ].color[ i ][ 3 ] = dv[ v ]->color[ i ][ 3 ];
						}
					}

					/* subdivide into area lights */
					RadSubdivideDiffuseLight( lightmapNum, ds, lm, si, scale, subdivide, qtrue, &rw, cw );
				}
			}
		}
	}

	/* free the mesh */
	FreeMesh( mesh );
}




/*
   RadLight()
   creates unbounced diffuse lights for a given surface
 */

void RadLight( int num ){
	int lightmapNum;
	float scale, subdivide;
	int contentFlags, surfaceFlags, compileFlags;
	bspDrawSurface_t    *ds;
	surfaceInfo_t       *info;
	rawLightmap_t       *lm;
	shaderInfo_t        *si;
	clipWork_t cw;


	/* get drawsurface, lightmap, and shader info */
	ds = &bspDrawSurfaces[ num ];
	info = &surfaceInfos[ num ];
	lm = info->lm;
	si = info->si;
	scale = si->bounceScale;

	/* find nodraw bit */
	contentFlags = surfaceFlags = compileFlags = 0;
	ApplySurfaceParm( "nodraw", &contentFlags, &surfaceFlags, &compileFlags );

	/* early outs? */
	if ( scale <= 0.0f || ( si->compileFlags & C_SKY ) || si->autosprite ||
		 ( bspShaders[ ds->shaderNum ].contentFlags & contentFlags ) || ( bspShaders[ ds->shaderNum ].surfaceFlags & surfaceFlags ) ||
		 ( si->compileFlags & compileFlags ) ) {
		return;
	}

	/* determine how much we need to chop up the surface */
	if ( si->lightSubdivide ) {
		subdivide = si->lightSubdivide;
	}
	else{
		subdivide = diffuseSubdivide;
	}

	/* inc counts */
	pthread_mutex_lock( &master_mutex );
	numDiffuseSurfaces++;
	pthread_mutex_unlock( &master_mutex );

	/* iterate through styles (this could be more efficient, yes) */
	for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
	{
		/* switch on type */
		if ( ds->lightmapStyles[ lightmapNum ] != LS_NONE && ds->lightmapStyles[ lightmapNum ] != LS_UNUSED ) {
			switch ( ds->surfaceType )
			{
			case MST_PLANAR:
			case MST_TRIANGLE_SOUP:
				RadLightForTriangles( num, lightmapNum, lm, si, scale, subdivide, &cw );
				break;

			case MST_PATCH:
				RadLightForPatch( num, lightmapNum, lm, si, scale, subdivide, &cw );
				break;

			default:
				break;
			}
		}
	}
}



/*
   RadCreateDiffuseLights()
   creates lights for unbounced light on surfaces in the bsp
 */

int iterations = 0;

void RadCreateDiffuseLights( void ){
	/* startup */
	Sys_FPrintf( SYS_VRB, "--- RadCreateDiffuseLights ---\n" );
	numDiffuseSurfaces = 0;
	numDiffuseLights = 0;
	numBrushDiffuseLights = 0;
	numTriangleDiffuseLights = 0;
	numPatchDiffuseLights = 0;
	numAreaLights = 0;

	/* hit every surface (threaded) */
	RunThreadsOnIndividual( numBSPDrawSurfaces, qtrue, RadLight );

	/* dump the lights generated to a file */
	if ( dump ) {
		char dumpName[ 1024 ], ext[ 64 ];
		FILE    *file;
		light_t *light;

		strcpy( dumpName, source );
		StripExtension( dumpName );
		sprintf( ext, "_bounce_%03d.map", iterations );
		strcat( dumpName, ext );
		file = fopen( dumpName, "wb" );
		Sys_Printf( "Writing %s...\n", dumpName );
		if ( file ) {
			for ( light = lights; light; light = light->next )
			{
				fprintf( file,
						 "{\n"
						 "\"classname\" \"light\"\n"
						 "\"light\" \"%d\"\n"
						 "\"origin\" \"%.0f %.0f %.0f\"\n"
						 "\"_color\" \"%.3f %.3f %.3f\"\n"
						 "}\n",

						 (int) light->add,

						 light->origin[ 0 ],
						 light->origin[ 1 ],
						 light->origin[ 2 ],

						 light->color[ 0 ],
						 light->color[ 1 ],
						 light->color[ 2 ] );
			}
			fclose( file );
		}
	}

	/* increment */
	iterations++;

	/* print counts */
	Sys_Printf( "%8d diffuse surfaces\n", numDiffuseSurfaces );
	Sys_FPrintf( SYS_VRB, "%8d total diffuse lights\n", numDiffuseLights );
	Sys_FPrintf( SYS_VRB, "%8d brush diffuse lights\n", numBrushDiffuseLights );
	Sys_FPrintf( SYS_VRB, "%8d patch diffuse lights\n", numPatchDiffuseLights );
	Sys_FPrintf( SYS_VRB, "%8d triangle diffuse lights\n", numTriangleDiffuseLights );
}
