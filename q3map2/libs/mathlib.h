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

#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h
#include <math.h>
#include <float.h>

#include "bytebool.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec5_t[5];
typedef vec_t vec4_t[4];

// Smallest positive value for vec_t such that 1.0 + VEC_SMALLEST_EPSILON_AROUND_ONE != 1.0.
// In the case of 32 bit floats (which is almost certainly the case), it's 0.00000011921.
// Don't forget that your epsilons should depend on the possible range of values,
// because for example adding VEC_SMALLEST_EPSILON_AROUND_ONE to 1024.0 will have no effect.
#define VEC_SMALLEST_EPSILON_AROUND_ONE FLT_EPSILON

#define SIDE_FRONT      0
#define SIDE_ON         2
#define SIDE_BACK       1
#define SIDE_CROSS      -2

// plane types are used to speed some tests
// 0-2 are axial planes
#define PLANE_X         0
#define PLANE_Y         1
#define PLANE_Z         2
#define PLANE_NON_AXIAL 3

#define Q_PI    3.14159265358979323846f

extern vec3_t vec3_origin;

#define EQUAL_EPSILON   0.001

#ifndef VEC_MAX
#define VEC_MAX 3.402823466e+38F
#endif

qboolean VectorCompare( vec3_t v1, vec3_t v2 );

#define DotProduct( x,y ) ( ( x )[0] * ( y )[0] + ( x )[1] * ( y )[1] + ( x )[2] * ( y )[2] )
#define VectorSubtract( a,b,c ) ( ( c )[0] = ( a )[0] - ( b )[0],( c )[1] = ( a )[1] - ( b )[1],( c )[2] = ( a )[2] - ( b )[2] )
#define VectorAdd( a,b,c ) ( ( c )[0] = ( a )[0] + ( b )[0],( c )[1] = ( a )[1] + ( b )[1],( c )[2] = ( a )[2] + ( b )[2] )
#define VectorIncrement( a,b ) ( ( b )[0] += ( a )[0],( b )[1] += ( a )[1],( b )[2] += ( a )[2] )
#define VectorCopy( a,b ) ( ( b )[0] = ( a )[0],( b )[1] = ( a )[1],( b )[2] = ( a )[2] )
#define VectorSet( v, a, b, c ) ( ( v )[0] = ( a ),( v )[1] = ( b ),( v )[2] = ( c ) )
#define VectorScale( a,b,c ) ( ( c )[0] = ( b ) * ( a )[0],( c )[1] = ( b ) * ( a )[1],( c )[2] = ( b ) * ( a )[2] )
#define VectorMid( a,b,c ) ( ( c )[0] = ( ( a )[0] + ( b )[0] ) * 0.5f,( c )[1] = ( ( a )[1] + ( b )[1] ) * 0.5f,( c )[2] = ( ( a )[2] + ( b )[2] ) * 0.5f )
#define VectorNegative( a,b ) ( ( b )[0] = -( a )[0],( b )[1] = -( a )[1],( b )[2] = -( a )[2] )
#define CrossProduct( a,b,c ) ( ( c )[0] = ( a )[1] * ( b )[2] - ( a )[2] * ( b )[1],( c )[1] = ( a )[2] * ( b )[0] - ( a )[0] * ( b )[2],( c )[2] = ( a )[0] * ( b )[1] - ( a )[1] * ( b )[0] )
#define VectorClear( x ) ( ( x )[0] = ( x )[1] = ( x )[2] = 0 )

#define Q_rint( in ) ( (vec_t)floor( in + 0.5 ) )

qboolean VectorIsOnAxis( vec3_t v );
qboolean VectorIsOnAxialPlane( vec3_t v );

vec_t VectorLength( vec3_t v );

void VectorMA( const vec3_t va, vec_t scale, const vec3_t vb, vec3_t vc );

void _CrossProduct( vec3_t v1, vec3_t v2, vec3_t cross );
// I need this define in order to test some of the regression tests from time to time.
// This define affect the precision of VectorNormalize() function only.
#define MATHLIB_VECTOR_NORMALIZE_PRECISION_FIX 1
vec_t VectorNormalize( const vec3_t in, vec3_t out );
vec_t ColorNormalize( const vec3_t in, vec3_t out );
void VectorInverse( vec3_t v );
void VectorPolar( vec3_t v, float radius, float theta, float phi );

// default snapping, to 1
void VectorSnap( vec3_t v );

// integer snapping
void VectorISnap( vec3_t point, int snap );

