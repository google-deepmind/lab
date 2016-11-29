/*
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
 */

#include <float.h>

#include "mathlib.h"

void aabb_construct_for_vec3( aabb_t *aabb, const vec3_t min, const vec3_t max ){
	VectorMid( min, max, aabb->origin );
	VectorSubtract( max, aabb->origin, aabb->extents );
}

void aabb_update_radius( aabb_t *aabb ){
	aabb->radius = VectorLength( aabb->extents );
}

void aabb_clear( aabb_t *aabb ){
	aabb->origin[0] = aabb->origin[1] = aabb->origin[2] = 0;
	aabb->extents[0] = aabb->extents[1] = aabb->extents[2] = -FLT_MAX;
}

void aabb_extend_by_point( aabb_t *aabb, const vec3_t point ){
	int i;
	vec_t min, max, displacement;
	for ( i = 0; i < 3; i++ )
	{
		displacement = point[i] - aabb->origin[i];
		if ( fabs( displacement ) > aabb->extents[i] ) {
			if ( aabb->extents[i] < 0 ) { // degenerate
				min = max = point[i];
			}
			else if ( displacement > 0 ) {
				min = aabb->origin[i] - aabb->extents[i];
				max = aabb->origin[i] + displacement;
			}
			else
			{
				max = aabb->origin[i] + aabb->extents[i];
				min = aabb->origin[i] + displacement;
			}
			aabb->origin[i] = ( min + max ) * 0.5f;
			aabb->extents[i] = max - aabb->origin[i];
		}
	}
}

void aabb_extend_by_aabb( aabb_t *aabb, const aabb_t *aabb_src ){
	int i;
	vec_t min, max, displacement, difference;
	for ( i = 0; i < 3; i++ )
	{
		displacement = aabb_src->origin[i] - aabb->origin[i];
		difference = aabb_src->extents[i] - aabb->extents[i];
		if ( aabb->extents[i] < 0
			 || difference >= fabs( displacement ) ) {
			// 2nd contains 1st
			aabb->extents[i] = aabb_src->extents[i];
			aabb->origin[i] = aabb_src->origin[i];
		}
		else if ( aabb_src->extents[i] < 0
				  || -difference >= fabs( displacement ) ) {
			// 1st contains 2nd
			continue;
		}
		else
		{
			// not contained
			if ( displacement > 0 ) {
				min = aabb->origin[i] - aabb->extents[i];
				max = aabb_src->origin[i] + aabb_src->extents[i];
			}
			else
			{
				min = aabb_src->origin[i] - aabb_src->extents[i];
				max = aabb->origin[i] + aabb->extents[i];
			}
			aabb->origin[i] = ( min + max ) * 0.5f;
			aabb->extents[i] = max - aabb->origin[i];
		}
	}
}

void aabb_extend_by_vec3( aabb_t *aabb, vec3_t extension ){
	VectorAdd( aabb->extents, extension, aabb->extents );
}

int aabb_intersect_point( const aabb_t *aabb, const vec3_t point ){
	int i;
	for ( i = 0; i < 3; i++ )
		if ( fabs( point[i] - aabb->origin[i] ) >= aabb->extents[i] ) {
			return 0;
		}
	return 1;
}

int aabb_intersect_aabb( const aabb_t *aabb, const aabb_t *aabb_src ){
	int i;
	for ( i = 0; i < 3; i++ )
		if ( fabs( aabb_src->origin[i] - aabb->origin[i] ) > ( fabs( aabb->extents[i] ) + fabs( aabb_src->extents[i] ) ) ) {
			return 0;
		}
	return 1;
}

int aabb_intersect_plane( const aabb_t *aabb, const float *plane ){
	float fDist, fIntersect;

	// calc distance of origin from plane
	fDist = DotProduct( plane, aabb->origin ) + plane[3];

	// trivial accept/reject using bounding sphere
	if ( fabs( fDist ) > aabb->radius ) {
		if ( fDist < 0 ) {
			return 2; // totally inside
		}
		else{
			return 0; // totally outside
		}
	}

	// calc extents distance relative to plane normal
	fIntersect = (vec_t)( fabs( plane[0] * aabb->extents[0] ) + fabs( plane[1] * aabb->extents[1] ) + fabs( plane[2] * aabb->extents[2] ) );
	// accept if origin is less than or equal to this distance
	if ( fabs( fDist ) < fIntersect ) {
		return 1;                         // partially inside
	}
	else if ( fDist < 0 ) {
		return 2;               // totally inside
	}
	return 0; // totally outside
}

/*
   Fast Ray-Box Intersection
   by Andrew Woo
   from "Graphics Gems", Academic Press, 1990
 */

