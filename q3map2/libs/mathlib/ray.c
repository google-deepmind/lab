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

#include "mathlib.h"
/*! for memcpy */
#include <memory.h>

vec3_t identity = { 0,0,0 };

void ray_construct_for_vec3( ray_t *ray, const vec3_t origin, const vec3_t direction ){
	VectorCopy( origin, ray->origin );
	VectorCopy( direction, ray->direction );
}

void ray_transform( ray_t *ray, const m4x4_t matrix ){
	m4x4_transform_point( matrix, ray->origin );
	m4x4_transform_normal( matrix, ray->direction );
}

vec_t ray_intersect_point( const ray_t *ray, const vec3_t point, vec_t epsilon, vec_t divergence ){
	vec3_t displacement;
	vec_t depth;

	// calc displacement of test point from ray origin
	VectorSubtract( point, ray->origin, displacement );
	// calc length of displacement vector along ray direction
	depth = DotProduct( displacement, ray->direction );
	if ( depth < 0.0f ) {
		return (vec_t)VEC_MAX;
	}
	// calc position of closest point on ray to test point
	VectorMA( ray->origin, depth, ray->direction, displacement );
	// calc displacement of test point from closest point
	VectorSubtract( point, displacement, displacement );
	// calc length of displacement, subtract depth-dependant epsilon
	if ( VectorLength( displacement ) - ( epsilon + ( depth * divergence ) ) > 0.0f ) {
		return (vec_t)VEC_MAX;
	}
	return depth;
}

// Tomas Moller and Ben Trumbore. Fast, minimum storage ray-triangle intersection. Journal of graphics tools, 2(1):21-28, 1997

#define EPSILON 0.000001

vec_t ray_intersect_triangle( const ray_t *ray, qboolean bCullBack, const vec3_t vert0, const vec3_t vert1, const vec3_t vert2 ){
	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float det,inv_det;
	float u, v;
	vec_t depth = (vec_t)VEC_MAX;

	/* find vectors for two edges sharing vert0 */
	VectorSubtract( vert1, vert0, edge1 );
	VectorSubtract( vert2, vert0, edge2 );

	/* begin calculating determinant - also used to calculate U parameter */
	CrossProduct( ray->direction, edge2, pvec );

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DotProduct( edge1, pvec );

	if ( bCullBack == qtrue ) {
		if ( det < EPSILON ) {
			return depth;
		}

		// calculate distance from vert0 to ray origin
		VectorSubtract( ray->origin, vert0, tvec );

		// calculate U parameter and test bounds
		u = DotProduct( tvec, pvec );
		if ( u < 0.0 || u > det ) {
			return depth;
		}

		// prepare to test V parameter
		CrossProduct( tvec, edge1, qvec );

		// calculate V parameter and test bounds
		v = DotProduct( ray->direction, qvec );
		if ( v < 0.0 || u + v > det ) {
			return depth;
		}

		// calculate t, scale parameters, ray intersects triangle
		depth = DotProduct( edge2, qvec );
		inv_det = 1.0f / det;
		depth *= inv_det;
		//u *= inv_det;
		//v *= inv_det;
	}
	else
	{
		/* the non-culling branch */
		if ( det > -EPSILON && det < EPSILON ) {
			return depth;
		}
		inv_det = 1.0f / det;

		/* calculate distance from vert0 to ray origin */
		VectorSubtract( ray->origin, vert0, tvec );

		/* calculate U parameter and test bounds */
		u = DotProduct( tvec, pvec ) * inv_det;
		if ( u < 0.0 || u > 1.0 ) {
			return depth;
		}

		/* prepare to test V parameter */
		CrossProduct( tvec, edge1, qvec );

		/* calculate V parameter and test bounds */
		v = DotProduct( ray->direction, qvec ) * inv_det;
		if ( v < 0.0 || u + v > 1.0 ) {
			return depth;
		}

		/* calculate t, ray intersects triangle */
		depth = DotProduct( edge2, qvec ) * inv_det;
	}
	return depth;
}
