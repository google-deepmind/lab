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
#define PORTALS_C



/* dependencies */
#include "q3map2.h"



/* ydnar: to fix broken portal windings */
extern qboolean FixWinding( winding_t *w );


int c_active_portals;
int c_peak_portals;
int c_boundary;
int c_boundary_sides;

/*
   ===========
   AllocPortal
   ===========
 */
portal_t *AllocPortal( void ){
	portal_t    *p;

	if ( numthreads == 1 ) {
		c_active_portals++;
	}
	if ( c_active_portals > c_peak_portals ) {
		c_peak_portals = c_active_portals;
	}

	p = safe_malloc( sizeof( portal_t ) );
	memset( p, 0, sizeof( portal_t ) );

	return p;
}

void FreePortal( portal_t *p ){
	if ( p->winding ) {
		FreeWinding( p->winding );
	}
	if ( numthreads == 1 ) {
		c_active_portals--;
	}
	free( p );
}



/*
   PortalPassable
   returns true if the portal has non-opaque leafs on both sides
 */

qboolean PortalPassable( portal_t *p ){
	/* is this to global outside leaf? */
	if ( !p->onnode ) {
		return qfalse;
	}

	/* this should never happen */
	if ( p->nodes[ 0 ]->planenum != PLANENUM_LEAF ||
		 p->nodes[ 1 ]->planenum != PLANENUM_LEAF ) {
		Error( "Portal_EntityFlood: not a leaf" );
	}

	/* ydnar: added antiportal to supress portal generation for visibility blocking */
	if ( p->compileFlags & C_ANTIPORTAL ) {
		return qfalse;
	}

	/* both leaves on either side of the portal must be passable */
	if ( p->nodes[ 0 ]->opaque == qfalse && p->nodes[ 1 ]->opaque == qfalse ) {
		return qtrue;
	}

	/* otherwise this isn't a passable portal */
	return qfalse;
}




int c_tinyportals;
int c_badportals;       /* ydnar */

/*
   =============
   AddPortalToNodes
   =============
 */
void AddPortalToNodes( portal_t *p, node_t *front, node_t *back ){
	if ( p->nodes[0] || p->nodes[1] ) {
		Error( "AddPortalToNode: allready included" );
	}

	p->nodes[0] = front;
	p->next[0] = front->portals;
	front->portals = p;

	p->nodes[1] = back;
	p->next[1] = back->portals;
	back->portals = p;
}


/*
   =============
   RemovePortalFromNode
   =============
 */
void RemovePortalFromNode( portal_t *portal, node_t *l ){
	portal_t    **pp, *t;

// remove reference to the current portal
	pp = &l->portals;
	while ( 1 )
	{
		t = *pp;
		if ( !t ) {
			Error( "RemovePortalFromNode: portal not in leaf" );
		}

		if ( t == portal ) {
			break;
		}

		if ( t->nodes[0] == l ) {
			pp = &t->next[0];
		}
		else if ( t->nodes[1] == l ) {
			pp = &t->next[1];
		}
		else{
			Error( "RemovePortalFromNode: portal not bounding leaf" );
		}
	}

	if ( portal->nodes[0] == l ) {
		*pp = portal->next[0];
		portal->nodes[0] = NULL;
	}
	else if ( portal->nodes[1] == l ) {
		*pp = portal->next[1];
		portal->nodes[1] = NULL;
	}
}

//============================================================================

void PrintPortal( portal_t *p ){
	int i;
	winding_t   *w;

	w = p->winding;
	for ( i = 0 ; i < w->numpoints ; i++ )
		Sys_Printf( "(%5.0f,%5.0f,%5.0f)\n",w->p[i][0]
					, w->p[i][1], w->p[i][2] );
}

/*
   ================
   MakeHeadnodePortals

   The created portals will face the global outside_node
   ================
 */
