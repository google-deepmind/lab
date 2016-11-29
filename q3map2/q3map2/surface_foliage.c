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

   Foliage code for Wolfenstein: Enemy Territory by ydnar@splashdamage.com

   ------------------------------------------------------------------------------- */



/* marker */
#define SURFACE_FOLIAGE_C



/* dependencies */
#include "q3map2.h"



#define MAX_FOLIAGE_INSTANCES   8192

static int numFoliageInstances;
static foliageInstance_t foliageInstances[ MAX_FOLIAGE_INSTANCES ];



/*
   SubdivideFoliageTriangle_r()
   recursively subdivides a triangle until the triangle is smaller than
   the desired density, then pseudo-randomly sets a point
 */

static void SubdivideFoliageTriangle_r( mapDrawSurface_t *ds, foliage_t *foliage, bspDrawVert_t **tri ){
	bspDrawVert_t mid, *tri2[ 3 ];
	int max;


	/* limit test */
	if ( numFoliageInstances >= MAX_FOLIAGE_INSTANCES ) {
		return;
	}

	/* plane test */
	{
		vec4_t plane;


		/* make a plane */
		if ( !PlaneFromPoints( plane, tri[ 0 ]->xyz, tri[ 1 ]->xyz, tri[ 2 ]->xyz ) ) {
			return;
		}

		/* if normal is too far off vertical, then don't place an instance */
		if ( plane[ 2 ] < 0.5f ) {
			return;
		}
	}

	/* subdivide calc */
	{
		int i;
		float               *a, *b, dx, dy, dz, dist, maxDist;
		foliageInstance_t   *fi;


		/* get instance */
		fi = &foliageInstances[ numFoliageInstances ];

		/* find the longest edge and split it */
		max = -1;
		maxDist = 0.0f;
		VectorClear( fi->xyz );
		VectorClear( fi->normal );
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

			/* add to centroid */
			VectorAdd( fi->xyz, tri[ i ]->xyz, fi->xyz );
			VectorAdd( fi->normal, tri[ i ]->normal, fi->normal );
		}

		/* is the triangle small enough? */
		if ( maxDist <= ( foliage->density * foliage->density ) ) {
			float alpha, odds, r;


			/* get average alpha */
			if ( foliage->inverseAlpha == 2 ) {
				alpha = 1.0f;
			}
			else
			{
				alpha = ( (float) tri[ 0 ]->color[ 0 ][ 3 ] + (float) tri[ 1 ]->color[ 0 ][ 3 ] + (float) tri[ 2 ]->color[ 0 ][ 3 ] ) / 765.0f;
				if ( foliage->inverseAlpha == 1 ) {
					alpha = 1.0f - alpha;
				}
				if ( alpha < 0.75f ) {
					return;
				}
			}

			/* roll the dice */
			odds = foliage->odds * alpha;
			r = Random();
			if ( r > odds ) {
				return;
			}

			/* scale centroid */
			VectorScale( fi->xyz, 0.33333333f, fi->xyz );
			if ( VectorNormalize( fi->normal, fi->normal ) == 0.0f ) {
				return;
			}

			/* add to count and return */
			numFoliageInstances++;
			return;
		}
	}

	/* split the longest edge and map it */
	LerpDrawVert( tri[ max ], tri[ ( max + 1 ) % 3 ], &mid );

	/* recurse to first triangle */
	VectorCopy( tri, tri2 );
	tri2[ max ] = &mid;
	SubdivideFoliageTriangle_r( ds, foliage, tri2 );

	/* recurse to second triangle */
	VectorCopy( tri, tri2 );
	tri2[ ( max + 1 ) % 3 ] = &mid;
	SubdivideFoliageTriangle_r( ds, foliage, tri2 );
}



/*
   GenFoliage()
   generates a foliage file for a bsp
 */

