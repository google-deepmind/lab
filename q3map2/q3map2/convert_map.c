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
#define CONVERT_MAP_C



/* dependencies */
#include "q3map2.h"



/*
   ConvertBrush()
   exports a map brush
 */

#define SNAP_FLOAT_TO_INT   4
#define SNAP_INT_TO_FLOAT   ( 1.0 / SNAP_FLOAT_TO_INT )

static void ConvertBrush( FILE *f, int num, bspBrush_t *brush, vec3_t origin ){
	int i, j;
	bspBrushSide_t  *side;
	side_t          *buildSide;
	bspShader_t     *shader;
	char            *texture;
	bspPlane_t      *plane;
	vec3_t pts[ 3 ];


	/* start brush */
	fprintf( f, "\t// brush %d\n", num );
	fprintf( f, "\t{\n" );

	/* clear out build brush */
	for ( i = 0; i < buildBrush->numsides; i++ )
	{
		buildSide = &buildBrush->sides[ i ];
		if ( buildSide->winding != NULL ) {
			FreeWinding( buildSide->winding );
			buildSide->winding = NULL;
		}
	}
	buildBrush->numsides = 0;

	/* iterate through bsp brush sides */
	for ( i = 0; i < brush->numSides; i++ )
	{
		/* get side */
		side = &bspBrushSides[ brush->firstSide + i ];

		/* get shader */
		if ( side->shaderNum < 0 || side->shaderNum >= numBSPShaders ) {
			continue;
		}
		shader = &bspShaders[ side->shaderNum ];
		if ( !Q_stricmp( shader->shader, "default" ) || !Q_stricmp( shader->shader, "noshader" ) ) {
			continue;
		}

		/* get plane */
		plane = &bspPlanes[ side->planeNum ];

		/* add build side */
		buildSide = &buildBrush->sides[ buildBrush->numsides ];
		buildBrush->numsides++;

		/* tag it */
		buildSide->shaderInfo = ShaderInfoForShader( shader->shader );
		buildSide->planenum = side->planeNum;
		buildSide->winding = NULL;
	}

	/* make brush windings */
	if ( !CreateBrushWindings( buildBrush ) ) {
		return;
	}

	/* iterate through build brush sides */
	for ( i = 0; i < buildBrush->numsides; i++ )
	{
		/* get build side */
		buildSide = &buildBrush->sides[ i ];

		/* dummy check */
		if ( buildSide->shaderInfo == NULL || buildSide->winding == NULL ) {
			continue;
		}

		/* get texture name */
		if ( !Q_strncasecmp( buildSide->shaderInfo->shader, "textures/", 9 ) ) {
			texture = buildSide->shaderInfo->shader + 9;
		}
		else{
			texture = buildSide->shaderInfo->shader;
		}

		/* get plane points and offset by origin */
		for ( j = 0; j < 3; j++ )
		{
			VectorAdd( buildSide->winding->p[ j ], origin, pts[ j ] );
			//%	pts[ j ][ 0 ] = SNAP_INT_TO_FLOAT * floor( pts[ j ][ 0 ] * SNAP_FLOAT_TO_INT + 0.5f );
			//%	pts[ j ][ 1 ] = SNAP_INT_TO_FLOAT * floor( pts[ j ][ 1 ] * SNAP_FLOAT_TO_INT + 0.5f );
			//%	pts[ j ][ 2 ] = SNAP_INT_TO_FLOAT * floor( pts[ j ][ 2 ] * SNAP_FLOAT_TO_INT + 0.5f );
		}

		/* print brush side */
		/* ( 640 24 -224 ) ( 448 24 -224 ) ( 448 -232 -224 ) common/caulk 0 48 0 0.500000 0.500000 0 0 0 */
		fprintf( f, "\t\t( %.3f %.3f %.3f ) ( %.3f %.3f %.3f ) ( %.3f %.3f %.3f ) %s 0 0 0 0.5 0.5 0 0 0\n",
				 pts[ 0 ][ 0 ], pts[ 0 ][ 1 ], pts[ 0 ][ 2 ],
				 pts[ 1 ][ 0 ], pts[ 1 ][ 1 ], pts[ 1 ][ 2 ],
				 pts[ 2 ][ 0 ], pts[ 2 ][ 1 ], pts[ 2 ][ 2 ],
				 texture );
	}

	/* end brush */
	fprintf( f, "\t}\n\n" );
}