#define SIDESPACE   8
void MakeHeadnodePortals( tree_t *tree ){
	vec3_t bounds[2];
	int i, j, n;
	portal_t    *p, *portals[6];
	plane_t bplanes[6], *pl;
	node_t *node;

	node = tree->headnode;

// pad with some space so there will never be null volume leafs
	for ( i = 0 ; i < 3 ; i++ )
	{
		bounds[0][i] = tree->mins[i] - SIDESPACE;
		bounds[1][i] = tree->maxs[i] + SIDESPACE;
		if ( bounds[0][i] >= bounds[1][i] ) {
			Error( "Backwards tree volume" );
		}
	}

	tree->outside_node.planenum = PLANENUM_LEAF;
	tree->outside_node.brushlist = NULL;
	tree->outside_node.portals = NULL;
	tree->outside_node.opaque = qfalse;

	for ( i = 0 ; i < 3 ; i++ )
		for ( j = 0 ; j < 2 ; j++ )
		{
			n = j * 3 + i;

			p = AllocPortal();
			portals[n] = p;

			pl = &bplanes[n];
			memset( pl, 0, sizeof( *pl ) );
			if ( j ) {
				pl->normal[i] = -1;
				pl->dist = -bounds[j][i];
			}
			else
			{
				pl->normal[i] = 1;
				pl->dist = bounds[j][i];
			}
			p->plane = *pl;
			p->winding = BaseWindingForPlane( pl->normal, pl->dist );
			AddPortalToNodes( p, node, &tree->outside_node );
		}

// clip the basewindings by all the other planes
	for ( i = 0 ; i < 6 ; i++ )
	{
		for ( j = 0 ; j < 6 ; j++ )
		{
			if ( j == i ) {
				continue;
			}
			ChopWindingInPlace( &portals[i]->winding, bplanes[j].normal, bplanes[j].dist, ON_EPSILON );
		}
	}
}

//===================================================


/*
   ================
   BaseWindingForNode
   ================
 */
#define BASE_WINDING_EPSILON    0.001
#define SPLIT_WINDING_EPSILON   0.001

winding_t   *BaseWindingForNode( node_t *node ){
	winding_t   *w;
	node_t      *n;
	plane_t     *plane;
	vec3_t normal;
	vec_t dist;

	w = BaseWindingForPlane( mapplanes[node->planenum].normal
							 , mapplanes[node->planenum].dist );

	// clip by all the parents
	for ( n = node->parent ; n && w ; )
	{
		plane = &mapplanes[n->planenum];

		if ( n->children[0] == node ) { // take front
			ChopWindingInPlace( &w, plane->normal, plane->dist, BASE_WINDING_EPSILON );
		}
		else
		{   // take back
			VectorSubtract( vec3_origin, plane->normal, normal );
			dist = -plane->dist;
			ChopWindingInPlace( &w, normal, dist, BASE_WINDING_EPSILON );
		}
		node = n;
		n = n->parent;
	}

	return w;
}

//============================================================

/*
   ==================
   MakeNodePortal

   create the new portal by taking the full plane winding for the cutting plane
   and clipping it by all of parents of this node
   ==================
 */
void MakeNodePortal( node_t *node ){
	portal_t    *new_portal, *p;
	winding_t   *w;
	vec3_t normal;
	float dist;
	int side;

	w = BaseWindingForNode( node );

	// clip the portal by all the other portals in the node
	for ( p = node->portals ; p && w; p = p->next[side] )
	{
		if ( p->nodes[0] == node ) {
			side = 0;
			VectorCopy( p->plane.normal, normal );
			dist = p->plane.dist;
		}
		else if ( p->nodes[1] == node ) {
			side = 1;
			VectorSubtract( vec3_origin, p->plane.normal, normal );
			dist = -p->plane.dist;
		}
		else{
			Error( "CutNodePortals_r: mislinked portal" );
		}

		ChopWindingInPlace( &w, normal, dist, CLIP_EPSILON );
	}

	if ( !w ) {
		return;
	}


	/* ydnar: adding this here to fix degenerate windings */
	#if 0
	if ( FixWinding( w ) == qfalse ) {
		c_badportals++;
		FreeWinding( w );
		return;
	}
	#endif

	if ( WindingIsTiny( w ) ) {
		c_tinyportals++;
		FreeWinding( w );
		return;
	}

	new_portal = AllocPortal();
	new_portal->plane = mapplanes[node->planenum];
	new_portal->onnode = node;
	new_portal->winding = w;
	new_portal->compileFlags = node->compileFlags;
	AddPortalToNodes( new_portal, node->children[0], node->children[1] );
}


