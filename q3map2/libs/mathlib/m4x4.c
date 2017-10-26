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
#include "memory.h"

void m4x4_identity( m4x4_t matrix ){
	matrix[1] = matrix[2] = matrix[3] =
								matrix[4] = matrix[6] = matrix[7] =
															matrix[8] = matrix[9] = matrix[11] =
																						matrix[12] = matrix[13] = matrix[14] = 0;

	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1;
}

void m4x4_translation_for_vec3( m4x4_t matrix, const vec3_t translation ){
	matrix[1] = matrix[2] = matrix[3] =
								matrix[4] = matrix[6] = matrix[7] =
															matrix[8] = matrix[9] = matrix[11] = 0;

	matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1;

	matrix[12] = translation[0];
	matrix[13] = translation[1];
	matrix[14] = translation[2];
}

void m4x4_rotation_for_vec3( m4x4_t matrix, const vec3_t euler, eulerOrder_t order ){
	double cx, sx, cy, sy, cz, sz;

	cx = cos( DEG2RAD( euler[0] ) );
	sx = sin( DEG2RAD( euler[0] ) );
	cy = cos( DEG2RAD( euler[1] ) );
	sy = sin( DEG2RAD( euler[1] ) );
	cz = cos( DEG2RAD( euler[2] ) );
	sz = sin( DEG2RAD( euler[2] ) );

	switch ( order )
	{
	case eXYZ:

#if 1

		{
			matrix[0]  = (vec_t)( cy * cz );
			matrix[1]  = (vec_t)( cy * sz );
			matrix[2]  = (vec_t)-sy;
			matrix[4]  = (vec_t)( sx * sy * cz + cx * -sz );
			matrix[5]  = (vec_t)( sx * sy * sz + cx * cz );
			matrix[6]  = (vec_t)( sx * cy );
			matrix[8]  = (vec_t)( cx * sy * cz + sx * sz );
			matrix[9]  = (vec_t)( cx * sy * sz + -sx * cz );
			matrix[10] = (vec_t)( cx * cy );
		}

		matrix[12]  =  matrix[13] = matrix[14] = matrix[3] = matrix[7] = matrix[11] = 0;
		matrix[15] =  1;

#else

		m4x4_identity( matrix );
		matrix[5] = (vec_t) cx; matrix[6] = (vec_t) sx;
		matrix[9] = (vec_t)-sx; matrix[10] = (vec_t) cx;

		{
			m4x4_t temp;
			m4x4_identity( temp );
			temp[0] = (vec_t) cy; temp[2] = (vec_t)-sy;
			temp[8] = (vec_t) sy; temp[10] = (vec_t) cy;
			m4x4_premultiply_by_m4x4( matrix, temp );
			m4x4_identity( temp );
			temp[0] = (vec_t) cz; temp[1] = (vec_t) sz;
			temp[4] = (vec_t)-sz; temp[5] = (vec_t) cz;
			m4x4_premultiply_by_m4x4( matrix, temp );
		}
#endif

		break;

	case eYZX:
		m4x4_identity( matrix );
		matrix[0] = (vec_t) cy; matrix[2] = (vec_t)-sy;
		matrix[8] = (vec_t) sy; matrix[10] = (vec_t) cy;

		{
			m4x4_t temp;
			m4x4_identity( temp );
			temp[5] = (vec_t) cx; temp[6] = (vec_t) sx;
			temp[9] = (vec_t)-sx; temp[10] = (vec_t) cx;
			m4x4_premultiply_by_m4x4( matrix, temp );
			m4x4_identity( temp );
			temp[0] = (vec_t) cz; temp[1] = (vec_t) sz;
			temp[4] = (vec_t)-sz; temp[5] = (vec_t) cz;
			m4x4_premultiply_by_m4x4( matrix, temp );
		}
		break;

	case eZXY:
		m4x4_identity( matrix );
		matrix[0] = (vec_t) cz; matrix[1] = (vec_t) sz;
		matrix[4] = (vec_t)-sz; matrix[5] = (vec_t) cz;

		{
			m4x4_t temp;
			m4x4_identity( temp );
			temp[5] = (vec_t) cx; temp[6] = (vec_t) sx;
			temp[9] = (vec_t)-sx; temp[10] = (vec_t) cx;
			m4x4_premultiply_by_m4x4( matrix, temp );
			m4x4_identity( temp );
			temp[0] = (vec_t) cy; temp[2] = (vec_t)-sy;
			temp[8] = (vec_t) sy; temp[10] = (vec_t) cy;
			m4x4_premultiply_by_m4x4( matrix, temp );
		}
		break;

	case eXZY:
		m4x4_identity( matrix );
		matrix[5] = (vec_t) cx; matrix[6] = (vec_t) sx;
		matrix[9] = (vec_t)-sx; matrix[10] = (vec_t) cx;

		{
			m4x4_t temp;
			m4x4_identity( temp );
			temp[0] = (vec_t) cz; temp[1] = (vec_t) sz;
			temp[4] = (vec_t)-sz; temp[5] = (vec_t) cz;
			m4x4_premultiply_by_m4x4( matrix, temp );
			m4x4_identity( temp );
			temp[0] = (vec_t) cy; temp[2] = (vec_t)-sy;
			temp[8] = (vec_t) sy; temp[10] = (vec_t) cy;
			m4x4_premultiply_by_m4x4( matrix, temp );
		}
		break;

	case eYXZ:

/* transposed
 |  cy.cz + sx.sy.-sz + -cx.sy.0   0.cz + cx.-sz + sx.0   sy.cz + -sx.cy.-sz + cx.cy.0 |
 |  cy.sz + sx.sy.cz + -cx.sy.0    0.sz + cx.cz + sx.0    sy.sz + -sx.cy.cz + cx.cy.0  |
 |  cy.0 + sx.sy.0 + -cx.sy.1      0.0 + cx.0 + sx.1      sy.0 + -sx.cy.0 + cx.cy.1    |
 */

#if 1

		{
			matrix[0]  = (vec_t)( cy * cz + sx * sy * -sz );
			matrix[1]  = (vec_t)( cy * sz + sx * sy * cz );
			matrix[2]  = (vec_t)( -cx * sy );
			matrix[4]  = (vec_t)( cx * -sz );
			matrix[5]  = (vec_t)( cx * cz );
			matrix[6]  = (vec_t)( sx );
			matrix[8]  = (vec_t)( sy * cz + -sx * cy * -sz );
			matrix[9]  = (vec_t)( sy * sz + -sx * cy * cz );
			matrix[10] = (vec_t)( cx * cy );
		}

		matrix[12]  =  matrix[13] = matrix[14] = matrix[3] = matrix[7] = matrix[11] = 0;
		matrix[15] =  1;

#else

		m4x4_identity( matrix );
		matrix[0] = (vec_t) cy; matrix[2] = (vec_t)-sy;
		matrix[8] = (vec_t) sy; matrix[10] = (vec_t) cy;

		{
			m4x4_t temp;
			m4x4_identity( temp );
			temp[5] = (vec_t) cx; temp[6] = (vec_t) sx;
			temp[9] = (vec_t)-sx; temp[10] = (vec_t) cx;
			m4x4_premultiply_by_m4x4( matrix, temp );
			m4x4_identity( temp );
			temp[0] = (vec_t) cz; temp[1] = (vec_t) sz;
			temp[4] = (vec_t)-sz; temp[5] = (vec_t) cz;
			m4x4_premultiply_by_m4x4( matrix, temp );
		}
#endif
		break;

	case eZYX:
#if 1

		{
			matrix[0]  = (vec_t)( cy * cz );
			matrix[4]  = (vec_t)( cy * -sz );
			matrix[8]  = (vec_t)sy;
			matrix[1]  = (vec_t)( sx * sy * cz + cx * sz );
			matrix[5]  = (vec_t)( sx * sy * -sz + cx * cz );
			matrix[9]  = (vec_t)( -sx * cy );
			matrix[2]  = (vec_t)( cx * -sy * cz + sx * sz );
			matrix[6]  = (vec_t)( cx * -sy * -sz + sx * cz );
			matrix[10] = (vec_t)( cx * cy );
		}

		matrix[12]  =  matrix[13] = matrix[14] = matrix[3] = matrix[7] = matrix[11] = 0;
		matrix[15] =  1;

#else

		m4x4_identity( matrix );
		matrix[0] = (vec_t) cz; matrix[1] = (vec_t) sz;
		matrix[4] = (vec_t)-sz; matrix[5] = (vec_t) cz;
		{
			m4x4_t temp;
			m4x4_identity( temp );
			temp[0] = (vec_t) cy; temp[2] = (vec_t)-sy;
			temp[8] = (vec_t) sy; temp[10] = (vec_t) cy;
			m4x4_premultiply_by_m4x4( matrix, temp );
			m4x4_identity( temp );
			temp[5] = (vec_t) cx; temp[6] = (vec_t) sx;
			temp[9] = (vec_t)-sx; temp[10] = (vec_t) cx;
			m4x4_premultiply_by_m4x4( matrix, temp );
		}

#endif
		break;

	}

}