// Gef:   added snap to float for sub-integer grid sizes
// TTimo: we still use the int version of VectorSnap when possible
//        to avoid potential rounding issues
// TTimo: renaming to VectorFSnap for C implementation
void VectorFSnap( vec3_t point, float snap );

// NOTE: added these from Ritual's Q3Radiant
void ClearBounds( vec3_t mins, vec3_t maxs );
void AddPointToBounds( vec3_t v, vec3_t mins, vec3_t maxs );

void AngleVectors( vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );
void VectorToAngles( vec3_t vec, vec3_t angles );

#define ZERO_EPSILON 1.0E-6
#define RAD2DEGMULT 57.29577951308232f
#define DEG2RADMULT 0.01745329251994329f
#define RAD2DEG( a ) ( ( a ) * RAD2DEGMULT )
#define DEG2RAD( a ) ( ( a ) * DEG2RADMULT )

void VectorRotate( vec3_t vIn, vec3_t vRotation, vec3_t out );
void VectorRotateOrigin( vec3_t vIn, vec3_t vRotation, vec3_t vOrigin, vec3_t out );

// some function merged from tools mathlib code

qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
void NormalToLatLong( const vec3_t normal, byte bytes[2] );
int PlaneTypeForNormal( vec3_t normal );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );

// Spog
// code imported from geomlib

/*!
   \todo
   FIXME test calls such as intersect tests should be named test_
 */

typedef vec_t m3x3_t[9];
/*!NOTE
   m4x4 looks like this..

                x  y  z
   x axis        ( 0  1  2)
   y axis        ( 4  5  6)
   z axis        ( 8  9 10)
   translation   (12 13 14)
   scale         ( 0  5 10)
 */
typedef vec_t m4x4_t[16];

#define M4X4_INDEX( m,row,col ) ( m[( col << 2 ) + row] )

typedef enum { TRANSLATE, SCALE, ROTATE } transformtype; // legacy, used only in pmesh.cpp

typedef enum { eXYZ, eYZX, eZXY, eXZY, eYXZ, eZYX } eulerOrder_t;

// constructors
/*! create m4x4 as identity matrix */
void m4x4_identity( m4x4_t matrix );
/*! create m4x4 as a translation matrix, for a translation vec3 */
void m4x4_translation_for_vec3( m4x4_t matrix, const vec3_t translation );
/*! create m4x4 as a rotation matrix, for an euler angles (degrees) vec3 */
void m4x4_rotation_for_vec3( m4x4_t matrix, const vec3_t euler, eulerOrder_t order );
/*! create m4x4 as a scaling matrix, for a scale vec3 */
void m4x4_scale_for_vec3( m4x4_t matrix, const vec3_t scale );
/*! create m4x4 as a rotation matrix, for a quaternion vec4 */
void m4x4_rotation_for_quat( m4x4_t matrix, const vec4_t rotation );
/*! create m4x4 as a rotation matrix, for an axis vec3 and an angle (radians) */
void m4x4_rotation_for_axisangle( m4x4_t matrix, const vec3_t axis, vec_t angle );

// a valid m4x4 to be modified is always first argument
/*! translate m4x4 by a translation vec3 */
void m4x4_translate_by_vec3( m4x4_t matrix, const vec3_t translation );
/*! rotate m4x4 by a euler (degrees) vec3 */
void m4x4_rotate_by_vec3( m4x4_t matrix, const vec3_t euler, eulerOrder_t order );
/*! scale m4x4 by a scaling vec3 */
void m4x4_scale_by_vec3( m4x4_t matrix, const vec3_t scale );
/*! rotate m4x4 by a quaternion vec4 */
void m4x4_rotate_by_quat( m4x4_t matrix, const vec4_t rotation );
/*! rotate m4x4 by an axis vec3 and an angle (radians) */
void m4x4_rotate_by_axisangle( m4x4_t matrix, const vec3_t axis, vec_t angle );
/*! transform m4x4 by translation/euler/scaling vec3 (transform = translation.euler.scale) */
void m4x4_transform_by_vec3( m4x4_t matrix, const vec3_t translation, const vec3_t euler, eulerOrder_t order, const vec3_t scale );
/*! rotate m4x4 around a pivot point by euler(degrees) vec3 */
void m4x4_pivoted_rotate_by_vec3( m4x4_t matrix, const vec3_t euler, eulerOrder_t order, const vec3_t pivotpoint );
/*! scale m4x4 around a pivot point by scaling vec3 */
void m4x4_pivoted_scale_by_vec3( m4x4_t matrix, const vec3_t scale, const vec3_t pivotpoint );
/*! transform m4x4 around a pivot point by translation/euler/scaling vec3 */
void m4x4_pivoted_transform_by_vec3( m4x4_t matrix, const vec3_t translation, const vec3_t euler, eulerOrder_t order, const vec3_t scale, const vec3_t pivotpoint );
/*! rotate m4x4 around a pivot point by quaternion vec4 */
void m4x4_pivoted_rotate_by_quat( m4x4_t matrix, const vec4_t rotation, const vec3_t pivotpoint );
/*! rotate m4x4 around a pivot point by axis vec3 and angle (radians) */
void m4x4_pivoted_rotate_by_axisangle( m4x4_t matrix, const vec3_t axis, vec_t angle, const vec3_t pivotpoint );
/*! post-multiply m4x4 by another m4x4 */
void m4x4_multiply_by_m4x4( m4x4_t matrix, const m4x4_t other );
/*! pre-multiply m4x4 by another m4x4 */
void m4x4_premultiply_by_m4x4( m4x4_t matrix, const m4x4_t other );

