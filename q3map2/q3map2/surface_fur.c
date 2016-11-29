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
#define SURFACE_FUR_C



/* dependencies */
#include "q3map2.h"




/* -------------------------------------------------------------------------------

   ydnar: fur module

   ------------------------------------------------------------------------------- */

/*
   Fur()
   runs the fur processing algorithm on a map drawsurface
 */

void Fur( mapDrawSurface_t *ds ){
	int i, j, k, numLayers;
	float offset, fade, a;
	mapDrawSurface_t    *fur;
	bspDrawVert_t       *dv;


	/* dummy check */
	if ( ds == NULL || ds->fur || ds->shaderInfo->furNumLayers < 1 ) {
		return;
	}

	/* get basic info */
	numLayers = ds->shaderInfo->furNumLayers;
	offset = ds->shaderInfo->furOffset;
	fade = ds->shaderInfo->furFade * 255.0f;

	/* debug code */
	//%	Sys_FPrintf( SYS_VRB, "Fur():  layers: %d  offset: %f   fade: %f  %s\n",
	//%		numLayers, offset, fade, ds->shaderInfo->shader );

	/* initial offset */
	for ( j = 0; j < ds->numVerts; j++ )
	{
		/* get surface vert */
		dv = &ds->verts[ j ];

		/* offset is scaled by original vertex alpha */
		a = (float) dv->color[ 0 ][ 3 ] / 255.0;

		/* offset it */
		VectorMA( dv->xyz, ( offset * a ), dv->normal, dv->xyz );
	}

	/* wash, rinse, repeat */
	for ( i = 1; i < numLayers; i++ )
	{
		/* clone the surface */
		fur = CloneSurface( ds, ds->shaderInfo );
		if ( fur == NULL ) {
			return;
		}

		/* set it to fur */
		fur->fur = qtrue;

		/* walk the verts */
		for ( j = 0; j < fur->numVerts; j++ )
		{
			/* get surface vert */
			dv = &ds->verts[ j ];

			/* offset is scaled by original vertex alpha */
			a = (float) dv->color[ 0 ][ 3 ] / 255.0;

			/* get fur vert */
			dv = &fur->verts[ j ];

			/* offset it */
			VectorMA( dv->xyz, ( offset * a * i ), dv->normal, dv->xyz );

			/* fade alpha */
			for ( k = 0; k < MAX_LIGHTMAPS; k++ )
			{
				a = (float) dv->color[ k ][ 3 ] - fade;
				if ( a > 255.0f ) {
					dv->color[ k ][ 3 ] = 255;
				}
				else if ( a < 0 ) {
					dv->color[ k ][ 3 ] = 0;
				}
				else{
					dv->color[ k ][ 3 ] = a;
				}
			}
		}
	}
}