/*
   ==============
   SplitNodePortals

   Move or split the portals that bound node so that the node's
   children have portals instead of node.
   ==============
 */
void SplitNodePortals( node_t *node ){
	portal_t    *p, *next_portal, *new_portal;
	node_t      *f, *b, *other_node;
	int side;
	plane_t     *plane;
	winding_t   *frontwinding, *backwinding;

	plane = &mapplanes[node->planenum];
	f = node->children[0];
	b = node->children[1];

	for ( p = node->portals ; p ; p = next_portal )
	{
		if ( p->nodes[0] == node ) {
			side = 0;
		}
		else if ( p->nodes[1] == node ) {
			side = 1;
		}
		else{
			Error( "SplitNodePortals: mislinked portal" );
		}
		next_portal = p->next[side];

		other_node = p->nodes[!side];
		RemovePortalFromNode( p, p->nodes[0] );
		RemovePortalFromNode( p, p->nodes[1] );

//
// cut the portal into two portals, one on each side of the cut plane
//
		ClipWindingEpsilon( p->winding, plane->normal, plane->dist,
							SPLIT_WINDING_EPSILON, &frontwinding, &backwinding );

		if ( frontwinding && WindingIsTiny( frontwinding ) ) {
			if ( !f->tinyportals ) {
				VectorCopy( frontwinding->p[0], f->referencepoint );
			}
			f->tinyportals++;
			if ( !other_node->tinyportals ) {
				VectorCopy( frontwinding->p[0], other_node->referencepoint );
			}
			other_node->tinyportals++;

			FreeWinding( frontwinding );
			frontwinding = NULL;
			c_tinyportals++;
		}

		if ( backwinding && WindingIsTiny( backwinding ) ) {
			if ( !b->tinyportals ) {
				VectorCopy( backwinding->p[0], b->referencepoint );
			}
			b->tinyportals++;
			if ( !other_node->tinyportals ) {
				VectorCopy( backwinding->p[0], other_node->referencepoint );
			}
			other_node->tinyportals++;

			FreeWinding( backwinding );
			backwinding = NULL;
			c_tinyportals++;
		}

		if ( !frontwinding && !backwinding ) { // tiny windings on both sides
			continue;
		}

		if ( !frontwinding ) {
			FreeWinding( backwinding );
			if ( side == 0 ) {
				AddPortalToNodes( p, b, other_node );
			}
			else{
				AddPortalToNodes( p, other_node, b );
			}
			continue;
		}
		if ( !backwinding ) {
			FreeWinding( frontwinding );
			if ( side == 0 ) {
				AddPortalToNodes( p, f, other_node );
			}
			else{
				AddPortalToNodes( p, other_node, f );
			}
			continue;
		}

		// the winding is split
		new_portal = AllocPortal();
		*new_portal = *p;
		new_portal->winding = backwinding;
		FreeWinding( p->winding );
		p->winding = frontwinding;

		if ( side == 0 ) {
			AddPortalToNodes( p, f, other_node );
			AddPortalToNodes( new_portal, b, other_node );
		}
		else
		{
			AddPortalToNodes( p, other_node, f );
			AddPortalToNodes( new_portal, other_node, b );
		}
	}

	node->portals = NULL;
}


/*
   ================
   CalcNodeBounds
   ================
 */
void CalcNodeBounds( node_t *node ){
	portal_t    *p;
	int s;
	int i;

	// calc mins/maxs for both leafs and nodes
	ClearBounds( node->mins, node->maxs );
	for ( p = node->portals ; p ; p = p->next[s] )
	{
		s = ( p->nodes[1] == node );
		for ( i = 0 ; i < p->winding->numpoints ; i++ )
			AddPointToBounds( p->winding->p[i], node->mins, node->maxs );
	}
}

