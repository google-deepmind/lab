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
#define SURFACE_META_C



/* dependencies */
#include "q3map2.h"



#define LIGHTMAP_EXCEEDED   -1
#define S_EXCEEDED          -2
#define T_EXCEEDED          -3
#define ST_EXCEEDED         -4
#define UNSUITABLE_TRIANGLE -10
#define VERTS_EXCEEDED      -1000
#define INDEXES_EXCEEDED    -2000

#define GROW_META_VERTS     1024
#define GROW_META_TRIANGLES 1024

static int numMetaSurfaces, numPatchMetaSurfaces;

static int maxMetaVerts = 0;
static int numMetaVerts = 0;
static int firstSearchMetaVert = 0;
static bspDrawVert_t        *metaVerts = NULL;

static int maxMetaTriangles = 0;
static int numMetaTriangles = 0;
static metaTriangle_t       *metaTriangles = NULL;



/*
   ClearMetaVertexes()
   called before staring a new entity to clear out the triangle list
 */

void ClearMetaTriangles( void ){
	numMetaVerts = 0;
	numMetaTriangles = 0;
}



/*
   FindMetaVertex()
   finds a matching metavertex in the global list, returning its index
 */

static int FindMetaVertex( bspDrawVert_t *src ){
	int i;
	bspDrawVert_t   *v, *temp;


	/* try to find an existing drawvert */
	for ( i = firstSearchMetaVert, v = &metaVerts[ i ]; i < numMetaVerts; i++, v++ )
	{
		if ( memcmp( src, v, sizeof( bspDrawVert_t ) ) == 0 ) {
			return i;
		}
	}

	/* enough space? */
	if ( numMetaVerts >= maxMetaVerts ) {
		/* reallocate more room */
		maxMetaVerts += GROW_META_VERTS;
		temp = safe_malloc( maxMetaVerts * sizeof( bspDrawVert_t ) );
		if ( metaVerts != NULL ) {
			memcpy( temp, metaVerts, numMetaVerts * sizeof( bspDrawVert_t ) );
			free( metaVerts );
		}
		metaVerts = temp;
	}

	/* add the triangle */
	memcpy( &metaVerts[ numMetaVerts ], src, sizeof( bspDrawVert_t ) );
	numMetaVerts++;

	/* return the count */
	return ( numMetaVerts - 1 );
}



/*
   AddMetaTriangle()
   adds a new meta triangle, allocating more memory if necessary
 */

static int AddMetaTriangle( void ){
	metaTriangle_t  *temp;


	/* enough space? */
	if ( numMetaTriangles >= maxMetaTriangles ) {
		/* reallocate more room */
		maxMetaTriangles += GROW_META_TRIANGLES;
		temp = safe_malloc( maxMetaTriangles * sizeof( metaTriangle_t ) );
		if ( metaTriangles != NULL ) {
			memcpy( temp, metaTriangles, numMetaTriangles * sizeof( metaTriangle_t ) );
			free( metaTriangles );
		}
		metaTriangles = temp;
	}

	/* increment and return */
	numMetaTriangles++;
	return numMetaTriangles - 1;
}



/*
   FindMetaTriangle()
   finds a matching metatriangle in the global list,
   otherwise adds it and returns the index to the metatriangle
 */

int FindMetaTriangle( metaTriangle_t *src, bspDrawVert_t *a, bspDrawVert_t *b, bspDrawVert_t *c, int planeNum ){
	int triIndex;
	vec3_t dir;



	/* detect degenerate triangles fixme: do something proper here */
	VectorSubtract( a->xyz, b->xyz, dir );
	if ( VectorLength( dir ) < 0.125f ) {
		return -1;
	}
	VectorSubtract( b->xyz, c->xyz, dir );
	if ( VectorLength( dir ) < 0.125f ) {
		return -1;
	}
	VectorSubtract( c->xyz, a->xyz, dir );
	if ( VectorLength( dir ) < 0.125f ) {
		return -1;
	}

	/* find plane */
	if ( planeNum >= 0 ) {
		/* because of precision issues with small triangles, try to use the specified plane */
		src->planeNum = planeNum;
		VectorCopy( mapplanes[ planeNum ].normal, src->plane );
		src->plane[ 3 ] = mapplanes[ planeNum ].dist;
	}
	else
	{
		/* calculate a plane from the triangle's points (and bail if a plane can't be constructed) */
		src->planeNum = -1;
		if ( PlaneFromPoints( src->plane, a->xyz, b->xyz, c->xyz ) == qfalse ) {
			return -1;
		}
	}

	/* ydnar 2002-10-03: repair any bogus normals (busted ase import kludge) */
	if ( VectorLength( a->normal ) <= 0.0f ) {
		VectorCopy( src->plane, a->normal );
	}
	if ( VectorLength( b->normal ) <= 0.0f ) {
		VectorCopy( src->plane, b->normal );
	}
	if ( VectorLength( c->normal ) <= 0.0f ) {
		VectorCopy( src->plane, c->normal );
	}

	/* ydnar 2002-10-04: set lightmap axis if not already set */
	if ( !( src->si->compileFlags & C_VERTEXLIT ) &&
		 src->lightmapAxis[ 0 ] == 0.0f && src->lightmapAxis[ 1 ] == 0.0f && src->lightmapAxis[ 2 ] == 0.0f ) {
		/* the shader can specify an explicit lightmap axis */
		if ( src->si->lightmapAxis[ 0 ] || src->si->lightmapAxis[ 1 ] || src->si->lightmapAxis[ 2 ] ) {
			VectorCopy( src->si->lightmapAxis, src->lightmapAxis );
		}

		/* new axis-finding code */
		else{
			CalcLightmapAxis( src->plane, src->lightmapAxis );
		}
	}

	/* fill out the src triangle */
	src->indexes[ 0 ] = FindMetaVertex( a );
	src->indexes[ 1 ] = FindMetaVertex( b );
	src->indexes[ 2 ] = FindMetaVertex( c );

	/* try to find an existing triangle */
	#ifdef USE_EXHAUSTIVE_SEARCH
	{
		int i;
		metaTriangle_t  *tri;


		for ( i = 0, tri = metaTriangles; i < numMetaTriangles; i++, tri++ )
		{
			if ( memcmp( src, tri, sizeof( metaTriangle_t ) ) == 0 ) {
				return i;
			}
		}
	}
	#endif

	/* get a new triangle */
	triIndex = AddMetaTriangle();

	/* add the triangle */
	memcpy( &metaTriangles[ triIndex ], src, sizeof( metaTriangle_t ) );

	/* return the triangle index */
	return triIndex;
}



/*
   SurfaceToMetaTriangles()
   converts a classified surface to metatriangles
 */

