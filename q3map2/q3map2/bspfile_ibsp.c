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
#define BSPFILE_IBSP_C



/* dependencies */
#include "q3map2.h"




/* -------------------------------------------------------------------------------

   this file handles translating the bsp file format used by quake 3, rtcw, and ef
   into the abstracted bsp file used by q3map2.

   ------------------------------------------------------------------------------- */

/* constants */
#define LUMP_ENTITIES       0
#define LUMP_SHADERS        1
#define LUMP_PLANES         2
#define LUMP_NODES          3
#define LUMP_LEAFS          4
#define LUMP_LEAFSURFACES   5
#define LUMP_LEAFBRUSHES    6
#define LUMP_MODELS         7
#define LUMP_BRUSHES        8
#define LUMP_BRUSHSIDES     9
#define LUMP_DRAWVERTS      10
#define LUMP_DRAWINDEXES    11
#define LUMP_FOGS           12
#define LUMP_SURFACES       13
#define LUMP_LIGHTMAPS      14
#define LUMP_LIGHTGRID      15
#define LUMP_VISIBILITY     16
#define LUMP_ADVERTISEMENTS 17
#define HEADER_LUMPS        18


/* types */
typedef struct
{
	char ident[ 4 ];
	int version;

	bspLump_t lumps[ HEADER_LUMPS ];
}
ibspHeader_t;



/* brush sides */
typedef struct
{
	int planeNum;
	int shaderNum;
}
ibspBrushSide_t;


static void CopyBrushSidesLump( ibspHeader_t *header ){
	int i;
	ibspBrushSide_t *in;
	bspBrushSide_t  *out;


	/* get count */
	numBSPBrushSides = GetLumpElements( (bspHeader_t*) header, LUMP_BRUSHSIDES, sizeof( *in ) );

	/* copy */
	in = GetLump( (bspHeader_t*) header, LUMP_BRUSHSIDES );
	out = bspBrushSides;
	for ( i = 0; i < numBSPBrushSides; i++ )
	{
		out->planeNum = in->planeNum;
		out->shaderNum = in->shaderNum;
		out->surfaceNum = -1;
		in++;
		out++;
	}
}


static void AddBrushSidesLump( FILE *file, ibspHeader_t *header ){
	int i, size;
	bspBrushSide_t  *in;
	ibspBrushSide_t *buffer, *out;


	/* allocate output buffer */
	size = numBSPBrushSides * sizeof( *buffer );
	buffer = safe_malloc( size );
	memset( buffer, 0, size );

	/* convert */
	in = bspBrushSides;
	out = buffer;
	for ( i = 0; i < numBSPBrushSides; i++ )
	{
		out->planeNum = in->planeNum;
		out->shaderNum = in->shaderNum;
		in++;
		out++;
	}

	/* write lump */
	AddLump( file, (bspHeader_t*) header, LUMP_BRUSHSIDES, buffer, size );

	/* free buffer */
	free( buffer );
}



/* drawsurfaces */
typedef struct ibspDrawSurface_s
{
	int shaderNum;
	int fogNum;
	int surfaceType;

	int firstVert;
	int numVerts;

	int firstIndex;
	int numIndexes;

	int lightmapNum;
	int lightmapX, lightmapY;
	int lightmapWidth, lightmapHeight;

	vec3_t lightmapOrigin;
	vec3_t lightmapVecs[ 3 ];

	int patchWidth;
	int patchHeight;
}
ibspDrawSurface_t;