void m4x4_scale_for_vec3( m4x4_t matrix, const vec3_t scale ){
	matrix[1] = matrix[2] = matrix[3] =
								matrix[4] = matrix[6] = matrix[7] =
															matrix[8] = matrix[9] = matrix[11] =
																						matrix[12] = matrix[13] = matrix[14] = 0;

	matrix[15] = 1;

	matrix[0] = scale[0];
	matrix[5] = scale[1];
	matrix[10] = scale[2];
}

void m4x4_rotation_for_quat( m4x4_t matrix, const vec4_t rotation ){
	float xx,xy,xz,xw,yy,yz,yw,zz,zw;

	xx      = rotation[0] * rotation[0];
	xy      = rotation[0] * rotation[1];
	xz      = rotation[0] * rotation[2];
	xw      = rotation[0] * rotation[3];

	yy      = rotation[1] * rotation[1];
	yz      = rotation[1] * rotation[2];
	yw      = rotation[1] * rotation[3];

	zz      = rotation[2] * rotation[2];
	zw      = rotation[2] * rotation[3];

	matrix[0]  = 1 - 2 * ( yy + zz );
	matrix[4]  =     2 * ( xy - zw );
	matrix[8]  =     2 * ( xz + yw );

	matrix[1]  =     2 * ( xy + zw );
	matrix[5]  = 1 - 2 * ( xx + zz );
	matrix[9]  =     2 * ( yz - xw );

	matrix[2]  =     2 * ( xz - yw );
	matrix[6]  =     2 * ( yz + xw );
	matrix[10] = 1 - 2 * ( xx + yy );

	matrix[3]  = matrix[7] = matrix[11] = matrix[12] = matrix[13] = matrix[14] = 0;
	matrix[15] = 1;
}