#if 0
/* iterate through the brush sides (ignore the first 6 bevel planes) */
for ( i = 0; i < brush->numSides; i++ )
{
	/* get side */
	side = &bspBrushSides[ brush->firstSide + i ];

	/* get shader */
	if ( side->shaderNum < 0 || side->shaderNum >= numBSPShaders ) {
		continue;
	}
	shader = &bspShaders[ side->shaderNum ];
	if ( !Q_stricmp( shader->shader, "default" ) || !Q_stricmp( shader->shader, "noshader" ) ) {
		continue;
	}

	/* get texture name */
	if ( !Q_strncasecmp( shader->shader, "textures/", 9 ) ) {
		texture = shader->shader + 9;
	}
	else{
		texture = shader->shader;
	}

	/* get plane */
	plane = &bspPlanes[ side->planeNum ];

	/* make plane points */
	{
		vec3_t vecs[ 2 ];


		MakeNormalVectors( plane->normal, vecs[ 0 ], vecs[ 1 ] );
		VectorMA( vec3_origin, plane->dist, plane->normal, pts[ 0 ] );
		VectorMA( pts[ 0 ], 256.0f, vecs[ 0 ], pts[ 1 ] );
		VectorMA( pts[ 0 ], 256.0f, vecs[ 1 ], pts[ 2 ] );
	}

	/* offset by origin */
	for ( j = 0; j < 3; j++ )
		VectorAdd( pts[ j ], origin, pts[ j ] );

	/* print brush side */
	/* ( 640 24 -224 ) ( 448 24 -224 ) ( 448 -232 -224 ) common/caulk 0 48 0 0.500000 0.500000 0 0 0 */
	fprintf( f, "\t\t( %.3f %.3f %.3f ) ( %.3f %.3f %.3f ) ( %.3f %.3f %.3f ) %s 0 0 0 0.5 0.5 0 0 0\n",
			 pts[ 0 ][ 0 ], pts[ 0 ][ 1 ], pts[ 0 ][ 2 ],
			 pts[ 1 ][ 0 ], pts[ 1 ][ 1 ], pts[ 1 ][ 2 ],
			 pts[ 2 ][ 0 ], pts[ 2 ][ 1 ], pts[ 2 ][ 2 ],
			 texture );
}
#endif



/*
   ConvertPatch()
   converts a bsp patch to a map patch

    {
        patchDef2
        {
            base_wall/concrete
            ( 9 3 0 0 0 )
            (
                ( ( 168 168 -192 0 2 ) ( 168 168 -64 0 1 ) ( 168 168 64 0 0 ) ... )
                ...
            )
        }
    }

 */

static void ConvertPatch( FILE *f, int num, bspDrawSurface_t *ds, vec3_t origin ){
	int x, y;
	bspShader_t     *shader;
	char            *texture;
	bspDrawVert_t   *dv;
	vec3_t xyz;


	/* only patches */
	if ( ds->surfaceType != MST_PATCH ) {
		return;
	}

	/* get shader */
	if ( ds->shaderNum < 0 || ds->shaderNum >= numBSPShaders ) {
		return;
	}
	shader = &bspShaders[ ds->shaderNum ];

	/* get texture name */
	if ( !Q_strncasecmp( shader->shader, "textures/", 9 ) ) {
		texture = shader->shader + 9;
	}
	else{
		texture = shader->shader;
	}

	/* start patch */
	fprintf( f, "\t// patch %d\n", num );
	fprintf( f, "\t{\n" );
	fprintf( f, "\t\tpatchDef2\n" );
	fprintf( f, "\t\t{\n" );
	fprintf( f, "\t\t\t%s\n", texture );
	fprintf( f, "\t\t\t( %d %d 0 0 0 )\n", ds->patchWidth, ds->patchHeight );
	fprintf( f, "\t\t\t(\n" );

	/* iterate through the verts */
	for ( x = 0; x < ds->patchWidth; x++ )
	{
		/* start row */
		fprintf( f, "\t\t\t\t(" );

		/* iterate through the row */
		for ( y = 0; y < ds->patchHeight; y++ )
		{
			/* get vert */
			dv = &bspDrawVerts[ ds->firstVert + ( y * ds->patchWidth ) + x ];

			/* offset it */
			VectorAdd( origin, dv->xyz, xyz );

			/* print vertex */
			fprintf( f, " ( %f %f %f %f %f )", xyz[ 0 ], xyz[ 1 ], xyz[ 2 ], dv->st[ 0 ], dv->st[ 1 ] );
		}

		/* end row */
		fprintf( f, " )\n" );
	}

	/* end patch */
	fprintf( f, "\t\t\t)\n" );
	fprintf( f, "\t\t}\n" );
	fprintf( f, "\t}\n\n" );
}



