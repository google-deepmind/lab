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
#define PATCH_C



/* dependencies */
#include "q3map2.h"



/*
   ExpandLongestCurve() - ydnar
   finds length of quadratic curve specified and determines if length is longer than the supplied max
 */

#define APPROX_SUBDIVISION  8

static void ExpandLongestCurve( float *longestCurve, vec3_t a, vec3_t b, vec3_t c ){
	int i;
	float t, len;
	vec3_t ab, bc, ac, pt, last, delta;


	/* calc vectors */
	VectorSubtract( b, a, ab );
	if ( VectorNormalize( ab, ab ) < 0.125f ) {
		return;
	}
	VectorSubtract( c, b, bc );
	if ( VectorNormalize( bc, bc ) < 0.125f ) {
		return;
	}
	VectorSubtract( c, a, ac );
	if ( VectorNormalize( ac, ac ) < 0.125f ) {
		return;
	}

	/* if all 3 vectors are the same direction, then this edge is linear, so we ignore it */
	if ( DotProduct( ab, bc ) > 0.99f && DotProduct( ab, ac ) > 0.99f ) {
		return;
	}

	/* recalculate vectors */
	VectorSubtract( b, a, ab );
	VectorSubtract( c, b, bc );

	/* determine length */
	VectorCopy( a, last );
	for ( i = 0, len = 0.0f, t = 0.0f; i < APPROX_SUBDIVISION; i++, t += ( 1.0f / APPROX_SUBDIVISION ) )
	{
		/* calculate delta */
		delta[ 0 ] = ( ( 1.0f - t ) * ab[ 0 ] ) + ( t * bc[ 0 ] );
		delta[ 1 ] = ( ( 1.0f - t ) * ab[ 1 ] ) + ( t * bc[ 1 ] );
		delta[ 2 ] = ( ( 1.0f - t ) * ab[ 2 ] ) + ( t * bc[ 2 ] );

		/* add to first point and calculate pt-pt delta */
		VectorAdd( a, delta, pt );
		VectorSubtract( pt, last, delta );

		/* add it to length and store last point */
		len += VectorLength( delta );
		VectorCopy( pt, last );
	}

	/* longer? */
	if ( len > *longestCurve ) {
		*longestCurve = len;
	}
}



/*
   ExpandMaxIterations() - ydnar
   determines how many iterations a quadratic curve needs to be subdivided with to fit the specified error
 */

static void ExpandMaxIterations( int *maxIterations, int maxError, vec3_t a, vec3_t b, vec3_t c ){
	int i, j;
	vec3_t prev, next, mid, delta, delta2;
	float len, len2;
	int numPoints, iterations;
	vec3_t points[ MAX_EXPANDED_AXIS ];


	/* initial setup */
	numPoints = 3;
	VectorCopy( a, points[ 0 ] );
	VectorCopy( b, points[ 1 ] );
	VectorCopy( c, points[ 2 ] );

	/* subdivide */
	for ( i = 0; i + 2 < numPoints; i += 2 )
	{
		/* check subdivision limit */
		if ( numPoints + 2 >= MAX_EXPANDED_AXIS ) {
			break;
		}

		/* calculate new curve deltas */
		for ( j = 0; j < 3; j++ )
		{
			prev[ j ] = points[ i + 1 ][ j ] - points[ i ][ j ];
			next[ j ] = points[ i + 2 ][ j ] - points[ i + 1 ][ j ];
			mid[ j ] = ( points[ i ][ j ] + points[ i + 1 ][ j ] * 2.0f + points[ i + 2 ][ j ] ) * 0.25f;
		}

		/* see if this midpoint is off far enough to subdivide */
		VectorSubtract( points[ i + 1 ], mid, delta );
		len = VectorLength( delta );
		if ( len < maxError ) {
			continue;
		}

		/* subdivide */
		numPoints += 2;

		/* create new points */
		for ( j = 0; j < 3; j++ )
		{
			prev[ j ] = 0.5f * ( points[ i ][ j ] + points[ i + 1 ][ j ] );
			next[ j ] = 0.5f * ( points[ i + 1 ][ j ] + points[ i + 2 ][ j ] );
			mid[ j ] = 0.5f * ( prev[ j ] + next[ j ] );
		}

		/* push points out */
		for ( j = numPoints - 1; j > i + 3; j-- )
			VectorCopy( points[ j - 2 ], points[ j ] );

		/* insert new points */
		VectorCopy( prev, points[ i + 1 ] );
		VectorCopy( mid, points[ i + 2 ] );
		VectorCopy( next, points[ i + 3 ] );

		/* back up and recheck this set again, it may need more subdivision */
		i -= 2;
	}

	/* put the line on the curve */
	for ( i = 1; i < numPoints; i += 2 )
	{
		for ( j = 0; j < 3; j++ )
		{
			prev[ j ] = 0.5f * ( points[ i ][ j ] + points[ i + 1 ][ j ] );
			next[ j ] = 0.5f * ( points[ i ][ j ] + points[ i - 1 ][ j ] );
			points[ i ][ j ] = 0.5f * ( prev[ j ] + next[ j ] );
		}
	}

	/* eliminate linear sections */
	for ( i = 0; i + 2 < numPoints; i++ )
	{
		/* create vectors */
		VectorSubtract( points[ i + 1 ], points[ i ], delta );
		len = VectorNormalize( delta, delta );
		VectorSubtract( points[ i + 2 ], points[ i + 1 ], delta2 );
		len2 = VectorNormalize( delta2, delta2 );

		/* if either edge is degenerate, then eliminate it */
		if ( len < 0.0625f || len2 < 0.0625f || DotProduct( delta, delta2 ) >= 1.0f ) {
			for ( j = i + 1; j + 1 < numPoints; j++ )
				VectorCopy( points[ j + 1 ], points[ j ] );
			numPoints--;
			continue;
		}
	}

	/* the number of iterations is 2^(points - 1) - 1 */
	numPoints >>= 1;
	iterations = 0;
	while ( numPoints > 1 )
	{
		numPoints >>= 1;
		iterations++;
	}

	/* more? */
	if ( iterations > *maxIterations ) {
		*maxIterations = iterations;
	}
}