void m4x4_rotation_for_axisangle( m4x4_t matrix, const vec3_t axis, vec_t angle ){
	vec4_t rotation;
	angle *= 0.5;

	rotation[3] = (float)sin( (float)( angle ) );

	rotation[0]    = axis[0] * rotation[3];
	rotation[1]    = axis[1] * rotation[3];
	rotation[2]    = axis[2] * rotation[3];
	rotation[3]    = (float)cos( (float)( angle ) );

	m4x4_rotation_for_quat( matrix, rotation );
}

void m4x4_translate_by_vec3( m4x4_t matrix, const vec3_t translation ){
	m4x4_t temp;
	m4x4_translation_for_vec3( temp, translation );
	m4x4_multiply_by_m4x4( matrix, temp );
}

void m4x4_rotate_by_vec3( m4x4_t matrix, const vec3_t euler, eulerOrder_t order ){
	m4x4_t temp;
	m4x4_rotation_for_vec3( temp, euler, order );
	m4x4_multiply_by_m4x4( matrix, temp );
}

void m4x4_scale_by_vec3( m4x4_t matrix, const vec3_t scale ){
	m4x4_t temp;
	m4x4_scale_for_vec3( temp, scale );
	m4x4_multiply_by_m4x4( matrix, temp );
}

void m4x4_rotate_by_quat( m4x4_t matrix, const vec4_t rotation ){
	m4x4_t temp;
	m4x4_rotation_for_quat( temp, rotation );
	m4x4_multiply_by_m4x4( matrix, temp );
}

