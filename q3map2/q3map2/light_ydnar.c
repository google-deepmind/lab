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
#define LIGHT_YDNAR_C



/* dependencies */
#include "q3map2.h"




/*
   ColorToBytes()
   ydnar: moved to here 2001-02-04
 */

void ColorToBytes( const float *color, byte *colorBytes, float scale ){
	int i;
	float max, gamma;
	vec3_t sample;


	/* ydnar: scaling necessary for simulating r_overbrightBits on external lightmaps */
	if ( scale <= 0.0f ) {
		scale = 1.0f;
	}

	/* make a local copy */
	VectorScale( color, scale, sample );

	/* muck with it */
	gamma = 1.0f / lightmapGamma;
	for ( i = 0; i < 3; i++ )
	{
		/* handle negative light */
		if ( sample[ i ] < 0.0f ) {
			sample[ i ] = 0.0f;
			continue;
		}

		/* gamma */
		sample[ i ] = pow( sample[ i ] / 255.0f, gamma ) * 255.0f;
	}

	/* clamp with color normalization */
	max = sample[ 0 ];
	if ( sample[ 1 ] > max ) {
		max = sample[ 1 ];
	}
	if ( sample[ 2 ] > max ) {
		max = sample[ 2 ];
	}
	if ( max > 255.0f ) {
		VectorScale( sample, ( 255.0f / max ), sample );
	}

	/* compensate for ingame overbrighting/bitshifting */
	VectorScale( sample, ( 1.0f / lightmapCompensate ), sample );

	/* store it off */
	colorBytes[ 0 ] = sample[ 0 ];
	colorBytes[ 1 ] = sample[ 1 ];
	colorBytes[ 2 ] = sample[ 2 ];
}



/* -------------------------------------------------------------------------------

   this section deals with phong shading (normal interpolation across brush faces)

   ------------------------------------------------------------------------------- */

/*
   SmoothNormals()
   smooths together coincident vertex normals across the bsp
 */

#define MAX_SAMPLES             256
#define THETA_EPSILON           0.000001
#define EQUAL_NORMAL_EPSILON    0.01

void SmoothNormals( void ){
	int i, j, k, f, cs, numVerts, numVotes, fOld, start;
	float shadeAngle, defaultShadeAngle, maxShadeAngle, dot, testAngle;
	bspDrawSurface_t    *ds;
	shaderInfo_t        *si;
	float               *shadeAngles;
	byte                *smoothed;
	vec3_t average, diff;
	int indexes[ MAX_SAMPLES ];
	vec3_t votes[ MAX_SAMPLES ];


	/* allocate shade angle table */
	shadeAngles = safe_malloc( numBSPDrawVerts * sizeof( float ) );
	memset( shadeAngles, 0, numBSPDrawVerts * sizeof( float ) );

	/* allocate smoothed table */
	cs = ( numBSPDrawVerts / 8 ) + 1;
	smoothed = safe_malloc( cs );
	memset( smoothed, 0, cs );

	/* set default shade angle */
	defaultShadeAngle = DEG2RAD( shadeAngleDegrees );
	maxShadeAngle = 0;

	/* run through every surface and flag verts belonging to non-lightmapped surfaces
	   and set per-vertex smoothing angle */
	for ( i = 0; i < numBSPDrawSurfaces; i++ )
	{
		/* get drawsurf */
		ds = &bspDrawSurfaces[ i ];

		/* get shader for shade angle */
		si = surfaceInfos[ i ].si;
		if ( si->shadeAngleDegrees ) {
			shadeAngle = DEG2RAD( si->shadeAngleDegrees );
		}
		else{
			shadeAngle = defaultShadeAngle;
		}
		if ( shadeAngle > maxShadeAngle ) {
			maxShadeAngle = shadeAngle;
		}

		/* flag its verts */
		for ( j = 0; j < ds->numVerts; j++ )
		{
			f = ds->firstVert + j;
			shadeAngles[ f ] = shadeAngle;
			if ( ds->surfaceType == MST_TRIANGLE_SOUP ) {
				smoothed[ f >> 3 ] |= ( 1 << ( f & 7 ) );
			}
		}

		/* ydnar: optional force-to-trisoup */
		if ( trisoup && ds->surfaceType == MST_PLANAR ) {
			ds->surfaceType = MST_TRIANGLE_SOUP;
			ds->lightmapNum[ 0 ] = -3;
		}
	}

	/* bail if no surfaces have a shade angle */
	if ( maxShadeAngle == 0 ) {
		free( shadeAngles );
		free( smoothed );
		return;
	}

	/* init pacifier */
	fOld = -1;
	start = I_FloatTime();

	/* go through the list of vertexes */
	for ( i = 0; i < numBSPDrawVerts; i++ )
	{
		/* print pacifier */
		f = 10 * i / numBSPDrawVerts;
		if ( f != fOld ) {
			fOld = f;
			Sys_Printf( "%i...", f );
		}

		/* already smoothed? */
		if ( smoothed[ i >> 3 ] & ( 1 << ( i & 7 ) ) ) {
			continue;
		}

		/* clear */
		VectorClear( average );
		numVerts = 0;
		numVotes = 0;

		/* build a table of coincident vertexes */
		for ( j = i; j < numBSPDrawVerts && numVerts < MAX_SAMPLES; j++ )
		{
			/* already smoothed? */
			if ( smoothed[ j >> 3 ] & ( 1 << ( j & 7 ) ) ) {
				continue;
			}

			/* test vertexes */
			if ( VectorCompare( yDrawVerts[ i ].xyz, yDrawVerts[ j ].xyz ) == qfalse ) {
				continue;
			}

			/* use smallest shade angle */
			shadeAngle = ( shadeAngles[ i ] < shadeAngles[ j ] ? shadeAngles[ i ] : shadeAngles[ j ] );

			/* check shade angle */
			dot = DotProduct( bspDrawVerts[ i ].normal, bspDrawVerts[ j ].normal );
			if ( dot > 1.0 ) {
				dot = 1.0;
			}
			else if ( dot < -1.0 ) {
				dot = -1.0;
			}
			testAngle = acos( dot ) + THETA_EPSILON;
			if ( testAngle >= shadeAngle ) {
				//Sys_Printf( "F(%3.3f >= %3.3f) ", RAD2DEG( testAngle ), RAD2DEG( shadeAngle ) );
				continue;
			}
			//Sys_Printf( "P(%3.3f < %3.3f) ", RAD2DEG( testAngle ), RAD2DEG( shadeAngle ) );

			/* add to the list */
			indexes[ numVerts++ ] = j;

			/* flag vertex */
			smoothed[ j >> 3 ] |= ( 1 << ( j & 7 ) );

			/* see if this normal has already been voted */
			for ( k = 0; k < numVotes; k++ )
			{
				VectorSubtract( bspDrawVerts[ j ].normal, votes[ k ], diff );
				if ( fabs( diff[ 0 ] ) < EQUAL_NORMAL_EPSILON &&
					 fabs( diff[ 1 ] ) < EQUAL_NORMAL_EPSILON &&
					 fabs( diff[ 2 ] ) < EQUAL_NORMAL_EPSILON ) {
					break;
				}
			}

			/* add a new vote? */
			if ( k == numVotes && numVotes < MAX_SAMPLES ) {
				VectorAdd( average, bspDrawVerts[ j ].normal, average );
				VectorCopy( bspDrawVerts[ j ].normal, votes[ numVotes ] );
				numVotes++;
			}
		}

		/* don't average for less than 2 verts */
		if ( numVerts < 2 ) {
			continue;
		}

		/* average normal */
		if ( VectorNormalize( average, average ) > 0 ) {
			/* smooth */
			for ( j = 0; j < numVerts; j++ )
				VectorCopy( average, yDrawVerts[ indexes[ j ] ].normal );
		}
	}

	/* free the tables */
	free( shadeAngles );
	free( smoothed );

	/* print time */
	Sys_Printf( " (%i)\n", (int) ( I_FloatTime() - start ) );
}



/* -------------------------------------------------------------------------------

   this section deals with phong shaded lightmap tracing

   ------------------------------------------------------------------------------- */

/* 9th rewrite (recursive subdivision of a lightmap triangle) */

/*
   CalcTangentVectors()
   calculates the st tangent vectors for normalmapping
 */

static qboolean CalcTangentVectors( int numVerts, bspDrawVert_t **dv, vec3_t *stv, vec3_t *ttv ){
	int i;
	float bb, s, t;
	vec3_t bary;


	/* calculate barycentric basis for the triangle */
	bb = ( dv[ 1 ]->st[ 0 ] - dv[ 0 ]->st[ 0 ] ) * ( dv[ 2 ]->st[ 1 ] - dv[ 0 ]->st[ 1 ] ) - ( dv[ 2 ]->st[ 0 ] - dv[ 0 ]->st[ 0 ] ) * ( dv[ 1 ]->st[ 1 ] - dv[ 0 ]->st[ 1 ] );
	if ( fabs( bb ) < 0.00000001f ) {
		return qfalse;
	}

	/* do each vertex */
	for ( i = 0; i < numVerts; i++ )
	{
		/* calculate s tangent vector */
		s = dv[ i ]->st[ 0 ] + 10.0f;
		t = dv[ i ]->st[ 1 ];
		bary[ 0 ] = ( ( dv[ 1 ]->st[ 0 ] - s ) * ( dv[ 2 ]->st[ 1 ] - t ) - ( dv[ 2 ]->st[ 0 ] - s ) * ( dv[ 1 ]->st[ 1 ] - t ) ) / bb;
		bary[ 1 ] = ( ( dv[ 2 ]->st[ 0 ] - s ) * ( dv[ 0 ]->st[ 1 ] - t ) - ( dv[ 0 ]->st[ 0 ] - s ) * ( dv[ 2 ]->st[ 1 ] - t ) ) / bb;
		bary[ 2 ] = ( ( dv[ 0 ]->st[ 0 ] - s ) * ( dv[ 1 ]->st[ 1 ] - t ) - ( dv[ 1 ]->st[ 0 ] - s ) * ( dv[ 0 ]->st[ 1 ] - t ) ) / bb;

		stv[ i ][ 0 ] = bary[ 0 ] * dv[ 0 ]->xyz[ 0 ] + bary[ 1 ] * dv[ 1 ]->xyz[ 0 ] + bary[ 2 ] * dv[ 2 ]->xyz[ 0 ];
		stv[ i ][ 1 ] = bary[ 0 ] * dv[ 0 ]->xyz[ 1 ] + bary[ 1 ] * dv[ 1 ]->xyz[ 1 ] + bary[ 2 ] * dv[ 2 ]->xyz[ 1 ];
		stv[ i ][ 2 ] = bary[ 0 ] * dv[ 0 ]->xyz[ 2 ] + bary[ 1 ] * dv[ 1 ]->xyz[ 2 ] + bary[ 2 ] * dv[ 2 ]->xyz[ 2 ];

		VectorSubtract( stv[ i ], dv[ i ]->xyz, stv[ i ] );
		VectorNormalize( stv[ i ], stv[ i ] );

		/* calculate t tangent vector */
		s = dv[ i ]->st[ 0 ];
		t = dv[ i ]->st[ 1 ] + 10.0f;
		bary[ 0 ] = ( ( dv[ 1 ]->st[ 0 ] - s ) * ( dv[ 2 ]->st[ 1 ] - t ) - ( dv[ 2 ]->st[ 0 ] - s ) * ( dv[ 1 ]->st[ 1 ] - t ) ) / bb;
		bary[ 1 ] = ( ( dv[ 2 ]->st[ 0 ] - s ) * ( dv[ 0 ]->st[ 1 ] - t ) - ( dv[ 0 ]->st[ 0 ] - s ) * ( dv[ 2 ]->st[ 1 ] - t ) ) / bb;
		bary[ 2 ] = ( ( dv[ 0 ]->st[ 0 ] - s ) * ( dv[ 1 ]->st[ 1 ] - t ) - ( dv[ 1 ]->st[ 0 ] - s ) * ( dv[ 0 ]->st[ 1 ] - t ) ) / bb;

		ttv[ i ][ 0 ] = bary[ 0 ] * dv[ 0 ]->xyz[ 0 ] + bary[ 1 ] * dv[ 1 ]->xyz[ 0 ] + bary[ 2 ] * dv[ 2 ]->xyz[ 0 ];
		ttv[ i ][ 1 ] = bary[ 0 ] * dv[ 0 ]->xyz[ 1 ] + bary[ 1 ] * dv[ 1 ]->xyz[ 1 ] + bary[ 2 ] * dv[ 2 ]->xyz[ 1 ];
		ttv[ i ][ 2 ] = bary[ 0 ] * dv[ 0 ]->xyz[ 2 ] + bary[ 1 ] * dv[ 1 ]->xyz[ 2 ] + bary[ 2 ] * dv[ 2 ]->xyz[ 2 ];

		VectorSubtract( ttv[ i ], dv[ i ]->xyz, ttv[ i ] );
		VectorNormalize( ttv[ i ], ttv[ i ] );

		/* debug code */
		//%	Sys_FPrintf( SYS_VRB, "%d S: (%f %f %f) T: (%f %f %f)\n", i,
		//%		stv[ i ][ 0 ], stv[ i ][ 1 ], stv[ i ][ 2 ], ttv[ i ][ 0 ], ttv[ i ][ 1 ], ttv[ i ][ 2 ] );
	}

	/* return to caller */
	return qtrue;
}




/*
   PerturbNormal()
   perterbs the normal by the shader's normalmap in tangent space
 */

static void PerturbNormal( bspDrawVert_t *dv, shaderInfo_t *si, vec3_t pNormal, vec3_t stv[ 3 ], vec3_t ttv[ 3 ] ){
	int i;
	vec4_t bump;


	/* passthrough */
	VectorCopy( dv->normal, pNormal );

	/* sample normalmap */
	if ( RadSampleImage( si->normalImage->pixels, si->normalImage->width, si->normalImage->height, dv->st, bump ) == qfalse ) {
		return;
	}

	/* remap sampled normal from [0,255] to [-1,-1] */
	for ( i = 0; i < 3; i++ )
		bump[ i ] = ( bump[ i ] - 127.0f ) * ( 1.0f / 127.5f );

	/* scale tangent vectors and add to original normal */
	VectorMA( dv->normal, bump[ 0 ], stv[ 0 ], pNormal );
	VectorMA( pNormal, bump[ 1 ], ttv[ 0 ], pNormal );
	VectorMA( pNormal, bump[ 2 ], dv->normal, pNormal );

	/* renormalize and return */
	VectorNormalize( pNormal, pNormal );
}



/*
   MapSingleLuxel()
   maps a luxel for triangle bv at
 */

#define NUDGE           0.5f
#define BOGUS_NUDGE     -99999.0f

