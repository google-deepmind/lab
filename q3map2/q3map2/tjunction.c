/* -------------------------------------------------------------------------------

   Copyright (C) 1999-2007 id Software, Inc., 2017 Google Inc. and contributors.
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
#define TJUNCTION_C



/* dependencies */
#include "q3map2.h"




typedef struct edgePoint_s {
	float intercept;
	vec3_t xyz;
	struct edgePoint_s  *prev, *next;
} edgePoint_t;

typedef struct edgeLine_s {
	vec3_t normal1;
	float dist1;

	vec3_t normal2;
	float dist2;

	vec3_t origin;
	vec3_t dir;

	edgePoint_t chain;      // unused element of doubly linked list
} edgeLine_t;

typedef struct {
	float length;
	bspDrawVert_t   *dv[2];
} originalEdge_t;

#define MAX_ORIGINAL_EDGES  0x20000
originalEdge_t originalEdges[MAX_ORIGINAL_EDGES];
int numOriginalEdges;


#define MAX_EDGE_LINES      0x10000
edgeLine_t edgeLines[MAX_EDGE_LINES];
int numEdgeLines;

int c_degenerateEdges;
int c_addedVerts;
int c_totalVerts;

int c_natural, c_rotate, c_cant;

// these should be whatever epsilon we actually expect,
// plus SNAP_INT_TO_FLOAT
#define LINE_POSITION_EPSILON   0.25
#define POINT_ON_LINE_EPSILON   0.25

/*
   ====================
   InsertPointOnEdge
   ====================
 */
void InsertPointOnEdge( vec3_t v, edgeLine_t *e ) {
	vec3_t delta;
	float d;
	edgePoint_t *p, *scan;

	VectorSubtract( v, e->origin, delta );
	d = DotProduct( delta, e->dir );

	p = safe_malloc( sizeof( edgePoint_t ) );
	p->intercept = d;
	VectorCopy( v, p->xyz );

	if ( e->chain.next == &e->chain ) {
		e->chain.next = e->chain.prev = p;
		p->next = p->prev = &e->chain;
		return;
	}

	scan = e->chain.next;
	for ( ; scan != &e->chain ; scan = scan->next ) {
		d = p->intercept - scan->intercept;
		if ( d > -LINE_POSITION_EPSILON && d < LINE_POSITION_EPSILON ) {
			free( p );
			return;     // the point is already set
		}

		if ( p->intercept < scan->intercept ) {
			// insert here
			p->prev = scan->prev;
			p->next = scan;
			scan->prev->next = p;
			scan->prev = p;
			return;
		}
	}

	// add at the end
	p->prev = scan->prev;
	p->next = scan;
	scan->prev->next = p;
	scan->prev = p;
}


/*
   ====================
   AddEdge
   ====================
 */
int AddEdge( vec3_t v1, vec3_t v2, qboolean createNonAxial ) {
	int i;
	edgeLine_t  *e;
	float d;
	vec3_t dir;

	VectorSubtract( v2, v1, dir );
	d = VectorNormalize( dir, dir );
	if ( d < 0.1 ) {
		// if we added a 0 length vector, it would make degenerate planes
		c_degenerateEdges++;
		return -1;
	}

	if ( !createNonAxial ) {
		if ( fabs( dir[0] + dir[1] + dir[2] ) != 1.0 ) {
			if ( numOriginalEdges == MAX_ORIGINAL_EDGES ) {
				Error( "MAX_ORIGINAL_EDGES" );
			}
			originalEdges[ numOriginalEdges ].dv[0] = (bspDrawVert_t *)v1;
			originalEdges[ numOriginalEdges ].dv[1] = (bspDrawVert_t *)v2;
			originalEdges[ numOriginalEdges ].length = d;
			numOriginalEdges++;
			return -1;
		}
	}

	for ( i = 0 ; i < numEdgeLines ; i++ ) {
		e = &edgeLines[i];

		d = DotProduct( v1, e->normal1 ) - e->dist1;
		if ( d < -POINT_ON_LINE_EPSILON || d > POINT_ON_LINE_EPSILON ) {
			continue;
		}
		d = DotProduct( v1, e->normal2 ) - e->dist2;
		if ( d < -POINT_ON_LINE_EPSILON || d > POINT_ON_LINE_EPSILON ) {
			continue;
		}

		d = DotProduct( v2, e->normal1 ) - e->dist1;
		if ( d < -POINT_ON_LINE_EPSILON || d > POINT_ON_LINE_EPSILON ) {
			continue;
		}
		d = DotProduct( v2, e->normal2 ) - e->dist2;
		if ( d < -POINT_ON_LINE_EPSILON || d > POINT_ON_LINE_EPSILON ) {
			continue;
		}

		// this is the edge
		InsertPointOnEdge( v1, e );
		InsertPointOnEdge( v2, e );
		return i;
	}

	// create a new edge
	if ( numEdgeLines >= MAX_EDGE_LINES ) {
		Error( "MAX_EDGE_LINES" );
	}

	e = &edgeLines[ numEdgeLines ];
	numEdgeLines++;

	e->chain.next = e->chain.prev = &e->chain;

	VectorCopy( v1, e->origin );
	VectorCopy( dir, e->dir );

	MakeNormalVectors( e->dir, e->normal1, e->normal2 );
	e->dist1 = DotProduct( e->origin, e->normal1 );
	e->dist2 = DotProduct( e->origin, e->normal2 );

	InsertPointOnEdge( v1, e );
	InsertPointOnEdge( v2, e );

	return numEdgeLines - 1;
}



