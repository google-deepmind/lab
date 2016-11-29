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
#define BRUSH_PRIMIT_C



/* dependencies */
#include "q3map2.h"



/* -------------------------------------------------------------------------------

   functions

   ------------------------------------------------------------------------------- */

/*
   ComputeAxisBase()
   computes the base texture axis for brush primitive texturing
   note: ComputeAxisBase here and in editor code must always BE THE SAME!
   warning: special case behaviour of atan2( y, x ) <-> atan( y / x ) might not be the same everywhere when x == 0
   rotation by (0,RotY,RotZ) assigns X to normal
 */

void ComputeAxisBase( vec3_t normal, vec3_t texX, vec3_t texY ){
	vec_t RotY, RotZ;


	/* do some cleaning */
	if ( fabs( normal[ 0 ] ) < 1e-6 ) {
		normal[ 0 ] = 0.0f;
	}
	if ( fabs( normal[ 1 ] ) < 1e-6 ) {
		normal[ 1 ] = 0.0f;
	}
	if ( fabs( normal[ 2 ] ) < 1e-6 ) {
		normal[ 2 ] = 0.0f;
	}

	/* compute the two rotations around y and z to rotate x to normal */
	RotY = -atan2( normal[ 2 ], sqrt( normal[ 1 ] * normal[ 1 ] + normal[ 0 ] * normal[ 0 ] ) );
	RotZ = atan2( normal[ 1 ], normal[ 0 ] );

	/* rotate (0,1,0) and (0,0,1) to compute texX and texY */
	texX[ 0 ] = -sin( RotZ );
	texX[ 1 ] = cos( RotZ );
	texX[ 2 ] = 0;

	/* the texY vector is along -z (t texture coorinates axis) */
	texY[ 0 ] = -sin( RotY ) * cos( RotZ );
	texY[ 1 ] = -sin( RotY ) * sin( RotZ );
	texY[ 2 ] = -cos( RotY );
}