/*
   ==================
   MakeTreePortals_r
   ==================
 */
void MakeTreePortals_r( node_t *node ){
	int i;

	CalcNodeBounds( node );
	if ( node->mins[0] >= node->maxs[0] ) {
		Sys_FPrintf( SYS_WRN, "WARNING: node without a volume\n" );
		Sys_Printf( "node has %d tiny portals\n", node->tinyportals );
		Sys_Printf( "node reference point %1.2f %1.2f %1.2f\n", node->referencepoint[0],
					node->referencepoint[1],
					node->referencepoint[2] );
	}

	for ( i = 0 ; i < 3 ; i++ )
	{
		if ( node->mins[i] < MIN_WORLD_COORD || node->maxs[i] > MAX_WORLD_COORD ) {
			if ( node->portals && node->portals->winding ) {
				xml_Winding( "WARNING: Node With Unbounded Volume", node->portals->winding->p, node->portals->winding->numpoints, qfalse );
			}

			break;
		}
	}
	if ( node->planenum == PLANENUM_LEAF ) {
		return;
	}

	MakeNodePortal( node );
	SplitNodePortals( node );

	MakeTreePortals_r( node->children[0] );
	MakeTreePortals_r( node->children[1] );
}

/*
   ==================
   MakeTreePortals
   ==================
 */
void MakeTreePortals( tree_t *tree ){
	Sys_FPrintf( SYS_VRB, "--- MakeTreePortals ---\n" );
	MakeHeadnodePortals( tree );
	MakeTreePortals_r( tree->headnode );
	Sys_FPrintf( SYS_VRB, "%9d tiny portals\n", c_tinyportals );
	Sys_FPrintf( SYS_VRB, "%9d bad portals\n", c_badportals );  /* ydnar */
}

/*
   =========================================================

   FLOOD ENTITIES

   =========================================================
 */

int c_floodedleafs;

/*
   =============
   FloodPortals_r
   =============
 */

void FloodPortals_r( node_t *node, int dist, qboolean skybox ){
	int s;
	portal_t    *p;


	if ( skybox ) {
		node->skybox = skybox;
	}

	if ( node->occupied || node->opaque ) {
		return;
	}

	c_floodedleafs++;
	node->occupied = dist;

	for ( p = node->portals; p; p = p->next[ s ] )
	{
		s = ( p->nodes[ 1 ] == node );
		FloodPortals_r( p->nodes[ !s ], dist + 1, skybox );
	}
}



/*
   =============
   PlaceOccupant
   =============
 */

qboolean PlaceOccupant( node_t *headnode, vec3_t origin, entity_t *occupant, qboolean skybox ){
	vec_t d;
	node_t  *node;
	plane_t *plane;


	// find the leaf to start in
	node = headnode;
	while ( node->planenum != PLANENUM_LEAF )
	{
		plane = &mapplanes[ node->planenum ];
		d = DotProduct( origin, plane->normal ) - plane->dist;
		if ( d >= 0 ) {
			node = node->children[ 0 ];
		}
		else{
			node = node->children[ 1 ];
		}
	}

	if ( node->opaque ) {
		return qfalse;
	}
	node->occupant = occupant;
	node->skybox = skybox;

	FloodPortals_r( node, 1, skybox );

	return qtrue;
}

/*
   =============
   FloodEntities

   Marks all nodes that can be reached by entites
   =============
 */