static int MapSingleLuxel( rawLightmap_t *lm, surfaceInfo_t *info, bspDrawVert_t *dv, vec4_t plane, float pass, vec3_t stv[ 3 ], vec3_t ttv[ 3 ] ){
	int i, x, y, numClusters, *clusters, pointCluster, *cluster;
	float           *luxel, *origin, *normal, d, lightmapSampleOffset;
	shaderInfo_t    *si;
	vec3_t pNormal;
	vec3_t vecs[ 3 ];
	vec3_t nudged;
	float           *nudge;
	static float nudges[][ 2 ] =
	{
		//%{ 0, 0 },		/* try center first */
		{ -NUDGE, 0 },                      /* left */
		{ NUDGE, 0 },                       /* right */
		{ 0, NUDGE },                       /* up */
		{ 0, -NUDGE },                      /* down */
		{ -NUDGE, NUDGE },                  /* left/up */
		{ NUDGE, -NUDGE },                  /* right/down */
		{ NUDGE, NUDGE },                   /* right/up */
		{ -NUDGE, -NUDGE },                 /* left/down */
		{ BOGUS_NUDGE, BOGUS_NUDGE }
	};


	/* find luxel xy coords (fixme: subtract 0.5?) */
	x = dv->lightmap[ 0 ][ 0 ];
	y = dv->lightmap[ 0 ][ 1 ];
	if ( x < 0 ) {
		x = 0;
	}
	else if ( x >= lm->sw ) {
		x = lm->sw - 1;
	}
	if ( y < 0 ) {
		y = 0;
	}
	else if ( y >= lm->sh ) {
		y = lm->sh - 1;
	}

	/* set shader and cluster list */
	if ( info != NULL ) {
		si = info->si;
		numClusters = info->numSurfaceClusters;
		clusters = &surfaceClusters[ info->firstSurfaceCluster ];
	}
	else
	{
		si = NULL;
		numClusters = 0;
		clusters = NULL;
	}

	/* get luxel, origin, cluster, and normal */
	luxel = SUPER_LUXEL( 0, x, y );
	origin = SUPER_ORIGIN( x, y );
	normal = SUPER_NORMAL( x, y );
	cluster = SUPER_CLUSTER( x, y );

	/* don't attempt to remap occluded luxels for planar surfaces */
	if ( ( *cluster ) == CLUSTER_OCCLUDED && lm->plane != NULL ) {
		return ( *cluster );
	}

	/* only average the normal for premapped luxels */
	else if ( ( *cluster ) >= 0 ) {
		/* do bumpmap calculations */
		if ( stv != NULL ) {
			PerturbNormal( dv, si, pNormal, stv, ttv );
		}
		else{
			VectorCopy( dv->normal, pNormal );
		}

		/* add the additional normal data */
		VectorAdd( normal, pNormal, normal );
		luxel[ 3 ] += 1.0f;
		return ( *cluster );
	}

	/* otherwise, unmapped luxels (*cluster == CLUSTER_UNMAPPED) will have their full attributes calculated */

	/* get origin */

	/* axial lightmap projection */
	if ( lm->vecs != NULL ) {
		/* calculate an origin for the sample from the lightmap vectors */
		VectorCopy( lm->origin, origin );
		for ( i = 0; i < 3; i++ )
		{
			/* add unless it's the axis, which is taken care of later */
			if ( i == lm->axisNum ) {
				continue;
			}
			origin[ i ] += ( x * lm->vecs[ 0 ][ i ] ) + ( y * lm->vecs[ 1 ][ i ] );
		}

		/* project the origin onto the plane */
		d = DotProduct( origin, plane ) - plane[ 3 ];
		d /= plane[ lm->axisNum ];
		origin[ lm->axisNum ] -= d;
	}

	/* non axial lightmap projection (explicit xyz) */
	else{
		VectorCopy( dv->xyz, origin );
	}

	/* planar surfaces have precalculated lightmap vectors for nudging */
	if ( lm->plane != NULL ) {
		VectorCopy( lm->vecs[ 0 ], vecs[ 0 ] );
		VectorCopy( lm->vecs[ 1 ], vecs[ 1 ] );
		VectorCopy( lm->plane, vecs[ 2 ] );
	}

	/* non-planar surfaces must calculate them */
	else
	{
		if ( plane != NULL ) {
			VectorCopy( plane, vecs[ 2 ] );
		}
		else{
			VectorCopy( dv->normal, vecs[ 2 ] );
		}
		MakeNormalVectors( vecs[ 2 ], vecs[ 0 ], vecs[ 1 ] );
	}

	/* push the origin off the surface a bit */
	if ( si != NULL ) {
		lightmapSampleOffset = si->lightmapSampleOffset;
	}
	else{
		lightmapSampleOffset = DEFAULT_LIGHTMAP_SAMPLE_OFFSET;
	}
	if ( lm->axisNum < 0 ) {
		VectorMA( origin, lightmapSampleOffset, vecs[ 2 ], origin );
	}
	else if ( vecs[ 2 ][ lm->axisNum ] < 0.0f ) {
		origin[ lm->axisNum ] -= lightmapSampleOffset;
	}
	else{
		origin[ lm->axisNum ] += lightmapSampleOffset;
	}

	/* get cluster */
	pointCluster = ClusterForPointExtFilter( origin, LUXEL_EPSILON, numClusters, clusters );

	/* another retarded hack, storing nudge count in luxel[ 1 ] */
	luxel[ 1 ] = 0.0f;

	/* point in solid? (except in dark mode) */
	if ( pointCluster < 0 && dark == qfalse ) {
		/* nudge the the location around */
		nudge = nudges[ 0 ];
		while ( nudge[ 0 ] > BOGUS_NUDGE && pointCluster < 0 )
		{
			/* nudge the vector around a bit */
			for ( i = 0; i < 3; i++ )
			{
				/* set nudged point*/
				nudged[ i ] = origin[ i ] + ( nudge[ 0 ] * vecs[ 0 ][ i ] ) + ( nudge[ 1 ] * vecs[ 1 ][ i ] );
			}
			nudge += 2;

			/* get pvs cluster */
			pointCluster = ClusterForPointExtFilter( nudged, LUXEL_EPSILON, numClusters, clusters ); //% + 0.625 );
			if ( pointCluster >= 0 ) {
				VectorCopy( nudged, origin );
			}
			luxel[ 1 ] += 1.0f;
		}
	}

	/* as a last resort, if still in solid, try drawvert origin offset by normal (except in dark mode) */
	if ( pointCluster < 0 && si != NULL && dark == qfalse ) {
		VectorMA( dv->xyz, lightmapSampleOffset, dv->normal, nudged );
		pointCluster = ClusterForPointExtFilter( nudged, LUXEL_EPSILON, numClusters, clusters );
		if ( pointCluster >= 0 ) {
			VectorCopy( nudged, origin );
		}
		luxel[ 1 ] += 1.0f;
	}

	/* valid? */
	if ( pointCluster < 0 ) {
		( *cluster ) = CLUSTER_OCCLUDED;
		VectorClear( origin );
		VectorClear( normal );
		pthread_mutex_lock( &master_mutex );
		numLuxelsOccluded++;
		pthread_mutex_unlock( &master_mutex );
		return ( *cluster );
	}

	/* debug code */
	//%	Sys_Printf( "%f %f %f\n", origin[ 0 ], origin[ 1 ], origin[ 2 ] );

	/* do bumpmap calculations */
	if ( stv ) {
		PerturbNormal( dv, si, pNormal, stv, ttv );
	}
	else{
		VectorCopy( dv->normal, pNormal );
	}

	/* store the cluster and normal */
	( *cluster ) = pointCluster;
	VectorCopy( pNormal, normal );

	/* store explicit mapping pass and implicit mapping pass */
	luxel[ 0 ] = pass;
	luxel[ 3 ] = 1.0f;

	/* add to count */
	pthread_mutex_lock( &master_mutex );
	numLuxelsMapped++;
	pthread_mutex_unlock( &master_mutex );

	/* return ok */
	return ( *cluster );
}



/*
   MapTriangle_r()
   recursively subdivides a triangle until its edges are shorter
   than the distance between two luxels (thanks jc :)
 */

static void MapTriangle_r( rawLightmap_t *lm, surfaceInfo_t *info, bspDrawVert_t *dv[ 3 ], vec4_t plane, vec3_t stv[ 3 ], vec3_t ttv[ 3 ] ){
	bspDrawVert_t mid, *dv2[ 3 ];
	int max;


	/* map the vertexes */
	#if 0
	MapSingleLuxel( lm, info, dv[ 0 ], plane, 1, stv, ttv );
	MapSingleLuxel( lm, info, dv[ 1 ], plane, 1, stv, ttv );
	MapSingleLuxel( lm, info, dv[ 2 ], plane, 1, stv, ttv );
	#endif

	/* subdivide calc */
	{
		int i;
		float       *a, *b, dx, dy, dist, maxDist;


		/* find the longest edge and split it */
		max = -1;
		maxDist = 0;
		for ( i = 0; i < 3; i++ )
		{
			/* get verts */
			a = dv[ i ]->lightmap[ 0 ];
			b = dv[ ( i + 1 ) % 3 ]->lightmap[ 0 ];

			/* get dists */
			dx = a[ 0 ] - b[ 0 ];
			dy = a[ 1 ] - b[ 1 ];
			dist = ( dx * dx ) + ( dy * dy );   //% sqrt( (dx * dx) + (dy * dy) );

			/* longer? */
			if ( dist > maxDist ) {
				maxDist = dist;
				max = i;
			}
		}

		/* try to early out */
		if ( max < 0 || maxDist <= subdivideThreshold ) { /* ydnar: was i < 0 instead of max < 0 (?) */
			return;
		}
	}

	/* split the longest edge and map it */
	LerpDrawVert( dv[ max ], dv[ ( max + 1 ) % 3 ], &mid );
	MapSingleLuxel( lm, info, &mid, plane, 1, stv, ttv );

	/* push the point up a little bit to account for fp creep (fixme: revisit this) */
	//%	VectorMA( mid.xyz, 2.0f, mid.normal, mid.xyz );

	/* recurse to first triangle */
	VectorCopy( dv, dv2 );
	dv2[ max ] = &mid;
	MapTriangle_r( lm, info, dv2, plane, stv, ttv );

	/* recurse to second triangle */
	VectorCopy( dv, dv2 );
	dv2[ ( max + 1 ) % 3 ] = &mid;
	MapTriangle_r( lm, info, dv2, plane, stv, ttv );
}



/*
   MapTriangle()
   seed function for MapTriangle_r()
   requires a cw ordered triangle
 */

static qboolean MapTriangle( rawLightmap_t *lm, surfaceInfo_t *info, bspDrawVert_t *dv[ 3 ], qboolean mapNonAxial ){
	int i;
	vec4_t plane;
	vec3_t          *stv, *ttv, stvStatic[ 3 ], ttvStatic[ 3 ];


	/* get plane if possible */
	if ( lm->plane != NULL ) {
		VectorCopy( lm->plane, plane );
		plane[ 3 ] = lm->plane[ 3 ];
	}

	/* otherwise make one from the points */
	else if ( PlaneFromPoints( plane, dv[ 0 ]->xyz, dv[ 1 ]->xyz, dv[ 2 ]->xyz ) == qfalse ) {
		return qfalse;
	}

	/* check to see if we need to calculate texture->world tangent vectors */
	if ( info->si->normalImage != NULL && CalcTangentVectors( 3, dv, stvStatic, ttvStatic ) ) {
		stv = stvStatic;
		ttv = ttvStatic;
	}
	else
	{
		stv = NULL;
		ttv = NULL;
	}

	/* map the vertexes */
	MapSingleLuxel( lm, info, dv[ 0 ], plane, 1, stv, ttv );
	MapSingleLuxel( lm, info, dv[ 1 ], plane, 1, stv, ttv );
	MapSingleLuxel( lm, info, dv[ 2 ], plane, 1, stv, ttv );

	/* 2002-11-20: prefer axial triangle edges */
	if ( mapNonAxial ) {
		/* subdivide the triangle */
		MapTriangle_r( lm, info, dv, plane, stv, ttv );
		return qtrue;
	}

	for ( i = 0; i < 3; i++ )
	{
		float           *a, *b;
		bspDrawVert_t   *dv2[ 3 ];


		/* get verts */
		a = dv[ i ]->lightmap[ 0 ];
		b = dv[ ( i + 1 ) % 3 ]->lightmap[ 0 ];

		/* make degenerate triangles for mapping edges */
		if ( fabs( a[ 0 ] - b[ 0 ] ) < 0.01f || fabs( a[ 1 ] - b[ 1 ] ) < 0.01f ) {
			dv2[ 0 ] = dv[ i ];
			dv2[ 1 ] = dv[ ( i + 1 ) % 3 ];
			dv2[ 2 ] = dv[ ( i + 1 ) % 3 ];

			/* map the degenerate triangle */
			MapTriangle_r( lm, info, dv2, plane, stv, ttv );
		}
	}

	return qtrue;
}



/*
   MapQuad_r()
   recursively subdivides a quad until its edges are shorter
   than the distance between two luxels
 */

static void MapQuad_r( rawLightmap_t *lm, surfaceInfo_t *info, bspDrawVert_t *dv[ 4 ], vec4_t plane, vec3_t stv[ 4 ], vec3_t ttv[ 4 ] ){
	bspDrawVert_t mid[ 2 ], *dv2[ 4 ];
	int max;


	/* subdivide calc */
	{
		int i;
		float       *a, *b, dx, dy, dist, maxDist;


		/* find the longest edge and split it */
		max = -1;
		maxDist = 0;
		for ( i = 0; i < 4; i++ )
		{
			/* get verts */
			a = dv[ i ]->lightmap[ 0 ];
			b = dv[ ( i + 1 ) % 4 ]->lightmap[ 0 ];

			/* get dists */
			dx = a[ 0 ] - b[ 0 ];
			dy = a[ 1 ] - b[ 1 ];
			dist = ( dx * dx ) + ( dy * dy );   //% sqrt( (dx * dx) + (dy * dy) );

			/* longer? */
			if ( dist > maxDist ) {
				maxDist = dist;
				max = i;
			}
		}

		/* try to early out */
		if ( max < 0 || maxDist <= subdivideThreshold ) {
			return;
		}
	}

	/* we only care about even/odd edges */
	max &= 1;

	/* split the longest edges */
	LerpDrawVert( dv[ max ], dv[ ( max + 1 ) % 4 ], &mid[ 0 ] );
	LerpDrawVert( dv[ max + 2 ], dv[ ( max + 3 ) % 4 ], &mid[ 1 ] );

	/* map the vertexes */
	MapSingleLuxel( lm, info, &mid[ 0 ], plane, 1, stv, ttv );
	MapSingleLuxel( lm, info, &mid[ 1 ], plane, 1, stv, ttv );

	/* 0 and 2 */
	if ( max == 0 ) {
		/* recurse to first quad */
		dv2[ 0 ] = dv[ 0 ];
		dv2[ 1 ] = &mid[ 0 ];
		dv2[ 2 ] = &mid[ 1 ];
		dv2[ 3 ] = dv[ 3 ];
		MapQuad_r( lm, info, dv2, plane, stv, ttv );

		/* recurse to second quad */
		dv2[ 0 ] = &mid[ 0 ];
		dv2[ 1 ] = dv[ 1 ];
		dv2[ 2 ] = dv[ 2 ];
		dv2[ 3 ] = &mid[ 1 ];
		MapQuad_r( lm, info, dv2, plane, stv, ttv );
	}

	/* 1 and 3 */
	else
	{
		/* recurse to first quad */
		dv2[ 0 ] = dv[ 0 ];
		dv2[ 1 ] = dv[ 1 ];
		dv2[ 2 ] = &mid[ 0 ];
		dv2[ 3 ] = &mid[ 1 ];
		MapQuad_r( lm, info, dv2, plane, stv, ttv );

		/* recurse to second quad */
		dv2[ 0 ] = &mid[ 1 ];
		dv2[ 1 ] = &mid[ 0 ];
		dv2[ 2 ] = dv[ 2 ];
		dv2[ 3 ] = dv[ 3 ];
		MapQuad_r( lm, info, dv2, plane, stv, ttv );
	}
}



/*
   MapQuad()
   seed function for MapQuad_r()
   requires a cw ordered triangle quad
 */