#define NUMDIM  3
#define RIGHT   0
#define LEFT    1
#define MIDDLE  2

int aabb_intersect_ray( const aabb_t *aabb, const ray_t *ray, vec_t *dist ){
	int inside = 1;
	char quadrant[NUMDIM];
	register int i;
	int whichPlane;
	double maxT[NUMDIM];
	double candidatePlane[NUMDIM];
	vec3_t coord, segment;

	const float *origin = ray->origin;
	const float *direction = ray->direction;

	/* Find candidate planes; this loop can be avoided if
	   rays cast all from the eye(assume perpsective view) */
	for ( i = 0; i < NUMDIM; i++ )
	{
		if ( origin[i] < ( aabb->origin[i] - aabb->extents[i] ) ) {
			quadrant[i] = LEFT;
			candidatePlane[i] = ( aabb->origin[i] - aabb->extents[i] );
			inside = 0;
		}
		else if ( origin[i] > ( aabb->origin[i] + aabb->extents[i] ) ) {
			quadrant[i] = RIGHT;
			candidatePlane[i] = ( aabb->origin[i] + aabb->extents[i] );
			inside = 0;
		}
		else
		{
			quadrant[i] = MIDDLE;
		}
	}

	/* Ray origin inside bounding box */
	if ( inside == 1 ) {
		*dist = 0.0f;
		return 1;
	}


	/* Calculate T distances to candidate planes */
	for ( i = 0; i < NUMDIM; i++ )
	{
		if ( quadrant[i] != MIDDLE && direction[i] != 0. ) {
			maxT[i] = ( candidatePlane[i] - origin[i] ) / direction[i];
		}
		else{
			maxT[i] = -1.;
		}
	}

	/* Get largest of the maxT's for final choice of intersection */
	whichPlane = 0;
	for ( i = 1; i < NUMDIM; i++ )
		if ( maxT[whichPlane] < maxT[i] ) {
			whichPlane = i;
		}

	/* Check final candidate actually inside box */
	if ( maxT[whichPlane] < 0. ) {
		return 0;
	}
	for ( i = 0; i < NUMDIM; i++ )
	{
		if ( whichPlane != i ) {
			coord[i] = (vec_t)( origin[i] + maxT[whichPlane] * direction[i] );
			if ( fabs( coord[i] - aabb->origin[i] ) > aabb->extents[i] ) {
				return 0;
			}
		}
		else
		{
			coord[i] = (vec_t)candidatePlane[i];
		}
	}

	VectorSubtract( coord, origin, segment );
	*dist = DotProduct( segment, direction );

	return 1;               /* ray hits box */
}

int aabb_test_ray( const aabb_t* aabb, const ray_t* ray ){
	vec3_t displacement, ray_absolute;
	vec_t f;

	displacement[0] = ray->origin[0] - aabb->origin[0];
	if ( fabs( displacement[0] ) > aabb->extents[0] && displacement[0] * ray->direction[0] >= 0.0f ) {
		return 0;
	}

	displacement[1] = ray->origin[1] - aabb->origin[1];
	if ( fabs( displacement[1] ) > aabb->extents[1] && displacement[1] * ray->direction[1] >= 0.0f ) {
		return 0;
	}

	displacement[2] = ray->origin[2] - aabb->origin[2];
	if ( fabs( displacement[2] ) > aabb->extents[2] && displacement[2] * ray->direction[2] >= 0.0f ) {
		return 0;
	}

	ray_absolute[0] = (float)fabs( ray->direction[0] );
	ray_absolute[1] = (float)fabs( ray->direction[1] );
	ray_absolute[2] = (float)fabs( ray->direction[2] );

	f = ray->direction[1] * displacement[2] - ray->direction[2] * displacement[1];
	if ( (float)fabs( f ) > aabb->extents[1] * ray_absolute[2] + aabb->extents[2] * ray_absolute[1] ) {
		return 0;
	}

	f = ray->direction[2] * displacement[0] - ray->direction[0] * displacement[2];
	if ( (float)fabs( f ) > aabb->extents[0] * ray_absolute[2] + aabb->extents[2] * ray_absolute[0] ) {
		return 0;
	}

	f = ray->direction[0] * displacement[1] - ray->direction[1] * displacement[0];
	if ( (float)fabs( f ) > aabb->extents[0] * ray_absolute[1] + aabb->extents[1] * ray_absolute[0] ) {
		return 0;
	}

	return 1;
}