/*! multiply a point (x,y,z,1) by matrix */
void m4x4_transform_point( const m4x4_t matrix, vec3_t point );
/*! multiply a normal (x,y,z,0) by matrix */
void m4x4_transform_normal( const m4x4_t matrix, vec3_t normal );
/*! multiply a vec4 (x,y,z,w) by matrix */
void m4x4_transform_vec4( const m4x4_t matrix, vec4_t vector );

/*! multiply a point (x,y,z,1) by matrix */
void m4x4_transform_point( const m4x4_t matrix, vec3_t point );
/*! multiply a normal (x,y,z,0) by matrix */
void m4x4_transform_normal( const m4x4_t matrix, vec3_t normal );

/*! transpose a m4x4 */
void m4x4_transpose( m4x4_t matrix );
/*! invert an orthogonal 4x3 subset of a 4x4 matrix */
void m4x4_orthogonal_invert( m4x4_t matrix );
/*! invert any m4x4 using Kramer's rule.. return 1 if matrix is singular, else return 0 */
int m4x4_invert( m4x4_t matrix );

/*!
   \todo object/ray intersection functions should maybe return a point rather than a distance?
 */

/*!
   aabb_t -  "axis-aligned" bounding box...
   origin: centre of bounding box...
   extents: +/- extents of box from origin...
   radius: cached length of extents vector...
 */
typedef struct aabb_s
{
	vec3_t origin;
	vec3_t extents;
	vec_t radius;
} aabb_t;

/*!
   bbox_t - oriented bounding box...
   aabb: axis-aligned bounding box...
   axes: orientation axes...
 */
typedef struct bbox_s
{
	aabb_t aabb;
	vec3_t axes[3];
} bbox_t;

/*!
   ray_t - origin point and direction unit-vector
 */
typedef struct ray_s
{
	vec3_t origin;
	vec3_t direction;
} ray_t;


/*! Generate AABB from min/max. */
void aabb_construct_for_vec3( aabb_t *aabb, const vec3_t min, const vec3_t max );
/*! Update bounding-sphere radius. */
void aabb_update_radius( aabb_t *aabb );
/*! Initialise AABB to negative size. */
void aabb_clear( aabb_t *aabb );

/*! Extend AABB to include point. */
void aabb_extend_by_point( aabb_t *aabb, const vec3_t point );
/*! Extend AABB to include aabb_src. */
void aabb_extend_by_aabb( aabb_t *aabb, const aabb_t *aabb_src );
/*! Extend AABB by +/- extension vector. */
void aabb_extend_by_vec3( aabb_t *aabb, vec3_t extension );

/*! Return 2 if point is inside, else 1 if point is on surface, else 0. */
int aabb_intersect_point( const aabb_t *aabb, const vec3_t point );
/*! Return 2 if aabb_src intersects, else 1 if aabb_src touches exactly, else 0. */
int aabb_intersect_aabb( const aabb_t *aabb, const aabb_t *aabb_src );
/*! Return 2 if aabb is behind plane, else 1 if aabb intersects plane, else 0. */
int aabb_intersect_plane( const aabb_t *aabb, const float *plane );
/*! Return 1 if aabb intersects ray, else 0... dist = closest intersection. */
int aabb_intersect_ray( const aabb_t *aabb, const ray_t *ray, vec_t *dist );
/*! Return 1 if aabb intersects ray, else 0. Faster, but does not provide point of intersection */
int aabb_test_ray( const aabb_t* aabb, const ray_t* ray );