static void SurfaceToMetaTriangles( mapDrawSurface_t *ds ){
	int i;
	metaTriangle_t src;
	bspDrawVert_t a, b, c;


	/* only handle certain types of surfaces */
	if ( ds->type != SURFACE_FACE &&
		 ds->type != SURFACE_META &&
		 ds->type != SURFACE_FORCED_META &&
		 ds->type != SURFACE_DECAL ) {
		return;
	}

	/* speed at the expense of memory */
	firstSearchMetaVert = numMetaVerts;

	/* only handle valid surfaces */
	if ( ds->type != SURFACE_BAD && ds->numVerts >= 3 && ds->numIndexes >= 3 ) {
		/* walk the indexes and create triangles */
		for ( i = 0; i < ds->numIndexes; i += 3 )
		{
			/* sanity check the indexes */
			if ( ds->indexes[ i ] == ds->indexes[ i + 1 ] ||
				 ds->indexes[ i ] == ds->indexes[ i + 2 ] ||
				 ds->indexes[ i + 1 ] == ds->indexes[ i + 2 ] ) {
				//%	Sys_Printf( "%d! ", ds->numVerts );
				continue;
			}

			/* build a metatriangle */
			src.si = ds->shaderInfo;
			src.side = ( ds->sideRef != NULL ? ds->sideRef->side : NULL );
			src.entityNum = ds->entityNum;
			src.surfaceNum = ds->surfaceNum;
			src.planeNum = ds->planeNum;
			src.castShadows = ds->castShadows;
			src.recvShadows = ds->recvShadows;
			src.fogNum = ds->fogNum;
			src.sampleSize = ds->sampleSize;
			VectorCopy( ds->lightmapAxis, src.lightmapAxis );

			/* copy drawverts */
			memcpy( &a, &ds->verts[ ds->indexes[ i ] ], sizeof( a ) );
			memcpy( &b, &ds->verts[ ds->indexes[ i + 1 ] ], sizeof( b ) );
			memcpy( &c, &ds->verts[ ds->indexes[ i + 2 ] ], sizeof( c ) );
			FindMetaTriangle( &src, &a, &b, &c, ds->planeNum );
		}

		/* add to count */
		numMetaSurfaces++;
	}

	/* clear the surface (free verts and indexes, sets it to SURFACE_BAD) */
	ClearSurface( ds );
}



/*
   TriangulatePatchSurface()
   creates triangles from a patch
 */

void TriangulatePatchSurface( mapDrawSurface_t *ds ){
	int iterations, x, y, pw[ 5 ], r;
	mapDrawSurface_t    *dsNew;
	mesh_t src, *subdivided, *mesh;


	/* try to early out */
	if ( ds->numVerts == 0 || ds->type != SURFACE_PATCH || patchMeta == qfalse ) {
		return;
	}

	/* make a mesh from the drawsurf */
	src.width = ds->patchWidth;
	src.height = ds->patchHeight;
	src.verts = ds->verts;
	//%	subdivided = SubdivideMesh( src, 8, 999 );
	iterations = IterationsForCurve( ds->longestCurve, patchSubdivisions );
	subdivided = SubdivideMesh2( src, iterations ); //%	ds->maxIterations

	/* fit it to the curve and remove colinear verts on rows/columns */
	PutMeshOnCurve( *subdivided );
	mesh = RemoveLinearMeshColumnsRows( subdivided );
	FreeMesh( subdivided );
	//% MakeMeshNormals( mesh );

	/* make a copy of the drawsurface */
	dsNew = AllocDrawSurface( SURFACE_META );
	memcpy( dsNew, ds, sizeof( *ds ) );

	/* if the patch is nonsolid, then discard it */
	if ( !( ds->shaderInfo->compileFlags & C_SOLID ) ) {
		ClearSurface( ds );
	}

	/* set new pointer */
	ds = dsNew;

	/* basic transmogrification */
	ds->type = SURFACE_META;
	ds->numIndexes = 0;
	ds->indexes = safe_malloc( mesh->width * mesh->height * 6 * sizeof( int ) );

	/* copy the verts in */
	ds->numVerts = ( mesh->width * mesh->height );
	ds->verts = mesh->verts;

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

			/* make first triangle */
			ds->indexes[ ds->numIndexes++ ] = pw[ r + 0 ];
			ds->indexes[ ds->numIndexes++ ] = pw[ r + 1 ];
			ds->indexes[ ds->numIndexes++ ] = pw[ r + 2 ];

			/* make second triangle */
			ds->indexes[ ds->numIndexes++ ] = pw[ r + 0 ];
			ds->indexes[ ds->numIndexes++ ] = pw[ r + 2 ];
			ds->indexes[ ds->numIndexes++ ] = pw[ r + 3 ];
		}
	}

	/* free the mesh, but not the verts */
	free( mesh );

	/* add to count */
	numPatchMetaSurfaces++;

	/* classify it */
	ClassifySurfaces( 1, ds );
}



/*
   FanFaceSurface() - ydnar
   creates a tri-fan from a brush face winding
   loosely based on SurfaceAsTriFan()
 */

void FanFaceSurface( mapDrawSurface_t *ds ){
	int i, j, k, a, b, c, color[ MAX_LIGHTMAPS ][ 4 ];
	bspDrawVert_t   *verts, *centroid, *dv;
	double iv;


	/* try to early out */
	if ( !ds->numVerts || ( ds->type != SURFACE_FACE && ds->type != SURFACE_DECAL ) ) {
		return;
	}

	/* add a new vertex at the beginning of the surface */
	verts = safe_malloc( ( ds->numVerts + 1 ) * sizeof( bspDrawVert_t ) );
	memset( verts, 0, sizeof( bspDrawVert_t ) );
	memcpy( &verts[ 1 ], ds->verts, ds->numVerts * sizeof( bspDrawVert_t ) );
	free( ds->verts );
	ds->verts = verts;

	/* add up the drawverts to create a centroid */
	centroid = &verts[ 0 ];
	memset( color, 0,  4 * MAX_LIGHTMAPS * sizeof( int ) );
	for ( i = 1, dv = &verts[ 1 ]; i < ( ds->numVerts + 1 ); i++, dv++ )
	{
		VectorAdd( centroid->xyz, dv->xyz, centroid->xyz );
		VectorAdd( centroid->normal, dv->normal, centroid->normal );
		for ( j = 0; j < 4; j++ )
		{
			for ( k = 0; k < MAX_LIGHTMAPS; k++ )
				color[ k ][ j ] += dv->color[ k ][ j ];
			if ( j < 2 ) {
				centroid->st[ j ] += dv->st[ j ];
				for ( k = 0; k < MAX_LIGHTMAPS; k++ )
					centroid->lightmap[ k ][ j ] += dv->lightmap[ k ][ j ];
			}
		}
	}

	/* average the centroid */
	iv = 1.0f / ds->numVerts;
	VectorScale( centroid->xyz, iv, centroid->xyz );
	if ( VectorNormalize( centroid->normal, centroid->normal ) <= 0 ) {
		VectorCopy( verts[ 1 ].normal, centroid->normal );
	}
	for ( j = 0; j < 4; j++ )
	{
		for ( k = 0; k < MAX_LIGHTMAPS; k++ )
		{
			color[ k ][ j ] /= ds->numVerts;
			centroid->color[ k ][ j ] = ( color[ k ][ j ] < 255.0f ? color[ k ][ j ] : 255 );
		}
		if ( j < 2 ) {
			centroid->st[ j ] *= iv;
			for ( k = 0; k < MAX_LIGHTMAPS; k++ )
				centroid->lightmap[ k ][ j ] *= iv;
		}
	}

	/* add to vert count */
	ds->numVerts++;

	/* fill indexes in triangle fan order */
	ds->numIndexes = 0;
	ds->indexes = safe_malloc( ds->numVerts * 3 * sizeof( int ) );
	for ( i = 1; i < ds->numVerts; i++ )
	{
		a = 0;
		b = i;
		c = ( i + 1 ) % ds->numVerts;
		c = c ? c : 1;
		ds->indexes[ ds->numIndexes++ ] = a;
		ds->indexes[ ds->numIndexes++ ] = b;
		ds->indexes[ ds->numIndexes++ ] = c;
	}

	/* add to count */
	numFanSurfaces++;

	/* classify it */
	ClassifySurfaces( 1, ds );
}