/*
   ConvertModel()
   exports a bsp model to a map file
 */

static void ConvertModel( FILE *f, bspModel_t *model, int modelNum, vec3_t origin ){
	int i, num;
	bspBrush_t          *brush;
	bspDrawSurface_t    *ds;


	/* convert bsp planes to map planes */
	nummapplanes = numBSPPlanes;
	for ( i = 0; i < numBSPPlanes; i++ )
	{
		VectorCopy( bspPlanes[ i ].normal, mapplanes[ i ].normal );
		mapplanes[ i ].dist = bspPlanes[ i ].dist;
		mapplanes[ i ].type = PlaneTypeForNormal( mapplanes[ i ].normal );
		mapplanes[ i ].hash_chain = NULL;
	}

	/* allocate a build brush */
	buildBrush = AllocBrush( 512 );
	buildBrush->entityNum = 0;
	buildBrush->original = buildBrush;

	/* go through each brush in the model */
	for ( i = 0; i < model->numBSPBrushes; i++ )
	{
		num = i + model->firstBSPBrush;
		brush = &bspBrushes[ num ];
		ConvertBrush( f, num, brush, origin );
	}

	/* free the build brush */
	free( buildBrush );

	/* go through each drawsurf in the model */
	for ( i = 0; i < model->numBSPSurfaces; i++ )
	{
		num = i + model->firstBSPSurface;
		ds = &bspDrawSurfaces[ num ];

		/* we only love patches */
		if ( ds->surfaceType == MST_PATCH ) {
			ConvertPatch( f, num, ds, origin );
		}
	}
}



/*
   ConvertEPairs()
   exports entity key/value pairs to a map file
 */

static void ConvertEPairs( FILE *f, entity_t *e ){
	epair_t *ep;


	/* walk epairs */
	for ( ep = e->epairs; ep != NULL; ep = ep->next )
	{
		/* ignore empty keys/values */
		if ( ep->key[ 0 ] == '\0' || ep->value[ 0 ] == '\0' ) {
			continue;
		}

		/* ignore model keys with * prefixed values */
		if ( !Q_stricmp( ep->key, "model" ) && ep->value[ 0 ] == '*' ) {
			continue;
		}

		/* emit the epair */
		fprintf( f, "\t\"%s\" \"%s\"\n", ep->key, ep->value );
	}
}



/*
   ConvertBSPToMap()
   exports an quake map file from the bsp
 */

int ConvertBSPToMap( char *bspName ){
	int i, modelNum;
	FILE            *f;
	bspModel_t      *model;
	entity_t        *e;
	vec3_t origin;
	const char      *value;
	char name[ 1024 ], base[ 1024 ];


	/* note it */
	Sys_Printf( "--- Convert BSP to MAP ---\n" );

	/* create the bsp filename from the bsp name */
	strcpy( name, bspName );
	StripExtension( name );
	strcat( name, "_converted.map" );
	Sys_Printf( "writing %s\n", name );

	ExtractFileBase( bspName, base );
	strcat( base, ".bsp" );

	/* open it */
	f = fopen( name, "wb" );
	if ( f == NULL ) {
		Error( "Open failed on %s\n", name );
	}

	/* print header */
	fprintf( f, "// Generated by Q3Map2 (ydnar) -convert -format map\n" );

	/* walk entity list */
	for ( i = 0; i < numEntities; i++ )
	{
		/* get entity */
		e = &entities[ i ];

		/* start entity */
		fprintf( f, "// entity %d\n", i );
		fprintf( f, "{\n" );

		/* export keys */
		ConvertEPairs( f, e );
		fprintf( f, "\n" );

		/* get model num */
		if ( i == 0 ) {
			modelNum = 0;
		}
		else
		{
			value = ValueForKey( e, "model" );
			if ( value[ 0 ] == '*' ) {
				modelNum = atoi( value + 1 );
			}
			else{
				modelNum = -1;
			}
		}

		/* only handle bsp models */
		if ( modelNum >= 0 ) {
			/* get model */
			model = &bspModels[ modelNum ];

			/* get entity origin */
			value = ValueForKey( e, "origin" );
			if ( value[ 0 ] == '\0' ) {
				VectorClear( origin );
			}
			else{
				GetVectorForKey( e, "origin", origin );
			}

			/* convert model */
			ConvertModel( f, model, modelNum, origin );
		}

		/* end entity */
		fprintf( f, "}\n\n" );
	}

	/* close the file and return */
	fclose( f );

	/* return to sender */
	return 0;
}