void m4x4_rotate_by_axisangle( m4x4_t matrix, const vec3_t axis, vec_t angle ){
	m4x4_t temp;
	m4x4_rotation_for_axisangle( temp, axis, angle );
	m4x4_multiply_by_m4x4( matrix, temp );
}

void m4x4_transform_by_vec3( m4x4_t matrix, const vec3_t translation, const vec3_t euler, eulerOrder_t order, const vec3_t scale ){
	m4x4_translate_by_vec3( matrix, translation );
	m4x4_rotate_by_vec3( matrix, euler, order );
	m4x4_scale_by_vec3( matrix, scale );
}

void m4x4_pivoted_rotate_by_vec3( m4x4_t matrix, const vec3_t euler, eulerOrder_t order, const vec3_t pivotpoint ){
	vec3_t vec3_temp;
	VectorNegative( pivotpoint, vec3_temp );

	m4x4_translate_by_vec3( matrix, pivotpoint );
	m4x4_rotate_by_vec3( matrix, euler, order );
	m4x4_translate_by_vec3( matrix, vec3_temp );
}

void m4x4_pivoted_scale_by_vec3( m4x4_t matrix, const vec3_t scale, const vec3_t pivotpoint ){
	vec3_t vec3_temp;
	VectorNegative( pivotpoint, vec3_temp );

	m4x4_translate_by_vec3( matrix, pivotpoint );
	m4x4_scale_by_vec3( matrix, scale );
	m4x4_translate_by_vec3( matrix, vec3_temp );
}

void m4x4_pivoted_transform_by_vec3( m4x4_t matrix, const vec3_t translation, const vec3_t euler, eulerOrder_t order, const vec3_t scale, const vec3_t pivotpoint ){
	vec3_t vec3_temp;

	VectorAdd( pivotpoint, translation, vec3_temp );
	m4x4_translate_by_vec3( matrix, vec3_temp );
	m4x4_rotate_by_vec3( matrix, euler, order );
	m4x4_scale_by_vec3( matrix, scale );
	VectorNegative( pivotpoint, vec3_temp );
	m4x4_translate_by_vec3( matrix, vec3_temp );
}

void m4x4_pivoted_rotate_by_quat( m4x4_t matrix, const vec4_t rotation, const vec3_t pivotpoint ){
	vec3_t vec3_temp;
	VectorNegative( pivotpoint, vec3_temp );

	m4x4_translate_by_vec3( matrix, pivotpoint );
	m4x4_rotate_by_quat( matrix, rotation );
	m4x4_translate_by_vec3( matrix, vec3_temp );
}

void m4x4_pivoted_rotate_by_axisangle( m4x4_t matrix, const vec3_t axis, vec_t angle, const vec3_t pivotpoint ){
	vec3_t vec3_temp;
	VectorNegative( pivotpoint, vec3_temp );

	m4x4_translate_by_vec3( matrix, pivotpoint );
	m4x4_rotate_by_axisangle( matrix, axis, angle );
	m4x4_translate_by_vec3( matrix, vec3_temp );
}