/*
   StripFaceSurface() - ydnar
   attempts to create a valid tri-strip w/o degenerate triangles from a brush face winding
   based on SurfaceAsTriStrip()
 */

#define MAX_INDEXES     1024

void StripFaceSurface( mapDrawSurface_t *ds ){
	int i, r, least, rotate, numIndexes, ni, a, b, c, indexes[ MAX_INDEXES ];
	vec_t       *v1, *v2;


	/* try to early out  */
	if ( !ds->numVerts || ( ds->type != SURFACE_FACE && ds->type != SURFACE_DECAL ) ) {
		return;
	}

	/* is this a simple triangle? */
	if ( ds->numVerts == 3 ) {
		numIndexes = 3;
		VectorSet( indexes, 0, 1, 2 );
	}
	else
	{
		/* ydnar: find smallest coordinate */
		least = 0;
		if ( ds->shaderInfo != NULL && ds->shaderInfo->autosprite == qfalse ) {
			for ( i = 0; i < ds->numVerts; i++ )
			{
				/* get points */
				v1 = ds->verts[ i ].xyz;
				v2 = ds->verts[ least ].xyz;

				/* compare */
				if ( v1[ 0 ] < v2[ 0 ] ||
					 ( v1[ 0 ] == v2[ 0 ] && v1[ 1 ] < v2[ 1 ] ) ||
					 ( v1[ 0 ] == v2[ 0 ] && v1[ 1 ] == v2[ 1 ] && v1[ 2 ] < v2[ 2 ] ) ) {
					least = i;
				}
			}
		}

		/* determine the triangle strip order */
		numIndexes = ( ds->numVerts - 2 ) * 3;
		if ( numIndexes > MAX_INDEXES ) {
			Error( "MAX_INDEXES exceeded for surface (%d > %d) (%d verts)", numIndexes, MAX_INDEXES, ds->numVerts );
		}

		/* try all possible orderings of the points looking for a non-degenerate strip order */
		for ( r = 0; r < ds->numVerts; r++ )
		{
			/* set rotation */
			rotate = ( r + least ) % ds->numVerts;

			/* walk the winding in both directions */
			for ( ni = 0, i = 0; i < ds->numVerts - 2 - i; i++ )
			{
				/* make indexes */
				a = ( ds->numVerts - 1 - i + rotate ) % ds->numVerts;
				b = ( i + rotate ) % ds->numVerts;
				c = ( ds->numVerts - 2 - i + rotate ) % ds->numVerts;

				/* test this triangle */
				if ( ds->numVerts > 4 && IsTriangleDegenerate( ds->verts, a, b, c ) ) {
					break;
				}
				indexes[ ni++ ] = a;
				indexes[ ni++ ] = b;
				indexes[ ni++ ] = c;

				/* handle end case */
				if ( i + 1 != ds->numVerts - 1 - i ) {
					/* make indexes */
					a = ( ds->numVerts - 2 - i + rotate ) % ds->numVerts;
					b = ( i + rotate ) % ds->numVerts;
					c = ( i + 1 + rotate ) % ds->numVerts;

					/* test triangle */
					if ( ds->numVerts > 4 && IsTriangleDegenerate( ds->verts, a, b, c ) ) {
						break;
					}
					indexes[ ni++ ] = a;
					indexes[ ni++ ] = b;
					indexes[ ni++ ] = c;
				}
			}

			/* valid strip? */
			if ( ni == numIndexes ) {
				break;
			}
		}

		/* if any triangle in the strip is degenerate, render from a centered fan point instead */
		if ( ni < numIndexes ) {
			FanFaceSurface( ds );
			return;
		}
	}

	/* copy strip triangle indexes */
	ds->numIndexes = numIndexes;
	ds->indexes = safe_malloc( ds->numIndexes * sizeof( int ) );
	memcpy( ds->indexes, indexes, ds->numIndexes * sizeof( int ) );

	/* add to count */
	numStripSurfaces++;

	/* classify it */
	ClassifySurfaces( 1, ds );
}



/*
   MakeEntityMetaTriangles()
   builds meta triangles from brush faces (tristrips and fans)
 */

void MakeEntityMetaTriangles( entity_t *e ){
	int i, f, fOld, start;
	mapDrawSurface_t    *ds;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- MakeEntityMetaTriangles ---\n" );

	/* init pacifier */
	fOld = -1;
	start = I_FloatTime();

	/* walk the list of surfaces in the entity */
	for ( i = e->firstDrawSurf; i < numMapDrawSurfs; i++ )
	{
		/* print pacifier */
		f = 10 * ( i - e->firstDrawSurf ) / ( numMapDrawSurfs - e->firstDrawSurf );
		if ( f != fOld ) {
			fOld = f;
			Sys_FPrintf( SYS_VRB, "%d...", f );
		}

		/* get surface */
		ds = &mapDrawSurfs[ i ];
		if ( ds->numVerts <= 0 ) {
			continue;
		}

		/* ignore autosprite surfaces */
		if ( ds->shaderInfo->autosprite ) {
			continue;
		}

		/* meta this surface? */
		if ( meta == qfalse && ds->shaderInfo->forceMeta == qfalse ) {
			continue;
		}

		/* switch on type */
		switch ( ds->type )
		{
		case SURFACE_FACE:
		case SURFACE_DECAL:
			StripFaceSurface( ds );
			SurfaceToMetaTriangles( ds );
			break;

		case SURFACE_PATCH:
			TriangulatePatchSurface( ds );
			break;

		case SURFACE_TRIANGLES:
			break;

		case SURFACE_FORCED_META:
		case SURFACE_META:
			SurfaceToMetaTriangles( ds );
			break;

		default:
			break;
		}
	}

	/* print time */
	if ( ( numMapDrawSurfs - e->firstDrawSurf ) ) {
		Sys_FPrintf( SYS_VRB, " (%d)\n", (int) ( I_FloatTime() - start ) );
	}

	/* emit some stats */
	Sys_FPrintf( SYS_VRB, "%9d total meta surfaces\n", numMetaSurfaces );
	Sys_FPrintf( SYS_VRB, "%9d stripped surfaces\n", numStripSurfaces );
	Sys_FPrintf( SYS_VRB, "%9d fanned surfaces\n", numFanSurfaces );
	Sys_FPrintf( SYS_VRB, "%9d patch meta surfaces\n", numPatchMetaSurfaces );
	Sys_FPrintf( SYS_VRB, "%9d meta verts\n", numMetaVerts );
	Sys_FPrintf( SYS_VRB, "%9d meta triangles\n", numMetaTriangles );

	/* tidy things up */
	TidyEntitySurfaces( e );
}



/*
   PointTriangleIntersect()
   assuming that all points lie in plane, determine if pt
   is inside the triangle abc
   code originally (c) 2001 softSurfer (www.softsurfer.com)
 */

#define MIN_OUTSIDE_EPSILON     -0.01f
#define MAX_OUTSIDE_EPSILON     1.01f