#define QUAD_PLANAR_EPSILON     0.5f

static qboolean MapQuad( rawLightmap_t *lm, surfaceInfo_t *info, bspDrawVert_t *dv[ 4 ] ){
	float dist;
	vec4_t plane;
	vec3_t          *stv, *ttv, stvStatic[ 4 ], ttvStatic[ 4 ];


	/* get plane if possible */
	if ( lm->plane != NULL ) {
		VectorCopy( lm->plane, plane );
		plane[ 3 ] = lm->plane[ 3 ];
	}

	/* otherwise make one from the points */
	else if ( PlaneFromPoints( plane, dv[ 0 ]->xyz, dv[ 1 ]->xyz, dv[ 2 ]->xyz ) == qfalse ) {
		return qfalse;
	}

	/* 4th point must fall on the plane */
	dist = DotProduct( plane, dv[ 3 ]->xyz ) - plane[ 3 ];
	if ( fabs( dist ) > QUAD_PLANAR_EPSILON ) {
		return qfalse;
	}

	/* check to see if we need to calculate texture->world tangent vectors */
	if ( info->si->normalImage != NULL && CalcTangentVectors( 4, dv, stvStatic, ttvStatic ) ) {
		stv = stvStatic;
		ttv = ttvStatic;
	}
	else
	{
		stv = NULL;
		ttv = NULL;
	}

	/* map the vertexes */
	MapSingleLuxel( lm, info, dv[ 0 ], plane, 1, stv, ttv );
	MapSingleLuxel( lm, info, dv[ 1 ], plane, 1, stv, ttv );
	MapSingleLuxel( lm, info, dv[ 2 ], plane, 1, stv, ttv );
	MapSingleLuxel( lm, info, dv[ 3 ], plane, 1, stv, ttv );

	/* subdivide the quad */
	MapQuad_r( lm, info, dv, plane, stv, ttv );
	return qtrue;
}



/*
   MapRawLightmap()
   maps the locations, normals, and pvs clusters for a raw lightmap
 */

#define VectorDivide( in, d, out )  VectorScale( in, ( 1.0f / ( d ) ), out )    //%	(out)[ 0 ] = (in)[ 0 ] / (d), (out)[ 1 ] = (in)[ 1 ] / (d), (out)[ 2 ] = (in)[ 2 ] / (d)

void MapRawLightmap( int rawLightmapNum ){
	int n, num, i, x, y, sx, sy, pw[ 5 ], r, *cluster, mapNonAxial;
	float               *luxel, *origin, *normal, samples, radius, pass;
	rawLightmap_t       *lm;
	bspDrawSurface_t    *ds;
	surfaceInfo_t       *info;
	mesh_t src, *subdivided, *mesh;
	bspDrawVert_t       *verts, *dv[ 4 ], fake;


	/* bail if this number exceeds the number of raw lightmaps */
	if ( rawLightmapNum >= numRawLightmaps ) {
		return;
	}

	/* get lightmap */
	lm = &rawLightmaps[ rawLightmapNum ];

	/* -----------------------------------------------------------------
	   map referenced surfaces onto the raw lightmap
	   ----------------------------------------------------------------- */

	/* walk the list of surfaces on this raw lightmap */
	for ( n = 0; n < lm->numLightSurfaces; n++ )
	{
		/* with > 1 surface per raw lightmap, clear occluded */
		if ( n > 0 ) {
			for ( y = 0; y < lm->sh; y++ )
			{
				for ( x = 0; x < lm->sw; x++ )
				{
					/* get cluster */
					cluster = SUPER_CLUSTER( x, y );
					if ( *cluster < 0 ) {
						*cluster = CLUSTER_UNMAPPED;
					}
				}
			}
		}

		/* get surface */
		num = lightSurfaces[ lm->firstLightSurface + n ];
		ds = &bspDrawSurfaces[ num ];
		info = &surfaceInfos[ num ];

		/* bail if no lightmap to calculate */
		if ( info->lm != lm ) {
			Sys_Printf( "!" );
			continue;
		}

		/* map the surface onto the lightmap origin/cluster/normal buffers */
		switch ( ds->surfaceType )
		{
		case MST_PLANAR:
			/* get verts */
			verts = yDrawVerts + ds->firstVert;

			/* map the triangles */
			for ( mapNonAxial = 0; mapNonAxial < 2; mapNonAxial++ )
			{
				for ( i = 0; i < ds->numIndexes; i += 3 )
				{
					dv[ 0 ] = &verts[ bspDrawIndexes[ ds->firstIndex + i ] ];
					dv[ 1 ] = &verts[ bspDrawIndexes[ ds->firstIndex + i + 1 ] ];
					dv[ 2 ] = &verts[ bspDrawIndexes[ ds->firstIndex + i + 2 ] ];
					MapTriangle( lm, info, dv, mapNonAxial );
				}
			}
			break;

		case MST_PATCH:
			/* make a mesh from the drawsurf */
			src.width = ds->patchWidth;
			src.height = ds->patchHeight;
			src.verts = &yDrawVerts[ ds->firstVert ];
			//%	subdivided = SubdivideMesh( src, 8, 512 );
			subdivided = SubdivideMesh2( src, info->patchIterations );

			/* fit it to the curve and remove colinear verts on rows/columns */
			PutMeshOnCurve( *subdivided );
			mesh = RemoveLinearMeshColumnsRows( subdivided );
			FreeMesh( subdivided );

			/* get verts */
			verts = mesh->verts;

			/* debug code */
				#if 0
			if ( lm->plane ) {
				Sys_Printf( "Planar patch: [%1.3f %1.3f %1.3f] [%1.3f %1.3f %1.3f] [%1.3f %1.3f %1.3f]\n",
							lm->plane[ 0 ], lm->plane[ 1 ], lm->plane[ 2 ],
							lm->vecs[ 0 ][ 0 ], lm->vecs[ 0 ][ 1 ], lm->vecs[ 0 ][ 2 ],
							lm->vecs[ 1 ][ 0 ], lm->vecs[ 1 ][ 1 ], lm->vecs[ 1 ][ 2 ] );
			}
				#endif

			/* map the mesh quads */
				#if 0

			for ( mapNonAxial = 0; mapNonAxial < 2; mapNonAxial++ )
			{
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

						/* get drawverts and map first triangle */
						dv[ 0 ] = &verts[ pw[ r + 0 ] ];
						dv[ 1 ] = &verts[ pw[ r + 1 ] ];
						dv[ 2 ] = &verts[ pw[ r + 2 ] ];
						MapTriangle( lm, info, dv, mapNonAxial );

						/* get drawverts and map second triangle */
						dv[ 0 ] = &verts[ pw[ r + 0 ] ];
						dv[ 1 ] = &verts[ pw[ r + 2 ] ];
						dv[ 2 ] = &verts[ pw[ r + 3 ] ];
						MapTriangle( lm, info, dv, mapNonAxial );
					}
				}
			}

				#else

			for ( y = 0; y < ( mesh->height - 1 ); y++ )
			{
				for ( x = 0; x < ( mesh->width - 1 ); x++ )
				{
					/* set indexes */
					pw[ 0 ] = x + ( y * mesh->width );
					pw[ 1 ] = x + ( ( y + 1 ) * mesh->width );
					pw[ 2 ] = x + 1 + ( ( y + 1 ) * mesh->width );
					pw[ 3 ] = x + 1 + ( y * mesh->width );
					pw[ 4 ] = pw[ 0 ];

					/* set radix */
					r = ( x + y ) & 1;

					/* attempt to map quad first */
					dv[ 0 ] = &verts[ pw[ r + 0 ] ];
					dv[ 1 ] = &verts[ pw[ r + 1 ] ];
					dv[ 2 ] = &verts[ pw[ r + 2 ] ];
					dv[ 3 ] = &verts[ pw[ r + 3 ] ];
					if ( MapQuad( lm, info, dv ) ) {
						continue;
					}

					/* get drawverts and map first triangle */
					MapTriangle( lm, info, dv, mapNonAxial );

					/* get drawverts and map second triangle */
					dv[ 1 ] = &verts[ pw[ r + 2 ] ];
					dv[ 2 ] = &verts[ pw[ r + 3 ] ];
					MapTriangle( lm, info, dv, mapNonAxial );
				}
			}

				#endif

			/* free the mesh */
			FreeMesh( mesh );
			break;

		default:
			break;
		}
	}

	/* -----------------------------------------------------------------
	   average and clean up luxel normals
	   ----------------------------------------------------------------- */

	/* walk the luxels */
	for ( y = 0; y < lm->sh; y++ )
	{
		for ( x = 0; x < lm->sw; x++ )
		{
			/* get luxel */
			luxel = SUPER_LUXEL( 0, x, y );
			normal = SUPER_NORMAL( x, y );
			cluster = SUPER_CLUSTER( x, y );

			/* only look at mapped luxels */
			if ( *cluster < 0 ) {
				continue;
			}

			/* the normal data could be the sum of multiple samples */
			if ( luxel[ 3 ] > 1.0f ) {
				VectorNormalize( normal, normal );
			}

			/* mark this luxel as having only one normal */
			luxel[ 3 ] = 1.0f;
		}
	}

	/* non-planar surfaces stop here */
	if ( lm->plane == NULL ) {
		return;
	}

	/* -----------------------------------------------------------------
	   map occluded or unuxed luxels
	   ----------------------------------------------------------------- */

	/* walk the luxels */
	radius = floor( superSample / 2 );
	radius = radius > 0 ? radius : 1.0f;
	radius += 1.0f;
	for ( pass = 2.0f; pass <= radius; pass += 1.0f )
	{
		for ( y = 0; y < lm->sh; y++ )
		{
			for ( x = 0; x < lm->sw; x++ )
			{
				/* get luxel */
				luxel = SUPER_LUXEL( 0, x, y );
				normal = SUPER_NORMAL( x, y );
				cluster = SUPER_CLUSTER( x, y );

				/* only look at unmapped luxels */
				if ( *cluster != CLUSTER_UNMAPPED ) {
					continue;
				}

				/* divine a normal and origin from neighboring luxels */
				VectorClear( fake.xyz );
				VectorClear( fake.normal );
				fake.lightmap[ 0 ][ 0 ] = x;    //% 0.0001 + x;
				fake.lightmap[ 0 ][ 1 ] = y;    //% 0.0001 + y;
				samples = 0.0f;
				for ( sy = ( y - 1 ); sy <= ( y + 1 ); sy++ )
				{
					if ( sy < 0 || sy >= lm->sh ) {
						continue;
					}

					for ( sx = ( x - 1 ); sx <= ( x + 1 ); sx++ )
					{
						if ( sx < 0 || sx >= lm->sw || ( sx == x && sy == y ) ) {
							continue;
						}

						/* get neighboring luxel */
						luxel = SUPER_LUXEL( 0, sx, sy );
						origin = SUPER_ORIGIN( sx, sy );
						normal = SUPER_NORMAL( sx, sy );
						cluster = SUPER_CLUSTER( sx, sy );

						/* only consider luxels mapped in previous passes */
						if ( *cluster < 0 || luxel[ 0 ] >= pass ) {
							continue;
						}

						/* add its distinctiveness to our own */
						VectorAdd( fake.xyz, origin, fake.xyz );
						VectorAdd( fake.normal, normal, fake.normal );
						samples += luxel[ 3 ];
					}
				}

				/* any samples? */
				if ( samples == 0.0f ) {
					continue;
				}

				/* average */
				VectorDivide( fake.xyz, samples, fake.xyz );
				//%	VectorDivide( fake.normal, samples, fake.normal );
				if ( VectorNormalize( fake.normal, fake.normal ) == 0.0f ) {
					continue;
				}

				/* map the fake vert */
				MapSingleLuxel( lm, NULL, &fake, lm->plane, pass, NULL, NULL );
			}
		}
	}

	/* -----------------------------------------------------------------
	   average and clean up luxel normals
	   ----------------------------------------------------------------- */

	/* walk the luxels */
	for ( y = 0; y < lm->sh; y++ )
	{
		for ( x = 0; x < lm->sw; x++ )
		{
			/* get luxel */
			luxel = SUPER_LUXEL( 0, x, y );
			normal = SUPER_NORMAL( x, y );
			cluster = SUPER_CLUSTER( x, y );

			/* only look at mapped luxels */
			if ( *cluster < 0 ) {
				continue;
			}

			/* the normal data could be the sum of multiple samples */
			if ( luxel[ 3 ] > 1.0f ) {
				VectorNormalize( normal, normal );
			}

			/* mark this luxel as having only one normal */
			luxel[ 3 ] = 1.0f;
		}
	}

	/* debug code */
	#if 0
	Sys_Printf( "\n" );
	for ( y = 0; y < lm->sh; y++ )
	{
		for ( x = 0; x < lm->sw; x++ )
		{
			vec3_t mins, maxs;


			cluster = SUPER_CLUSTER( x, y );
			origin = SUPER_ORIGIN( x, y );
			normal = SUPER_NORMAL( x, y );
			luxel = SUPER_LUXEL( x, y );

			if ( *cluster < 0 ) {
				continue;
			}

			/* check if within the bounding boxes of all surfaces referenced */
			ClearBounds( mins, maxs );
			for ( n = 0; n < lm->numLightSurfaces; n++ )
			{
				int TOL;
				info = &surfaceInfos[ lightSurfaces[ lm->firstLightSurface + n ] ];
				TOL = info->sampleSize + 2;
				AddPointToBounds( info->mins, mins, maxs );
				AddPointToBounds( info->maxs, mins, maxs );
				if ( origin[ 0 ] > ( info->mins[ 0 ] - TOL ) && origin[ 0 ] < ( info->maxs[ 0 ] + TOL ) &&
					 origin[ 1 ] > ( info->mins[ 1 ] - TOL ) && origin[ 1 ] < ( info->maxs[ 1 ] + TOL ) &&
					 origin[ 2 ] > ( info->mins[ 2 ] - TOL ) && origin[ 2 ] < ( info->maxs[ 2 ] + TOL ) ) {
					break;
				}
			}

			/* inside? */
			if ( n < lm->numLightSurfaces ) {
				continue;
			}

			/* report bogus origin */
			Sys_Printf( "%6d [%2d,%2d] (%4d): XYZ(%+4.1f %+4.1f %+4.1f) LO(%+4.1f %+4.1f %+4.1f) HI(%+4.1f %+4.1f %+4.1f) <%3.0f>\n",
						rawLightmapNum, x, y, *cluster,
						origin[ 0 ], origin[ 1 ], origin[ 2 ],
						mins[ 0 ], mins[ 1 ], mins[ 2 ],
						maxs[ 0 ], maxs[ 1 ], maxs[ 2 ],
						luxel[ 3 ] );
		}
	}
	#endif
}



/*
   SetupDirt()
   sets up dirtmap (ambient occlusion)
 */

#define DIRT_CONE_ANGLE             88  /* degrees */
#define DIRT_NUM_ANGLE_STEPS        16
#define DIRT_NUM_ELEVATION_STEPS    3
#define DIRT_NUM_VECTORS            ( DIRT_NUM_ANGLE_STEPS * DIRT_NUM_ELEVATION_STEPS )

static vec3_t dirtVectors[ DIRT_NUM_VECTORS ];
static int numDirtVectors = 0;