/*
   ParsePatch()
   creates a mapDrawSurface_t from the patch text
 */

void ParsePatch( qboolean onlyLights ){
	vec_t info[ 5 ];
	int i, j, k;
	parseMesh_t     *pm;
	char texture[ MAX_QPATH ];
	char shader[ MAX_QPATH ];
	mesh_t m;
	bspDrawVert_t   *verts;
	epair_t         *ep;
	vec4_t delta, delta2, delta3;
	qboolean degenerate;
	float longestCurve;
	int maxIterations;


	MatchToken( "{" );

	/* get texture */
	GetToken( qtrue );
	strcpy( texture, token );

	Parse1DMatrix( 5, info );
	m.width = info[0];
	m.height = info[1];
	m.verts = verts = safe_malloc( m.width * m.height * sizeof( m.verts[0] ) );

	if ( m.width < 0 || m.width > MAX_PATCH_SIZE || m.height < 0 || m.height > MAX_PATCH_SIZE ) {
		Error( "ParsePatch: bad size" );
	}

	MatchToken( "(" );
	for ( j = 0; j < m.width ; j++ )
	{
		MatchToken( "(" );
		for ( i = 0; i < m.height ; i++ )
		{
			Parse1DMatrix( 5, verts[ i * m.width + j ].xyz );

			/* ydnar: fix colors */
			for ( k = 0; k < MAX_LIGHTMAPS; k++ )
			{
				verts[ i * m.width + j ].color[ k ][ 0 ] = 255;
				verts[ i * m.width + j ].color[ k ][ 1 ] = 255;
				verts[ i * m.width + j ].color[ k ][ 2 ] = 255;
				verts[ i * m.width + j ].color[ k ][ 3 ] = 255;
			}
		}
		MatchToken( ")" );
	}
	MatchToken( ")" );

	// if brush primitives format, we may have some epairs to ignore here
	GetToken( qtrue );
	if ( g_bBrushPrimit != BPRIMIT_OLDBRUSHES && strcmp( token,"}" ) ) {
		// NOTE: we leak that!
		ep = ParseEPair();
	}
	else{
		UnGetToken();
	}

	MatchToken( "}" );
	MatchToken( "}" );

	/* short circuit */
	if ( noCurveBrushes || onlyLights ) {
		return;
	}


	/* ydnar: delete and warn about degenerate patches */
	j = ( m.width * m.height );
	VectorClear( delta );
	delta[ 3 ] = 0;
	degenerate = qtrue;

	/* find first valid vector */
	for ( i = 1; i < j && delta[ 3 ] == 0; i++ )
	{
		VectorSubtract( m.verts[ 0 ].xyz, m.verts[ i ].xyz, delta );
		delta[ 3 ] = VectorNormalize( delta, delta );
	}

	/* secondary degenerate test */
	if ( delta[ 3 ] == 0 ) {
		degenerate = qtrue;
	}
	else
	{
		/* if all vectors match this or are zero, then this is a degenerate patch */
		for ( i = 1; i < j && degenerate == qtrue; i++ )
		{
			VectorSubtract( m.verts[ 0 ].xyz, m.verts[ i ].xyz, delta2 );
			delta2[ 3 ] = VectorNormalize( delta2, delta2 );
			if ( delta2[ 3 ] != 0 ) {
				/* create inverse vector */
				VectorCopy( delta2, delta3 );
				delta3[ 3 ] = delta2[ 3 ];
				VectorInverse( delta3 );

				/* compare */
				if ( VectorCompare( delta, delta2 ) == qfalse && VectorCompare( delta, delta3 ) == qfalse ) {
					degenerate = qfalse;
				}
			}
		}
	}

	/* warn and select degenerate patch */
	if ( degenerate ) {
		xml_Select( "degenerate patch", mapEnt->mapEntityNum, entitySourceBrushes, qfalse );
		free( m.verts );
		return;
	}

	/* find longest curve on the mesh */
	longestCurve = 0.0f;
	maxIterations = 0;
	for ( j = 0; j + 2 < m.width; j += 2 )
	{
		for ( i = 0; i + 2 < m.height; i += 2 )
		{
			ExpandLongestCurve( &longestCurve, verts[ i * m.width + j ].xyz, verts[ i * m.width + ( j + 1 ) ].xyz, verts[ i * m.width + ( j + 2 ) ].xyz );      /* row */
			ExpandLongestCurve( &longestCurve, verts[ i * m.width + j ].xyz, verts[ ( i + 1 ) * m.width + j ].xyz, verts[ ( i + 2 ) * m.width + j ].xyz );      /* col */
			ExpandMaxIterations( &maxIterations, patchSubdivisions, verts[ i * m.width + j ].xyz, verts[ i * m.width + ( j + 1 ) ].xyz, verts[ i * m.width + ( j + 2 ) ].xyz );     /* row */
			ExpandMaxIterations( &maxIterations, patchSubdivisions, verts[ i * m.width + j ].xyz, verts[ ( i + 1 ) * m.width + j ].xyz, verts[ ( i + 2 ) * m.width + j ].xyz  );    /* col */
		}
	}

	/* allocate patch mesh */
	pm = safe_malloc( sizeof( *pm ) );
	memset( pm, 0, sizeof( *pm ) );

	/* ydnar: add entity/brush numbering */
	pm->entityNum = mapEnt->mapEntityNum;
	pm->brushNum = entitySourceBrushes;

	/* set shader */
	sprintf( shader, "textures/%s", texture );
	pm->shaderInfo = ShaderInfoForShader( shader );

	/* set mesh */
	pm->mesh = m;

	/* set longest curve */
	pm->longestCurve = longestCurve;
	pm->maxIterations = maxIterations;

	/* link to the entity */
	pm->next = mapEnt->patches;
	mapEnt->patches = pm;
}