static qboolean PointTriangleIntersect( vec3_t pt, vec4_t plane, vec3_t a, vec3_t b, vec3_t c, vec3_t bary ){
	vec3_t u, v, w;
	float uu, uv, vv, wu, wv, d;


	/* make vectors */
	VectorSubtract( b, a, u );
	VectorSubtract( c, a, v );
	VectorSubtract( pt, a, w );

	/* more setup */
	uu = DotProduct( u, u );
	uv = DotProduct( u, v );
	vv = DotProduct( v, v );
	wu = DotProduct( w, u );
	wv = DotProduct( w, v );
	d = uv * uv - uu * vv;

	/* calculate barycentric coordinates */
	bary[ 1 ] = ( uv * wv - vv * wu ) / d;
	if ( bary[ 1 ] < MIN_OUTSIDE_EPSILON || bary[ 1 ] > MAX_OUTSIDE_EPSILON ) {
		return qfalse;
	}
	bary[ 2 ] = ( uv * wv - uu * wv ) / d;
	if ( bary[ 2 ] < MIN_OUTSIDE_EPSILON || bary[ 2 ] > MAX_OUTSIDE_EPSILON ) {
		return qfalse;
	}
	bary[ 0 ] = 1.0f - ( bary[ 1 ] + bary[ 2 ] );

	/* point is in triangle */
	return qtrue;
}



/*
   CreateEdge()
   sets up an edge structure from a plane and 2 points that the edge ab falls lies in
 */

typedef struct edge_s
{
	vec3_t origin;
	vec4_t edge;
	vec_t length, kingpinLength;
	int kingpin;
	vec4_t plane;
}
edge_t;

void CreateEdge( vec4_t plane, vec3_t a, vec3_t b, edge_t *edge ){
	/* copy edge origin */
	VectorCopy( a, edge->origin );

	/* create vector aligned with winding direction of edge */
	VectorSubtract( b, a, edge->edge );

	if ( fabs( edge->edge[ 0 ] ) > fabs( edge->edge[ 1 ] ) && fabs( edge->edge[ 0 ] ) > fabs( edge->edge[ 2 ] ) ) {
		edge->kingpin = 0;
	}
	else if ( fabs( edge->edge[ 1 ] ) > fabs( edge->edge[ 0 ] ) && fabs( edge->edge[ 1 ] ) > fabs( edge->edge[ 2 ] ) ) {
		edge->kingpin = 1;
	}
	else{
		edge->kingpin = 2;
	}
	edge->kingpinLength = edge->edge[ edge->kingpin ];

	VectorNormalize( edge->edge, edge->edge );
	edge->edge[ 3 ] = DotProduct( a, edge->edge );
	edge->length = DotProduct( b, edge->edge ) - edge->edge[ 3 ];

	/* create perpendicular plane that edge lies in */
	CrossProduct( plane, edge->edge, edge->plane );
	edge->plane[ 3 ] = DotProduct( a, edge->plane );
}



/*
   FixMetaTJunctions()
   fixes t-junctions on meta triangles
 */

#define TJ_PLANE_EPSILON    ( 1.0f / 8.0f )
#define TJ_EDGE_EPSILON     ( 1.0f / 8.0f )
#define TJ_POINT_EPSILON    ( 1.0f / 8.0f )

void FixMetaTJunctions( void ){
	int i, j, k, f, fOld, start, vertIndex, triIndex, numTJuncs;
	metaTriangle_t  *tri, *newTri;
	shaderInfo_t    *si;
	bspDrawVert_t   *a, *b, *c, junc;
	float dist, amount;
	vec3_t pt;
	vec4_t plane;
	edge_t edges[ 3 ];


	/* this code is crap; revisit later */
	return;

	/* note it */
	Sys_FPrintf( SYS_VRB, "--- FixMetaTJunctions ---\n" );

	/* init pacifier */
	fOld = -1;
	start = I_FloatTime();

	/* walk triangle list */
	numTJuncs = 0;
	for ( i = 0; i < numMetaTriangles; i++ )
	{
		/* get triangle */
		tri = &metaTriangles[ i ];

		/* print pacifier */
		f = 10 * i / numMetaTriangles;
		if ( f != fOld ) {
			fOld = f;
			Sys_FPrintf( SYS_VRB, "%d...", f );
		}

		/* attempt to early out */
		si = tri->si;
		if ( ( si->compileFlags & C_NODRAW ) || si->autosprite || si->notjunc ) {
			continue;
		}

		/* calculate planes */
		VectorCopy( tri->plane, plane );
		plane[ 3 ] = tri->plane[ 3 ];
		CreateEdge( plane, metaVerts[ tri->indexes[ 0 ] ].xyz, metaVerts[ tri->indexes[ 1 ] ].xyz, &edges[ 0 ] );
		CreateEdge( plane, metaVerts[ tri->indexes[ 1 ] ].xyz, metaVerts[ tri->indexes[ 2 ] ].xyz, &edges[ 1 ] );
		CreateEdge( plane, metaVerts[ tri->indexes[ 2 ] ].xyz, metaVerts[ tri->indexes[ 0 ] ].xyz, &edges[ 2 ] );

		/* walk meta vert list */
		for ( j = 0; j < numMetaVerts; j++ )
		{
			/* get vert */
			VectorCopy( metaVerts[ j ].xyz, pt );

			/* debug code: darken verts */
			if ( i == 0 ) {
				VectorSet( metaVerts[ j ].color[ 0 ], 8, 8, 8 );
			}

			/* determine if point lies in the triangle's plane */
			dist = DotProduct( pt, plane ) - plane[ 3 ];
			if ( fabs( dist ) > TJ_PLANE_EPSILON ) {
				continue;
			}

			/* skip this point if it already exists in the triangle */
			for ( k = 0; k < 3; k++ )
			{
				if ( fabs( pt[ 0 ] - metaVerts[ tri->indexes[ k ] ].xyz[ 0 ] ) <= TJ_POINT_EPSILON &&
					 fabs( pt[ 1 ] - metaVerts[ tri->indexes[ k ] ].xyz[ 1 ] ) <= TJ_POINT_EPSILON &&
					 fabs( pt[ 2 ] - metaVerts[ tri->indexes[ k ] ].xyz[ 2 ] ) <= TJ_POINT_EPSILON ) {
					break;
				}
			}
			if ( k < 3 ) {
				continue;
			}

			/* walk edges */
			for ( k = 0; k < 3; k++ )
			{
				/* ignore bogus edges */
				if ( fabs( edges[ k ].kingpinLength ) < TJ_EDGE_EPSILON ) {
					continue;
				}

				/* determine if point lies on the edge */
				dist = DotProduct( pt, edges[ k ].plane ) - edges[ k ].plane[ 3 ];
				if ( fabs( dist ) > TJ_EDGE_EPSILON ) {
					continue;
				}

				/* determine how far along the edge the point lies */
				amount = ( pt[ edges[ k ].kingpin ] - edges[ k ].origin[ edges[ k ].kingpin ] ) / edges[ k ].kingpinLength;
				if ( amount <= 0.0f || amount >= 1.0f ) {
					continue;
				}

				#if 0
				dist = DotProduct( pt, edges[ k ].edge ) - edges[ k ].edge[ 3 ];
				if ( dist <= -0.0f || dist >= edges[ k ].length ) {
					continue;
				}
				amount = dist / edges[ k ].length;
				#endif

				/* debug code: brighten this point */
				//%	metaVerts[ j ].color[ 0 ][ 0 ] += 5;
				//%	metaVerts[ j ].color[ 0 ][ 1 ] += 4;
				VectorSet( metaVerts[ tri->indexes[ k ] ].color[ 0 ], 255, 204, 0 );
				VectorSet( metaVerts[ tri->indexes[ ( k + 1 ) % 3 ] ].color[ 0 ], 255, 204, 0 );


				/* the edge opposite the zero-weighted vertex was hit, so use that as an amount */
				a = &metaVerts[ tri->indexes[ k % 3 ] ];
				b = &metaVerts[ tri->indexes[ ( k + 1 ) % 3 ] ];
				c = &metaVerts[ tri->indexes[ ( k + 2 ) % 3 ] ];

				/* make new vert */
				LerpDrawVertAmount( a, b, amount, &junc );
				VectorCopy( pt, junc.xyz );

				/* compare against existing verts */
				if ( VectorCompare( junc.xyz, a->xyz ) || VectorCompare( junc.xyz, b->xyz ) || VectorCompare( junc.xyz, c->xyz ) ) {
					continue;
				}

				/* see if we can just re-use the existing vert */
				if ( !memcmp( &metaVerts[ j ], &junc, sizeof( junc ) ) ) {
					vertIndex = j;
				}
				else
				{
					/* find new vertex (note: a and b are invalid pointers after this) */
					firstSearchMetaVert = numMetaVerts;
					vertIndex = FindMetaVertex( &junc );
					if ( vertIndex < 0 ) {
						continue;
					}
				}

				/* make new triangle */
				triIndex = AddMetaTriangle();
				if ( triIndex < 0 ) {
					continue;
				}

				/* get triangles */
				tri = &metaTriangles[ i ];
				newTri = &metaTriangles[ triIndex ];

				/* copy the triangle */
				memcpy( newTri, tri, sizeof( *tri ) );

				/* fix verts */
				tri->indexes[ ( k + 1 ) % 3 ] = vertIndex;
				newTri->indexes[ k ] = vertIndex;

				/* recalculate edges */
				CreateEdge( plane, metaVerts[ tri->indexes[ 0 ] ].xyz, metaVerts[ tri->indexes[ 1 ] ].xyz, &edges[ 0 ] );
				CreateEdge( plane, metaVerts[ tri->indexes[ 1 ] ].xyz, metaVerts[ tri->indexes[ 2 ] ].xyz, &edges[ 1 ] );
				CreateEdge( plane, metaVerts[ tri->indexes[ 2 ] ].xyz, metaVerts[ tri->indexes[ 0 ] ].xyz, &edges[ 2 ] );

				/* debug code */
				metaVerts[ vertIndex ].color[ 0 ][ 0 ] = 255;
				metaVerts[ vertIndex ].color[ 0 ][ 1 ] = 204;
				metaVerts[ vertIndex ].color[ 0 ][ 2 ] = 0;

				/* add to counter and end processing of this vert */
				numTJuncs++;
				break;
			}
		}
	}

	/* print time */
	Sys_FPrintf( SYS_VRB, " (%d)\n", (int) ( I_FloatTime() - start ) );

	/* emit some stats */
	Sys_FPrintf( SYS_VRB, "%9d T-junctions added\n", numTJuncs );
}