void aabb_for_bbox( aabb_t *aabb, const bbox_t *bbox ){
	int i;
	vec3_t temp[3];

	VectorCopy( bbox->aabb.origin, aabb->origin );

	// calculate the AABB extents in local coord space from the OBB extents and axes
	VectorScale( bbox->axes[0], bbox->aabb.extents[0], temp[0] );
	VectorScale( bbox->axes[1], bbox->aabb.extents[1], temp[1] );
	VectorScale( bbox->axes[2], bbox->aabb.extents[2], temp[2] );
	for ( i = 0; i < 3; i++ ) aabb->extents[i] = (vec_t)( fabs( temp[0][i] ) + fabs( temp[1][i] ) + fabs( temp[2][i] ) );
}

void aabb_for_area( aabb_t *aabb, vec3_t area_tl, vec3_t area_br, int axis ){
	aabb_clear( aabb );
	aabb->extents[axis] = FLT_MAX;
	aabb_extend_by_point( aabb, area_tl );
	aabb_extend_by_point( aabb, area_br );
}

void aabb_for_transformed_aabb( aabb_t* dst, const aabb_t* src, const m4x4_t transform ){
	VectorCopy( src->origin, dst->origin );
	m4x4_transform_point( transform, dst->origin );

	dst->extents[0] = (vec_t)( fabs( transform[0]  * src->extents[0] )
							   + fabs( transform[4]  * src->extents[1] )
							   + fabs( transform[8]  * src->extents[2] ) );
	dst->extents[1] = (vec_t)( fabs( transform[1]  * src->extents[0] )
							   + fabs( transform[5]  * src->extents[1] )
							   + fabs( transform[9]  * src->extents[2] ) );
	dst->extents[2] = (vec_t)( fabs( transform[2]  * src->extents[0] )
							   + fabs( transform[6]  * src->extents[1] )
							   + fabs( transform[10] * src->extents[2] ) );
}


void bbox_for_oriented_aabb( bbox_t *bbox, const aabb_t *aabb, const m4x4_t matrix, const vec3_t euler, const vec3_t scale ){
	double rad[3];
	double pi_180 = Q_PI / 180;
	double A, B, C, D, E, F, AD, BD;

	VectorCopy( aabb->origin, bbox->aabb.origin );

	m4x4_transform_point( matrix, bbox->aabb.origin );

	bbox->aabb.extents[0] = aabb->extents[0] * scale[0];
	bbox->aabb.extents[1] = aabb->extents[1] * scale[1];
	bbox->aabb.extents[2] = aabb->extents[2] * scale[2];

	rad[0] = euler[0] * pi_180;
	rad[1] = euler[1] * pi_180;
	rad[2] = euler[2] * pi_180;

	A       = cos( rad[0] );
	B       = sin( rad[0] );
	C       = cos( rad[1] );
	D       = sin( rad[1] );
	E       = cos( rad[2] );
	F       = sin( rad[2] );

	AD      =   A * -D;
	BD      =   B * -D;

	bbox->axes[0][0] = (vec_t)( C * E );
	bbox->axes[0][1] = (vec_t)( -BD * E + A * F );
	bbox->axes[0][2] = (vec_t)( AD * E + B * F );
	bbox->axes[1][0] = (vec_t)( -C * F );
	bbox->axes[1][1] = (vec_t)( BD * F + A * E );
	bbox->axes[1][2] = (vec_t)( -AD * F + B * E );
	bbox->axes[2][0] = (vec_t)D;
	bbox->axes[2][1] = (vec_t)( -B * C );
	bbox->axes[2][2] = (vec_t)( A * C );

	aabb_update_radius( &bbox->aabb );
}

int bbox_intersect_plane( const bbox_t *bbox, const vec_t* plane ){
	vec_t fDist, fIntersect;

	// calc distance of origin from plane
	fDist = DotProduct( plane, bbox->aabb.origin ) + plane[3];

	// trivial accept/reject using bounding sphere
	if ( fabs( fDist ) > bbox->aabb.radius ) {
		if ( fDist < 0 ) {
			return 2; // totally inside
		}
		else{
			return 0; // totally outside
		}
	}

	// calc extents distance relative to plane normal
	fIntersect = (vec_t)( fabs( bbox->aabb.extents[0] * DotProduct( plane, bbox->axes[0] ) )
						  + fabs( bbox->aabb.extents[1] * DotProduct( plane, bbox->axes[1] ) )
						  + fabs( bbox->aabb.extents[2] * DotProduct( plane, bbox->axes[2] ) ) );
	// accept if origin is less than this distance
	if ( fabs( fDist ) < fIntersect ) {
		return 1;                         // partially inside
	}
	else if ( fDist < 0 ) {
		return 2;               // totally inside
	}
	return 0; // totally outside
}