qboolean FloodEntities( tree_t *tree ){
	int i, s;
	vec3_t origin, offset, scale, angles;
	qboolean r, inside, tripped, skybox;
	node_t      *headnode;
	entity_t    *e;
	const char  *value;


	headnode = tree->headnode;
	Sys_FPrintf( SYS_VRB,"--- FloodEntities ---\n" );
	inside = qfalse;
	tree->outside_node.occupied = 0;

	tripped = qfalse;
	c_floodedleafs = 0;
	for ( i = 1; i < numEntities; i++ )
	{
		/* get entity */
		e = &entities[ i ];

		/* get origin */
		GetVectorForKey( e, "origin", origin );
		if ( VectorCompare( origin, vec3_origin ) ) {
			continue;
		}

		/* handle skybox entities */
		value = ValueForKey( e, "classname" );
		if ( !Q_stricmp( value, "_skybox" ) ) {
			skybox = qtrue;
			skyboxPresent = qtrue;

			/* invert origin */
			VectorScale( origin, -1.0f, offset );

			/* get scale */
			VectorSet( scale, 64.0f, 64.0f, 64.0f );
			value = ValueForKey( e, "_scale" );
			if ( value[ 0 ] != '\0' ) {
				s = sscanf( value, "%f %f %f", &scale[ 0 ], &scale[ 1 ], &scale[ 2 ] );
				if ( s == 1 ) {
					scale[ 1 ] = scale[ 0 ];
					scale[ 2 ] = scale[ 0 ];
				}
			}

			/* get "angle" (yaw) or "angles" (pitch yaw roll) */
			VectorClear( angles );
			angles[ 2 ] = FloatForKey( e, "angle" );
			value = ValueForKey( e, "angles" );
			if ( value[ 0 ] != '\0' ) {
				sscanf( value, "%f %f %f", &angles[ 1 ], &angles[ 2 ], &angles[ 0 ] );
			}

			/* set transform matrix (thanks spog) */
			m4x4_identity( skyboxTransform );
			m4x4_pivoted_transform_by_vec3( skyboxTransform, offset, angles, eXYZ, scale, origin );
		}
		else{
			skybox = qfalse;
		}

		/* nudge off floor */
		origin[ 2 ] += 1;

		/* debugging code */
		//%	if( i == 1 )
		//%		origin[ 2 ] += 4096;

		/* find leaf */
		r = PlaceOccupant( headnode, origin, e, skybox );
		if ( r ) {
			inside = qtrue;
		}
		if ( ( !r || tree->outside_node.occupied ) && !tripped ) {
			xml_Select( "Entity leaked", e->mapEntityNum, 0, qfalse );
			tripped = qtrue;
		}
	}

	Sys_FPrintf( SYS_VRB, "%9d flooded leafs\n", c_floodedleafs );

	if ( !inside ) {
		Sys_FPrintf( SYS_VRB, "no entities in open -- no filling\n" );
	}
	else if ( tree->outside_node.occupied ) {
		Sys_FPrintf( SYS_VRB, "entity reached from outside -- no filling\n" );
	}

	return (qboolean) ( inside && !tree->outside_node.occupied );
}

/*
   =========================================================

   FLOOD AREAS

   =========================================================
 */

int c_areas;



/*
   FloodAreas_r()
   floods through leaf portals to tag leafs with an area
 */

void FloodAreas_r( node_t *node ){
	int s;
	portal_t    *p;
	brush_t     *b;


	if ( node->areaportal ) {
		if ( node->area == -1 ) {
			node->area = c_areas;
		}

		/* this node is part of an area portal brush */
		b = node->brushlist->original;

		/* if the current area has already touched this portal, we are done */
		if ( b->portalareas[ 0 ] == c_areas || b->portalareas[ 1 ] == c_areas ) {
			return;
		}

		// note the current area as bounding the portal
		if ( b->portalareas[ 1 ] != -1 ) {
			Sys_FPrintf( SYS_WRN, "WARNING: areaportal brush %i touches > 2 areas\n", b->brushNum );
			return;
		}
		if ( b->portalareas[ 0 ] != -1 ) {
			b->portalareas[ 1 ] = c_areas;
		}
		else{
			b->portalareas[ 0 ] = c_areas;
		}

		return;
	}

	if ( node->area != -1 ) {
		return;
	}
	if ( node->cluster == -1 ) {
		return;
	}

	node->area = c_areas;

	/* ydnar: skybox nodes set the skybox area */
	if ( node->skybox ) {
		skyboxArea = c_areas;
	}

	for ( p = node->portals; p; p = p->next[ s ] )
	{
		s = ( p->nodes[1] == node );

		/* ydnar: allow areaportal portals to block area flow */
		if ( p->compileFlags & C_AREAPORTAL ) {
			continue;
		}

		if ( !PortalPassable( p ) ) {
			continue;
		}

		FloodAreas_r( p->nodes[ !s ] );
	}
}