void SetupDirt( void ){
	int i, j;
	float angle, elevation, angleStep, elevationStep;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- SetupDirt ---\n" );

	/* calculate angular steps */
	angleStep = DEG2RAD( 360.0f / DIRT_NUM_ANGLE_STEPS );
	elevationStep = DEG2RAD( DIRT_CONE_ANGLE / DIRT_NUM_ELEVATION_STEPS );

	/* iterate angle */
	angle = 0.0f;
	for ( i = 0, angle = 0.0f; i < DIRT_NUM_ANGLE_STEPS; i++, angle += angleStep )
	{
		/* iterate elevation */
		for ( j = 0, elevation = elevationStep * 0.5f; j < DIRT_NUM_ELEVATION_STEPS; j++, elevation += elevationStep )
		{
			dirtVectors[ numDirtVectors ][ 0 ] = sin( elevation ) * cos( angle );
			dirtVectors[ numDirtVectors ][ 1 ] = sin( elevation ) * sin( angle );
			dirtVectors[ numDirtVectors ][ 2 ] = cos( elevation );
			numDirtVectors++;
		}
	}

	/* emit some statistics */
	Sys_FPrintf( SYS_VRB, "%9d dirtmap vectors\n", numDirtVectors );
}


/*
   DirtForSample()
   calculates dirt value for a given sample
 */

float DirtForSample( trace_t *trace ){
	int i;
	float gatherDirt, outDirt, angle, elevation, ooDepth;
	vec3_t normal, worldUp, myUp, myRt, temp, direction, displacement;


	/* dummy check */
	if ( !dirty ) {
		return 1.0f;
	}
	if ( trace == NULL || trace->cluster < 0 ) {
		return 0.0f;
	}

	/* setup */
	gatherDirt = 0.0f;
	ooDepth = 1.0f / dirtDepth;
	VectorCopy( trace->normal, normal );

	/* check if the normal is aligned to the world-up */
	if ( normal[ 0 ] == 0.0f && normal[ 1 ] == 0.0f ) {
		if ( normal[ 2 ] == -1.0f ) {
			VectorSet( myRt, -1.0f, 0.0f, 0.0f );
			VectorSet( myUp,  0.0f, 1.0f, 0.0f );
		}
		else {
			VectorSet( myRt, 1.0f, 0.0f, 0.0f );
			VectorSet( myUp, 0.0f, 1.0f, 0.0f );
		}
	}
	else
	{
		VectorSet( worldUp, 0.0f, 0.0f, 1.0f );
		CrossProduct( normal, worldUp, myRt );
		VectorNormalize( myRt, myRt );
		CrossProduct( myRt, normal, myUp );
		VectorNormalize( myUp, myUp );
	}

	/* 1 = random mode, 0 (well everything else) = non-random mode */
	if ( dirtMode == 1 ) {
		/* iterate */
		for ( i = 0; i < numDirtVectors; i++ )
		{
			/* get random vector */
			angle = Random() * DEG2RAD( 360.0f );
			elevation = Random() * DEG2RAD( DIRT_CONE_ANGLE );
			temp[ 0 ] = cos( angle ) * sin( elevation );
			temp[ 1 ] = sin( angle ) * sin( elevation );
			temp[ 2 ] = cos( elevation );

			/* transform into tangent space */
			direction[ 0 ] = myRt[ 0 ] * temp[ 0 ] + myUp[ 0 ] * temp[ 1 ] + normal[ 0 ] * temp[ 2 ];
			direction[ 1 ] = myRt[ 1 ] * temp[ 0 ] + myUp[ 1 ] * temp[ 1 ] + normal[ 1 ] * temp[ 2 ];
			direction[ 2 ] = myRt[ 2 ] * temp[ 0 ] + myUp[ 2 ] * temp[ 1 ] + normal[ 2 ] * temp[ 2 ];

			/* set endpoint */
			VectorMA( trace->origin, dirtDepth, direction, trace->end );
			SetupTrace( trace );

			/* trace */
			TraceLine( trace );
			if ( trace->opaque ) {
				VectorSubtract( trace->hit, trace->origin, displacement );
				gatherDirt += 1.0f - ooDepth * VectorLength( displacement );
			}
		}
	}
	else
	{
		/* iterate through ordered vectors */
		for ( i = 0; i < numDirtVectors; i++ )
		{
			/* transform vector into tangent space */
			direction[ 0 ] = myRt[ 0 ] * dirtVectors[ i ][ 0 ] + myUp[ 0 ] * dirtVectors[ i ][ 1 ] + normal[ 0 ] * dirtVectors[ i ][ 2 ];
			direction[ 1 ] = myRt[ 1 ] * dirtVectors[ i ][ 0 ] + myUp[ 1 ] * dirtVectors[ i ][ 1 ] + normal[ 1 ] * dirtVectors[ i ][ 2 ];
			direction[ 2 ] = myRt[ 2 ] * dirtVectors[ i ][ 0 ] + myUp[ 2 ] * dirtVectors[ i ][ 1 ] + normal[ 2 ] * dirtVectors[ i ][ 2 ];

			/* set endpoint */
			VectorMA( trace->origin, dirtDepth, direction, trace->end );
			SetupTrace( trace );

			/* trace */
			TraceLine( trace );
			if ( trace->opaque ) {
				VectorSubtract( trace->hit, trace->origin, displacement );
				gatherDirt += 1.0f - ooDepth * VectorLength( displacement );
			}
		}
	}

	/* direct ray */
	VectorMA( trace->origin, dirtDepth, normal, trace->end );
	SetupTrace( trace );

	/* trace */
	TraceLine( trace );
	if ( trace->opaque ) {
		VectorSubtract( trace->hit, trace->origin, displacement );
		gatherDirt += 1.0f - ooDepth * VectorLength( displacement );
	}

	/* early out */
	if ( gatherDirt <= 0.0f ) {
		return 1.0f;
	}

	/* apply gain (does this even do much? heh) */
	outDirt = pow( gatherDirt / ( numDirtVectors + 1 ), dirtGain );
	if ( outDirt > 1.0f ) {
		outDirt = 1.0f;
	}

	/* apply scale */
	outDirt *= dirtScale;
	if ( outDirt > 1.0f ) {
		outDirt = 1.0f;
	}

	/* return to sender */
	return 1.0f - outDirt;
}



/*
   DirtyRawLightmap()
   calculates dirty fraction for each luxel
 */

void DirtyRawLightmap( int rawLightmapNum ){
	int i, x, y, sx, sy, *cluster;
	float               *origin, *normal, *dirt, *dirt2, average, samples;
	rawLightmap_t       *lm;
	surfaceInfo_t       *info;
	trace_t trace;


	/* bail if this number exceeds the number of raw lightmaps */
	if ( rawLightmapNum >= numRawLightmaps ) {
		return;
	}

	/* get lightmap */
	lm = &rawLightmaps[ rawLightmapNum ];

	/* setup trace */
	trace.testOcclusion = qtrue;
	trace.forceSunlight = qfalse;
	trace.recvShadows = lm->recvShadows;
	trace.numSurfaces = lm->numLightSurfaces;
	trace.surfaces = &lightSurfaces[ lm->firstLightSurface ];
	trace.inhibitRadius = DEFAULT_INHIBIT_RADIUS;
	trace.testAll = qfalse;

	/* twosided lighting (may or may not be a good idea for lightmapped stuff) */
	trace.twoSided = qfalse;
	for ( i = 0; i < trace.numSurfaces; i++ )
	{
		/* get surface */
		info = &surfaceInfos[ trace.surfaces[ i ] ];

		/* check twosidedness */
		if ( info->si->twoSided ) {
			trace.twoSided = qtrue;
			break;
		}
	}

	/* gather dirt */
	for ( y = 0; y < lm->sh; y++ )
	{
		for ( x = 0; x < lm->sw; x++ )
		{
			/* get luxel */
			cluster = SUPER_CLUSTER( x, y );
			origin = SUPER_ORIGIN( x, y );
			normal = SUPER_NORMAL( x, y );
			dirt = SUPER_DIRT( x, y );

			/* set default dirt */
			*dirt = 0.0f;

			/* only look at mapped luxels */
			if ( *cluster < 0 ) {
				continue;
			}

			/* copy to trace */
			trace.cluster = *cluster;
			VectorCopy( origin, trace.origin );
			VectorCopy( normal, trace.normal );

			/* get dirt */
			*dirt = DirtForSample( &trace );
		}
	}

	/* testing no filtering */
	//%	return;

	/* filter dirt */
	for ( y = 0; y < lm->sh; y++ )
	{
		for ( x = 0; x < lm->sw; x++ )
		{
			/* get luxel */
			cluster = SUPER_CLUSTER( x, y );
			dirt = SUPER_DIRT( x, y );

			/* filter dirt by adjacency to unmapped luxels */
			average = *dirt;
			samples = 1.0f;
			for ( sy = ( y - 1 ); sy <= ( y + 1 ); sy++ )
			{
				if ( sy < 0 || sy >= lm->sh ) {
					continue;
				}

				for ( sx = ( x - 1 ); sx <= ( x + 1 ); sx++ )
				{
					if ( sx < 0 || sx >= lm->sw || ( sx == x && sy == y ) ) {
						continue;
					}

					/* get neighboring luxel */
					cluster = SUPER_CLUSTER( sx, sy );
					dirt2 = SUPER_DIRT( sx, sy );
					if ( *cluster < 0 || *dirt2 <= 0.0f ) {
						continue;
					}

					/* add it */
					average += *dirt2;
					samples += 1.0f;
				}

				/* bail */
				if ( samples <= 0.0f ) {
					break;
				}
			}

			/* bail */
			if ( samples <= 0.0f ) {
				continue;
			}

			/* scale dirt */
			*dirt = average / samples;
		}
	}
}



/*
   SubmapRawLuxel()
   calculates the pvs cluster, origin, normal of a sub-luxel
 */

static qboolean SubmapRawLuxel( rawLightmap_t *lm, int x, int y, float bx, float by, int *sampleCluster, vec3_t sampleOrigin, vec3_t sampleNormal ){
	int i, *cluster, *cluster2;
	float       *origin, *origin2, *normal;
	vec3_t originVecs[ 2 ];

	/* calulate x vector */
	if ( ( x < ( lm->sw - 1 ) && bx >= 0.0f ) || ( x == 0 && bx <= 0.0f ) ) {
		cluster = SUPER_CLUSTER( x, y );
		origin = SUPER_ORIGIN( x, y );
		cluster2 = SUPER_CLUSTER( x + 1, y );
		origin2 = *cluster2 < 0 ? SUPER_ORIGIN( x, y ) : SUPER_ORIGIN( x + 1, y );
	}
	else if ( ( x > 0 && bx <= 0.0f ) || ( x == ( lm->sw - 1 ) && bx >= 0.0f ) ) {
		cluster = SUPER_CLUSTER( x - 1, y );
		origin = *cluster < 0 ? SUPER_ORIGIN( x, y ) : SUPER_ORIGIN( x - 1, y );
		cluster2 = SUPER_CLUSTER( x, y );
		origin2 = SUPER_ORIGIN( x, y );
	}
	else{
		Sys_FPrintf( SYS_WRN, "WARNING: Spurious lightmap S vector\n" );
		VectorClear( originVecs[0] );
		origin = originVecs[0];
		origin2 = originVecs[0];
	}

	VectorSubtract( origin2, origin, originVecs[ 0 ] );

	/* calulate y vector */
	if ( ( y < ( lm->sh - 1 ) && bx >= 0.0f ) || ( y == 0 && bx <= 0.0f ) ) {
		cluster = SUPER_CLUSTER( x, y );
		origin = SUPER_ORIGIN( x, y );
		cluster2 = SUPER_CLUSTER( x, y + 1 );
		origin2 = *cluster2 < 0 ? SUPER_ORIGIN( x, y ) : SUPER_ORIGIN( x, y + 1 );
	}
	else if ( ( y > 0 && bx <= 0.0f ) || ( y == ( lm->sh - 1 ) && bx >= 0.0f ) ) {
		cluster = SUPER_CLUSTER( x, y - 1 );
		origin = *cluster < 0 ? SUPER_ORIGIN( x, y ) : SUPER_ORIGIN( x, y - 1 );
		cluster2 = SUPER_CLUSTER( x, y );
		origin2 = SUPER_ORIGIN( x, y );
	}
	else{
		Sys_FPrintf( SYS_WRN, "WARNING: Spurious lightmap T vector\n" );
		VectorClear( originVecs[1] );
		origin = originVecs[1];
		origin2 = originVecs[1];
	}

	VectorSubtract( origin2, origin, originVecs[ 1 ] );

	/* calculate new origin */
	for ( i = 0; i < 3; i++ )
		sampleOrigin[ i ] = sampleOrigin[ i ] + ( bx * originVecs[ 0 ][ i ] ) + ( by * originVecs[ 1 ][ i ] );

	/* get cluster */
	*sampleCluster = ClusterForPointExtFilter( sampleOrigin, ( LUXEL_EPSILON * 2 ), lm->numLightClusters, lm->lightClusters );
	if ( *sampleCluster < 0 ) {
		return qfalse;
	}

	/* calculate new normal */
	normal = SUPER_NORMAL( x, y );
	VectorCopy( normal, sampleNormal );

	/* return ok */
	return qtrue;
}


/*
   SubsampleRawLuxel_r()
   recursively subsamples a luxel until its color gradient is low enough or subsampling limit is reached
 */

static void SubsampleRawLuxel_r( rawLightmap_t *lm, trace_t *trace, vec3_t sampleOrigin, int x, int y, float bias, float *lightLuxel ){
	int b, samples, mapped, lighted;
	int cluster[ 4 ];
	vec4_t luxel[ 4 ];
	vec3_t origin[ 4 ], normal[ 4 ];
	float biasDirs[ 4 ][ 2 ] = { { -1.0f, -1.0f }, { 1.0f, -1.0f }, { -1.0f, 1.0f }, { 1.0f, 1.0f } };
	vec3_t color, total;


	/* limit check */
	if ( lightLuxel[ 3 ] >= lightSamples ) {
		return;
	}

	/* setup */
	VectorClear( total );
	mapped = 0;
	lighted = 0;

	/* make 2x2 subsample stamp */
	for ( b = 0; b < 4; b++ )
	{
		/* set origin */
		VectorCopy( sampleOrigin, origin[ b ] );

		/* calculate position */
		if ( !SubmapRawLuxel( lm, x, y, ( bias * biasDirs[ b ][ 0 ] ), ( bias * biasDirs[ b ][ 1 ] ), &cluster[ b ], origin[ b ], normal[ b ] ) ) {
			cluster[ b ] = -1;
			continue;
		}
		mapped++;

		/* increment sample count */
		luxel[ b ][ 3 ] = lightLuxel[ 3 ] + 1.0f;

		/* setup trace */
		trace->cluster = *cluster;
		VectorCopy( origin[ b ], trace->origin );
		VectorCopy( normal[ b ], trace->normal );

		/* sample light */

		LightContributionToSample( trace );

		/* add to totals (fixme: make contrast function) */
		VectorCopy( trace->color, luxel[ b ] );
		VectorAdd( total, trace->color, total );
		if ( ( luxel[ b ][ 0 ] + luxel[ b ][ 1 ] + luxel[ b ][ 2 ] ) > 0.0f ) {
			lighted++;
		}
	}

	/* subsample further? */
	if ( ( lightLuxel[ 3 ] + 1.0f ) < lightSamples &&
		 ( total[ 0 ] > 4.0f || total[ 1 ] > 4.0f || total[ 2 ] > 4.0f ) &&
		 lighted != 0 && lighted != mapped ) {
		for ( b = 0; b < 4; b++ )
		{
			if ( cluster[ b ] < 0 ) {
				continue;
			}
			SubsampleRawLuxel_r( lm, trace, origin[ b ], x, y, ( bias * 0.25f ), luxel[ b ] );
		}
	}

	/* average */
	//%	VectorClear( color );
	//%	samples = 0;
	VectorCopy( lightLuxel, color );
	samples = 1;
	for ( b = 0; b < 4; b++ )
	{
		if ( cluster[ b ] < 0 ) {
			continue;
		}
		VectorAdd( color, luxel[ b ], color );
		samples++;
	}

	/* add to luxel */
	if ( samples > 0 ) {
		/* average */
		color[ 0 ] /= samples;
		color[ 1 ] /= samples;
		color[ 2 ] /= samples;

		/* add to color */
		VectorCopy( color, lightLuxel );
		lightLuxel[ 3 ] += 1.0f;
	}
}