/*
   A = A.B

   A0 = B0 * A0 + B1 * A4 + B2 * A8 + B3 * A12
   A4 = B4 * A0 + B5 * A4 + B6 * A8 + B7 * A12
   A8 = B8 * A0 + B9 * A4 + B10* A8 + B11* A12
   A12= B12* A0 + B13* A4 + B14* A8 + B15* A12

   A1 = B0 * A1 + B1 * A5 + B2 * A9 + B3 * A13
   A5 = B4 * A1 + B5 * A5 + B6 * A9 + B7 * A13
   A9 = B8 * A1 + B9 * A5 + B10* A9 + B11* A13
   A13= B12* A1 + B13* A5 + B14* A9 + B15* A13

   A2 = B0 * A2 + B1 * A6 + B2 * A10+ B3 * A14
   A6 = B4 * A2 + B5 * A6 + B6 * A10+ B7 * A14
   A10= B8 * A2 + B9 * A6 + B10* A10+ B11* A14
   A14= B12* A2 + B13* A6 + B14* A10+ B15* A14

   A3 = B0 * A3 + B1 * A7 + B2 * A11+ B3 * A15
   A7 = B4 * A3 + B5 * A7 + B6 * A11+ B7 * A15
   A11= B8 * A3 + B9 * A7 + B10* A11+ B11* A15
   A15= B12* A3 + B13* A7 + B14* A11+ B15* A15
 */

void m4x4_multiply_by_m4x4( m4x4_t dst, const m4x4_t src ){
	vec_t dst0, dst1, dst2, dst3;

#if 1

	dst0 = src[0] * dst[0] + src[1] * dst[4] + src[2] * dst[8] + src[3] * dst[12];
	dst1 = src[4] * dst[0] + src[5] * dst[4] + src[6] * dst[8] + src[7] * dst[12];
	dst2 = src[8] * dst[0] + src[9] * dst[4] + src[10] * dst[8] + src[11] * dst[12];
	dst3 = src[12] * dst[0] + src[13] * dst[4] + src[14] * dst[8] + src[15] * dst[12];
	dst[0] = dst0; dst[4] = dst1; dst[8] = dst2; dst[12] = dst3;

	dst0 = src[0] * dst[1] + src[1] * dst[5] + src[2] * dst[9] + src[3] * dst[13];
	dst1 = src[4] * dst[1] + src[5] * dst[5] + src[6] * dst[9] + src[7] * dst[13];
	dst2 = src[8] * dst[1] + src[9] * dst[5] + src[10] * dst[9] + src[11] * dst[13];
	dst3 = src[12] * dst[1] + src[13] * dst[5] + src[14] * dst[9] + src[15] * dst[13];
	dst[1] = dst0; dst[5] = dst1; dst[9] = dst2; dst[13] = dst3;

	dst0 = src[0] * dst[2] + src[1] * dst[6] + src[2] * dst[10] + src[3] * dst[14];
	dst1 = src[4] * dst[2] + src[5] * dst[6] + src[6] * dst[10] + src[7] * dst[14];
	dst2 = src[8] * dst[2] + src[9] * dst[6] + src[10] * dst[10] + src[11] * dst[14];
	dst3 = src[12] * dst[2] + src[13] * dst[6] + src[14] * dst[10] + src[15] * dst[14];
	dst[2] = dst0; dst[6] = dst1; dst[10] = dst2; dst[14] = dst3;

	dst0 = src[0] * dst[3] + src[1] * dst[7] + src[2] * dst[11] + src[3] * dst[15];
	dst1 = src[4] * dst[3] + src[5] * dst[7] + src[6] * dst[11] + src[7] * dst[15];
	dst2 = src[8] * dst[3] + src[9] * dst[7] + src[10] * dst[11] + src[11] * dst[15];
	dst3 = src[12] * dst[3] + src[13] * dst[7] + src[14] * dst[11] + src[15] * dst[15];
	dst[3] = dst0; dst[7] = dst1; dst[11] = dst2; dst[15] = dst3;

#else

	vec_t * p = dst;
	for ( int i = 0; i < 4; i++ )
	{
		dst1 =  src[0]  * p[0];
		dst1 += src[1]  * p[4];
		dst1 += src[2]  * p[8];
		dst1 += src[3]  * p[12];
		dst2 =  src[4]  * p[0];
		dst2 += src[5]  * p[4];
		dst2 += src[6]  * p[8];
		dst2 += src[7]  * p[12];
		dst3 =  src[8]  * p[0];
		dst3 += src[9]  * p[4];
		dst3 += src[10] * p[8];
		dst3 += src[11] * p[12];
		dst4 =  src[12] * p[0];
		dst4 += src[13] * p[4];
		dst4 += src[14] * p[8];
		dst4 += src[15] * p[12];

		p[0] = dst1;
		p[4] = dst2;
		p[8] = dst3;
		p[12] = dst4;
		p++;
	}

#endif
}