/*
   AddSurfaceEdges()
   adds a surface's edges
 */

void AddSurfaceEdges( mapDrawSurface_t *ds ){
	int i;


	for ( i = 0; i < ds->numVerts; i++ )
	{
		/* save the edge number in the lightmap field so we don't need to look it up again */
		ds->verts[i].lightmap[ 0 ][ 0 ] =
			AddEdge( ds->verts[ i ].xyz, ds->verts[ ( i + 1 ) % ds->numVerts ].xyz, qfalse );
	}
}



/*
   ColinearEdge()
   determines if an edge is colinear
 */

qboolean ColinearEdge( vec3_t v1, vec3_t v2, vec3_t v3 ){
	vec3_t midpoint, dir, offset, on;
	float d;

	VectorSubtract( v2, v1, midpoint );
	VectorSubtract( v3, v1, dir );
	d = VectorNormalize( dir, dir );
	if ( d == 0 ) {
		return qfalse;  // degenerate
	}

	d = DotProduct( midpoint, dir );
	VectorScale( dir, d, on );
	VectorSubtract( midpoint, on, offset );
	d = VectorLength( offset );

	if ( d < 0.1 ) {
		return qtrue;
	}

	return qfalse;
}



/*
   ====================
   AddPatchEdges

   Add colinear border edges, which will fix some classes of patch to
   brush tjunctions
   ====================
 */
void AddPatchEdges( mapDrawSurface_t *ds ) {
	int i;
	float   *v1, *v2, *v3;

	for ( i = 0 ; i < ds->patchWidth - 2; i += 2 ) {
		v1 = ds->verts[ i ].xyz;
		v2 = ds->verts[ i + 1 ].xyz;
		v3 = ds->verts[ i + 2 ].xyz;

		// if v2 is the midpoint of v1 to v3, add an edge from v1 to v3
		if ( ColinearEdge( v1, v2, v3 ) ) {
			AddEdge( v1, v3, qfalse );
		}

		v1 = ds->verts[ ( ds->patchHeight - 1 ) * ds->patchWidth + i ].xyz;
		v2 = ds->verts[ ( ds->patchHeight - 1 ) * ds->patchWidth + i + 1 ].xyz;
		v3 = ds->verts[ ( ds->patchHeight - 1 ) * ds->patchWidth + i + 2 ].xyz;

		// if v2 is on the v1 to v3 line, add an edge from v1 to v3
		if ( ColinearEdge( v1, v2, v3 ) ) {
			AddEdge( v1, v3, qfalse );
		}
	}

	for ( i = 0 ; i < ds->patchHeight - 2 ; i += 2 ) {
		v1 = ds->verts[ i * ds->patchWidth ].xyz;
		v2 = ds->verts[ ( i + 1 ) * ds->patchWidth ].xyz;
		v3 = ds->verts[ ( i + 2 ) * ds->patchWidth ].xyz;

		// if v2 is the midpoint of v1 to v3, add an edge from v1 to v3
		if ( ColinearEdge( v1, v2, v3 ) ) {
			AddEdge( v1, v3, qfalse );
		}

		v1 = ds->verts[ ( ds->patchWidth - 1 ) + i * ds->patchWidth ].xyz;
		v2 = ds->verts[ ( ds->patchWidth - 1 ) + ( i + 1 ) * ds->patchWidth ].xyz;
		v3 = ds->verts[ ( ds->patchWidth - 1 ) + ( i + 2 ) * ds->patchWidth ].xyz;

		// if v2 is the midpoint of v1 to v3, add an edge from v1 to v3
		if ( ColinearEdge( v1, v2, v3 ) ) {
			AddEdge( v1, v3, qfalse );
		}
	}


}