/*! Generate AABB from oriented bounding box. */
void aabb_for_bbox( aabb_t *aabb, const bbox_t *bbox );
/*! Generate AABB from 2-dimensions of min/max, specified by axis. */
void aabb_for_area( aabb_t *aabb, vec3_t area_tl, vec3_t area_br, int axis );
/*! Generate AABB to contain src * transform. NOTE: transform must be orthogonal */
void aabb_for_transformed_aabb( aabb_t* dst, const aabb_t* src, const m4x4_t transform );


/*! Generate oriented bounding box from AABB and transformation matrix. */
/*!\todo Remove need to specify euler/scale. */
void bbox_for_oriented_aabb( bbox_t *bbox, const aabb_t *aabb,
							 const m4x4_t matrix, const vec3_t euler, const vec3_t scale );
/*! Return 2 is bbox is behind plane, else return 1 if bbox intersects plane, else return 0. */
int bbox_intersect_plane( const bbox_t *bbox, const vec_t* plane );


/*! Generate a ray from an origin point and a direction unit-vector */
void ray_construct_for_vec3( ray_t *ray, const vec3_t origin, const vec3_t direction );

/*! Transform a ray */
void ray_transform( ray_t *ray, const m4x4_t matrix );

/*! return true if point intersects cone formed by ray, divergence and epsilon */
vec_t ray_intersect_point( const ray_t *ray, const vec3_t point, vec_t epsilon, vec_t divergence );
/*! return true if triangle intersects ray... dist = dist from intersection point to ray-origin */
vec_t ray_intersect_triangle( const ray_t *ray, qboolean bCullBack, const vec3_t vert0, const vec3_t vert1, const vec3_t vert2 );


////////////////////////////////////////////////////////////////////////////////
// Below is double-precision math stuff.  This was initially needed by the new
// "base winding" code in q3map2 brush processing in order to fix the famous
// "disappearing triangles" issue.  These definitions can be used wherever extra
// precision is needed.
////////////////////////////////////////////////////////////////////////////////

typedef double vec_accu_t;
typedef vec_accu_t vec3_accu_t[3];

// Smallest positive value for vec_accu_t such that 1.0 + VEC_ACCU_SMALLEST_EPSILON_AROUND_ONE != 1.0.
// In the case of 64 bit doubles (which is almost certainly the case), it's 0.00000000000000022204.
// Don't forget that your epsilons should depend on the possible range of values,
// because for example adding VEC_ACCU_SMALLEST_EPSILON_AROUND_ONE to 1024.0 will have no effect.
#define VEC_ACCU_SMALLEST_EPSILON_AROUND_ONE DBL_EPSILON

vec_accu_t VectorLengthAccu( const vec3_accu_t v );

// I have a feeling it may be safer to break these #define functions out into actual functions
// in order to avoid accidental loss of precision.  For example, say you call
// VectorScaleAccu(vec3_t, vec_t, vec3_accu_t).  The scale would take place in 32 bit land
// and the result would be cast to 64 bit, which would cause total loss of precision when
// scaling by a large factor.
//#define DotProductAccu(x, y) ((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])
//#define VectorSubtractAccu(a, b, c) ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
//#define VectorAddAccu(a, b, c) ((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
//#define VectorCopyAccu(a, b) ((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2])
//#define VectorScaleAccu(a, b, c) ((c)[0] = (b) * (a)[0], (c)[1] = (b) * (a)[1], (c)[2] = (b) * (a)[2])
//#define CrossProductAccu(a, b, c) ((c)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1], (c)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2], (c)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0])
//#define Q_rintAccu(in) ((vec_accu_t) floor(in + 0.5))

vec_accu_t DotProductAccu( const vec3_accu_t a, const vec3_accu_t b );
void VectorSubtractAccu( const vec3_accu_t a, const vec3_accu_t b, vec3_accu_t out );
void VectorAddAccu( const vec3_accu_t a, const vec3_accu_t b, vec3_accu_t out );
void VectorCopyAccu( const vec3_accu_t in, vec3_accu_t out );
void VectorScaleAccu( const vec3_accu_t in, vec_accu_t scaleFactor, vec3_accu_t out );
void CrossProductAccu( const vec3_accu_t a, const vec3_accu_t b, vec3_accu_t out );
vec_accu_t Q_rintAccu( vec_accu_t val );

void VectorCopyAccuToRegular( const vec3_accu_t in, vec3_t out );
void VectorCopyRegularToAccu( const vec3_t in, vec3_accu_t out );
vec_accu_t VectorNormalizeAccu( const vec3_accu_t in, vec3_accu_t out );

#ifdef __cplusplus
}
#endif

#endif /* __MATHLIB__ */