/*
   SmoothMetaTriangles()
   averages coincident vertex normals in the meta triangles
 */

#define MAX_SAMPLES             256
#define THETA_EPSILON           0.000001
#define EQUAL_NORMAL_EPSILON    0.01

void SmoothMetaTriangles( void ){
	int i, j, k, f, fOld, start, cs, numVerts, numVotes, numSmoothed;
	float shadeAngle, defaultShadeAngle, maxShadeAngle, dot, testAngle;
	metaTriangle_t  *tri;
	float           *shadeAngles;
	byte            *smoothed;
	vec3_t average, diff;
	int indexes[ MAX_SAMPLES ];
	vec3_t votes[ MAX_SAMPLES ];


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- SmoothMetaTriangles ---\n" );

	/* allocate shade angle table */
	shadeAngles = safe_malloc( numMetaVerts * sizeof( float ) );
	memset( shadeAngles, 0, numMetaVerts * sizeof( float ) );

	/* allocate smoothed table */
	cs = ( numMetaVerts / 8 ) + 1;
	smoothed = safe_malloc( cs );
	memset( smoothed, 0, cs );

	/* set default shade angle */
	defaultShadeAngle = DEG2RAD( npDegrees );
	maxShadeAngle = 0.0f;

	/* run through every surface and flag verts belonging to non-lightmapped surfaces
	   and set per-vertex smoothing angle */
	for ( i = 0, tri = &metaTriangles[ i ]; i < numMetaTriangles; i++, tri++ )
	{
		/* get shader for shade angle */
		if ( tri->si->shadeAngleDegrees > 0.0f ) {
			shadeAngle = DEG2RAD( tri->si->shadeAngleDegrees );
		}
		else{
			shadeAngle = defaultShadeAngle;
		}
		if ( shadeAngle > maxShadeAngle ) {
			maxShadeAngle = shadeAngle;
		}

		/* flag its verts */
		for ( j = 0; j < 3; j++ )
		{
			shadeAngles[ tri->indexes[ j ] ] = shadeAngle;
			if ( shadeAngle <= 0 ) {
				smoothed[ tri->indexes[ j ] >> 3 ] |= ( 1 << ( tri->indexes[ j ] & 7 ) );
			}
		}
	}

	/* bail if no surfaces have a shade angle */
	if ( maxShadeAngle <= 0 ) {
		Sys_FPrintf( SYS_VRB, "No smoothing angles specified, aborting\n" );
		free( shadeAngles );
		free( smoothed );
		return;
	}

	/* init pacifier */
	fOld = -1;
	start = I_FloatTime();

	/* go through the list of vertexes */
	numSmoothed = 0;
	for ( i = 0; i < numMetaVerts; i++ )
	{
		/* print pacifier */
		f = 10 * i / numMetaVerts;
		if ( f != fOld ) {
			fOld = f;
			Sys_FPrintf( SYS_VRB, "%d...", f );
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
		for ( j = i; j < numMetaVerts && numVerts < MAX_SAMPLES; j++ )
		{
			/* already smoothed? */
			if ( smoothed[ j >> 3 ] & ( 1 << ( j & 7 ) ) ) {
				continue;
			}

			/* test vertexes */
			if ( VectorCompare( metaVerts[ i ].xyz, metaVerts[ j ].xyz ) == qfalse ) {
				continue;
			}

			/* use smallest shade angle */
			shadeAngle = ( shadeAngles[ i ] < shadeAngles[ j ] ? shadeAngles[ i ] : shadeAngles[ j ] );

			/* check shade angle */
			dot = DotProduct( metaVerts[ i ].normal, metaVerts[ j ].normal );
			if ( dot > 1.0 ) {
				dot = 1.0;
			}
			else if ( dot < -1.0 ) {
				dot = -1.0;
			}
			testAngle = acos( dot ) + THETA_EPSILON;
			if ( testAngle >= shadeAngle ) {
				continue;
			}

			/* add to the list */
			indexes[ numVerts++ ] = j;

			/* flag vertex */
			smoothed[ j >> 3 ] |= ( 1 << ( j & 7 ) );

			/* see if this normal has already been voted */
			for ( k = 0; k < numVotes; k++ )
			{
				VectorSubtract( metaVerts[ j ].normal, votes[ k ], diff );
				if ( fabs( diff[ 0 ] ) < EQUAL_NORMAL_EPSILON &&
					 fabs( diff[ 1 ] ) < EQUAL_NORMAL_EPSILON &&
					 fabs( diff[ 2 ] ) < EQUAL_NORMAL_EPSILON ) {
					break;
				}
			}

			/* add a new vote? */
			if ( k == numVotes && numVotes < MAX_SAMPLES ) {
				VectorAdd( average, metaVerts[ j ].normal, average );
				VectorCopy( metaVerts[ j ].normal, votes[ numVotes ] );
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
				VectorCopy( average, metaVerts[ indexes[ j ] ].normal );
			numSmoothed++;
		}
	}

	/* free the tables */
	free( shadeAngles );
	free( smoothed );

	/* print time */
	Sys_FPrintf( SYS_VRB, " (%d)\n", (int) ( I_FloatTime() - start ) );

	/* emit some stats */
	Sys_FPrintf( SYS_VRB, "%9d smoothed vertexes\n", numSmoothed );
}



/*
   AddMetaVertToSurface()
   adds a drawvert to a surface unless an existing vert matching already exists
   returns the index of that vert (or < 0 on failure)
 */

int AddMetaVertToSurface( mapDrawSurface_t *ds, bspDrawVert_t *dv1, int *coincident ){
	int i;
	bspDrawVert_t   *dv2;


	/* go through the verts and find a suitable candidate */
	for ( i = 0; i < ds->numVerts; i++ )
	{
		/* get test vert */
		dv2 = &ds->verts[ i ];

		/* compare xyz and normal */
		if ( VectorCompare( dv1->xyz, dv2->xyz ) == qfalse ) {
			continue;
		}
		if ( VectorCompare( dv1->normal, dv2->normal ) == qfalse ) {
			continue;
		}

		/* good enough at this point */
		( *coincident )++;

		/* compare texture coordinates and color */
		if ( dv1->st[ 0 ] != dv2->st[ 0 ] || dv1->st[ 1 ] != dv2->st[ 1 ] ) {
			continue;
		}
		if ( dv1->color[ 0 ][ 3 ] != dv2->color[ 0 ][ 3 ] ) {
			continue;
		}

		/* found a winner */
		numMergedVerts++;
		return i;
	}

	/* overflow check */
	if ( ds->numVerts >= ( ( ds->shaderInfo->compileFlags & C_VERTEXLIT ) ? maxSurfaceVerts : maxLMSurfaceVerts ) ) {
		return VERTS_EXCEEDED;
	}

	/* made it this far, add the vert and return */
	dv2 = &ds->verts[ ds->numVerts++ ];
	*dv2 = *dv1;
	return ( ds->numVerts - 1 );
}




/*
   AddMetaTriangleToSurface()
   attempts to add a metatriangle to a surface
   returns the score of the triangle added
 */

#define AXIS_SCORE          100000
#define AXIS_MIN            100000
#define VERT_SCORE          10000
#define SURFACE_SCORE       1000
#define ST_SCORE            50
#define ST_SCORE2           ( 2 * ( ST_SCORE ) )

#define ADEQUATE_SCORE      ( (AXIS_MIN) +1 * ( VERT_SCORE ) )
#define GOOD_SCORE          ( (AXIS_MIN) +2 * (VERT_SCORE) +4 * ( ST_SCORE ) )
#define PERFECT_SCORE       ( ( AXIS_MIN ) + +3 * ( VERT_SCORE ) + (SURFACE_SCORE) +4 * ( ST_SCORE ) )

static int AddMetaTriangleToSurface( mapDrawSurface_t *ds, metaTriangle_t *tri, qboolean testAdd ){
	int i, score, coincident, ai, bi, ci, oldTexRange[ 2 ];
	float lmMax;
	vec3_t mins, maxs;
	qboolean inTexRange, es, et;
	mapDrawSurface_t old;


	/* overflow check */
	if ( ds->numIndexes >= maxSurfaceIndexes ) {
		return 0;
	}

	/* test the triangle */
	if ( ds->entityNum != tri->entityNum ) { /* ydnar: added 2002-07-06 */
		return 0;
	}
	if ( ds->castShadows != tri->castShadows || ds->recvShadows != tri->recvShadows ) {
		return 0;
	}
	if ( ds->shaderInfo != tri->si || ds->fogNum != tri->fogNum || ds->sampleSize != tri->sampleSize ) {
		return 0;
	}
	#if 0
	if ( !( ds->shaderInfo->compileFlags & C_VERTEXLIT ) &&
	     //% VectorCompare( ds->lightmapAxis, tri->lightmapAxis ) == qfalse )
		 DotProduct( ds->lightmapAxis, tri->plane ) < 0.25f ) {
		return 0;
	}
	#endif

	/* planar surfaces will only merge with triangles in the same plane */
	if ( npDegrees == 0.0f && ds->shaderInfo->nonplanar == qfalse && ds->planeNum >= 0 ) {
		if ( VectorCompare( mapplanes[ ds->planeNum ].normal, tri->plane ) == qfalse || mapplanes[ ds->planeNum ].dist != tri->plane[ 3 ] ) {
			return 0;
		}
		if ( tri->planeNum >= 0 && tri->planeNum != ds->planeNum ) {
			return 0;
		}
	}

	/* set initial score */
	score = tri->surfaceNum == ds->surfaceNum ? SURFACE_SCORE : 0;

	/* score the the dot product of lightmap axis to plane */
	if ( ( ds->shaderInfo->compileFlags & C_VERTEXLIT ) || VectorCompare( ds->lightmapAxis, tri->lightmapAxis ) ) {
		score += AXIS_SCORE;
	}
	else{
		score += AXIS_SCORE * DotProduct( ds->lightmapAxis, tri->plane );
	}

	/* preserve old drawsurface if this fails */
	memcpy( &old, ds, sizeof( *ds ) );

	/* attempt to add the verts */
	coincident = 0;
	ai = AddMetaVertToSurface( ds, &metaVerts[ tri->indexes[ 0 ] ], &coincident );
	bi = AddMetaVertToSurface( ds, &metaVerts[ tri->indexes[ 1 ] ], &coincident );
	ci = AddMetaVertToSurface( ds, &metaVerts[ tri->indexes[ 2 ] ], &coincident );

	/* check vertex underflow */
	if ( ai < 0 || bi < 0 || ci < 0 ) {
		memcpy( ds, &old, sizeof( *ds ) );
		return 0;
	}

	/* score coincident vertex count (2003-02-14: changed so this only matters on planar surfaces) */
	score += ( coincident * VERT_SCORE );

	/* add new vertex bounds to mins/maxs */
	VectorCopy( ds->mins, mins );
	VectorCopy( ds->maxs, maxs );
	AddPointToBounds( metaVerts[ tri->indexes[ 0 ] ].xyz, mins, maxs );
	AddPointToBounds( metaVerts[ tri->indexes[ 1 ] ].xyz, mins, maxs );
	AddPointToBounds( metaVerts[ tri->indexes[ 2 ] ].xyz, mins, maxs );

	/* check lightmap bounds overflow (after at least 1 triangle has been added) */
	if ( !( ds->shaderInfo->compileFlags & C_VERTEXLIT ) &&
		 ds->numIndexes > 0 && VectorLength( ds->lightmapAxis ) > 0.0f &&
		 ( VectorCompare( ds->mins, mins ) == qfalse || VectorCompare( ds->maxs, maxs ) == qfalse ) ) {
		/* set maximum size before lightmap scaling (normally 2032 units) */
		/* 2004-02-24: scale lightmap test size by 2 to catch larger brush faces */
		/* 2004-04-11: reverting to actual lightmap size */
		lmMax = ( ds->sampleSize * ( ds->shaderInfo->lmCustomWidth - 1 ) );
		for ( i = 0; i < 3; i++ )
		{
			if ( ( maxs[ i ] - mins[ i ] ) > lmMax ) {
				memcpy( ds, &old, sizeof( *ds ) );
				return 0;
			}
		}
	}

	/* check texture range overflow */
	oldTexRange[ 0 ] = ds->texRange[ 0 ];
	oldTexRange[ 1 ] = ds->texRange[ 1 ];
	inTexRange = CalcSurfaceTextureRange( ds );

	es = ( ds->texRange[ 0 ] > oldTexRange[ 0 ] ) ? qtrue : qfalse;
	et = ( ds->texRange[ 1 ] > oldTexRange[ 1 ] ) ? qtrue : qfalse;

	if ( inTexRange == qfalse && ds->numIndexes > 0 ) {
		memcpy( ds, &old, sizeof( *ds ) );
		return UNSUITABLE_TRIANGLE;
	}

	/* score texture range */
	if ( ds->texRange[ 0 ] <= oldTexRange[ 0 ] ) {
		score += ST_SCORE2;
	}
	else if ( ds->texRange[ 0 ] > oldTexRange[ 0 ] && oldTexRange[ 1 ] > oldTexRange[ 0 ] ) {
		score += ST_SCORE;
	}

	if ( ds->texRange[ 1 ] <= oldTexRange[ 1 ] ) {
		score += ST_SCORE2;
	}
	else if ( ds->texRange[ 1 ] > oldTexRange[ 1 ] && oldTexRange[ 0 ] > oldTexRange[ 1 ] ) {
		score += ST_SCORE;
	}


	/* go through the indexes and try to find an existing triangle that matches abc */
	for ( i = 0; i < ds->numIndexes; i += 3 )
	{
		/* 2002-03-11 (birthday!): rotate the triangle 3x to find an existing triangle */
		if ( ( ai == ds->indexes[ i ] && bi == ds->indexes[ i + 1 ] && ci == ds->indexes[ i + 2 ] ) ||
			 ( bi == ds->indexes[ i ] && ci == ds->indexes[ i + 1 ] && ai == ds->indexes[ i + 2 ] ) ||
			 ( ci == ds->indexes[ i ] && ai == ds->indexes[ i + 1 ] && bi == ds->indexes[ i + 2 ] ) ) {
			/* triangle already present */
			memcpy( ds, &old, sizeof( *ds ) );
			tri->si = NULL;
			return 0;
		}

		/* rotate the triangle 3x to find an inverse triangle (error case) */
		if ( ( ai == ds->indexes[ i ] && bi == ds->indexes[ i + 2 ] && ci == ds->indexes[ i + 1 ] ) ||
			 ( bi == ds->indexes[ i ] && ci == ds->indexes[ i + 2 ] && ai == ds->indexes[ i + 1 ] ) ||
			 ( ci == ds->indexes[ i ] && ai == ds->indexes[ i + 2 ] && bi == ds->indexes[ i + 1 ] ) ) {
			/* warn about it */
			Sys_FPrintf( SYS_WRN, "WARNING: Flipped triangle: (%6.0f %6.0f %6.0f) (%6.0f %6.0f %6.0f) (%6.0f %6.0f %6.0f)\n",
						ds->verts[ ai ].xyz[ 0 ], ds->verts[ ai ].xyz[ 1 ], ds->verts[ ai ].xyz[ 2 ],
						ds->verts[ bi ].xyz[ 0 ], ds->verts[ bi ].xyz[ 1 ], ds->verts[ bi ].xyz[ 2 ],
						ds->verts[ ci ].xyz[ 0 ], ds->verts[ ci ].xyz[ 1 ], ds->verts[ ci ].xyz[ 2 ] );

			/* reverse triangle already present */
			memcpy( ds, &old, sizeof( *ds ) );
			tri->si = NULL;
			return 0;
		}
	}

	/* add the triangle indexes */
	if ( ds->numIndexes < maxSurfaceIndexes ) {
		ds->indexes[ ds->numIndexes++ ] = ai;
	}
	if ( ds->numIndexes < maxSurfaceIndexes ) {
		ds->indexes[ ds->numIndexes++ ] = bi;
	}
	if ( ds->numIndexes < maxSurfaceIndexes ) {
		ds->indexes[ ds->numIndexes++ ] = ci;
	}

	/* check index overflow */
	if ( ds->numIndexes >= maxSurfaceIndexes  ) {
		memcpy( ds, &old, sizeof( *ds ) );
		return 0;
	}

	/* sanity check the indexes */
	if ( ds->numIndexes >= 3 &&
		 ( ds->indexes[ ds->numIndexes - 3 ] == ds->indexes[ ds->numIndexes - 2 ] ||
		   ds->indexes[ ds->numIndexes - 3 ] == ds->indexes[ ds->numIndexes - 1 ] ||
		   ds->indexes[ ds->numIndexes - 2 ] == ds->indexes[ ds->numIndexes - 1 ] ) ) {
		Sys_Printf( "DEG:%d! ", ds->numVerts );
	}

	/* testing only? */
	if ( testAdd ) {
		memcpy( ds, &old, sizeof( *ds ) );
	}
	else
	{
		/* copy bounds back to surface */
		VectorCopy( mins, ds->mins );
		VectorCopy( maxs, ds->maxs );

		/* mark triangle as used */
		tri->si = NULL;
	}

	/* add a side reference */
	ds->sideRef = AllocSideRef( tri->side, ds->sideRef );

	/* return to sender */
	return score;
}



/*
   MetaTrianglesToSurface()
   creates map drawsurface(s) from the list of possibles
 */

static void MetaTrianglesToSurface( int numPossibles, metaTriangle_t *possibles, int *fOld, int *numAdded ){
	int i, j, f, best, score, bestScore;
	metaTriangle_t      *seed, *test;
	mapDrawSurface_t    *ds;
	bspDrawVert_t       *verts;
	int                 *indexes;
	qboolean added;


	/* allocate arrays */
	verts = safe_malloc( sizeof( *verts ) * maxSurfaceVerts );
	indexes = safe_malloc( sizeof( *indexes ) * maxSurfaceIndexes );

	/* walk the list of triangles */
	for ( i = 0, seed = possibles; i < numPossibles; i++, seed++ )
	{
		/* skip this triangle if it has already been merged */
		if ( seed->si == NULL ) {
			continue;
		}

		/* -----------------------------------------------------------------
		   initial drawsurf construction
		   ----------------------------------------------------------------- */

		/* start a new drawsurface */
		ds = AllocDrawSurface( SURFACE_META );
		ds->entityNum = seed->entityNum;
		ds->surfaceNum = seed->surfaceNum;
		ds->castShadows = seed->castShadows;
		ds->recvShadows = seed->recvShadows;

		ds->shaderInfo = seed->si;
		ds->planeNum = seed->planeNum;
		ds->fogNum = seed->fogNum;
		ds->sampleSize = seed->sampleSize;
		ds->verts = verts;
		ds->indexes = indexes;
		VectorCopy( seed->lightmapAxis, ds->lightmapAxis );
		ds->sideRef = AllocSideRef( seed->side, NULL );

		ClearBounds( ds->mins, ds->maxs );

		/* clear verts/indexes */
		memset( verts, 0, sizeof( *verts ) * maxSurfaceVerts );
		memset( indexes, 0, sizeof( *indexes ) * maxSurfaceIndexes );

		/* add the first triangle */
		if ( AddMetaTriangleToSurface( ds, seed, qfalse ) ) {
			( *numAdded )++;
		}

		/* -----------------------------------------------------------------
		   add triangles
		   ----------------------------------------------------------------- */

		/* progressively walk the list until no more triangles can be added */
		added = qtrue;
		while ( added )
		{
			/* print pacifier */
			f = 10 * *numAdded / numMetaTriangles;
			if ( f > *fOld ) {
				*fOld = f;
				Sys_FPrintf( SYS_VRB, "%d...", f );
			}

			/* reset best score */
			best = -1;
			bestScore = 0;
			added = qfalse;

			/* walk the list of possible candidates for merging */
			for ( j = i + 1, test = &possibles[ j ]; j < numPossibles; j++, test++ )
			{
				/* skip this triangle if it has already been merged */
				if ( test->si == NULL ) {
					continue;
				}

				/* score this triangle */
				score = AddMetaTriangleToSurface( ds, test, qtrue );
				if ( score > bestScore ) {
					best = j;
					bestScore = score;

					/* if we have a score over a certain threshold, just use it */
					if ( bestScore >= GOOD_SCORE ) {
						if ( AddMetaTriangleToSurface( ds, &possibles[ best ], qfalse ) ) {
							( *numAdded )++;
						}

						/* reset */
						best = -1;
						bestScore = 0;
						added = qtrue;
					}
				}
			}

			/* add best candidate */
			if ( best >= 0 && bestScore > ADEQUATE_SCORE ) {
				if ( AddMetaTriangleToSurface( ds, &possibles[ best ], qfalse ) ) {
					( *numAdded )++;
				}

				/* reset */
				added = qtrue;
			}
		}

		/* copy the verts and indexes to the new surface */
		ds->verts = safe_malloc( ds->numVerts * sizeof( bspDrawVert_t ) );
		memcpy( ds->verts, verts, ds->numVerts * sizeof( bspDrawVert_t ) );
		ds->indexes = safe_malloc( ds->numIndexes * sizeof( int ) );
		memcpy( ds->indexes, indexes, ds->numIndexes * sizeof( int ) );

		/* classify the surface */
		ClassifySurfaces( 1, ds );

		/* add to count */
		numMergedSurfaces++;
	}

	/* free arrays */
	free( verts );
	free( indexes );
}



/*
   CompareMetaTriangles()
   compare function for qsort()
 */

static int CompareMetaTriangles( const void *a, const void *b ){
	int i, j, av, bv;
	vec3_t aMins, bMins;


	/* shader first */
	if ( ( (metaTriangle_t*) a )->si < ( (metaTriangle_t*) b )->si ) {
		return 1;
	}
	else if ( ( (metaTriangle_t*) a )->si > ( (metaTriangle_t*) b )->si ) {
		return -1;
	}

	/* then fog */
	else if ( ( (metaTriangle_t*) a )->fogNum < ( (metaTriangle_t*) b )->fogNum ) {
		return 1;
	}
	else if ( ( (metaTriangle_t*) a )->fogNum > ( (metaTriangle_t*) b )->fogNum ) {
		return -1;
	}

	/* then plane */
	#if 0
	else if ( npDegrees == 0.0f && ( (metaTriangle_t*) a )->si->nonplanar == qfalse &&
			  ( (metaTriangle_t*) a )->planeNum >= 0 && ( (metaTriangle_t*) a )->planeNum >= 0 ) {
		if ( ( (metaTriangle_t*) a )->plane[ 3 ] < ( (metaTriangle_t*) b )->plane[ 3 ] ) {
			return 1;
		}
		else if ( ( (metaTriangle_t*) a )->plane[ 3 ] > ( (metaTriangle_t*) b )->plane[ 3 ] ) {
			return -1;
		}
		else if ( ( (metaTriangle_t*) a )->plane[ 0 ] < ( (metaTriangle_t*) b )->plane[ 0 ] ) {
			return 1;
		}
		else if ( ( (metaTriangle_t*) a )->plane[ 0 ] > ( (metaTriangle_t*) b )->plane[ 0 ] ) {
			return -1;
		}
		else if ( ( (metaTriangle_t*) a )->plane[ 1 ] < ( (metaTriangle_t*) b )->plane[ 1 ] ) {
			return 1;
		}
		else if ( ( (metaTriangle_t*) a )->plane[ 1 ] > ( (metaTriangle_t*) b )->plane[ 1 ] ) {
			return -1;
		}
		else if ( ( (metaTriangle_t*) a )->plane[ 2 ] < ( (metaTriangle_t*) b )->plane[ 2 ] ) {
			return 1;
		}
		else if ( ( (metaTriangle_t*) a )->plane[ 2 ] > ( (metaTriangle_t*) b )->plane[ 2 ] ) {
			return -1;
		}
	}
	#endif

	/* then position in world */

	/* find mins */
	VectorSet( aMins, 999999, 999999, 999999 );
	VectorSet( bMins, 999999, 999999, 999999 );
	for ( i = 0; i < 3; i++ )
	{
		av = ( (metaTriangle_t*) a )->indexes[ i ];
		bv = ( (metaTriangle_t*) b )->indexes[ i ];
		for ( j = 0; j < 3; j++ )
		{
			if ( metaVerts[ av ].xyz[ j ] < aMins[ j ] ) {
				aMins[ j ] = metaVerts[ av ].xyz[ j ];
			}
			if ( metaVerts[ bv ].xyz[ j ] < bMins[ j ] ) {
				bMins[ j ] = metaVerts[ bv ].xyz[ j ];
			}
		}
	}

	/* test it */
	for ( i = 0; i < 3; i++ )
	{
		if ( aMins[ i ] < bMins[ i ] ) {
			return 1;
		}
		else if ( aMins[ i ] > bMins[ i ] ) {
			return -1;
		}
	}

	/* functionally equivalent */
	return 0;
}



/*
   MergeMetaTriangles()
   merges meta triangles into drawsurfaces
 */

void MergeMetaTriangles( void ){
	int i, j, fOld, start, numAdded;
	metaTriangle_t      *head, *end;


	/* only do this if there are meta triangles */
	if ( numMetaTriangles <= 0 ) {
		return;
	}

	/* note it */
	Sys_FPrintf( SYS_VRB, "--- MergeMetaTriangles ---\n" );

	/* sort the triangles by shader major, fognum minor */
	qsort( metaTriangles, numMetaTriangles, sizeof( metaTriangle_t ), CompareMetaTriangles );

	/* init pacifier */
	fOld = -1;
	start = I_FloatTime();
	numAdded = 0;

	/* merge */
	for ( i = 0, j = 0; i < numMetaTriangles; i = j )
	{
		/* get head of list */
		head = &metaTriangles[ i ];

		/* skip this triangle if it has already been merged */
		if ( head->si == NULL ) {
			continue;
		}

		/* find end */
		if ( j <= i ) {
			for ( j = i + 1; j < numMetaTriangles; j++ )
			{
				/* get end of list */
				end = &metaTriangles[ j ];
				if ( head->si != end->si || head->fogNum != end->fogNum ) {
					break;
				}
			}
		}

		/* try to merge this list of possible merge candidates */
		MetaTrianglesToSurface( ( j - i ), head, &fOld, &numAdded );
	}

	/* clear meta triangle list */
	ClearMetaTriangles();

	/* print time */
	if ( i ) {
		Sys_FPrintf( SYS_VRB, " (%d)\n", (int) ( I_FloatTime() - start ) );
	}

	/* emit some stats */
	Sys_FPrintf( SYS_VRB, "%9d surfaces merged\n", numMergedSurfaces );
	Sys_FPrintf( SYS_VRB, "%9d vertexes merged\n", numMergedVerts );
}