static void CopyDrawSurfacesLump( ibspHeader_t *header ){
	int i, j;
	ibspDrawSurface_t   *in;
	bspDrawSurface_t    *out;


	/* get count */
	numBSPDrawSurfaces = GetLumpElements( (bspHeader_t*) header, LUMP_SURFACES, sizeof( *in ) );
	SetDrawSurfaces( numBSPDrawSurfaces );

	/* copy */
	in = GetLump( (bspHeader_t*) header, LUMP_SURFACES );
	out = bspDrawSurfaces;
	for ( i = 0; i < numBSPDrawSurfaces; i++ )
	{
		out->shaderNum = in->shaderNum;
		out->fogNum = in->fogNum;
		out->surfaceType = in->surfaceType;
		out->firstVert = in->firstVert;
		out->numVerts = in->numVerts;
		out->firstIndex = in->firstIndex;
		out->numIndexes = in->numIndexes;

		out->lightmapStyles[ 0 ] = LS_NORMAL;
		out->vertexStyles[ 0 ] = LS_NORMAL;
		out->lightmapNum[ 0 ] = in->lightmapNum;
		out->lightmapX[ 0 ] = in->lightmapX;
		out->lightmapY[ 0 ] = in->lightmapY;

		for ( j = 1; j < MAX_LIGHTMAPS; j++ )
		{
			out->lightmapStyles[ j ] = LS_NONE;
			out->vertexStyles[ j ] = LS_NONE;
			out->lightmapNum[ j ] = -3;
			out->lightmapX[ j ] = 0;
			out->lightmapY[ j ] = 0;
		}

		out->lightmapWidth = in->lightmapWidth;
		out->lightmapHeight = in->lightmapHeight;

		VectorCopy( in->lightmapOrigin, out->lightmapOrigin );
		VectorCopy( in->lightmapVecs[ 0 ], out->lightmapVecs[ 0 ] );
		VectorCopy( in->lightmapVecs[ 1 ], out->lightmapVecs[ 1 ] );
		VectorCopy( in->lightmapVecs[ 2 ], out->lightmapVecs[ 2 ] );

		out->patchWidth = in->patchWidth;
		out->patchHeight = in->patchHeight;

		in++;
		out++;
	}
}


static void AddDrawSurfacesLump( FILE *file, ibspHeader_t *header ){
	int i, size;
	bspDrawSurface_t    *in;
	ibspDrawSurface_t   *buffer, *out;


	/* allocate output buffer */
	size = numBSPDrawSurfaces * sizeof( *buffer );
	buffer = safe_malloc( size );
	memset( buffer, 0, size );

	/* convert */
	in = bspDrawSurfaces;
	out = buffer;
	for ( i = 0; i < numBSPDrawSurfaces; i++ )
	{
		out->shaderNum = in->shaderNum;
		out->fogNum = in->fogNum;
		out->surfaceType = in->surfaceType;
		out->firstVert = in->firstVert;
		out->numVerts = in->numVerts;
		out->firstIndex = in->firstIndex;
		out->numIndexes = in->numIndexes;

		out->lightmapNum = in->lightmapNum[ 0 ];
		out->lightmapX = in->lightmapX[ 0 ];
		out->lightmapY = in->lightmapY[ 0 ];
		out->lightmapWidth = in->lightmapWidth;
		out->lightmapHeight = in->lightmapHeight;

		VectorCopy( in->lightmapOrigin, out->lightmapOrigin );
		VectorCopy( in->lightmapVecs[ 0 ], out->lightmapVecs[ 0 ] );
		VectorCopy( in->lightmapVecs[ 1 ], out->lightmapVecs[ 1 ] );
		VectorCopy( in->lightmapVecs[ 2 ], out->lightmapVecs[ 2 ] );

		out->patchWidth = in->patchWidth;
		out->patchHeight = in->patchHeight;

		in++;
		out++;
	}

	/* write lump */
	AddLump( file, (bspHeader_t*) header, LUMP_SURFACES, buffer, size );

	/* free buffer */
	free( buffer );
}



/* drawverts */
typedef struct
{
	vec3_t xyz;
	float st[ 2 ];
	float lightmap[ 2 ];
	vec3_t normal;
	byte color[ 4 ];
}
ibspDrawVert_t;