/*
   =============
   FindAreas_r

   Just decend the tree, and for each node that hasn't had an
   area set, flood fill out from there
   =============
 */
void FindAreas_r( node_t *node ){
	if ( node->planenum != PLANENUM_LEAF ) {
		FindAreas_r( node->children[ 0 ] );
		FindAreas_r( node->children[ 1 ] );
		return;
	}

	if ( node->opaque || node->areaportal || node->area != -1 ) {
		return;
	}

	FloodAreas_r( node );
	c_areas++;
}

/*
   =============
   CheckAreas_r
   =============
 */
void CheckAreas_r( node_t *node ){
	brush_t *b;

	if ( node->planenum != PLANENUM_LEAF ) {
		CheckAreas_r( node->children[0] );
		CheckAreas_r( node->children[1] );
		return;
	}

	if ( node->opaque ) {
		return;
	}

	if ( node->cluster != -1 ) {
		if ( node->area == -1 ) {
			Sys_FPrintf( SYS_WRN, "WARNING: cluster %d has area set to -1\n", node->cluster );
		}
	}
	if ( node->areaportal ) {
		b = node->brushlist->original;

		// check if the areaportal touches two areas
		if ( b->portalareas[0] == -1 || b->portalareas[1] == -1 ) {
			Sys_FPrintf( SYS_WRN, "WARNING: areaportal brush %i doesn't touch two areas\n", b->brushNum );
		}
	}
}



/*
   FloodSkyboxArea_r() - ydnar
   sets all nodes with the skybox area to skybox
 */

void FloodSkyboxArea_r( node_t *node ){
	if ( skyboxArea < 0 ) {
		return;
	}

	if ( node->planenum != PLANENUM_LEAF ) {
		FloodSkyboxArea_r( node->children[ 0 ] );
		FloodSkyboxArea_r( node->children[ 1 ] );
		return;
	}

	if ( node->opaque || node->area != skyboxArea ) {
		return;
	}

	node->skybox = qtrue;
}



/*
   FloodAreas()
   mark each leaf with an area, bounded by C_AREAPORTAL
 */

void FloodAreas( tree_t *tree ){
	Sys_FPrintf( SYS_VRB,"--- FloodAreas ---\n" );
	FindAreas_r( tree->headnode );

	/* ydnar: flood all skybox nodes */
	FloodSkyboxArea_r( tree->headnode );

	/* check for areaportal brushes that don't touch two areas */
	/* ydnar: fix this rather than just silence the warnings */
	//%	CheckAreas_r( tree->headnode );

	Sys_FPrintf( SYS_VRB, "%9d areas\n", c_areas );
}



//======================================================

int c_outside;
int c_inside;
int c_solid;

void FillOutside_r( node_t *node ){
	if ( node->planenum != PLANENUM_LEAF ) {
		FillOutside_r( node->children[0] );
		FillOutside_r( node->children[1] );
		return;
	}

	// anything not reachable by an entity
	// can be filled away
	if ( !node->occupied ) {
		if ( !node->opaque ) {
			c_outside++;
			node->opaque = qtrue;
		}
		else {
			c_solid++;
		}
	}
	else {
		c_inside++;
	}

}

/*
   =============
   FillOutside

   Fill all nodes that can't be reached by entities
   =============
 */
void FillOutside( node_t *headnode ){
	c_outside = 0;
	c_inside = 0;
	c_solid = 0;
	Sys_FPrintf( SYS_VRB,"--- FillOutside ---\n" );
	FillOutside_r( headnode );
	Sys_FPrintf( SYS_VRB,"%9d solid leafs\n", c_solid );
	Sys_Printf( "%9d leafs filled\n", c_outside );
	Sys_FPrintf( SYS_VRB, "%9d inside leafs\n", c_inside );
}


//==============================================================
