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

#define LIGHT_SHADOWS_C

#include "light.h"
#include "inout.h"



/* -------------------------------------------------------------------------------

   ydnar: this code deals with shadow volume bsps

   ------------------------------------------------------------------------------- */

typedef struct shadowNode_s
{
	vec4_t plane;
	int children[ 2 ];
}
shadowNode_t;

int numShadowNodes;
shadowNode_t    *shadowNodes;



/*
   AddShadow()
   adds a shadow, returning the index into the shadow list
 */



/*
   MakeShadowFromPoints()
   creates a shadow volume from 4 points (the first being the light origin)
 */



/*
   SetupShadows()
   sets up the shadow volumes for all lights in the world
 */

void SetupShadows( void ){
	int i, j, s;
	light_t         *light;
	dleaf_t         *leaf;
	dsurface_t      *ds;
	surfaceInfo_t   *info;
	shaderInfo_t    *si;
	byte            *tested;


	/* early out for weird cases where there are no lights */
	if ( lights == NULL ) {
		return;
	}

	/* note it */
	Sys_FPrintf( SYS_VRB, "--- SetupShadows ---\n" );

	/* allocate a surface test list */
	tested = safe_malloc( numDrawSurfaces / 8 + 1 );

	/* walk the list of lights */
	for ( light = lights; light != NULL; light = light->next )
	{
		/* do some early out testing */
		if ( light->cluster < 0 ) {
			continue;
		}

		/* clear surfacetest list */
		memset( tested, 0, numDrawSurfaces / 8 + 1 );

		/* walk the bsp leaves */
		for ( i = 0, leaf = dleafs; i < numleafs; i++, leaf++ )
		{
			/* in pvs? */
			if ( ClusterVisible( light->cluster, leaf->cluster ) == qfalse ) {
				continue;
			}

			/* walk the surface list for this leaf */
			for ( j = 0; j < leaf->numLeafSurfaces; j++ )
			{
				/* don't filter a surface more than once */
				s = dleafsurfaces[ leaf->firstLeafSurface + j ];
				if ( tested[ s >> 3 ] & ( 1 << ( s & 7 ) ) ) {
					continue;
				}
				tested[ s >> 3 ] |= ( 1 << ( s & 7 ) );

				/* get surface and info */
				ds = &drawSurfaces[ s ];
				info = &surfaceInfos[ s ];
				si = info->si;

				/* don't create shadow volumes from translucent surfaces */
				if ( si->contents & CONTENTS_TRANSLUCENT ) {
					continue;
				}
			}
		}
	}
}