static void CopyDrawVertsLump( ibspHeader_t *header ){
	int i;
	ibspDrawVert_t  *in;
	bspDrawVert_t   *out;


	/* get count */
	numBSPDrawVerts = GetLumpElements( (bspHeader_t*) header, LUMP_DRAWVERTS, sizeof( *in ) );
	SetDrawVerts( numBSPDrawVerts );

	/* copy */
	in = GetLump( (bspHeader_t*) header, LUMP_DRAWVERTS );
	out = bspDrawVerts;
	for ( i = 0; i < numBSPDrawVerts; i++ )
	{
		VectorCopy( in->xyz, out->xyz );
		out->st[ 0 ] = in->st[ 0 ];
		out->st[ 1 ] = in->st[ 1 ];

		out->lightmap[ 0 ][ 0 ] = in->lightmap[ 0 ];
		out->lightmap[ 0 ][ 1 ] = in->lightmap[ 1 ];

		VectorCopy( in->normal, out->normal );

		out->color[ 0 ][ 0 ] = in->color[ 0 ];
		out->color[ 0 ][ 1 ] = in->color[ 1 ];
		out->color[ 0 ][ 2 ] = in->color[ 2 ];
		out->color[ 0 ][ 3 ] = in->color[ 3 ];

		in++;
		out++;
	}
}


static void AddDrawVertsLump( FILE *file, ibspHeader_t *header ){
	int i, size;
	bspDrawVert_t   *in;
	ibspDrawVert_t  *buffer, *out;


	/* allocate output buffer */
	size = numBSPDrawVerts * sizeof( *buffer );
	buffer = safe_malloc( size );
	memset( buffer, 0, size );

	/* convert */
	in = bspDrawVerts;
	out = buffer;
	for ( i = 0; i < numBSPDrawVerts; i++ )
	{
		VectorCopy( in->xyz, out->xyz );
		out->st[ 0 ] = in->st[ 0 ];
		out->st[ 1 ] = in->st[ 1 ];

		out->lightmap[ 0 ] = in->lightmap[ 0 ][ 0 ];
		out->lightmap[ 1 ] = in->lightmap[ 0 ][ 1 ];

		VectorCopy( in->normal, out->normal );

		out->color[ 0 ] = in->color[ 0 ][ 0 ];
		out->color[ 1 ] = in->color[ 0 ][ 1 ];
		out->color[ 2 ] = in->color[ 0 ][ 2 ];
		out->color[ 3 ] = in->color[ 0 ][ 3 ];

		in++;
		out++;
	}

	/* write lump */
	AddLump( file, (bspHeader_t*) header, LUMP_DRAWVERTS, buffer, size );

	/* free buffer */
	free( buffer );
}



/* light grid */
typedef struct
{
	byte ambient[ 3 ];
	byte directed[ 3 ];
	byte latLong[ 2 ];
}
ibspGridPoint_t;


static void CopyLightGridLumps( ibspHeader_t *header ){
	int i, j;
	ibspGridPoint_t *in;
	bspGridPoint_t  *out;


	/* get count */
	numBSPGridPoints = GetLumpElements( (bspHeader_t*) header, LUMP_LIGHTGRID, sizeof( *in ) );

	/* allocate buffer */
	bspGridPoints = safe_malloc( numBSPGridPoints * sizeof( *bspGridPoints ) );
	memset( bspGridPoints, 0, numBSPGridPoints * sizeof( *bspGridPoints ) );

	/* copy */
	in = GetLump( (bspHeader_t*) header, LUMP_LIGHTGRID );
	out = bspGridPoints;
	for ( i = 0; i < numBSPGridPoints; i++ )
	{
		for ( j = 0; j < MAX_LIGHTMAPS; j++ )
		{
			VectorCopy( in->ambient, out->ambient[ j ] );
			VectorCopy( in->directed, out->directed[ j ] );
			out->styles[ j ] = LS_NONE;
		}

		out->styles[ 0 ] = LS_NORMAL;

		out->latLong[ 0 ] = in->latLong[ 0 ];
		out->latLong[ 1 ] = in->latLong[ 1 ];

		in++;
		out++;
	}
}