/*
   A = B.A

   A0 = A0 * B0 + A1 * B4 + A2 * B8 + A3 * B12
   A1 = A0 * B1 + A1 * B5 + A2 * B9 + A3 * B13
   A2 = A0 * B2 + A1 * B6 + A2 * B10+ A3 * B14
   A3 = A0 * B3 + A1 * B7 + A2 * B11+ A3 * B15

   A4 = A4 * B0 + A5 * B4 + A6 * B8 + A7 * B12
   A5 = A4 * B1 + A5 * B5 + A6 * B9 + A7 * B13
   A6 = A4 * B2 + A5 * B6 + A6 * B10+ A7 * B14
   A7 = A4 * B3 + A5 * B7 + A6 * B11+ A7 * B15

   A8 = A8 * B0 + A9 * B4 + A10* B8 + A11* B12
   A9 = A8 * B1 + A9 * B5 + A10* B9 + A11* B13
   A10= A8 * B2 + A9 * B6 + A10* B10+ A11* B14
   A11= A8 * B3 + A9 * B7 + A10* B11+ A11* B15

   A12= A12* B0 + A13* B4 + A14* B8 + A15* B12
   A13= A12* B1 + A13* B5 + A14* B9 + A15* B13
   A14= A12* B2 + A13* B6 + A14* B10+ A15* B14
   A15= A12* B3 + A13* B7 + A14* B11+ A15* B15
 */

void m4x4_premultiply_by_m4x4( m4x4_t dst, const m4x4_t src ){
	vec_t dst0, dst1, dst2, dst3;

#if 1

	dst0 = dst[0] * src[0] + dst[1] * src[4] + dst[2] * src[8] + dst[3] * src[12];
	dst1 = dst[0] * src[1] + dst[1] * src[5] + dst[2] * src[9] + dst[3] * src[13];
	dst2 = dst[0] * src[2] + dst[1] * src[6] + dst[2] * src[10] + dst[3] * src[14];
	dst3 = dst[0] * src[3] + dst[1] * src[7] + dst[2] * src[11] + dst[3] * src[15];
	dst[0] = dst0; dst[1] = dst1; dst[2] = dst2; dst[3] = dst3;

	dst0 = dst[4] * src[0] + dst[5] * src[4] + dst[6] * src[8] + dst[7] * src[12];
	dst1 = dst[4] * src[1] + dst[5] * src[5] + dst[6] * src[9] + dst[7] * src[13];
	dst2 = dst[4] * src[2] + dst[5] * src[6] + dst[6] * src[10] + dst[7] * src[14];
	dst3 = dst[4] * src[3] + dst[5] * src[7] + dst[6] * src[11] + dst[7] * src[15];
	dst[4] = dst0; dst[5] = dst1; dst[6] = dst2; dst[7] = dst3;

	dst0 = dst[8] * src[0] + dst[9] * src[4] + dst[10] * src[8] + dst[11] * src[12];
	dst1 = dst[8] * src[1] + dst[9] * src[5] + dst[10] * src[9] + dst[11] * src[13];
	dst2 = dst[8] * src[2] + dst[9] * src[6] + dst[10] * src[10] + dst[11] * src[14];
	dst3 = dst[8] * src[3] + dst[9] * src[7] + dst[10] * src[11] + dst[11] * src[15];
	dst[8] = dst0; dst[9] = dst1; dst[10] = dst2; dst[11] = dst3;

	dst0 = dst[12] * src[0] + dst[13] * src[4] + dst[14] * src[8] + dst[15] * src[12];
	dst1 = dst[12] * src[1] + dst[13] * src[5] + dst[14] * src[9] + dst[15] * src[13];
	dst2 = dst[12] * src[2] + dst[13] * src[6] + dst[14] * src[10] + dst[15] * src[14];
	dst3 = dst[12] * src[3] + dst[13] * src[7] + dst[14] * src[11] + dst[15] * src[15];
	dst[12] = dst0; dst[13] = dst1; dst[14] = dst2; dst[15] = dst3;

#else

	vec_t* p = dst;
	for ( int i = 0; i < 4; i++ )
	{
		dst1 =  src[0]  * p[0];
		dst2 =  src[1]  * p[0];
		dst3 =  src[2]  * p[0];
		dst4 =  src[3]  * p[0];
		dst1 += src[4]  * p[1];
		dst2 += src[5]  * p[1];
		dst3 += src[6]  * p[1];
		dst4 += src[7]  * p[1];
		dst1 += src[8]  * p[2];
		dst2 += src[9]  * p[2];
		dst4 += src[11] * p[2];
		dst3 += src[10] * p[2];
		dst1 += src[12] * p[3];
		dst2 += src[13] * p[3];
		dst3 += src[14] * p[3];
		dst4 += src[15] * p[3];

		*p++ = dst1;
		*p++ = dst2;
		*p++ = dst3;
		*p++ = dst4;
	}

#endif
}