/*
   ====================
   FixSurfaceJunctions
   ====================
 */
#define MAX_SURFACE_VERTS   256
void FixSurfaceJunctions( mapDrawSurface_t *ds ) {
	int i, j, k;
	edgeLine_t  *e;
	edgePoint_t *p;
	int originalVerts;
	int counts[MAX_SURFACE_VERTS];
	int originals[MAX_SURFACE_VERTS];
	int firstVert[MAX_SURFACE_VERTS];
	bspDrawVert_t verts[MAX_SURFACE_VERTS], *v1, *v2;
	int numVerts;
	float start, end, frac, c;
	vec3_t delta;


	originalVerts = ds->numVerts;

	numVerts = 0;
	for ( i = 0 ; i < ds->numVerts ; i++ )
	{
		counts[i] = 0;
		firstVert[i] = numVerts;

		// copy first vert
		if ( numVerts == MAX_SURFACE_VERTS ) {
			Error( "MAX_SURFACE_VERTS" );
		}
		verts[numVerts] = ds->verts[i];
		originals[numVerts] = i;
		numVerts++;

		// check to see if there are any t junctions before the next vert
		v1 = &ds->verts[i];
		v2 = &ds->verts[ ( i + 1 ) % ds->numVerts ];

		j = (int)ds->verts[i].lightmap[ 0 ][ 0 ];
		if ( j == -1 ) {
			continue;       // degenerate edge
		}
		e = &edgeLines[ j ];

		VectorSubtract( v1->xyz, e->origin, delta );
		start = DotProduct( delta, e->dir );

		VectorSubtract( v2->xyz, e->origin, delta );
		end = DotProduct( delta, e->dir );


		if ( start < end ) {
			p = e->chain.next;
		}
		else {
			p = e->chain.prev;
		}

		for (  ; p != &e->chain ; ) {
			if ( start < end ) {
				if ( p->intercept > end - ON_EPSILON ) {
					break;
				}
			}
			else {
				if ( p->intercept < end + ON_EPSILON ) {
					break;
				}
			}

			if (
				( start < end && p->intercept > start + ON_EPSILON ) ||
				( start > end && p->intercept < start - ON_EPSILON ) ) {
				// insert this point
				if ( numVerts == MAX_SURFACE_VERTS ) {
					Error( "MAX_SURFACE_VERTS" );
				}
				memset(&verts[ numVerts ], 0, sizeof(verts[ numVerts ]));

				/* take the exact intercept point */
				VectorCopy( p->xyz, verts[ numVerts ].xyz );

				/* interpolate the texture coordinates */
				frac = ( p->intercept - start ) / ( end - start );
				for ( j = 0 ; j < 2 ; j++ ) {
					verts[ numVerts ].st[j] = v1->st[j] +
											  frac * ( v2->st[j] - v1->st[j] );
				}

				/* copy the normal (FIXME: what about nonplanar surfaces? */
				VectorCopy( v1->normal, verts[ numVerts ].normal );

				/* ydnar: interpolate the color */
				for ( k = 0; k < MAX_LIGHTMAPS; k++ )
				{
					for ( j = 0; j < 4; j++ )
					{
						c = (float) v1->color[ k ][ j ] + frac * ( (float) v2->color[ k ][ j ] - (float) v1->color[ k ][ j ] );
						verts[ numVerts ].color[ k ][ j ] = (byte) ( c < 255.0f ? c : 255 );
					}
				}

				/* next... */
				originals[ numVerts ] = i;
				numVerts++;
				counts[ i ]++;
			}

			if ( start < end ) {
				p = p->next;
			}
			else {
				p = p->prev;
			}
		}
	}

	c_addedVerts += numVerts - ds->numVerts;
	c_totalVerts += numVerts;


	// FIXME: check to see if the entire surface degenerated
	// after snapping

	// rotate the points so that the initial vertex is between
	// two non-subdivided edges
	for ( i = 0 ; i < numVerts ; i++ ) {
		if ( originals[ ( i + 1 ) % numVerts ] == originals[ i ] ) {
			continue;
		}
		j = ( i + numVerts - 1 ) % numVerts;
		k = ( i + numVerts - 2 ) % numVerts;
		if ( originals[ j ] == originals[ k ] ) {
			continue;
		}
		break;
	}

	if ( i == 0 ) {
		// fine the way it is
		c_natural++;

		ds->numVerts = numVerts;
		ds->verts = safe_malloc( numVerts * sizeof( *ds->verts ) );
		memcpy( ds->verts, verts, numVerts * sizeof( *ds->verts ) );

		return;
	}
	if ( i == numVerts ) {
		// create a vertex in the middle to start the fan
		c_cant++;

/*
        memset ( &verts[numVerts], 0, sizeof( verts[numVerts] ) );
        for ( i = 0 ; i < numVerts ; i++ ) {
            for ( j = 0 ; j < 10 ; j++ ) {
                verts[numVerts].xyz[j] += verts[i].xyz[j];
            }
        }
        for ( j = 0 ; j < 10 ; j++ ) {
            verts[numVerts].xyz[j] /= numVerts;
        }

        i = numVerts;
        numVerts++;
 */
	}
	else {
		// just rotate the vertexes
		c_rotate++;

	}

	ds->numVerts = numVerts;
	ds->verts = safe_malloc( numVerts * sizeof( *ds->verts ) );

	for ( j = 0 ; j < ds->numVerts ; j++ ) {
		ds->verts[j] = verts[ ( j + i ) % ds->numVerts ];
	}
}