static void AddLightGridLumps( FILE *file, ibspHeader_t *header ){
	int i;
	bspGridPoint_t  *in;
	ibspGridPoint_t *buffer, *out;


	/* dummy check */
	if ( bspGridPoints == NULL ) {
		return;
	}

	/* allocate temporary buffer */
	buffer = safe_malloc( numBSPGridPoints * sizeof( *out ) );

	/* convert */
	in = bspGridPoints;
	out = buffer;
	for ( i = 0; i < numBSPGridPoints; i++ )
	{
		VectorCopy( in->ambient[ 0 ], out->ambient );
		VectorCopy( in->directed[ 0 ], out->directed );

		out->latLong[ 0 ] = in->latLong[ 0 ];
		out->latLong[ 1 ] = in->latLong[ 1 ];

		in++;
		out++;
	}

	/* write lumps */
	AddLump( file, (bspHeader_t*) header, LUMP_LIGHTGRID, buffer, ( numBSPGridPoints * sizeof( *out ) ) );

	/* free buffer (ydnar 2002-10-22: [bug 641] thanks Rap70r! */
	free( buffer );
}

/*
   LoadIBSPFile()
   loads a quake 3 bsp file into memory
 */

void LoadIBSPFile( const char *filename ){
	ibspHeader_t    *header;


	/* load the file header */
	LoadFile( filename, (void**) &header );

	/* swap the header (except the first 4 bytes) */
	SwapBlock( (int*) ( (byte*) header + sizeof( int ) ), sizeof( *header ) - sizeof( int ) );

	/* make sure it matches the format we're trying to load */
	if ( force == qfalse && *( (int*) header->ident ) != *( (int*) game->bspIdent ) ) {
		Error( "%s is not a %s file", filename, game->bspIdent );
	}
	if ( force == qfalse && header->version != game->bspVersion ) {
		Error( "%s is version %d, not %d", filename, header->version, game->bspVersion );
	}

	/* load/convert lumps */
	numBSPShaders = CopyLump( (bspHeader_t*) header, LUMP_SHADERS, bspShaders, sizeof( bspShader_t ) );

	numBSPModels = CopyLump( (bspHeader_t*) header, LUMP_MODELS, bspModels, sizeof( bspModel_t ) );

	numBSPPlanes = CopyLump( (bspHeader_t*) header, LUMP_PLANES, bspPlanes, sizeof( bspPlane_t ) );

	numBSPLeafs = CopyLump( (bspHeader_t*) header, LUMP_LEAFS, bspLeafs, sizeof( bspLeaf_t ) );

	numBSPNodes = CopyLump( (bspHeader_t*) header, LUMP_NODES, bspNodes, sizeof( bspNode_t ) );

	numBSPLeafSurfaces = CopyLump( (bspHeader_t*) header, LUMP_LEAFSURFACES, bspLeafSurfaces, sizeof( bspLeafSurfaces[ 0 ] ) );

	numBSPLeafBrushes = CopyLump( (bspHeader_t*) header, LUMP_LEAFBRUSHES, bspLeafBrushes, sizeof( bspLeafBrushes[ 0 ] ) );

	numBSPBrushes = CopyLump( (bspHeader_t*) header, LUMP_BRUSHES, bspBrushes, sizeof( bspBrush_t ) );

	CopyBrushSidesLump( header );

	CopyDrawVertsLump( header );

	CopyDrawSurfacesLump( header );

	numBSPFogs = CopyLump( (bspHeader_t*) header, LUMP_FOGS, bspFogs, sizeof( bspFog_t ) );

	numBSPDrawIndexes = CopyLump( (bspHeader_t*) header, LUMP_DRAWINDEXES, bspDrawIndexes, sizeof( bspDrawIndexes[ 0 ] ) );

	numBSPVisBytes = CopyLump( (bspHeader_t*) header, LUMP_VISIBILITY, bspVisBytes, 1 );

	numBSPLightBytes = GetLumpElements( (bspHeader_t*) header, LUMP_LIGHTMAPS, 1 );
	bspLightBytes = safe_malloc( numBSPLightBytes );
	CopyLump( (bspHeader_t*) header, LUMP_LIGHTMAPS, bspLightBytes, 1 );

	bspEntDataSize = CopyLump( (bspHeader_t*) header, LUMP_ENTITIES, bspEntData, 1 );

	CopyLightGridLumps( header );

	/* advertisements */
	if ( header->version == 47 ) { // quake live's bsp version
		numBSPAds = CopyLump( (bspHeader_t*) header, LUMP_ADVERTISEMENTS, bspAds, sizeof( bspAdvertisement_t ) );
	}
	else{
		numBSPAds = 0;
	}

	/* free the file buffer */
	free( header );
}