void m4x4_transform_point( const m4x4_t matrix, vec3_t point ){
	float out1, out2, out3;

	out1 =  matrix[0]  * point[0];
	out2 =  matrix[1]  * point[0];
	out3 =  matrix[2]  * point[0];
	out1 += matrix[4]  * point[1];
	out2 += matrix[5]  * point[1];
	out3 += matrix[6]  * point[1];
	out1 += matrix[8]  * point[2];
	out2 += matrix[9]  * point[2];
	out3 += matrix[10] * point[2];
	out1 += matrix[12];
	out2 += matrix[13];
	out3 += matrix[14];

	point[0] = out1;
	point[1] = out2;
	point[2] = out3;
}

void m4x4_transform_normal( const m4x4_t matrix, vec3_t normal ){
	float out1, out2, out3;

	out1 =  matrix[0]  * normal[0];
	out2 =  matrix[1]  * normal[0];
	out3 =  matrix[2]  * normal[0];
	out1 += matrix[4]  * normal[1];
	out2 += matrix[5]  * normal[1];
	out3 += matrix[6]  * normal[1];
	out1 += matrix[8]  * normal[2];
	out2 += matrix[9]  * normal[2];
	out3 += matrix[10] * normal[2];

	normal[0] = out1;
	normal[1] = out2;
	normal[2] = out3;
}

void m4x4_transform_vec4( const m4x4_t matrix, vec4_t vector ){
	float out1, out2, out3, out4;

	out1 =  matrix[0]  * vector[0];
	out2 =  matrix[1]  * vector[0];
	out3 =  matrix[2]  * vector[0];
	out4 =  matrix[3]  * vector[0];
	out1 += matrix[4]  * vector[1];
	out2 += matrix[5]  * vector[1];
	out3 += matrix[6]  * vector[1];
	out4 += matrix[7]  * vector[1];
	out1 += matrix[8]  * vector[2];
	out2 += matrix[9]  * vector[2];
	out3 += matrix[10] * vector[2];
	out4 += matrix[11] * vector[2];
	out1 += matrix[12] * vector[3];
	out2 += matrix[13] * vector[3];
	out3 += matrix[14] * vector[3];
	out4 += matrix[15] * vector[3];

	vector[0] = out1;
	vector[1] = out2;
	vector[2] = out3;
	vector[3] = out4;
}

void m4x4_transpose( m4x4_t matrix ){
	int i, j;
	float temp, *p1, *p2;

	for ( i = 1; i < 4; i++ ) {
		for ( j = 0; j < i; j++ ) {
			p1 = matrix + ( j * 4 + i );
			p2 = matrix + ( i * 4 + j );
			temp = *p1;
			*p1 = *p2;
			*p2 = temp;
		}
	}
}