/*
   FixBrokenSurface() - ydnar
   removes nearly coincident verts from a planar winding surface
   returns qfalse if the surface is broken
 */

extern void SnapWeldVector( vec3_t a, vec3_t b, vec3_t out );

#define DEGENERATE_EPSILON  0.1

int c_broken = 0;

qboolean FixBrokenSurface( mapDrawSurface_t *ds ){
	qboolean valid = qtrue;
	bspDrawVert_t   *dv1, *dv2, avg;
	int i, j, k;
	float dist;


	/* dummy check */
	if ( ds == NULL ) {
		return qfalse;
	}
	if ( ds->type != SURFACE_FACE ) {
		return qfalse;
	}

	/* check all verts */
	for ( i = 0; i < ds->numVerts; i++ )
	{
		/* don't remove points if winding is a triangle */
		if ( ds->numVerts == 3 ) {
			return valid;
		}

		/* get verts */
		dv1 = &ds->verts[ i ];
		dv2 = &ds->verts[ ( i + 1 ) % ds->numVerts ];

		/* degenerate edge? */
		VectorSubtract( dv1->xyz, dv2->xyz, avg.xyz );
		dist = VectorLength( avg.xyz );
		if ( dist < DEGENERATE_EPSILON ) {
			valid = qfalse;
			Sys_FPrintf( SYS_VRB, "WARNING: Degenerate T-junction edge found, fixing...\n" );

			/* create an average drawvert */
			/* ydnar 2002-01-26: added nearest-integer welding preference */
			SnapWeldVector( dv1->xyz, dv2->xyz, avg.xyz );
			VectorAdd( dv1->normal, dv2->normal, avg.normal );
			VectorNormalize( avg.normal, avg.normal );
			avg.st[ 0 ] = ( dv1->st[ 0 ] + dv2->st[ 0 ] ) * 0.5f;
			avg.st[ 1 ] = ( dv1->st[ 1 ] + dv2->st[ 1 ] ) * 0.5f;

			/* lightmap st/colors */
			for ( k = 0; k < MAX_LIGHTMAPS; k++ )
			{
				avg.lightmap[ k ][ 0 ] = ( dv1->lightmap[ k ][ 0 ] + dv2->lightmap[ k ][ 0 ] ) * 0.5f;
				avg.lightmap[ k ][ 1 ] = ( dv1->lightmap[ k ][ 1 ] + dv2->lightmap[ k ][ 1 ] ) * 0.5f;
				for ( j = 0; j < 4; j++ )
					avg.color[ k ][ j ] = (int) ( dv1->color[ k ][ j ] + dv2->color[ k ][ j ] ) >> 1;
			}

			/* ydnar: der... */
			memcpy( dv1, &avg, sizeof( avg ) );

			/* move the remaining verts */
			for ( k = i + 2; k < ds->numVerts; k++ )
			{
				/* get verts */
				dv1 = &ds->verts[ k ];
				dv2 = &ds->verts[ k - 1 ];

				/* copy */
				memcpy( dv2, dv1, sizeof( bspDrawVert_t ) );
			}
			ds->numVerts--;
		}
	}

	/* one last check and return */
	if ( ds->numVerts < 3 ) {
		valid = qfalse;
	}
	return valid;
}