/*
   GrowGroup_r()
   recursively adds patches to a lod group
 */

static void GrowGroup_r( parseMesh_t *pm, int patchNum, int patchCount, parseMesh_t **meshes, byte *bordering, byte *group ){
	int i;
	const byte  *row;


	/* early out check */
	if ( group[ patchNum ] ) {
		return;
	}


	/* set it */
	group[ patchNum ] = 1;
	row = bordering + patchNum * patchCount;

	/* check maximums */
	if ( meshes[ patchNum ]->longestCurve > pm->longestCurve ) {
		pm->longestCurve = meshes[ patchNum ]->longestCurve;
	}
	if ( meshes[ patchNum ]->maxIterations > pm->maxIterations ) {
		pm->maxIterations = meshes[ patchNum ]->maxIterations;
	}

	/* walk other patches */
	for ( i = 0; i < patchCount; i++ )
	{
		if ( row[ i ] ) {
			GrowGroup_r( pm, i, patchCount, meshes, bordering, group );
		}
	}
}


/*
   PatchMapDrawSurfs()
   any patches that share an edge need to choose their
   level of detail as a unit, otherwise the edges would
   pull apart.
 */

void PatchMapDrawSurfs( entity_t *e ){
	int i, j, k, l, c1, c2;
	parseMesh_t             *pm;
	parseMesh_t             *check, *scan;
	mapDrawSurface_t        *ds;
	int patchCount, groupCount;
	bspDrawVert_t           *v1, *v2;
	vec3_t bounds[ 2 ];
	byte                    *bordering;

	/* ydnar: mac os x fails with these if not static */
	MAC_STATIC parseMesh_t  *meshes[ MAX_MAP_DRAW_SURFS ];
	MAC_STATIC qb_t grouped[ MAX_MAP_DRAW_SURFS ];
	MAC_STATIC byte group[ MAX_MAP_DRAW_SURFS ];


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- PatchMapDrawSurfs ---\n" );

	patchCount = 0;
	for ( pm = e->patches ; pm ; pm = pm->next  ) {
		meshes[patchCount] = pm;
		patchCount++;
	}

	if ( !patchCount ) {
		return;
	}
	bordering = safe_malloc( patchCount * patchCount );
	memset( bordering, 0, patchCount * patchCount );

	// build the bordering matrix
	for ( k = 0 ; k < patchCount ; k++ ) {
		bordering[k * patchCount + k] = 1;

		for ( l = k + 1 ; l < patchCount ; l++ ) {
			check = meshes[k];
			scan = meshes[l];
			c1 = scan->mesh.width * scan->mesh.height;
			v1 = scan->mesh.verts;

			for ( i = 0 ; i < c1 ; i++, v1++ ) {
				c2 = check->mesh.width * check->mesh.height;
				v2 = check->mesh.verts;
				for ( j = 0 ; j < c2 ; j++, v2++ ) {
					if ( fabs( v1->xyz[0] - v2->xyz[0] ) < 1.0
						 && fabs( v1->xyz[1] - v2->xyz[1] ) < 1.0
						 && fabs( v1->xyz[2] - v2->xyz[2] ) < 1.0 ) {
						break;
					}
				}
				if ( j != c2 ) {
					break;
				}
			}
			if ( i != c1 ) {
				// we have a connection
				bordering[k * patchCount + l] =
					bordering[l * patchCount + k] = 1;
			}
			else {
				// no connection
				bordering[k * patchCount + l] =
					bordering[l * patchCount + k] = 0;
			}

		}
	}

	/* build groups */
	memset( grouped, 0, patchCount );
	groupCount = 0;
	for ( i = 0; i < patchCount; i++ )
	{
		/* get patch */
		scan = meshes[ i ];

		/* start a new group */
		if ( !grouped[ i ] ) {
			groupCount++;
		}

		/* recursively find all patches that belong in the same group */
		memset( group, 0, patchCount );
		GrowGroup_r( scan, i, patchCount, meshes, bordering, group );

		/* bound them */
		ClearBounds( bounds[ 0 ], bounds[ 1 ] );
		for ( j = 0; j < patchCount; j++ )
		{
			if ( group[ j ] ) {
				grouped[ j ] = qtrue;
				check = meshes[ j ];
				c1 = check->mesh.width * check->mesh.height;
				v1 = check->mesh.verts;
				for ( k = 0; k < c1; k++, v1++ )
					AddPointToBounds( v1->xyz, bounds[ 0 ], bounds[ 1 ] );
			}
		}

		/* debug code */
		//%	Sys_Printf( "Longest curve: %f Iterations: %d\n", scan->longestCurve, scan->maxIterations );

		/* create drawsurf */
		scan->grouped = qtrue;
		ds = DrawSurfaceForMesh( e, scan, NULL );   /* ydnar */
		VectorCopy( bounds[ 0 ], ds->bounds[ 0 ] );
		VectorCopy( bounds[ 1 ], ds->bounds[ 1 ] );
	}

	/* emit some statistics */
	Sys_FPrintf( SYS_VRB, "%9d patches\n", patchCount );
	Sys_FPrintf( SYS_VRB, "%9d patch LOD groups\n", groupCount );
}