/*
   WriteIBSPFile()
   writes an id bsp file
 */

void WriteIBSPFile( const char *filename ){
	ibspHeader_t outheader, *header;
	FILE            *file;
	time_t t;
	char marker[ 1024 ];
	int size;


	/* set header */
	header = &outheader;
	memset( header, 0, sizeof( *header ) );

	//%	Swapfile();

	/* set up header */
	*( (int*) (bspHeader_t*) header->ident ) = *( (int*) game->bspIdent );
	header->version = LittleLong( game->bspVersion );

	/* write initial header */
	file = SafeOpenWrite( filename );
	SafeWrite( file, (bspHeader_t*) header, sizeof( *header ) );    /* overwritten later */

	/* add marker lump */
	time( &t );
	sprintf( marker, "I LOVE MY Q3MAP2 %s on %s)", Q3MAP_VERSION, asctime( localtime( &t ) ) );
	AddLump( file, (bspHeader_t*) header, 0, marker, strlen( marker ) + 1 );

	/* add lumps */
	AddLump( file, (bspHeader_t*) header, LUMP_SHADERS, bspShaders, numBSPShaders * sizeof( bspShader_t ) );
	AddLump( file, (bspHeader_t*) header, LUMP_PLANES, bspPlanes, numBSPPlanes * sizeof( bspPlane_t ) );
	AddLump( file, (bspHeader_t*) header, LUMP_LEAFS, bspLeafs, numBSPLeafs * sizeof( bspLeaf_t ) );
	AddLump( file, (bspHeader_t*) header, LUMP_NODES, bspNodes, numBSPNodes * sizeof( bspNode_t ) );
	AddLump( file, (bspHeader_t*) header, LUMP_BRUSHES, bspBrushes, numBSPBrushes * sizeof( bspBrush_t ) );
	AddBrushSidesLump( file, header );
	AddLump( file, (bspHeader_t*) header, LUMP_LEAFSURFACES, bspLeafSurfaces, numBSPLeafSurfaces * sizeof( bspLeafSurfaces[ 0 ] ) );
	AddLump( file, (bspHeader_t*) header, LUMP_LEAFBRUSHES, bspLeafBrushes, numBSPLeafBrushes * sizeof( bspLeafBrushes[ 0 ] ) );
	AddLump( file, (bspHeader_t*) header, LUMP_MODELS, bspModels, numBSPModels * sizeof( bspModel_t ) );
	AddDrawVertsLump( file, header );
	AddDrawSurfacesLump( file, header );
	AddLump( file, (bspHeader_t*) header, LUMP_VISIBILITY, bspVisBytes, numBSPVisBytes );
	AddLump( file, (bspHeader_t*) header, LUMP_LIGHTMAPS, bspLightBytes, numBSPLightBytes );
	AddLightGridLumps( file, header );
	AddLump( file, (bspHeader_t*) header, LUMP_ENTITIES, bspEntData, bspEntDataSize );
	AddLump( file, (bspHeader_t*) header, LUMP_FOGS, bspFogs, numBSPFogs * sizeof( bspFog_t ) );
	AddLump( file, (bspHeader_t*) header, LUMP_DRAWINDEXES, bspDrawIndexes, numBSPDrawIndexes * sizeof( bspDrawIndexes[ 0 ] ) );

	/* advertisements */
	AddLump( file, (bspHeader_t*) header, LUMP_ADVERTISEMENTS, bspAds, numBSPAds * sizeof( bspAdvertisement_t ) );

	/* emit bsp size */
	size = ftell( file );
	Sys_Printf( "Wrote %.1f MB (%d bytes)\n", (float) size / ( 1024 * 1024 ), size );

	/* write the completed header */
	fseek( file, 0, SEEK_SET );
	SafeWrite( file, header, sizeof( *header ) );

	/* close the file */
	fclose( file );
}