/*
   ================
   EdgeCompare
   ================
 */
int EdgeCompare( const void *elem1, const void *elem2 ) {
	float d1, d2;

	d1 = ( (originalEdge_t *)elem1 )->length;
	d2 = ( (originalEdge_t *)elem2 )->length;

	if ( d1 < d2 ) {
		return -1;
	}
	if ( d2 > d1 ) {
		return 1;
	}
	return 0;
}



/*
   FixTJunctions
   call after the surface list has been pruned
 */

void FixTJunctions( entity_t *ent ){
	int i;
	mapDrawSurface_t    *ds;
	shaderInfo_t        *si;
	int axialEdgeLines;
	originalEdge_t      *e;


	/* meta mode has its own t-junction code (currently not as good as this code) */
	//%	if( meta )
	//%		return;

	/* note it */
	Sys_FPrintf( SYS_VRB, "--- FixTJunctions ---\n" );
	numEdgeLines = 0;
	numOriginalEdges = 0;

	// add all the edges
	// this actually creates axial edges, but it
	// only creates originalEdge_t structures
	// for non-axial edges
	for ( i = ent->firstDrawSurf ; i < numMapDrawSurfs ; i++ )
	{
		/* get surface and early out if possible */
		ds = &mapDrawSurfs[ i ];
		si = ds->shaderInfo;
		if ( ( si->compileFlags & C_NODRAW ) || si->autosprite || si->notjunc || ds->numVerts == 0 ) {
			continue;
		}

		/* ydnar: gs mods: handle the various types of surfaces */
		switch ( ds->type )
		{
		/* handle brush faces */
		case SURFACE_FACE:
			AddSurfaceEdges( ds );
			break;

		/* handle patches */
		case SURFACE_PATCH:
			AddPatchEdges( ds );
			break;

		/* fixme: make triangle surfaces t-junction */
		default:
			break;
		}
	}

	axialEdgeLines = numEdgeLines;

	// sort the non-axial edges by length
	qsort( originalEdges, numOriginalEdges, sizeof( originalEdges[0] ), EdgeCompare );

	// add the non-axial edges, longest first
	// this gives the most accurate edge description
	for ( i = 0 ; i < numOriginalEdges ; i++ ) {
		e = &originalEdges[i];
		e->dv[ 0 ]->lightmap[ 0 ][ 0 ] = AddEdge( e->dv[ 0 ]->xyz, e->dv[ 1 ]->xyz, qtrue );
	}

	Sys_FPrintf( SYS_VRB, "%9d axial edge lines\n", axialEdgeLines );
	Sys_FPrintf( SYS_VRB, "%9d non-axial edge lines\n", numEdgeLines - axialEdgeLines );
	Sys_FPrintf( SYS_VRB, "%9d degenerate edges\n", c_degenerateEdges );

	// insert any needed vertexes
	for ( i = ent->firstDrawSurf; i < numMapDrawSurfs ; i++ )
	{
		/* get surface and early out if possible */
		ds = &mapDrawSurfs[ i ];
		si = ds->shaderInfo;
		if ( ( si->compileFlags & C_NODRAW ) || si->autosprite || si->notjunc || ds->numVerts == 0 || ds->type != SURFACE_FACE ) {
			continue;
		}

		/* ydnar: gs mods: handle the various types of surfaces */
		switch ( ds->type )
		{
		/* handle brush faces */
		case SURFACE_FACE:
			FixSurfaceJunctions( ds );
			if ( FixBrokenSurface( ds ) == qfalse ) {
				c_broken++;
				ClearSurface( ds );
			}
			break;

		/* fixme: t-junction triangle models and patches */
		default:
			break;
		}
	}

	/* emit some statistics */
	Sys_FPrintf( SYS_VRB, "%9d verts added for T-junctions\n", c_addedVerts );
	Sys_FPrintf( SYS_VRB, "%9d total verts\n", c_totalVerts );
	Sys_FPrintf( SYS_VRB, "%9d naturally ordered\n", c_natural );
	Sys_FPrintf( SYS_VRB, "%9d rotated orders\n", c_rotate );
	Sys_FPrintf( SYS_VRB, "%9d can't order\n", c_cant );
	Sys_FPrintf( SYS_VRB, "%9d broken (degenerate) surfaces removed\n", c_broken );
}