void m4x4_orthogonal_invert( m4x4_t matrix ){
	float temp;

	temp = -matrix[3];
	matrix[3] = matrix[12];
	matrix[12] = temp;

	temp = -matrix[7];
	matrix[7] = matrix[13];
	matrix[13] = temp;

	temp = -matrix[11];
	matrix[11] = matrix[14];
	matrix[14] = temp;

	/*
	   temp = matrix[1];
	   matrix[1] = matrix[4];
	   matrix[4] = temp;

	   temp = matrix[2];
	   matrix[2] = matrix[8];
	   matrix[8] = temp;

	   temp = matrix[6];
	   matrix[6] = matrix[9];
	   matrix[9] = temp;

	   matrix[3] = -matrix[3];
	   matrix[7] = -matrix[7];
	   matrix[11] = -matrix[11];
	 */
}

float m3_det( m3x3_t mat ){
	float det;

	det = mat[0] * ( mat[4] * mat[8] - mat[7] * mat[5] )
		  - mat[1] * ( mat[3] * mat[8] - mat[6] * mat[5] )
		  + mat[2] * ( mat[3] * mat[7] - mat[6] * mat[4] );

	return( det );
}

/*
   void m3_inverse( m3x3_t mr, m3x3_t ma )
   {
   float det = m3_det( ma );

   if ( fabs( det ) < 0.0005 )
   {
    m3_identity( ma );
    return;
   }

   mr[0] =    ma[4]*ma[8] - ma[5]*ma[7]   / det;
   mr[1] = -( ma[1]*ma[8] - ma[7]*ma[2] ) / det;
   mr[2] =    ma[1]*ma[5] - ma[4]*ma[2]   / det;

   mr[3] = -( ma[3]*ma[8] - ma[5]*ma[6] ) / det;
   mr[4] =    ma[0]*ma[8] - ma[6]*ma[2]   / det;
   mr[5] = -( ma[0]*ma[5] - ma[3]*ma[2] ) / det;

   mr[6] =    ma[3]*ma[7] - ma[6]*ma[4]   / det;
   mr[7] = -( ma[0]*ma[7] - ma[6]*ma[1] ) / det;
   mr[8] =    ma[0]*ma[4] - ma[1]*ma[3]   / det;
   }
 */

void m4_submat( m4x4_t mr, m3x3_t mb, int i, int j ){
	int ti, tj, idst, jdst;

	idst = 0;
	for ( ti = 0; ti < 4; ti++ )
	{
		if ( ti == i ) {
			continue;
		}
		if ( ti < i ) {
			idst = ti;
		}
		else
		{
			idst = ti - 1;
		}

		for ( tj = 0; tj < 4; tj++ )
		{
			if ( tj == j ) {
				continue;
			}
			if ( tj < j ) {
				jdst = tj;
			}
			else
			{
				jdst = tj - 1;
			}

			mb[idst * 3 + jdst] = mr[ti * 4 + tj ];
		}
	}
}

float m4_det( m4x4_t mr ){
	float det, result = 0, i = 1;
	m3x3_t msub3;
	int n;

	for ( n = 0; n < 4; n++, i *= -1 )
	{
		m4_submat( mr, msub3, 0, n );

		det     = m3_det( msub3 );
		result += mr[n] * det * i;
	}

	return result;
}

int m4x4_invert( m4x4_t matrix ){
	float mdet = m4_det( matrix );
	m3x3_t mtemp;
	int i, j, sign;
	m4x4_t m4x4_temp;

	if ( fabs( mdet ) < 0.0000000001 ) { //% 0.0005
		return 1;
	}

	memcpy( m4x4_temp, matrix, sizeof( m4x4_t ) );

	for ( i = 0; i < 4; i++ )
		for ( j = 0; j < 4; j++ )
		{
			sign = 1 - ( ( i + j ) % 2 ) * 2;

			m4_submat( m4x4_temp, mtemp, i, j );

			matrix[i + j * 4] = ( m3_det( mtemp ) * sign ) / mdet;
		}

	return 0;
}