/*
   IlluminateRawLightmap()
   illuminates the luxels
 */

#define STACK_LL_SIZE           ( SUPER_LUXEL_SIZE * 64 * 64 )
#define LIGHT_LUXEL( x, y )     ( lightLuxels + ( ( ( ( y ) * lm->sw ) + ( x ) ) * SUPER_LUXEL_SIZE ) )

void IlluminateRawLightmap( int rawLightmapNum ){
	int i, t, x, y, sx, sy, size, llSize, luxelFilterRadius, lightmapNum;
	int                 *cluster, *cluster2, mapped, lighted, totalLighted;
	rawLightmap_t       *lm;
	surfaceInfo_t       *info;
	qboolean filterColor, filterDir;
	float brightness;
	float               *origin, *normal, *dirt, *luxel, *luxel2, *deluxel, *deluxel2;
	float               *lightLuxels, *lightLuxel, samples, filterRadius, weight;
	vec3_t color, averageColor, averageDir, total, temp, temp2;
	float tests[ 4 ][ 2 ] = { { 0.0f, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
	trace_t trace;
	float stackLightLuxels[ STACK_LL_SIZE ];


	/* bail if this number exceeds the number of raw lightmaps */
	if ( rawLightmapNum >= numRawLightmaps ) {
		return;
	}

	/* get lightmap */
	lm = &rawLightmaps[ rawLightmapNum ];

	/* setup trace */
	trace.testOcclusion = !noTrace;
	trace.forceSunlight = qfalse;
	trace.recvShadows = lm->recvShadows;
	trace.numSurfaces = lm->numLightSurfaces;
	trace.surfaces = &lightSurfaces[ lm->firstLightSurface ];
	trace.inhibitRadius = DEFAULT_INHIBIT_RADIUS;

	/* twosided lighting (may or may not be a good idea for lightmapped stuff) */
	trace.twoSided = qfalse;
	for ( i = 0; i < trace.numSurfaces; i++ )
	{
		/* get surface */
		info = &surfaceInfos[ trace.surfaces[ i ] ];

		/* check twosidedness */
		if ( info->si->twoSided ) {
			trace.twoSided = qtrue;
			break;
		}
	}

	/* create a culled light list for this raw lightmap */
	CreateTraceLightsForBounds( lm->mins, lm->maxs, lm->plane, lm->numLightClusters, lm->lightClusters, LIGHT_SURFACES, &trace );

	/* -----------------------------------------------------------------
	   fill pass
	   ----------------------------------------------------------------- */

	/* set counts */
	pthread_mutex_lock( &master_mutex );
	numLuxelsIlluminated += ( lm->sw * lm->sh );
	pthread_mutex_unlock( &master_mutex );

	/* test debugging state */
	if ( debugSurfaces || debugAxis || debugCluster || debugOrigin || dirtDebug || normalmap ) {
		/* debug fill the luxels */
		for ( y = 0; y < lm->sh; y++ )
		{
			for ( x = 0; x < lm->sw; x++ )
			{
				/* get cluster */
				cluster = SUPER_CLUSTER( x, y );

				/* only fill mapped luxels */
				if ( *cluster < 0 ) {
					continue;
				}

				/* get particulars */
				luxel = SUPER_LUXEL( 0, x, y );
				origin = SUPER_ORIGIN( x, y );
				normal = SUPER_NORMAL( x, y );

				/* color the luxel with raw lightmap num? */
				if ( debugSurfaces ) {
					VectorCopy( debugColors[ rawLightmapNum % 12 ], luxel );
				}

				/* color the luxel with lightmap axis? */
				else if ( debugAxis ) {
					luxel[ 0 ] = ( lm->axis[ 0 ] + 1.0f ) * 127.5f;
					luxel[ 1 ] = ( lm->axis[ 1 ] + 1.0f ) * 127.5f;
					luxel[ 2 ] = ( lm->axis[ 2 ] + 1.0f ) * 127.5f;
				}

				/* color the luxel with luxel cluster? */
				else if ( debugCluster ) {
					VectorCopy( debugColors[ *cluster % 12 ], luxel );
				}

				/* color the luxel with luxel origin? */
				else if ( debugOrigin ) {
					VectorSubtract( lm->maxs, lm->mins, temp );
					VectorScale( temp, ( 1.0f / 255.0f ), temp );
					VectorSubtract( origin, lm->mins, temp2 );
					luxel[ 0 ] = lm->mins[ 0 ] + ( temp[ 0 ] * temp2[ 0 ] );
					luxel[ 1 ] = lm->mins[ 1 ] + ( temp[ 1 ] * temp2[ 1 ] );
					luxel[ 2 ] = lm->mins[ 2 ] + ( temp[ 2 ] * temp2[ 2 ] );
				}

				/* color the luxel with the normal */
				else if ( normalmap ) {
					luxel[ 0 ] = ( normal[ 0 ] + 1.0f ) * 127.5f;
					luxel[ 1 ] = ( normal[ 1 ] + 1.0f ) * 127.5f;
					luxel[ 2 ] = ( normal[ 2 ] + 1.0f ) * 127.5f;
				}

				/* otherwise clear it */
				else{
					VectorClear( luxel );
				}

				/* add to counts */
				luxel[ 3 ] = 1.0f;
			}
		}
	}
	else
	{
		/* allocate temporary per-light luxel storage */
		llSize = lm->sw * lm->sh * SUPER_LUXEL_SIZE * sizeof( float );
		if ( llSize <= ( STACK_LL_SIZE * sizeof( float ) ) ) {
			lightLuxels = stackLightLuxels;
		}
		else{
			lightLuxels = safe_malloc( llSize );
		}

		/* clear luxels */
		//%	memset( lm->superLuxels[ 0 ], 0, llSize );

		/* set ambient color */
		for ( y = 0; y < lm->sh; y++ )
		{
			for ( x = 0; x < lm->sw; x++ )
			{
				/* get cluster */
				cluster = SUPER_CLUSTER( x, y );
				luxel = SUPER_LUXEL( 0, x, y );
				normal = SUPER_NORMAL( x, y );
				deluxel = SUPER_DELUXEL( x, y );

				/* blacken unmapped clusters */
				if ( *cluster < 0 ) {
					VectorClear( luxel );
				}

				/* set ambient */
				else
				{
					VectorCopy( ambientColor, luxel );
					if ( deluxemap ) {
						VectorScale( normal, 0.00390625f, deluxel );
					}
					luxel[ 3 ] = 1.0f;
				}
			}
		}

		/* clear styled lightmaps */
		size = lm->sw * lm->sh * SUPER_LUXEL_SIZE * sizeof( float );
		for ( lightmapNum = 1; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
		{
			if ( lm->superLuxels[ lightmapNum ] != NULL ) {
				memset( lm->superLuxels[ lightmapNum ], 0, size );
			}
		}

		/* debugging code */
		//%	if( trace.numLights <= 0 )
		//%		Sys_Printf( "Lightmap %9d: 0 lights, axis: %.2f, %.2f, %.2f\n", rawLightmapNum, lm->axis[ 0 ], lm->axis[ 1 ], lm->axis[ 2 ] );

		/* walk light list */
		for ( i = 0; i < trace.numLights; i++ )
		{
			/* setup trace */
			trace.light = trace.lights[ i ];

			/* style check */
			for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
			{
				if ( lm->styles[ lightmapNum ] == trace.light->style ||
					 lm->styles[ lightmapNum ] == LS_NONE ) {
					break;
				}
			}

			/* max of MAX_LIGHTMAPS (4) styles allowed to hit a surface/lightmap */
			if ( lightmapNum >= MAX_LIGHTMAPS ) {
				Sys_FPrintf( SYS_WRN, "WARNING: Hit per-surface style limit (%d)\n", MAX_LIGHTMAPS );
				continue;
			}

			/* setup */
			memset( lightLuxels, 0, llSize );
			totalLighted = 0;

			/* initial pass, one sample per luxel */
			for ( y = 0; y < lm->sh; y++ )
			{
				for ( x = 0; x < lm->sw; x++ )
				{
					/* get cluster */
					cluster = SUPER_CLUSTER( x, y );
					if ( *cluster < 0 ) {
						continue;
					}

					/* get particulars */
					lightLuxel = LIGHT_LUXEL( x, y );
					deluxel = SUPER_DELUXEL( x, y );
					origin = SUPER_ORIGIN( x, y );
					normal = SUPER_NORMAL( x, y );

					/* set contribution count */
					lightLuxel[ 3 ] = 1.0f;

					/* setup trace */
					trace.cluster = *cluster;
					VectorCopy( origin, trace.origin );
					VectorCopy( normal, trace.normal );

					/* get light for this sample */
					LightContributionToSample( &trace );
					VectorCopy( trace.color, lightLuxel );

					/* add to count */
					if ( trace.color[ 0 ] || trace.color[ 1 ] || trace.color[ 2 ] ) {
						totalLighted++;
					}

					/* add to light direction map (fixme: use luxel normal as starting point for deluxel?) */
					if ( deluxemap ) {
						/* color to grayscale (photoshop rgb weighting) */
						brightness = trace.color[ 0 ] * 0.3f + trace.color[ 1 ] * 0.59f + trace.color[ 2 ] * 0.11f;
						brightness *= ( 1.0 / 255.0 );
						VectorScale( trace.direction, brightness, trace.direction );
						VectorAdd( deluxel, trace.direction, deluxel );
					}
				}
			}

			/* don't even bother with everything else if nothing was lit */
			if ( totalLighted == 0 ) {
				continue;
			}

			/* determine filter radius */
			filterRadius = lm->filterRadius > trace.light->filterRadius
						   ? lm->filterRadius
						   : trace.light->filterRadius;
			if ( filterRadius < 0.0f ) {
				filterRadius = 0.0f;
			}

			/* set luxel filter radius */
			luxelFilterRadius = lm->sampleSize != 0 ? superSample * filterRadius / lm->sampleSize : 0;
			if ( luxelFilterRadius == 0 && ( filterRadius > 0.0f || filter ) ) {
				luxelFilterRadius = 1;
			}

			/* secondary pass, adaptive supersampling (fixme: use a contrast function to determine if subsampling is necessary) */
			/* 2003-09-27: changed it so filtering disamples supersampling, as it would waste time */
			if ( lightSamples > 1 && luxelFilterRadius == 0 ) {
				/* walk luxels */
				for ( y = 0; y < ( lm->sh - 1 ); y++ )
				{
					for ( x = 0; x < ( lm->sw - 1 ); x++ )
					{
						/* setup */
						mapped = 0;
						lighted = 0;
						VectorClear( total );

						/* test 2x2 stamp */
						for ( t = 0; t < 4; t++ )
						{
							/* set sample coords */
							sx = x + tests[ t ][ 0 ];
							sy = y + tests[ t ][ 1 ];

							/* get cluster */
							cluster = SUPER_CLUSTER( sx, sy );
							if ( *cluster < 0 ) {
								continue;
							}
							mapped++;

							/* get luxel */
							lightLuxel = LIGHT_LUXEL( sx, sy );
							VectorAdd( total, lightLuxel, total );
							if ( ( lightLuxel[ 0 ] + lightLuxel[ 1 ] + lightLuxel[ 2 ] ) > 0.0f ) {
								lighted++;
							}
						}

						/* if total color is under a certain amount, then don't bother subsampling */
						if ( total[ 0 ] <= 4.0f && total[ 1 ] <= 4.0f && total[ 2 ] <= 4.0f ) {
							continue;
						}

						/* if all 4 pixels are either in shadow or light, then don't subsample */
						if ( lighted != 0 && lighted != mapped ) {
							for ( t = 0; t < 4; t++ )
							{
								/* set sample coords */
								sx = x + tests[ t ][ 0 ];
								sy = y + tests[ t ][ 1 ];

								/* get luxel */
								cluster = SUPER_CLUSTER( sx, sy );
								if ( *cluster < 0 ) {
									continue;
								}
								lightLuxel = LIGHT_LUXEL( sx, sy );
								origin = SUPER_ORIGIN( sx, sy );

								/* only subsample shadowed luxels */
								//%	if( (lightLuxel[ 0 ] + lightLuxel[ 1 ] + lightLuxel[ 2 ]) <= 0.0f )
								//%		continue;

								/* subsample it */
								SubsampleRawLuxel_r( lm, &trace, origin, sx, sy, 0.25f, lightLuxel );

								/* debug code to colorize subsampled areas to yellow */
								//%	luxel = SUPER_LUXEL( lightmapNum, sx, sy );
								//%	VectorSet( luxel, 255, 204, 0 );
							}
						}
					}
				}
			}

			/* tertiary pass, apply dirt map (ambient occlusion) */
			if ( 0 && dirty ) {
				/* walk luxels */
				for ( y = 0; y < lm->sh; y++ )
				{
					for ( x = 0; x < lm->sw; x++ )
					{
						/* get cluster  */
						cluster = SUPER_CLUSTER( x, y );
						if ( *cluster < 0 ) {
							continue;
						}

						/* get particulars */
						lightLuxel = LIGHT_LUXEL( x, y );
						dirt = SUPER_DIRT( x, y );

						/* scale light value */
						VectorScale( lightLuxel, *dirt, lightLuxel );
					}
				}
			}

			/* allocate sampling lightmap storage */
			if ( lm->superLuxels[ lightmapNum ] == NULL ) {
				/* allocate sampling lightmap storage */
				size = lm->sw * lm->sh * SUPER_LUXEL_SIZE * sizeof( float );
				lm->superLuxels[ lightmapNum ] = safe_malloc( size );
				memset( lm->superLuxels[ lightmapNum ], 0, size );
			}

			/* set style */
			if ( lightmapNum > 0 ) {
				lm->styles[ lightmapNum ] = trace.light->style;
				//%	Sys_Printf( "Surface %6d has lightstyle %d\n", rawLightmapNum, trace.light->style );
			}

			/* copy to permanent luxels */
			for ( y = 0; y < lm->sh; y++ )
			{
				for ( x = 0; x < lm->sw; x++ )
				{
					/* get cluster and origin */
					cluster = SUPER_CLUSTER( x, y );
					if ( *cluster < 0 ) {
						continue;
					}
					origin = SUPER_ORIGIN( x, y );

					/* filter? */
					if ( luxelFilterRadius ) {
						/* setup */
						VectorClear( averageColor );
						samples = 0.0f;

						/* cheaper distance-based filtering */
						for ( sy = ( y - luxelFilterRadius ); sy <= ( y + luxelFilterRadius ); sy++ )
						{
							if ( sy < 0 || sy >= lm->sh ) {
								continue;
							}

							for ( sx = ( x - luxelFilterRadius ); sx <= ( x + luxelFilterRadius ); sx++ )
							{
								if ( sx < 0 || sx >= lm->sw ) {
									continue;
								}

								/* get particulars */
								cluster = SUPER_CLUSTER( sx, sy );
								if ( *cluster < 0 ) {
									continue;
								}
								lightLuxel = LIGHT_LUXEL( sx, sy );

								/* create weight */
								weight = ( abs( sx - x ) == luxelFilterRadius ? 0.5f : 1.0f );
								weight *= ( abs( sy - y ) == luxelFilterRadius ? 0.5f : 1.0f );

								/* scale luxel by filter weight */
								VectorScale( lightLuxel, weight, color );
								VectorAdd( averageColor, color, averageColor );
								samples += weight;
							}
						}

						/* any samples? */
						if ( samples <= 0.0f ) {
							continue;
						}

						/* scale into luxel */
						luxel = SUPER_LUXEL( lightmapNum, x, y );
						luxel[ 3 ] = 1.0f;

						/* handle negative light */
						if ( trace.light->flags & LIGHT_NEGATIVE ) {
							luxel[ 0 ] -= averageColor[ 0 ] / samples;
							luxel[ 1 ] -= averageColor[ 1 ] / samples;
							luxel[ 2 ] -= averageColor[ 2 ] / samples;
						}

						/* handle normal light */
						else
						{
							luxel[ 0 ] += averageColor[ 0 ] / samples;
							luxel[ 1 ] += averageColor[ 1 ] / samples;
							luxel[ 2 ] += averageColor[ 2 ] / samples;
						}
					}

					/* single sample */
					else
					{
						/* get particulars */
						lightLuxel = LIGHT_LUXEL( x, y );
						luxel = SUPER_LUXEL( lightmapNum, x, y );

						/* handle negative light */
						if ( trace.light->flags & LIGHT_NEGATIVE ) {
							VectorScale( averageColor, -1.0f, averageColor );
						}

						/* add color */
						luxel[ 3 ] = 1.0f;

						/* handle negative light */
						if ( trace.light->flags & LIGHT_NEGATIVE ) {
							VectorSubtract( luxel, lightLuxel, luxel );
						}

						/* handle normal light */
						else{
							VectorAdd( luxel, lightLuxel, luxel );
						}
					}
				}
			}
		}

		/* free temporary luxels */
		if ( lightLuxels != stackLightLuxels ) {
			free( lightLuxels );
		}
	}

	/* free light list */
	FreeTraceLights( &trace );

	/*	-----------------------------------------------------------------
	    dirt pass
	    ----------------------------------------------------------------- */

	if ( dirty ) {
		/* walk lightmaps */
		for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
		{
			/* early out */
			if ( lm->superLuxels[ lightmapNum ] == NULL ) {
				continue;
			}

			/* apply dirt to each luxel */
			for ( y = 0; y < lm->sh; y++ )
			{
				for ( x = 0; x < lm->sw; x++ )
				{
					/* get cluster */
					cluster = SUPER_CLUSTER( x, y );
					//%	if( *cluster < 0 )
					//%		continue;

					/* get particulars */
					luxel = SUPER_LUXEL( lightmapNum, x, y );
					dirt = SUPER_DIRT( x, y );

					/* apply dirt */
					VectorScale( luxel, *dirt, luxel );

					/* debugging */
					if ( dirtDebug ) {
						VectorSet( luxel, *dirt * 255.0f, *dirt * 255.0f, *dirt * 255.0f );
					}
				}
			}
		}
	}

	/* -----------------------------------------------------------------
	   filter pass
	   ----------------------------------------------------------------- */

	/* walk lightmaps */
	for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
	{
		/* early out */
		if ( lm->superLuxels[ lightmapNum ] == NULL ) {
			continue;
		}

		/* average occluded luxels from neighbors */
		for ( y = 0; y < lm->sh; y++ )
		{
			for ( x = 0; x < lm->sw; x++ )
			{
				/* get particulars */
				cluster = SUPER_CLUSTER( x, y );
				luxel = SUPER_LUXEL( lightmapNum, x, y );
				deluxel = SUPER_DELUXEL( x, y );
				normal = SUPER_NORMAL( x, y );

				/* determine if filtering is necessary */
				filterColor = qfalse;
				filterDir = qfalse;
				if ( *cluster < 0 ||
					 ( lm->splotchFix && ( luxel[ 0 ] <= ambientColor[ 0 ] || luxel[ 1 ] <= ambientColor[ 1 ] || luxel[ 2 ] <= ambientColor[ 2 ] ) ) ) {
					filterColor = qtrue;
				}
				if ( deluxemap && lightmapNum == 0 && ( *cluster < 0 || filter ) ) {
					filterDir = qtrue;
				}

				if ( !filterColor && !filterDir ) {
					continue;
				}

				/* choose seed amount */
				VectorClear( averageColor );
				VectorClear( averageDir );
				samples = 0.0f;

				/* walk 3x3 matrix */
				for ( sy = ( y - 1 ); sy <= ( y + 1 ); sy++ )
				{
					if ( sy < 0 || sy >= lm->sh ) {
						continue;
					}

					for ( sx = ( x - 1 ); sx <= ( x + 1 ); sx++ )
					{
						if ( sx < 0 || sx >= lm->sw || ( sx == x && sy == y ) ) {
							continue;
						}

						/* get neighbor's particulars */
						cluster2 = SUPER_CLUSTER( sx, sy );
						luxel2 = SUPER_LUXEL( lightmapNum, sx, sy );
						deluxel2 = SUPER_DELUXEL( sx, sy );

						/* ignore unmapped/unlit luxels */
						if ( *cluster2 < 0 || luxel2[ 3 ] == 0.0f ||
							 ( lm->splotchFix && VectorCompare( luxel2, ambientColor ) ) ) {
							continue;
						}

						/* add its distinctiveness to our own */
						VectorAdd( averageColor, luxel2, averageColor );
						samples += luxel2[ 3 ];
						if ( filterDir ) {
							VectorAdd( averageDir, deluxel2, averageDir );
						}
					}
				}

				/* fall through */
				if ( samples <= 0.0f ) {
					continue;
				}

				/* dark lightmap seams */
				if ( dark ) {
					if ( lightmapNum == 0 ) {
						VectorMA( averageColor, 2.0f, ambientColor, averageColor );
					}
					samples += 2.0f;
				}

				/* average it */
				if ( filterColor ) {
					VectorDivide( averageColor, samples, luxel );
					luxel[ 3 ] = 1.0f;
				}
				if ( filterDir ) {
					VectorDivide( averageDir, samples, deluxel );
				}

				/* set cluster to -3 */
				if ( *cluster < 0 ) {
					*cluster = CLUSTER_FLOODED;
				}
			}
		}
	}
}



/*
   IlluminateVertexes()
   light the surface vertexes
 */

#define VERTEX_NUDGE    4.0f

void IlluminateVertexes( int num ){
	int i, x, y, z, x1, y1, z1, sx, sy, radius, maxRadius, *cluster;
	int lightmapNum, numAvg;
	float samples, *vertLuxel, *radVertLuxel, *luxel, dirt = 0.0f;
	vec3_t origin = {0, 0, 0}, temp, temp2, colors[ MAX_LIGHTMAPS ], avgColors[ MAX_LIGHTMAPS ];
	bspDrawSurface_t    *ds;
	surfaceInfo_t       *info;
	rawLightmap_t       *lm;
	bspDrawVert_t       *verts;
	trace_t             trace = {};

	/* get surface, info, and raw lightmap */
	ds = &bspDrawSurfaces[ num ];
	info = &surfaceInfos[ num ];
	lm = info->lm;

	/* -----------------------------------------------------------------
	   illuminate the vertexes
	   ----------------------------------------------------------------- */

	/* calculate vertex lighting for surfaces without lightmaps */
	if ( lm == NULL || cpmaHack ) {
		/* setup trace */
		trace.testOcclusion = ( cpmaHack && lm != NULL ) ? qfalse : !noTrace;
		trace.forceSunlight = info->si->forceSunlight;
		trace.recvShadows = info->recvShadows;
		trace.numSurfaces = 1;
		trace.surfaces = &num;
		trace.inhibitRadius = DEFAULT_INHIBIT_RADIUS;
		trace.testAll = qfalse;

		/* twosided lighting */
		trace.twoSided = info->si->twoSided;

		/* make light list for this surface */
		CreateTraceLightsForSurface( num, &trace );

		/* setup */
		verts = yDrawVerts + ds->firstVert;
		numAvg = 0;
		memset( avgColors, 0, sizeof( avgColors ) );

		/* walk the surface verts */
		for ( i = 0; i < ds->numVerts; i++ )
		{
			/* get vertex luxel */
			radVertLuxel = RAD_VERTEX_LUXEL( 0, ds->firstVert + i );

			/* color the luxel with raw lightmap num? */
			if ( debugSurfaces ) {
				VectorCopy( debugColors[ num % 12 ], radVertLuxel );
			}

			/* color the luxel with luxel origin? */
			else if ( debugOrigin ) {
				VectorSubtract( info->maxs, info->mins, temp );
				VectorScale( temp, ( 1.0f / 255.0f ), temp );
				VectorSubtract( origin, lm->mins, temp2 );
				radVertLuxel[ 0 ] = info->mins[ 0 ] + ( temp[ 0 ] * temp2[ 0 ] );
				radVertLuxel[ 1 ] = info->mins[ 1 ] + ( temp[ 1 ] * temp2[ 1 ] );
				radVertLuxel[ 2 ] = info->mins[ 2 ] + ( temp[ 2 ] * temp2[ 2 ] );
			}

			/* color the luxel with the normal */
			else if ( normalmap ) {
				radVertLuxel[ 0 ] = ( verts[ i ].normal[ 0 ] + 1.0f ) * 127.5f;
				radVertLuxel[ 1 ] = ( verts[ i ].normal[ 1 ] + 1.0f ) * 127.5f;
				radVertLuxel[ 2 ] = ( verts[ i ].normal[ 2 ] + 1.0f ) * 127.5f;
			}

			/* illuminate the vertex */
			else
			{
				/* clear vertex luxel */
				VectorSet( radVertLuxel, -1.0f, -1.0f, -1.0f );

				/* try at initial origin */
				trace.cluster = ClusterForPointExtFilter( verts[ i ].xyz, VERTEX_EPSILON, info->numSurfaceClusters, &surfaceClusters[ info->firstSurfaceCluster ] );
				if ( trace.cluster >= 0 ) {
					/* setup trace */
					VectorCopy( verts[ i ].xyz, trace.origin );
					VectorCopy( verts[ i ].normal, trace.normal );

					/* r7 dirt */
					if ( dirty ) {
						dirt = DirtForSample( &trace );
					}
					else{
						dirt = 1.0f;
					}

					/* trace */
					LightingAtSample( &trace, ds->vertexStyles, colors );

					/* store */
					for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
					{
						/* r7 dirt */
						VectorScale( colors[ lightmapNum ], dirt, colors[ lightmapNum ] );

						/* store */
						radVertLuxel = RAD_VERTEX_LUXEL( lightmapNum, ds->firstVert + i );
						VectorCopy( colors[ lightmapNum ], radVertLuxel );
						VectorAdd( avgColors[ lightmapNum ], colors[ lightmapNum ], colors[ lightmapNum ] );
					}
				}

				/* is this sample bright enough? */
				radVertLuxel = RAD_VERTEX_LUXEL( 0, ds->firstVert + i );
				if ( radVertLuxel[ 0 ] <= ambientColor[ 0 ] &&
					 radVertLuxel[ 1 ] <= ambientColor[ 1 ] &&
					 radVertLuxel[ 2 ] <= ambientColor[ 2 ] ) {
					/* nudge the sample point around a bit */
					for ( x = 0; x < 4; x++ )
					{
						/* two's complement 0, 1, -1, 2, -2, etc */
						x1 = ( ( x >> 1 ) ^ ( x & 1 ? -1 : 0 ) ) + ( x & 1 );

						for ( y = 0; y < 4; y++ )
						{
							y1 = ( ( y >> 1 ) ^ ( y & 1 ? -1 : 0 ) ) + ( y & 1 );

							for ( z = 0; z < 4; z++ )
							{
								z1 = ( ( z >> 1 ) ^ ( z & 1 ? -1 : 0 ) ) + ( z & 1 );

								/* nudge origin */
								trace.origin[ 0 ] = verts[ i ].xyz[ 0 ] + ( VERTEX_NUDGE * x1 );
								trace.origin[ 1 ] = verts[ i ].xyz[ 1 ] + ( VERTEX_NUDGE * y1 );
								trace.origin[ 2 ] = verts[ i ].xyz[ 2 ] + ( VERTEX_NUDGE * z1 );

								/* try at nudged origin */
								trace.cluster = ClusterForPointExtFilter( origin, VERTEX_EPSILON, info->numSurfaceClusters, &surfaceClusters[ info->firstSurfaceCluster ] );
								if ( trace.cluster < 0 ) {
									continue;
								}

								/* trace */
								LightingAtSample( &trace, ds->vertexStyles, colors );

								/* store */
								for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
								{
									/* r7 dirt */
									VectorScale( colors[ lightmapNum ], dirt, colors[ lightmapNum ] );

									/* store */
									radVertLuxel = RAD_VERTEX_LUXEL( lightmapNum, ds->firstVert + i );
									VectorCopy( colors[ lightmapNum ], radVertLuxel );
								}

								/* bright enough? */
								radVertLuxel = RAD_VERTEX_LUXEL( 0, ds->firstVert + i );
								if ( radVertLuxel[ 0 ] > ambientColor[ 0 ] ||
									 radVertLuxel[ 1 ] > ambientColor[ 1 ] ||
									 radVertLuxel[ 2 ] > ambientColor[ 2 ] ) {
									x = y = z = 1000;
								}
							}
						}
					}
				}

				/* add to average? */
				radVertLuxel = RAD_VERTEX_LUXEL( 0, ds->firstVert + i );
				if ( radVertLuxel[ 0 ] > ambientColor[ 0 ] ||
					 radVertLuxel[ 1 ] > ambientColor[ 1 ] ||
					 radVertLuxel[ 2 ] > ambientColor[ 2 ] ) {
					numAvg++;
					for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
					{
						radVertLuxel = RAD_VERTEX_LUXEL( lightmapNum, ds->firstVert + i );
						VectorAdd( avgColors[ lightmapNum ], radVertLuxel, avgColors[ lightmapNum ] );
					}
				}
			}

			/* another happy customer */
			pthread_mutex_lock( &master_mutex );
			numVertsIlluminated++;
			pthread_mutex_unlock( &master_mutex );
		}

		/* set average color */
		if ( numAvg > 0 ) {
			for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
				VectorScale( avgColors[ lightmapNum ], ( 1.0f / numAvg ), avgColors[ lightmapNum ] );
		}
		else
		{
			VectorCopy( ambientColor, avgColors[ 0 ] );
		}

		/* clean up and store vertex color */
		for ( i = 0; i < ds->numVerts; i++ )
		{
			/* get vertex luxel */
			radVertLuxel = RAD_VERTEX_LUXEL( 0, ds->firstVert + i );

			/* store average in occluded vertexes */
			if ( radVertLuxel[ 0 ] < 0.0f ) {
				for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
				{
					radVertLuxel = RAD_VERTEX_LUXEL( lightmapNum, ds->firstVert + i );
					VectorCopy( avgColors[ lightmapNum ], radVertLuxel );

					/* debug code */
					//%	VectorSet( radVertLuxel, 255.0f, 0.0f, 0.0f );
				}
			}

			/* store it */
			for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
			{
				/* get luxels */
				vertLuxel = VERTEX_LUXEL( lightmapNum, ds->firstVert + i );
				radVertLuxel = RAD_VERTEX_LUXEL( lightmapNum, ds->firstVert + i );

				/* store */
				if ( bouncing || bounce == 0 || !bounceOnly ) {
					VectorAdd( vertLuxel, radVertLuxel, vertLuxel );
				}
				if ( !info->si->noVertexLight ) {
					ColorToBytes( vertLuxel, verts[ i ].color[ lightmapNum ], info->si->vertexScale );
				}
			}
		}

		/* free light list */
		FreeTraceLights( &trace );

		/* return to sender */
		return;
	}

	/* -----------------------------------------------------------------
	   reconstitute vertex lighting from the luxels
	   ----------------------------------------------------------------- */

	/* set styles from lightmap */
	for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
		ds->vertexStyles[ lightmapNum ] = lm->styles[ lightmapNum ];

	/* get max search radius */
	maxRadius = lm->sw;
	maxRadius = maxRadius > lm->sh ? maxRadius : lm->sh;

	/* walk the surface verts */
	verts = yDrawVerts + ds->firstVert;
	for ( i = 0; i < ds->numVerts; i++ )
	{
		/* do each lightmap */
		for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
		{
			/* early out */
			if ( lm->superLuxels[ lightmapNum ] == NULL ) {
				continue;
			}

			/* get luxel coords */
			x = verts[ i ].lightmap[ lightmapNum ][ 0 ];
			y = verts[ i ].lightmap[ lightmapNum ][ 1 ];
			if ( x < 0 ) {
				x = 0;
			}
			else if ( x >= lm->sw ) {
				x = lm->sw - 1;
			}
			if ( y < 0 ) {
				y = 0;
			}
			else if ( y >= lm->sh ) {
				y = lm->sh - 1;
			}

			/* get vertex luxels */
			vertLuxel = VERTEX_LUXEL( lightmapNum, ds->firstVert + i );
			radVertLuxel = RAD_VERTEX_LUXEL( lightmapNum, ds->firstVert + i );

			/* color the luxel with the normal? */
			if ( normalmap ) {
				radVertLuxel[ 0 ] = ( verts[ i ].normal[ 0 ] + 1.0f ) * 127.5f;
				radVertLuxel[ 1 ] = ( verts[ i ].normal[ 1 ] + 1.0f ) * 127.5f;
				radVertLuxel[ 2 ] = ( verts[ i ].normal[ 2 ] + 1.0f ) * 127.5f;
			}

			/* color the luxel with surface num? */
			else if ( debugSurfaces ) {
				VectorCopy( debugColors[ num % 12 ], radVertLuxel );
			}

			/* divine color from the superluxels */
			else
			{
				/* increasing radius */
				VectorClear( radVertLuxel );
				samples = 0.0f;
				for ( radius = 0; radius < maxRadius && samples <= 0.0f; radius++ )
				{
					/* sample within radius */
					for ( sy = ( y - radius ); sy <= ( y + radius ); sy++ )
					{
						if ( sy < 0 || sy >= lm->sh ) {
							continue;
						}

						for ( sx = ( x - radius ); sx <= ( x + radius ); sx++ )
						{
							if ( sx < 0 || sx >= lm->sw ) {
								continue;
							}

							/* get luxel particulars */
							luxel = SUPER_LUXEL( lightmapNum, sx, sy );
							cluster = SUPER_CLUSTER( sx, sy );
							if ( *cluster < 0 ) {
								continue;
							}

							/* testing: must be brigher than ambient color */
							//%	if( luxel[ 0 ] <= ambientColor[ 0 ] || luxel[ 1 ] <= ambientColor[ 1 ] || luxel[ 2 ] <= ambientColor[ 2 ] )
							//%		continue;

							/* add its distinctiveness to our own */
							VectorAdd( radVertLuxel, luxel, radVertLuxel );
							samples += luxel[ 3 ];
						}
					}
				}

				/* any color? */
				if ( samples > 0.0f ) {
					VectorDivide( radVertLuxel, samples, radVertLuxel );
				}
				else{
					VectorCopy( ambientColor, radVertLuxel );
				}
			}

			/* store into floating point storage */
			VectorAdd( vertLuxel, radVertLuxel, vertLuxel );
			pthread_mutex_lock( &master_mutex );
			numVertsIlluminated++;
			pthread_mutex_unlock( &master_mutex );

			/* store into bytes (for vertex approximation) */
			if ( !info->si->noVertexLight ) {
				ColorToBytes( vertLuxel, verts[ i ].color[ lightmapNum ], 1.0f );
			}
		}
	}
}



/* -------------------------------------------------------------------------------

   light optimization (-fast)

   creates a list of lights that will affect a surface and stores it in tw
   this is to optimize surface lighting by culling out as many of the
   lights in the world as possible from further calculation

   ------------------------------------------------------------------------------- */

/*
   SetupBrushes()
   determines opaque brushes in the world and find sky shaders for sunlight calculations
 */

void SetupBrushes( void ){
	int i, j, b, compileFlags;
	qboolean inside;
	bspBrush_t      *brush;
	bspBrushSide_t  *side;
	bspShader_t     *shader;
	shaderInfo_t    *si;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- SetupBrushes ---\n" );

	/* allocate */
	if ( opaqueBrushes == NULL ) {
		opaqueBrushes = safe_malloc( numBSPBrushes / 8 + 1 );
	}

	/* clear */
	memset( opaqueBrushes, 0, numBSPBrushes / 8 + 1 );
	numOpaqueBrushes = 0;

	/* walk the list of worldspawn brushes */
	for ( i = 0; i < bspModels[ 0 ].numBSPBrushes; i++ )
	{
		/* get brush */
		b = bspModels[ 0 ].firstBSPBrush + i;
		brush = &bspBrushes[ b ];

		/* check all sides */
		inside = qtrue;
		compileFlags = 0;
		for ( j = 0; j < brush->numSides && inside; j++ )
		{
			/* do bsp shader calculations */
			side = &bspBrushSides[ brush->firstSide + j ];
			shader = &bspShaders[ side->shaderNum ];

			/* get shader info */
			si = ShaderInfoForShader( shader->shader );
			if ( si == NULL ) {
				continue;
			}

			/* or together compile flags */
			compileFlags |= si->compileFlags;
		}

		/* determine if this brush is opaque to light */
		if ( !( compileFlags & C_TRANSLUCENT ) ) {
			opaqueBrushes[ b >> 3 ] |= ( 1 << ( b & 7 ) );
			numOpaqueBrushes++;
			maxOpaqueBrush = i;
		}
	}

	/* emit some statistics */
	Sys_FPrintf( SYS_VRB, "%9d opaque brushes\n", numOpaqueBrushes );
}



/*
   ClusterVisible()
   determines if two clusters are visible to each other using the PVS
 */

qboolean ClusterVisible( int a, int b ){
	int portalClusters, leafBytes;
	byte        *pvs;


	/* dummy check */
	if ( a < 0 || b < 0 ) {
		return qfalse;
	}

	/* early out */
	if ( a == b ) {
		return qtrue;
	}

	/* not vised? */
	if ( numBSPVisBytes <= 8 ) {
		return qtrue;
	}

	/* get pvs data */
	portalClusters = ( (int *) bspVisBytes )[ 0 ];
	leafBytes = ( (int*) bspVisBytes )[ 1 ];
	pvs = bspVisBytes + VIS_HEADER_SIZE + ( a * leafBytes );

	/* check */
	if ( ( pvs[ b >> 3 ] & ( 1 << ( b & 7 ) ) ) ) {
		return qtrue;
	}
	return qfalse;
}



/*
   PointInLeafNum_r()
   borrowed from vlight.c
 */

int PointInLeafNum_r( vec3_t point, int nodenum ){
	int leafnum;
	vec_t dist;
	bspNode_t       *node;
	bspPlane_t  *plane;


	while ( nodenum >= 0 )
	{
		node = &bspNodes[ nodenum ];
		plane = &bspPlanes[ node->planeNum ];
		dist = DotProduct( point, plane->normal ) - plane->dist;
		if ( dist > 0.1 ) {
			nodenum = node->children[ 0 ];
		}
		else if ( dist < -0.1 ) {
			nodenum = node->children[ 1 ];
		}
		else
		{
			leafnum = PointInLeafNum_r( point, node->children[ 0 ] );
			if ( bspLeafs[ leafnum ].cluster != -1 ) {
				return leafnum;
			}
			nodenum = node->children[ 1 ];
		}
	}

	leafnum = -nodenum - 1;
	return leafnum;
}



/*
   PointInLeafnum()
   borrowed from vlight.c
 */

int PointInLeafNum( vec3_t point ){
	return PointInLeafNum_r( point, 0 );
}



/*
   ClusterVisibleToPoint() - ydnar
   returns qtrue if point can "see" cluster
 */

qboolean ClusterVisibleToPoint( vec3_t point, int cluster ){
	int pointCluster;


	/* get leafNum for point */
	pointCluster = ClusterForPoint( point );
	if ( pointCluster < 0 ) {
		return qfalse;
	}

	/* check pvs */
	return ClusterVisible( pointCluster, cluster );
}



/*
   ClusterForPoint() - ydnar
   returns the pvs cluster for point
 */

int ClusterForPoint( vec3_t point ){
	int leafNum;


	/* get leafNum for point */
	leafNum = PointInLeafNum( point );
	if ( leafNum < 0 ) {
		return -1;
	}

	/* return the cluster */
	return bspLeafs[ leafNum ].cluster;
}



/*
   ClusterForPointExt() - ydnar
   also takes brushes into account for occlusion testing
 */

int ClusterForPointExt( vec3_t point, float epsilon ){
	int i, j, b, leafNum, cluster;
	float dot;
	qboolean inside;
	int             *brushes, numBSPBrushes;
	bspLeaf_t       *leaf;
	bspBrush_t      *brush;
	bspPlane_t      *plane;


	/* get leaf for point */
	leafNum = PointInLeafNum( point );
	if ( leafNum < 0 ) {
		return -1;
	}
	leaf = &bspLeafs[ leafNum ];

	/* get the cluster */
	cluster = leaf->cluster;
	if ( cluster < 0 ) {
		return -1;
	}

	/* transparent leaf, so check point against all brushes in the leaf */
	brushes = &bspLeafBrushes[ leaf->firstBSPLeafBrush ];
	numBSPBrushes = leaf->numBSPLeafBrushes;
	for ( i = 0; i < numBSPBrushes; i++ )
	{
		/* get parts */
		b = brushes[ i ];
		if ( b > maxOpaqueBrush ) {
			continue;
		}
		brush = &bspBrushes[ b ];
		if ( !( opaqueBrushes[ b >> 3 ] & ( 1 << ( b & 7 ) ) ) ) {
			continue;
		}

		/* check point against all planes */
		inside = qtrue;
		for ( j = 0; j < brush->numSides && inside; j++ )
		{
			plane = &bspPlanes[ bspBrushSides[ brush->firstSide + j ].planeNum ];
			dot = DotProduct( point, plane->normal );
			dot -= plane->dist;
			if ( dot > epsilon ) {
				inside = qfalse;
			}
		}

		/* if inside, return bogus cluster */
		if ( inside ) {
			return -1 - b;
		}
	}

	/* if the point made it this far, it's not inside any opaque brushes */
	return cluster;
}



/*
   ClusterForPointExtFilter() - ydnar
   adds cluster checking against a list of known valid clusters
 */

int ClusterForPointExtFilter( vec3_t point, float epsilon, int numClusters, int *clusters ){
	int i, cluster;


	/* get cluster for point */
	cluster = ClusterForPointExt( point, epsilon );

	/* check if filtering is necessary */
	if ( cluster < 0 || numClusters <= 0 || clusters == NULL ) {
		return cluster;
	}

	/* filter */
	for ( i = 0; i < numClusters; i++ )
	{
		if ( cluster == clusters[ i ] || ClusterVisible( cluster, clusters[ i ] ) ) {
			return cluster;
		}
	}

	/* failed */
	return -1;
}



/*
   ShaderForPointInLeaf() - ydnar
   checks a point against all brushes in a leaf, returning the shader of the brush
   also sets the cumulative surface and content flags for the brush hit
 */

int ShaderForPointInLeaf( vec3_t point, int leafNum, float epsilon, int wantContentFlags, int wantSurfaceFlags, int *contentFlags, int *surfaceFlags ){
	int i, j;
	float dot;
	qboolean inside;
	int             *brushes, numBSPBrushes;
	bspLeaf_t           *leaf;
	bspBrush_t      *brush;
	bspBrushSide_t  *side;
	bspPlane_t      *plane;
	bspShader_t     *shader;
	int allSurfaceFlags, allContentFlags;


	/* clear things out first */
	*surfaceFlags = 0;
	*contentFlags = 0;

	/* get leaf */
	if ( leafNum < 0 ) {
		return -1;
	}
	leaf = &bspLeafs[ leafNum ];

	/* transparent leaf, so check point against all brushes in the leaf */
	brushes = &bspLeafBrushes[ leaf->firstBSPLeafBrush ];
	numBSPBrushes = leaf->numBSPLeafBrushes;
	for ( i = 0; i < numBSPBrushes; i++ )
	{
		/* get parts */
		brush = &bspBrushes[ brushes[ i ] ];

		/* check point against all planes */
		inside = qtrue;
		allSurfaceFlags = 0;
		allContentFlags = 0;
		for ( j = 0; j < brush->numSides && inside; j++ )
		{
			side = &bspBrushSides[ brush->firstSide + j ];
			plane = &bspPlanes[ side->planeNum ];
			dot = DotProduct( point, plane->normal );
			dot -= plane->dist;
			if ( dot > epsilon ) {
				inside = qfalse;
			}
			else
			{
				shader = &bspShaders[ side->shaderNum ];
				allSurfaceFlags |= shader->surfaceFlags;
				allContentFlags |= shader->contentFlags;
			}
		}

		/* handle if inside */
		if ( inside ) {
			/* if there are desired flags, check for same and continue if they aren't matched */
			if ( wantContentFlags && !( wantContentFlags & allContentFlags ) ) {
				continue;
			}
			if ( wantSurfaceFlags && !( wantSurfaceFlags & allSurfaceFlags ) ) {
				continue;
			}

			/* store the cumulative flags and return the brush shader (which is mostly useless) */
			*surfaceFlags = allSurfaceFlags;
			*contentFlags = allContentFlags;
			return brush->shaderNum;
		}
	}

	/* if the point made it this far, it's not inside any brushes */
	return -1;
}



/*
   ChopBounds()
   chops a bounding box by the plane defined by origin and normal
   returns qfalse if the bounds is entirely clipped away

   this is not exactly the fastest way to do this...
 */

qboolean ChopBounds( vec3_t mins, vec3_t maxs, vec3_t origin, vec3_t normal ){
	/* FIXME: rewrite this so it doesn't use bloody brushes */
	return qtrue;
}



/*
   SetupEnvelopes()
   calculates each light's effective envelope,
   taking into account brightness, type, and pvs.
 */

#define LIGHT_EPSILON   0.125f
#define LIGHT_NUDGE     2.0f

void SetupEnvelopes( qboolean forGrid, qboolean fastFlag ){
	int i, x, y, z, x1, y1, z1;
	light_t     *light, *light2, **owner;
	bspLeaf_t   *leaf;
	vec3_t origin, dir, mins, maxs;
	float radius, intensity;
	light_t     *buckets[ 256 ];


	/* early out for weird cases where there are no lights */
	if ( lights == NULL ) {
		return;
	}

	/* note it */
	Sys_FPrintf( SYS_VRB, "--- SetupEnvelopes%s ---\n", fastFlag ? " (fast)" : "" );

	/* count lights */
	numLights = 0;
	numCulledLights = 0;
	owner = &lights;
	while ( *owner != NULL )
	{
		/* get light */
		light = *owner;

		/* handle negative lights */
		if ( light->photons < 0.0f || light->add < 0.0f ) {
			light->photons *= -1.0f;
			light->add *= -1.0f;
			light->flags |= LIGHT_NEGATIVE;
		}

		/* sunlight? */
		if ( light->type == EMIT_SUN ) {
			/* special cased */
			light->cluster = 0;
			light->envelope = MAX_WORLD_COORD * 8.0f;
			VectorSet( light->mins, MIN_WORLD_COORD * 8.0f, MIN_WORLD_COORD * 8.0f, MIN_WORLD_COORD * 8.0f );
			VectorSet( light->maxs, MAX_WORLD_COORD * 8.0f, MAX_WORLD_COORD * 8.0f, MAX_WORLD_COORD * 8.0f );
		}

		/* everything else */
		else
		{
			/* get pvs cluster for light */
			light->cluster = ClusterForPointExt( light->origin, LIGHT_EPSILON );

			/* invalid cluster? */
			if ( light->cluster < 0 ) {
				/* nudge the sample point around a bit */
				for ( x = 0; x < 4; x++ )
				{
					/* two's complement 0, 1, -1, 2, -2, etc */
					x1 = ( ( x >> 1 ) ^ ( x & 1 ? -1 : 0 ) ) + ( x & 1 );

					for ( y = 0; y < 4; y++ )
					{
						y1 = ( ( y >> 1 ) ^ ( y & 1 ? -1 : 0 ) ) + ( y & 1 );

						for ( z = 0; z < 4; z++ )
						{
							z1 = ( ( z >> 1 ) ^ ( z & 1 ? -1 : 0 ) ) + ( z & 1 );

							/* nudge origin */
							origin[ 0 ] = light->origin[ 0 ] + ( LIGHT_NUDGE * x1 );
							origin[ 1 ] = light->origin[ 1 ] + ( LIGHT_NUDGE * y1 );
							origin[ 2 ] = light->origin[ 2 ] + ( LIGHT_NUDGE * z1 );

							/* try at nudged origin */
							light->cluster = ClusterForPointExt( origin, LIGHT_EPSILON );
							if ( light->cluster < 0 ) {
								continue;
							}

							/* set origin */
							VectorCopy( origin, light->origin );
						}
					}
				}
			}

			/* only calculate for lights in pvs and outside of opaque brushes */
			if ( light->cluster >= 0 ) {
				/* set light fast flag */
				if ( fastFlag ) {
					light->flags |= LIGHT_FAST_TEMP;
				}
				else{
					light->flags &= ~LIGHT_FAST_TEMP;
				}
				if ( light->si && light->si->noFast ) {
					light->flags &= ~( LIGHT_FAST | LIGHT_FAST_TEMP );
				}

				/* clear light envelope */
				light->envelope = 0;

				/* handle area lights */
				if ( exactPointToPolygon && light->type == EMIT_AREA && light->w != NULL ) {
					/* ugly hack to calculate extent for area lights, but only done once */
					VectorScale( light->normal, -1.0f, dir );
					for ( radius = 100.0f; radius < 130000.0f && light->envelope == 0; radius += 10.0f )
					{
						float factor;

						VectorMA( light->origin, radius, light->normal, origin );
						factor = PointToPolygonFormFactor( origin, dir, light->w );
						if ( factor < 0.0f ) {
							factor *= -1.0f;
						}
						if ( ( factor * light->add ) <= light->falloffTolerance ) {
							light->envelope = radius;
						}
					}

					/* check for fast mode */
					if ( !( light->flags & LIGHT_FAST ) && !( light->flags & LIGHT_FAST_TEMP ) ) {
						light->envelope = MAX_WORLD_COORD * 8.0f;
					}
				}
				else
				{
					radius = 0.0f;
					intensity = light->photons;
				}

				/* other calcs */
				if ( light->envelope <= 0.0f ) {
					/* solve distance for non-distance lights */
					if ( !( light->flags & LIGHT_ATTEN_DISTANCE ) ) {
						light->envelope = MAX_WORLD_COORD * 8.0f;
					}

					/* solve distance for linear lights */
					else if ( ( light->flags & LIGHT_ATTEN_LINEAR ) ) {
						//% light->envelope = ((intensity / light->falloffTolerance) * linearScale - 1 + radius) / light->fade;
						light->envelope = ( ( intensity * linearScale ) - light->falloffTolerance ) / light->fade;
					}

					/*
					   add = angle * light->photons * linearScale - (dist * light->fade);
					   T = (light->photons * linearScale) - (dist * light->fade);
					   T + (dist * light->fade) = (light->photons * linearScale);
					   dist * light->fade = (light->photons * linearScale) - T;
					   dist = ((light->photons * linearScale) - T) / light->fade;
					 */

					/* solve for inverse square falloff */
					else{
						light->envelope = sqrt( intensity / light->falloffTolerance ) + radius;
					}

					/*
					   add = light->photons / (dist * dist);
					   T = light->photons / (dist * dist);
					   T * (dist * dist) = light->photons;
					   dist = sqrt( light->photons / T );
					 */
				}

				/* chop radius against pvs */
				{
					/* clear bounds */
					ClearBounds( mins, maxs );

					/* check all leaves */
					for ( i = 0; i < numBSPLeafs; i++ )
					{
						/* get test leaf */
						leaf = &bspLeafs[ i ];

						/* in pvs? */
						if ( leaf->cluster < 0 ) {
							continue;
						}
						if ( ClusterVisible( light->cluster, leaf->cluster ) == qfalse ) { /* ydnar: thanks Arnout for exposing my stupid error (this never failed before) */
							continue;
						}

						/* add this leafs bbox to the bounds */
						VectorCopy( leaf->mins, origin );
						AddPointToBounds( origin, mins, maxs );
						VectorCopy( leaf->maxs, origin );
						AddPointToBounds( origin, mins, maxs );
					}

					/* test to see if bounds encompass light */
					for ( i = 0; i < 3; i++ )
					{
						if ( mins[ i ] > light->origin[ i ] || maxs[ i ] < light->origin[ i ] ) {
							//% Sys_FPrintf( SYS_WRN, "WARNING: Light PVS bounds (%.0f, %.0f, %.0f) -> (%.0f, %.0f, %.0f)\ndo not encompass light %d (%f, %f, %f)\n",
							//%     mins[ 0 ], mins[ 1 ], mins[ 2 ],
							//%     maxs[ 0 ], maxs[ 1 ], maxs[ 2 ],
							//%     numLights, light->origin[ 0 ], light->origin[ 1 ], light->origin[ 2 ] );
							AddPointToBounds( light->origin, mins, maxs );
						}
					}

					/* chop the bounds by a plane for area lights and spotlights */
					if ( light->type == EMIT_AREA || light->type == EMIT_SPOT ) {
						ChopBounds( mins, maxs, light->origin, light->normal );
					}

					/* copy bounds */
					VectorCopy( mins, light->mins );
					VectorCopy( maxs, light->maxs );

					/* reflect bounds around light origin */
					//%	VectorMA( light->origin, -1.0f, origin, origin );
					VectorScale( light->origin, 2, origin );
					VectorSubtract( origin, maxs, origin );
					AddPointToBounds( origin, mins, maxs );
					//%	VectorMA( light->origin, -1.0f, mins, origin );
					VectorScale( light->origin, 2, origin );
					VectorSubtract( origin, mins, origin );
					AddPointToBounds( origin, mins, maxs );

					/* calculate spherical bounds */
					VectorSubtract( maxs, light->origin, dir );
					radius = (float) VectorLength( dir );

					/* if this radius is smaller than the envelope, then set the envelope to it */
					if ( radius < light->envelope ) {
						light->envelope = radius;
						//%	Sys_FPrintf( SYS_VRB, "PVS Cull (%d): culled\n", numLights );
					}
					//%	else
					//%		Sys_FPrintf( SYS_VRB, "PVS Cull (%d): failed (%8.0f > %8.0f)\n", numLights, radius, light->envelope );
				}

				/* add grid/surface only check */
				if ( forGrid ) {
					if ( !( light->flags & LIGHT_GRID ) ) {
						light->envelope = 0.0f;
					}
				}
				else
				{
					if ( !( light->flags & LIGHT_SURFACES ) ) {
						light->envelope = 0.0f;
					}
				}
			}

			/* culled? */
			if ( light->cluster < 0 || light->envelope <= 0.0f ) {
				/* debug code */
				//%	Sys_Printf( "Culling light: Cluster: %d Envelope: %f\n", light->cluster, light->envelope );

				/* delete the light */
				numCulledLights++;
				*owner = light->next;
				if ( light->w != NULL ) {
					free( light->w );
				}
				free( light );
				continue;
			}
		}

		/* square envelope */
		light->envelope2 = ( light->envelope * light->envelope );

		/* increment light count */
		numLights++;

		/* set next light */
		owner = &( ( **owner ).next );
	}

	/* bucket sort lights by style */
	memset( buckets, 0, sizeof( buckets ) );
	light2 = NULL;
	for ( light = lights; light != NULL; light = light2 )
	{
		/* get next light */
		light2 = light->next;

		/* filter into correct bucket */
		light->next = buckets[ light->style ];
		buckets[ light->style ] = light;

		/* if any styled light is present, automatically set nocollapse */
		if ( light->style != LS_NORMAL ) {
			noCollapse = qtrue;
		}
	}

	/* filter back into light list */
	lights = NULL;
	for ( i = 255; i >= 0; i-- )
	{
		light2 = NULL;
		for ( light = buckets[ i ]; light != NULL; light = light2 )
		{
			light2 = light->next;
			light->next = lights;
			lights = light;
		}
	}

	/* emit some statistics */
	Sys_Printf( "%9d total lights\n", numLights );
	Sys_Printf( "%9d culled lights\n", numCulledLights );
}



/*
   CreateTraceLightsForBounds()
   creates a list of lights that affect the given bounding box and pvs clusters (bsp leaves)
 */

void CreateTraceLightsForBounds( vec3_t mins, vec3_t maxs, vec3_t normal, int numClusters, int *clusters, int flags, trace_t *trace ){
	int i;
	light_t     *light;
	vec3_t origin, dir, nullVector = { 0.0f, 0.0f, 0.0f };
	float radius, dist, length;


	/* potential pre-setup  */
	if ( numLights == 0 ) {
		SetupEnvelopes( qfalse, fast );
	}

	/* debug code */
	//% Sys_Printf( "CTWLFB: (%4.1f %4.1f %4.1f) (%4.1f %4.1f %4.1f)\n", mins[ 0 ], mins[ 1 ], mins[ 2 ], maxs[ 0 ], maxs[ 1 ], maxs[ 2 ] );

	/* allocate the light list */
	trace->lights = safe_malloc( sizeof( light_t* ) * ( numLights + 1 ) );
	trace->numLights = 0;

	/* calculate spherical bounds */
	VectorAdd( mins, maxs, origin );
	VectorScale( origin, 0.5f, origin );
	VectorSubtract( maxs, origin, dir );
	radius = (float) VectorLength( dir );

	/* get length of normal vector */
	if ( normal != NULL ) {
		length = VectorLength( normal );
	}
	else
	{
		normal = nullVector;
		length = 0;
	}

	/* test each light and see if it reaches the sphere */
	/* note: the attenuation code MUST match LightingAtSample() */
	for ( light = lights; light; light = light->next )
	{
		/* check zero sized envelope */
		if ( light->envelope <= 0 ) {
			pthread_mutex_lock( &master_mutex );
			lightsEnvelopeCulled++;
			pthread_mutex_unlock( &master_mutex );
			continue;
		}

		/* check flags */
		if ( !( light->flags & flags ) ) {
			continue;
		}

		/* sunlight skips all this nonsense */
		if ( light->type != EMIT_SUN ) {
			/* sun only? */
			if ( sunOnly ) {
				continue;
			}

			/* check against pvs cluster */
			if ( numClusters > 0 && clusters != NULL ) {
				for ( i = 0; i < numClusters; i++ )
				{
					if ( ClusterVisible( light->cluster, clusters[ i ] ) ) {
						break;
					}
				}

				/* fixme! */
				if ( i == numClusters ) {
					pthread_mutex_lock( &master_mutex );
					lightsClusterCulled++;
					pthread_mutex_unlock( &master_mutex );
					continue;
				}
			}

			/* if the light's bounding sphere intersects with the bounding sphere then this light needs to be tested */
			VectorSubtract( light->origin, origin, dir );
			dist = VectorLength( dir );
			dist -= light->envelope;
			dist -= radius;
			if ( dist > 0 ) {
				pthread_mutex_lock( &master_mutex );
				lightsEnvelopeCulled++;
				pthread_mutex_unlock( &master_mutex );
				continue;
			}

			/* check bounding box against light's pvs envelope (note: this code never eliminated any lights, so disabling it) */
			#if 0
			skip = qfalse;
			for ( i = 0; i < 3; i++ )
			{
				if ( mins[ i ] > light->maxs[ i ] || maxs[ i ] < light->mins[ i ] ) {
					skip = qtrue;
				}
			}
			if ( skip ) {
				lightsBoundsCulled++;
				continue;
			}
			#endif
		}

		/* planar surfaces (except twosided surfaces) have a couple more checks */
		if ( length > 0.0f && trace->twoSided == qfalse ) {
			/* lights coplanar with a surface won't light it */
			if ( !( light->flags & LIGHT_TWOSIDED ) && DotProduct( light->normal, normal ) > 0.999f ) {
				pthread_mutex_lock( &master_mutex );
				lightsPlaneCulled++;
				pthread_mutex_unlock( &master_mutex );
				continue;
			}

			/* check to see if light is behind the plane */
			if ( DotProduct( light->origin, normal ) - DotProduct( origin, normal ) < -1.0f ) {
				pthread_mutex_lock( &master_mutex );
				lightsPlaneCulled++;
				pthread_mutex_unlock( &master_mutex );
				continue;
			}
		}

		/* add this light */
		trace->lights[ trace->numLights++ ] = light;
	}

	/* make last night null */
	trace->lights[ trace->numLights ] = NULL;
}



void FreeTraceLights( trace_t *trace ){
	if ( trace->lights != NULL ) {
		free( trace->lights );
	}
}



/*
   CreateTraceLightsForSurface()
   creates a list of lights that can potentially affect a drawsurface
 */

void CreateTraceLightsForSurface( int num, trace_t *trace ){
	int i;
	vec3_t mins, maxs, normal;
	bspDrawVert_t       *dv;
	bspDrawSurface_t    *ds;
	surfaceInfo_t       *info;


	/* dummy check */
	if ( num < 0 ) {
		return;
	}

	/* get drawsurface and info */
	ds = &bspDrawSurfaces[ num ];
	info = &surfaceInfos[ num ];

	/* get the mins/maxs for the dsurf */
	ClearBounds( mins, maxs );
	VectorCopy( bspDrawVerts[ ds->firstVert ].normal, normal );
	for ( i = 0; i < ds->numVerts; i++ )
	{
		dv = &yDrawVerts[ ds->firstVert + i ];
		AddPointToBounds( dv->xyz, mins, maxs );
		if ( !VectorCompare( dv->normal, normal ) ) {
			VectorClear( normal );
		}
	}

	/* create the lights for the bounding box */
	CreateTraceLightsForBounds( mins, maxs, normal, info->numSurfaceClusters, &surfaceClusters[ info->firstSurfaceCluster ], LIGHT_SURFACES, trace );
}