void Foliage( mapDrawSurface_t *src ){
	int i, j, k, x, y, pw[ 5 ], r, oldNumMapDrawSurfs;
	mapDrawSurface_t    *ds;
	shaderInfo_t        *si;
	foliage_t           *foliage;
	mesh_t srcMesh, *subdivided, *mesh;
	bspDrawVert_t       *verts, *dv[ 3 ], *fi;
	vec3_t scale;
	m4x4_t transform;


	/* get shader */
	si = src->shaderInfo;
	if ( si == NULL || si->foliage == NULL ) {
		return;
	}

	/* do every foliage */
	for ( foliage = si->foliage; foliage != NULL; foliage = foliage->next )
	{
		/* zero out */
		numFoliageInstances = 0;

		/* map the surface onto the lightmap origin/cluster/normal buffers */
		switch ( src->type )
		{
		case SURFACE_META:
		case SURFACE_FORCED_META:
		case SURFACE_TRIANGLES:
			/* get verts */
			verts = src->verts;

			/* map the triangles */
			for ( i = 0; i < src->numIndexes; i += 3 )
			{
				dv[ 0 ] = &verts[ src->indexes[ i ] ];
				dv[ 1 ] = &verts[ src->indexes[ i + 1 ] ];
				dv[ 2 ] = &verts[ src->indexes[ i + 2 ] ];
				SubdivideFoliageTriangle_r( src, foliage, dv );
			}
			break;

		case SURFACE_PATCH:
			/* make a mesh from the drawsurf */
			srcMesh.width = src->patchWidth;
			srcMesh.height = src->patchHeight;
			srcMesh.verts = src->verts;
			subdivided = SubdivideMesh( srcMesh, 8, 512 );

			/* fit it to the curve and remove colinear verts on rows/columns */
			PutMeshOnCurve( *subdivided );
			mesh = RemoveLinearMeshColumnsRows( subdivided );
			FreeMesh( subdivided );

			/* get verts */
			verts = mesh->verts;

			/* map the mesh quads */
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
					SubdivideFoliageTriangle_r( src, foliage, dv );

					/* get drawverts and map second triangle */
					dv[ 0 ] = &verts[ pw[ r + 0 ] ];
					dv[ 1 ] = &verts[ pw[ r + 2 ] ];
					dv[ 2 ] = &verts[ pw[ r + 3 ] ];
					SubdivideFoliageTriangle_r( src, foliage, dv );
				}
			}

			/* free the mesh */
			FreeMesh( mesh );
			break;

		default:
			break;
		}

		/* any origins? */
		if ( numFoliageInstances < 1 ) {
			continue;
		}

		/* remember surface count */
		oldNumMapDrawSurfs = numMapDrawSurfs;

		/* set transform matrix */
		VectorSet( scale, foliage->scale, foliage->scale, foliage->scale );
		m4x4_scale_for_vec3( transform, scale );

		/* add the model to the bsp */
		InsertModel( foliage->model, 0, transform, NULL, NULL, src->entityNum, src->castShadows, src->recvShadows, 0, src->lightmapScale );

		/* walk each new surface */
		for ( i = oldNumMapDrawSurfs; i < numMapDrawSurfs; i++ )
		{
			/* get surface */
			ds = &mapDrawSurfs[ i ];

			/* set up */
			ds->type = SURFACE_FOLIAGE;
			ds->numFoliageInstances = numFoliageInstances;

			/* a wee hack */
			ds->patchWidth = ds->numFoliageInstances;
			ds->patchHeight = ds->numVerts;

			/* set fog to be same as source surface */
			ds->fogNum = src->fogNum;

			/* add a drawvert for every instance */
			verts = safe_malloc( ( ds->numVerts + ds->numFoliageInstances ) * sizeof( *verts ) );
			memset( verts, 0, ( ds->numVerts + ds->numFoliageInstances ) * sizeof( *verts ) );
			memcpy( verts, ds->verts, ds->numVerts * sizeof( *verts ) );
			free( ds->verts );
			ds->verts = verts;

			/* copy the verts */
			for ( j = 0; j < ds->numFoliageInstances; j++ )
			{
				/* get vert (foliage instance) */
				fi = &ds->verts[ ds->numVerts + j ];

				/* copy xyz and normal */
				VectorCopy( foliageInstances[ j ].xyz, fi->xyz );
				VectorCopy( foliageInstances[ j ].normal, fi->normal );

				/* ydnar: set color */
				for ( k = 0; k < MAX_LIGHTMAPS; k++ )
				{
					fi->color[ k ][ 0 ] = 255;
					fi->color[ k ][ 1 ] = 255;
					fi->color[ k ][ 2 ] = 255;
					fi->color[ k ][ 3 ] = 255;
				}
			}

			/* increment */
			ds->numVerts += ds->numFoliageInstances;
		}
	}
}
