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
#define LIGHTMAPS_YDNAR_C



/* dependencies */
#include "q3map2.h"




/* -------------------------------------------------------------------------------

   this file contains code that doe lightmap allocation and projection that
   runs in the -light phase.

   this is handled here rather than in the bsp phase for a few reasons--
   surfaces are no longer necessarily convex polygons, patches may or may not be
   planar or have lightmaps projected directly onto control points.

   also, this allows lightmaps to be calculated before being allocated and stored
   in the bsp. lightmaps that have little high-frequency information are candidates
   for having their resolutions scaled down.

   ------------------------------------------------------------------------------- */

/*
   WriteTGA24()
   based on WriteTGA() from imagelib.c
 */

void WriteTGA24( char *filename, byte *data, int width, int height, qboolean flip ){
	int i, c;
	byte    *buffer, *in;
	FILE    *file;


	/* allocate a buffer and set it up */
	buffer = safe_malloc( width * height * 3 + 18 );
	memset( buffer, 0, 18 );
	buffer[ 2 ] = 2;
	buffer[ 12 ] = width & 255;
	buffer[ 13 ] = width >> 8;
	buffer[ 14 ] = height & 255;
	buffer[ 15 ] = height >> 8;
	buffer[ 16 ] = 24;

	/* swap rgb to bgr */
	c = ( width * height * 3 ) + 18;
	for ( i = 18; i < c; i += 3 )
	{
		buffer[ i ] = data[ i - 18 + 2 ];       /* blue */
		buffer[ i + 1 ] = data[ i - 18 + 1 ];   /* green */
		buffer[ i + 2 ] = data[ i - 18 + 0 ];   /* red */
	}

	/* write it and free the buffer */
	file = fopen( filename, "wb" );
	if ( file == NULL ) {
		Error( "Unable to open %s for writing", filename );
	}

	/* flip vertically? */
	if ( flip ) {
		fwrite( buffer, 1, 18, file );
		for ( in = buffer + ( ( height - 1 ) * width * 3 ) + 18; in >= buffer; in -= ( width * 3 ) )
			fwrite( in, 1, ( width * 3 ), file );
	}
	else{
		fwrite( buffer, 1, c, file );
	}

	/* close the file */
	fclose( file );
	free( buffer );
}



/*
   ExportLightmaps()
   exports the lightmaps as a list of numbered tga images
 */

void ExportLightmaps( void ){
	int i;
	char dirname[ 1024 ], filename[ 1024 ];
	byte        *lightmap;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- ExportLightmaps ---\n" );

	/* do some path mangling */
	strcpy( dirname, source );
	StripExtension( dirname );

	/* sanity check */
	if ( bspLightBytes == NULL ) {
		Sys_FPrintf( SYS_WRN, "WARNING: No BSP lightmap data\n" );
		return;
	}

	/* make a directory for the lightmaps */
	Q_mkdir( dirname );

	/* iterate through the lightmaps */
	for ( i = 0, lightmap = bspLightBytes; lightmap < ( bspLightBytes + numBSPLightBytes ); i++, lightmap += ( game->lightmapSize * game->lightmapSize * 3 ) )
	{
		/* write a tga image out */
		sprintf( filename, "%s/lightmap_%04d.tga", dirname, i );
		Sys_Printf( "Writing %s\n", filename );
		WriteTGA24( filename, lightmap, game->lightmapSize, game->lightmapSize, qfalse );
	}
}



/*
   ExportLightmapsMain()
   exports the lightmaps as a list of numbered tga images
 */

int ExportLightmapsMain( int argc, char **argv ){
	/* arg checking */
	if ( argc < 1 ) {
		Sys_Printf( "Usage: q3map -export [-v] <mapname>\n" );
		return 0;
	}

	/* do some path mangling */
	strcpy( source, ExpandArg( argv[ argc - 1 ] ) );
	StripExtension( source );
	DefaultExtension( source, ".bsp" );

	/* load the bsp */
	Sys_Printf( "Loading %s\n", source );
	LoadBSPFile( source );

	/* export the lightmaps */
	ExportLightmaps();

	/* return to sender */
	return 0;
}



/*
   ImportLightmapsMain()
   imports the lightmaps from a list of numbered tga images
 */

int ImportLightmapsMain( int argc, char **argv ){
	int i, x, y, len, width, height;
	char dirname[ 1024 ], filename[ 1024 ];
	byte        *lightmap, *buffer, *pixels, *in, *out;


	/* arg checking */
	if ( argc < 1 ) {
		Sys_Printf( "Usage: q3map -import [-v] <mapname>\n" );
		return 0;
	}

	/* do some path mangling */
	strcpy( source, ExpandArg( argv[ argc - 1 ] ) );
	StripExtension( source );
	DefaultExtension( source, ".bsp" );

	/* load the bsp */
	Sys_Printf( "Loading %s\n", source );
	LoadBSPFile( source );

	/* note it */
	Sys_FPrintf( SYS_VRB, "--- ImportLightmaps ---\n" );

	/* do some path mangling */
	strcpy( dirname, source );
	StripExtension( dirname );

	/* sanity check */
	if ( bspLightBytes == NULL ) {
		Error( "No lightmap data" );
	}

	/* make a directory for the lightmaps */
	Q_mkdir( dirname );

	/* iterate through the lightmaps */
	for ( i = 0, lightmap = bspLightBytes; lightmap < ( bspLightBytes + numBSPLightBytes ); i++, lightmap += ( game->lightmapSize * game->lightmapSize * 3 ) )
	{
		/* read a tga image */
		sprintf( filename, "%s/lightmap_%04d.tga", dirname, i );
		Sys_Printf( "Loading %s\n", filename );
		buffer = NULL;
		len = vfsLoadFile( filename, (void*) &buffer, -1 );
		if ( len < 0 ) {
			Sys_FPrintf( SYS_WRN, "WARNING: Unable to load image %s\n", filename );
			continue;
		}

		/* parse file into an image */
		pixels = NULL;
		LoadTGABuffer( buffer, buffer + len, &pixels, &width, &height );
		free( buffer );

		/* sanity check it */
		if ( pixels == NULL ) {
			Sys_FPrintf( SYS_WRN, "WARNING: Unable to load image %s\n", filename );
			continue;
		}
		if ( width != game->lightmapSize || height != game->lightmapSize ) {
			Sys_FPrintf( SYS_WRN, "WARNING: Image %s is not the right size (%d, %d) != (%d, %d)\n",
						filename, width, height, game->lightmapSize, game->lightmapSize );
		}

		/* copy the pixels */
		in = pixels;
		for ( y = 1; y <= game->lightmapSize; y++ )
		{
			out = lightmap + ( ( game->lightmapSize - y ) * game->lightmapSize * 3 );
			for ( x = 0; x < game->lightmapSize; x++, in += 4, out += 3 )
				VectorCopy( in, out );
		}

		/* free the image */
		free( pixels );
	}

	/* write the bsp */
	Sys_Printf( "writing %s\n", source );
	WriteBSPFile( source );

	/* return to sender */
	return 0;
}



/* -------------------------------------------------------------------------------

   this section deals with projecting a lightmap onto a raw drawsurface

   ------------------------------------------------------------------------------- */

/*
   CompareLightSurface()
   compare function for qsort()
 */

static int CompareLightSurface( const void *a, const void *b ){
	shaderInfo_t    *asi, *bsi;


	/* get shaders */
	asi = surfaceInfos[ *( (int*) a ) ].si;
	bsi = surfaceInfos[ *( (int*) b ) ].si;

	/* dummy check */
	if ( asi == NULL ) {
		return -1;
	}
	if ( bsi == NULL ) {
		return 1;
	}

	/* compare shader names */
	return strcmp( asi->shader, bsi->shader );
}



/*
   FinishRawLightmap()
   allocates a raw lightmap's necessary buffers
 */

void FinishRawLightmap( rawLightmap_t *lm ){
	int i, j, c, size, *sc;
	float is;
	surfaceInfo_t       *info;


	/* sort light surfaces by shader name */
	qsort( &lightSurfaces[ lm->firstLightSurface ], lm->numLightSurfaces, sizeof( int ), CompareLightSurface );

	/* count clusters */
	lm->numLightClusters = 0;
	for ( i = 0; i < lm->numLightSurfaces; i++ )
	{
		/* get surface info */
		info = &surfaceInfos[ lightSurfaces[ lm->firstLightSurface + i ] ];

		/* add surface clusters */
		lm->numLightClusters += info->numSurfaceClusters;
	}

	/* allocate buffer for clusters and copy */
	lm->lightClusters = safe_malloc( lm->numLightClusters * sizeof( *lm->lightClusters ) );
	c = 0;
	for ( i = 0; i < lm->numLightSurfaces; i++ )
	{
		/* get surface info */
		info = &surfaceInfos[ lightSurfaces[ lm->firstLightSurface + i ] ];

		/* add surface clusters */
		for ( j = 0; j < info->numSurfaceClusters; j++ )
			lm->lightClusters[ c++ ] = surfaceClusters[ info->firstSurfaceCluster + j ];
	}

	/* set styles */
	lm->styles[ 0 ] = LS_NORMAL;
	for ( i = 1; i < MAX_LIGHTMAPS; i++ )
		lm->styles[ i ] = LS_NONE;

	/* set supersampling size */
	lm->sw = lm->w * superSample;
	lm->sh = lm->h * superSample;

	/* add to super luxel count */
	numRawSuperLuxels += ( lm->sw * lm->sh );

	/* manipulate origin/vecs for supersampling */
	if ( superSample > 1 && lm->vecs != NULL ) {
		/* calc inverse supersample */
		is = 1.0f / superSample;

		/* scale the vectors and shift the origin */
		#if 1
		/* new code that works for arbitrary supersampling values */
		VectorMA( lm->origin, -0.5, lm->vecs[ 0 ], lm->origin );
		VectorMA( lm->origin, -0.5, lm->vecs[ 1 ], lm->origin );
		VectorScale( lm->vecs[ 0 ], is, lm->vecs[ 0 ] );
		VectorScale( lm->vecs[ 1 ], is, lm->vecs[ 1 ] );
		VectorMA( lm->origin, is, lm->vecs[ 0 ], lm->origin );
		VectorMA( lm->origin, is, lm->vecs[ 1 ], lm->origin );
		#else
		/* old code that only worked with a value of 2 */
		VectorScale( lm->vecs[ 0 ], is, lm->vecs[ 0 ] );
		VectorScale( lm->vecs[ 1 ], is, lm->vecs[ 1 ] );
		VectorMA( lm->origin, -is, lm->vecs[ 0 ], lm->origin );
		VectorMA( lm->origin, -is, lm->vecs[ 1 ], lm->origin );
		#endif
	}

	/* allocate bsp lightmap storage */
	size = lm->w * lm->h * BSP_LUXEL_SIZE * sizeof( float );
	if ( lm->bspLuxels[ 0 ] == NULL ) {
		lm->bspLuxels[ 0 ] = safe_malloc( size );
	}
	memset( lm->bspLuxels[ 0 ], 0, size );

	/* allocate radiosity lightmap storage */
	if ( bounce ) {
		size = lm->w * lm->h * RAD_LUXEL_SIZE * sizeof( float );
		if ( lm->radLuxels[ 0 ] == NULL ) {
			lm->radLuxels[ 0 ] = safe_malloc( size );
		}
		memset( lm->radLuxels[ 0 ], 0, size );
	}

	/* allocate sampling lightmap storage */
	size = lm->sw * lm->sh * SUPER_LUXEL_SIZE * sizeof( float );
	if ( lm->superLuxels[ 0 ] == NULL ) {
		lm->superLuxels[ 0 ] = safe_malloc( size );
	}
	memset( lm->superLuxels[ 0 ], 0, size );

	/* allocate origin map storage */
	size = lm->sw * lm->sh * SUPER_ORIGIN_SIZE * sizeof( float );
	if ( lm->superOrigins == NULL ) {
		lm->superOrigins = safe_malloc( size );
	}
	memset( lm->superOrigins, 0, size );

	/* allocate normal map storage */
	size = lm->sw * lm->sh * SUPER_NORMAL_SIZE * sizeof( float );
	if ( lm->superNormals == NULL ) {
		lm->superNormals = safe_malloc( size );
	}
	memset( lm->superNormals, 0, size );

	/* allocate cluster map storage */
	size = lm->sw * lm->sh * sizeof( int );
	if ( lm->superClusters == NULL ) {
		lm->superClusters = safe_malloc( size );
	}
	size = lm->sw * lm->sh;
	sc = lm->superClusters;
	for ( i = 0; i < size; i++ )
		( *sc++ ) = CLUSTER_UNMAPPED;

	/* deluxemap allocation */
	if ( deluxemap ) {
		/* allocate sampling deluxel storage */
		size = lm->sw * lm->sh * SUPER_DELUXEL_SIZE * sizeof( float );
		if ( lm->superDeluxels == NULL ) {
			lm->superDeluxels = safe_malloc( size );
		}
		memset( lm->superDeluxels, 0, size );

		/* allocate bsp deluxel storage */
		size = lm->w * lm->h * BSP_DELUXEL_SIZE * sizeof( float );
		if ( lm->bspDeluxels == NULL ) {
			lm->bspDeluxels = safe_malloc( size );
		}
		memset( lm->bspDeluxels, 0, size );
	}

	/* add to count */
	numLuxels += ( lm->sw * lm->sh );
}



/*
   AddPatchToRawLightmap()
   projects a lightmap for a patch surface
   since lightmap calculation for surfaces is now handled in a general way (light_ydnar.c),
   it is no longer necessary for patch verts to fall exactly on a lightmap sample
   based on AllocateLightmapForPatch()
 */

qboolean AddPatchToRawLightmap( int num, rawLightmap_t *lm ){
	bspDrawSurface_t    *ds;
	surfaceInfo_t       *info;
	int x, y;
	bspDrawVert_t       *verts, *a, *b;
	vec3_t delta;
	mesh_t src, *subdivided, *mesh;
	float sBasis, tBasis, s, t;
	float length, widthTable[ MAX_EXPANDED_AXIS ], heightTable[ MAX_EXPANDED_AXIS ];


	/* patches finish a raw lightmap */
	lm->finished = qtrue;

	/* get surface and info  */
	ds = &bspDrawSurfaces[ num ];
	info = &surfaceInfos[ num ];

	/* make a temporary mesh from the drawsurf */
	src.width = ds->patchWidth;
	src.height = ds->patchHeight;
	src.verts = &yDrawVerts[ ds->firstVert ];
	//%	subdivided = SubdivideMesh( src, 8, 512 );
	subdivided = SubdivideMesh2( src, info->patchIterations );

	/* fit it to the curve and remove colinear verts on rows/columns */
	PutMeshOnCurve( *subdivided );
	mesh = RemoveLinearMeshColumnsRows( subdivided );
	FreeMesh( subdivided );

	/* find the longest distance on each row/column */
	verts = mesh->verts;
	memset( widthTable, 0, sizeof( widthTable ) );
	memset( heightTable, 0, sizeof( heightTable ) );
	for ( y = 0; y < mesh->height; y++ )
	{
		for ( x = 0; x < mesh->width; x++ )
		{
			/* get width */
			if ( x + 1 < mesh->width ) {
				a = &verts[ ( y * mesh->width ) + x ];
				b = &verts[ ( y * mesh->width ) + x + 1 ];
				VectorSubtract( a->xyz, b->xyz, delta );
				length = VectorLength( delta );
				if ( length > widthTable[ x ] ) {
					widthTable[ x ] = length;
				}
			}

			/* get height */
			if ( y + 1 < mesh->height ) {
				a = &verts[ ( y * mesh->width ) + x ];
				b = &verts[ ( ( y + 1 ) * mesh->width ) + x ];
				VectorSubtract( a->xyz, b->xyz, delta );
				length = VectorLength( delta );
				if ( length > heightTable[ y ] ) {
					heightTable[ y ] = length;
				}
			}
		}
	}

	/* determine lightmap width */
	length = 0;
	for ( x = 0; x < ( mesh->width - 1 ); x++ )
		length += widthTable[ x ];
	lm->w = lm->sampleSize != 0 ? ceil( length / lm->sampleSize ) + 1 : 0;
	if ( lm->w < ds->patchWidth ) {
		lm->w = ds->patchWidth;
	}
	if ( lm->w > lm->customWidth ) {
		lm->w = lm->customWidth;
	}
	sBasis = (float) ( lm->w - 1 ) / (float) ( ds->patchWidth - 1 );

	/* determine lightmap height */
	length = 0;
	for ( y = 0; y < ( mesh->height - 1 ); y++ )
		length += heightTable[ y ];
	lm->h = lm->sampleSize != 0 ? ceil( length / lm->sampleSize ) + 1 : 0;
	if ( lm->h < ds->patchHeight ) {
		lm->h = ds->patchHeight;
	}
	if ( lm->h > lm->customHeight ) {
		lm->h = lm->customHeight;
	}
	tBasis = (float) ( lm->h - 1 ) / (float) ( ds->patchHeight - 1 );

	/* free the temporary mesh */
	FreeMesh( mesh );

	/* set the lightmap texture coordinates in yDrawVerts */
	lm->wrap[ 0 ] = qtrue;
	lm->wrap[ 1 ] = qtrue;
	verts = &yDrawVerts[ ds->firstVert ];
	for ( y = 0; y < ds->patchHeight; y++ )
	{
		t = ( tBasis * y ) + 0.5f;
		for ( x = 0; x < ds->patchWidth; x++ )
		{
			s = ( sBasis * x ) + 0.5f;
			verts[ ( y * ds->patchWidth ) + x ].lightmap[ 0 ][ 0 ] = s * superSample;
			verts[ ( y * ds->patchWidth ) + x ].lightmap[ 0 ][ 1 ] = t * superSample;

			if ( y == 0 && !VectorCompare( verts[ x ].xyz, verts[ ( ( ds->patchHeight - 1 ) * ds->patchWidth ) + x ].xyz ) ) {
				lm->wrap[ 1 ] = qfalse;
			}
		}

		if ( !VectorCompare( verts[ ( y * ds->patchWidth ) ].xyz, verts[ ( y * ds->patchWidth ) + ( ds->patchWidth - 1 ) ].xyz ) ) {
			lm->wrap[ 0 ] = qfalse;
		}
	}

	/* debug code: */
	//%	Sys_Printf( "wrap S: %d wrap T: %d\n", lm->wrap[ 0 ], lm->wrap[ 1 ] );
	//% if( lm->w > (ds->lightmapWidth & 0xFF) || lm->h > (ds->lightmapHeight & 0xFF) )
	//%		Sys_Printf( "Patch lightmap: (%3d %3d) > (%3d, %3d)\n", lm->w, lm->h, ds->lightmapWidth & 0xFF, ds->lightmapHeight & 0xFF );
	//% ds->lightmapWidth = lm->w | (ds->lightmapWidth & 0xFFFF0000);
	//% ds->lightmapHeight = lm->h | (ds->lightmapHeight & 0xFFFF0000);

	/* add to counts */
	numPatchesLightmapped++;

	/* return */
	return qtrue;
}



/*
   AddSurfaceToRawLightmap()
   projects a lightmap for a surface
   based on AllocateLightmapForSurface()
 */

qboolean AddSurfaceToRawLightmap( int num, rawLightmap_t *lm ){
	bspDrawSurface_t    *ds, *ds2;
	surfaceInfo_t       *info, *info2;
	int num2, n, i, axisNum;
	float s, t, d, len, sampleSize;
	vec3_t mins, maxs, origin, faxis, size, exactSize, delta, normalized, vecs[ 2 ];
	vec4_t plane;
	bspDrawVert_t       *verts;


	/* get surface and info  */
	ds = &bspDrawSurfaces[ num ];
	info = &surfaceInfos[ num ];

	/* add the surface to the raw lightmap */
	lightSurfaces[ numLightSurfaces++ ] = num;
	lm->numLightSurfaces++;

	/* does this raw lightmap already have any surfaces? */
	if ( lm->numLightSurfaces > 1 ) {
		/* surface and raw lightmap must have the same lightmap projection axis */
		if ( VectorCompare( info->axis, lm->axis ) == qfalse ) {
			return qfalse;
		}

		/* match identical attributes */
		if ( info->sampleSize != lm->sampleSize ||
			 info->entityNum != lm->entityNum ||
			 info->recvShadows != lm->recvShadows ||
			 info->si->lmCustomWidth != lm->customWidth ||
			 info->si->lmCustomHeight != lm->customHeight ||
			 info->si->lmBrightness != lm->brightness ||
			 info->si->lmFilterRadius != lm->filterRadius ||
			 info->si->splotchFix != lm->splotchFix ) {
			return qfalse;
		}

		/* surface bounds must intersect with raw lightmap bounds */
		for ( i = 0; i < 3; i++ )
		{
			if ( info->mins[ i ] > lm->maxs[ i ] ) {
				return qfalse;
			}
			if ( info->maxs[ i ] < lm->mins[ i ] ) {
				return qfalse;
			}
		}

		/* plane check (fixme: allow merging of nonplanars) */
		if ( info->si->lmMergable == qfalse ) {
			if ( info->plane == NULL || lm->plane == NULL ) {
				return qfalse;
			}

			/* compare planes */
			for ( i = 0; i < 4; i++ )
				if ( fabs( info->plane[ i ] - lm->plane[ i ] ) > EQUAL_EPSILON ) {
					return qfalse;
				}
		}

		/* debug code hacking */
		//%	if( lm->numLightSurfaces > 1 )
		//%		return qfalse;
	}

	/* set plane */
	if ( info->plane == NULL ) {
		lm->plane = NULL;
	}

	/* add surface to lightmap bounds */
	AddPointToBounds( info->mins, lm->mins, lm->maxs );
	AddPointToBounds( info->maxs, lm->mins, lm->maxs );

	/* check to see if this is a non-planar patch */
	if ( ds->surfaceType == MST_PATCH &&
		 lm->axis[ 0 ] == 0.0f && lm->axis[ 1 ] == 0.0f && lm->axis[ 2 ] == 0.0f ) {
		return AddPatchToRawLightmap( num, lm );
	}

	/* start with initially requested sample size */
	sampleSize = lm->sampleSize;

	/* round to the lightmap resolution */
	for ( i = 0; i < 3; i++ )
	{
		exactSize[ i ] = lm->maxs[ i ] - lm->mins[ i ];
		mins[ i ] = sampleSize * floor( lm->mins[ i ] / sampleSize );
		maxs[ i ] = sampleSize * ceil( lm->maxs[ i ] / sampleSize );
		size[ i ] = ( maxs[ i ] - mins[ i ] ) / sampleSize + 1.0f;

		/* hack (god this sucks) */
		if ( size[ i ] > lm->customWidth || size[ i ] > lm->customHeight ) {
			i = -1;
			sampleSize += 1.0f;
		}
	}

	/* set actual sample size */
	lm->actualSampleSize = sampleSize;

	/* fixme: copy rounded mins/maxes to lightmap record? */
	if ( lm->plane == NULL ) {
		VectorCopy( mins, lm->mins );
		VectorCopy( maxs, lm->maxs );
		VectorCopy( mins, origin );
	}

	/* set lightmap origin */
	VectorCopy( lm->mins, origin );

	/* make absolute axis */
	faxis[ 0 ] = fabs( lm->axis[ 0 ] );
	faxis[ 1 ] = fabs( lm->axis[ 1 ] );
	faxis[ 2 ] = fabs( lm->axis[ 2 ] );

	/* clear out lightmap vectors */
	memset( vecs, 0, sizeof( vecs ) );

	/* classify the plane (x y or z major) (ydnar: biased to z axis projection) */
	if ( faxis[ 2 ] >= faxis[ 0 ] && faxis[ 2 ] >= faxis[ 1 ] ) {
		axisNum = 2;
		lm->w = size[ 0 ];
		lm->h = size[ 1 ];
		vecs[ 0 ][ 0 ] = 1.0f / sampleSize;
		vecs[ 1 ][ 1 ] = 1.0f / sampleSize;
	}
	else if ( faxis[ 0 ] >= faxis[ 1 ] && faxis[ 0 ] >= faxis[ 2 ] ) {
		axisNum = 0;
		lm->w = size[ 1 ];
		lm->h = size[ 2 ];
		vecs[ 0 ][ 1 ] = 1.0f / sampleSize;
		vecs[ 1 ][ 2 ] = 1.0f / sampleSize;
	}
	else
	{
		axisNum = 1;
		lm->w = size[ 0 ];
		lm->h = size[ 2 ];
		vecs[ 0 ][ 0 ] = 1.0f / sampleSize;
		vecs[ 1 ][ 2 ] = 1.0f / sampleSize;
	}

	/* check for bogus axis */
	if ( faxis[ axisNum ] == 0.0f ) {
		Sys_FPrintf( SYS_WRN, "WARNING: ProjectSurfaceLightmap: Chose a 0 valued axis\n" );
		lm->w = lm->h = 0;
		return qfalse;
	}

	/* store the axis number in the lightmap */
	lm->axisNum = axisNum;

	/* walk the list of surfaces on this raw lightmap */
	for ( n = 0; n < lm->numLightSurfaces; n++ )
	{
		/* get surface */
		num2 = lightSurfaces[ lm->firstLightSurface + n ];
		ds2 = &bspDrawSurfaces[ num2 ];
		info2 = &surfaceInfos[ num2 ];
		verts = &yDrawVerts[ ds2->firstVert ];

		/* set the lightmap texture coordinates in yDrawVerts in [0, superSample * lm->customWidth] space */
		for ( i = 0; i < ds2->numVerts; i++ )
		{
			VectorSubtract( verts[ i ].xyz, origin, delta );
			s = DotProduct( delta, vecs[ 0 ] ) + 0.5f;
			t = DotProduct( delta, vecs[ 1 ] ) + 0.5f;
			verts[ i ].lightmap[ 0 ][ 0 ] = s * superSample;
			verts[ i ].lightmap[ 0 ][ 1 ] = t * superSample;

			if ( s > (float) lm->w || t > (float) lm->h ) {
				Sys_FPrintf( SYS_VRB, "WARNING: Lightmap texture coords out of range: S %1.4f > %3d || T %1.4f > %3d\n",
							 s, lm->w, t, lm->h );
			}
		}
	}

	/* get first drawsurface */
	num2 = lightSurfaces[ lm->firstLightSurface ];
	ds2 = &bspDrawSurfaces[ num2 ];
	info2 = &surfaceInfos[ num2 ];
	verts = &yDrawVerts[ ds2->firstVert ];

	/* calculate lightmap origin */
	if ( VectorLength( ds2->lightmapVecs[ 2 ] ) ) {
		VectorCopy( ds2->lightmapVecs[ 2 ], plane );
	}
	else{
		VectorCopy( lm->axis, plane );
	}
	plane[ 3 ] = DotProduct( verts[ 0 ].xyz, plane );

	VectorCopy( origin, lm->origin );
	d = DotProduct( lm->origin, plane ) - plane[ 3 ];
	d /= plane[ axisNum ];
	lm->origin[ axisNum ] -= d;

	/* legacy support */
	VectorCopy( lm->origin, ds->lightmapOrigin );

	/* for planar surfaces, create lightmap vectors for st->xyz conversion */
	if ( VectorLength( ds->lightmapVecs[ 2 ] ) || 1 ) {  /* ydnar: can't remember what exactly i was thinking here... */
		/* allocate space for the vectors */
		lm->vecs = safe_malloc( 3 * sizeof( vec3_t ) );
		memset( lm->vecs, 0, 3 * sizeof( vec3_t ) );
		VectorCopy( ds->lightmapVecs[ 2 ], lm->vecs[ 2 ] );

		/* project stepped lightmap blocks and subtract to get planevecs */
		for ( i = 0; i < 2; i++ )
		{
			len = VectorNormalize( vecs[ i ], normalized );
			VectorScale( normalized, ( 1.0 / len ), lm->vecs[ i ] );
			d = DotProduct( lm->vecs[ i ], plane );
			d /= plane[ axisNum ];
			lm->vecs[ i ][ axisNum ] -= d;
		}
	}
	else
	{
		/* lightmap vectors are useless on a non-planar surface */
		lm->vecs = NULL;
	}

	/* add to counts */
	if ( ds->surfaceType == MST_PATCH ) {
		numPatchesLightmapped++;
		if ( lm->plane != NULL ) {
			numPlanarPatchesLightmapped++;
		}
	}
	else
	{
		if ( lm->plane != NULL ) {
			numPlanarsLightmapped++;
		}
		else{
			numNonPlanarsLightmapped++;
		}
	}

	/* return */
	return qtrue;
}



/*
   CompareSurfaceInfo()
   compare function for qsort()
 */

static int CompareSurfaceInfo( const void *a, const void *b ){
	surfaceInfo_t   *aInfo, *bInfo;
	int i;


	/* get surface info */
	aInfo = &surfaceInfos[ *( (int*) a ) ];
	bInfo = &surfaceInfos[ *( (int*) b ) ];

	/* model first */
	if ( aInfo->model < bInfo->model ) {
		return 1;
	}
	else if ( aInfo->model > bInfo->model ) {
		return -1;
	}

	/* then lightmap status */
	if ( aInfo->hasLightmap < bInfo->hasLightmap ) {
		return 1;
	}
	else if ( aInfo->hasLightmap > bInfo->hasLightmap ) {
		return -1;
	}

	/* then lightmap sample size */
	if ( aInfo->sampleSize < bInfo->sampleSize ) {
		return 1;
	}
	else if ( aInfo->sampleSize > bInfo->sampleSize ) {
		return -1;
	}

	/* then lightmap axis */
	for ( i = 0; i < 3; i++ )
	{
		if ( aInfo->axis[ i ] < bInfo->axis[ i ] ) {
			return 1;
		}
		else if ( aInfo->axis[ i ] > bInfo->axis[ i ] ) {
			return -1;
		}
	}

	/* then plane */
	if ( aInfo->plane == NULL && bInfo->plane != NULL ) {
		return 1;
	}
	else if ( aInfo->plane != NULL && bInfo->plane == NULL ) {
		return -1;
	}
	else if ( aInfo->plane != NULL && bInfo->plane != NULL ) {
		for ( i = 0; i < 4; i++ )
		{
			if ( aInfo->plane[ i ] < bInfo->plane[ i ] ) {
				return 1;
			}
			else if ( aInfo->plane[ i ] > bInfo->plane[ i ] ) {
				return -1;
			}
		}
	}

	/* then position in world */
	for ( i = 0; i < 3; i++ )
	{
		if ( aInfo->mins[ i ] < bInfo->mins[ i ] ) {
			return 1;
		}
		else if ( aInfo->mins[ i ] > bInfo->mins[ i ] ) {
			return -1;
		}
	}

	/* these are functionally identical (this should almost never happen) */
	return 0;
}



/*
   SetupSurfaceLightmaps()
   allocates lightmaps for every surface in the bsp that needs one
   this depends on yDrawVerts being allocated
 */

void SetupSurfaceLightmaps( void ){
	int i, j, k, s,num, num2;
	bspModel_t          *model;
	bspLeaf_t           *leaf;
	bspDrawSurface_t    *ds, *ds2;
	surfaceInfo_t       *info, *info2;
	rawLightmap_t       *lm;
	qboolean added;
	vec3_t mapSize, entityOrigin;


	/* note it */
	Sys_FPrintf( SYS_VRB, "--- SetupSurfaceLightmaps ---\n" );

	/* determine supersample amount */
	if ( superSample < 1 ) {
		superSample = 1;
	}
	else if ( superSample > 8 ) {
		Sys_FPrintf( SYS_WRN, "WARNING: Insane supersampling amount (%d) detected.\n", superSample );
		superSample = 8;
	}

	/* clear map bounds */
	ClearBounds( mapMins, mapMaxs );

	/* allocate a list of surface clusters */
	numSurfaceClusters = 0;
	maxSurfaceClusters = numBSPLeafSurfaces;
	surfaceClusters = safe_malloc( maxSurfaceClusters * sizeof( *surfaceClusters ) );
	memset( surfaceClusters, 0, maxSurfaceClusters * sizeof( *surfaceClusters ) );

	/* allocate a list for per-surface info */
	surfaceInfos = safe_malloc( numBSPDrawSurfaces * sizeof( *surfaceInfos ) );
	memset( surfaceInfos, 0, numBSPDrawSurfaces * sizeof( *surfaceInfos ) );
	for ( i = 0; i < numBSPDrawSurfaces; i++ )
		surfaceInfos[ i ].childSurfaceNum = -1;

	/* allocate a list of surface indexes to be sorted */
	sortSurfaces = safe_malloc( numBSPDrawSurfaces * sizeof( int ) );
	memset( sortSurfaces, 0, numBSPDrawSurfaces * sizeof( int ) );

	/* walk each model in the bsp */
	for ( i = 0; i < numBSPModels; i++ )
	{
		/* get model */
		model = &bspModels[ i ];

		/* walk the list of surfaces in this model and fill out the info structs */
		for ( j = 0; j < model->numBSPSurfaces; j++ )
		{
			/* make surface index */
			num = model->firstBSPSurface + j;

			/* copy index to sort list */
			sortSurfaces[ num ] = num;

			/* get surface and info */
			ds = &bspDrawSurfaces[ num ];
			info = &surfaceInfos[ num ];

			/* set entity origin */
			if ( ds->numVerts > 0 ) {
				VectorSubtract( yDrawVerts[ ds->firstVert ].xyz, bspDrawVerts[ ds->firstVert ].xyz, entityOrigin );
			}
			else{
				VectorClear( entityOrigin );
			}

			/* basic setup */
			info->model = model;
			info->lm = NULL;
			info->plane = NULL;
			info->firstSurfaceCluster = numSurfaceClusters;

			/* get extra data */
			info->si = GetSurfaceExtraShaderInfo( num );
			if ( info->si == NULL ) {
				info->si = ShaderInfoForShader( bspShaders[ ds->shaderNum ].shader );
			}
			info->parentSurfaceNum = GetSurfaceExtraParentSurfaceNum( num );
			info->entityNum = GetSurfaceExtraEntityNum( num );
			info->castShadows = GetSurfaceExtraCastShadows( num );
			info->recvShadows = GetSurfaceExtraRecvShadows( num );
			info->sampleSize = GetSurfaceExtraSampleSize( num );
			info->longestCurve = GetSurfaceExtraLongestCurve( num );
			info->patchIterations = IterationsForCurve( info->longestCurve, patchSubdivisions );
			GetSurfaceExtraLightmapAxis( num, info->axis );

			/* mark parent */
			if ( info->parentSurfaceNum >= 0 ) {
				surfaceInfos[ info->parentSurfaceNum ].childSurfaceNum = j;
			}

			/* determine surface bounds */
			ClearBounds( info->mins, info->maxs );
			for ( k = 0; k < ds->numVerts; k++ )
			{
				AddPointToBounds( yDrawVerts[ ds->firstVert + k ].xyz, mapMins, mapMaxs );
				AddPointToBounds( yDrawVerts[ ds->firstVert + k ].xyz, info->mins, info->maxs );
			}

			/* find all the bsp clusters the surface falls into */
			for ( k = 0; k < numBSPLeafs; k++ )
			{
				/* get leaf */
				leaf = &bspLeafs[ k ];

				/* test bbox */
				if ( leaf->mins[ 0 ] > info->maxs[ 0 ] || leaf->maxs[ 0 ] < info->mins[ 0 ] ||
					 leaf->mins[ 1 ] > info->maxs[ 1 ] || leaf->maxs[ 1 ] < info->mins[ 1 ] ||
					 leaf->mins[ 2 ] > info->maxs[ 2 ] || leaf->maxs[ 2 ] < info->mins[ 2 ] ) {
					continue;
				}

				/* test leaf surfaces */
				for ( s = 0; s < leaf->numBSPLeafSurfaces; s++ )
				{
					if ( bspLeafSurfaces[ leaf->firstBSPLeafSurface + s ] == num ) {
						if ( numSurfaceClusters >= maxSurfaceClusters ) {
							Error( "maxSurfaceClusters exceeded" );
						}
						surfaceClusters[ numSurfaceClusters ] = leaf->cluster;
						numSurfaceClusters++;
						info->numSurfaceClusters++;
					}
				}
			}

			/* determine if surface is planar */
			if ( VectorLength( ds->lightmapVecs[ 2 ] ) > 0.0f ) {
				/* make a plane */
				info->plane = safe_malloc( 4 * sizeof( float ) );
				VectorCopy( ds->lightmapVecs[ 2 ], info->plane );
				info->plane[ 3 ] = DotProduct( yDrawVerts[ ds->firstVert ].xyz, info->plane );
			}

			/* determine if surface requires a lightmap */
			if ( ds->surfaceType == MST_TRIANGLE_SOUP ||
				 ds->surfaceType == MST_FOLIAGE ||
				 ( info->si->compileFlags & C_VERTEXLIT ) ) {
				numSurfsVertexLit++;
			}
			else
			{
				numSurfsLightmapped++;
				info->hasLightmap = qtrue;
			}
		}
	}

	/* find longest map distance */
	VectorSubtract( mapMaxs, mapMins, mapSize );
	maxMapDistance = VectorLength( mapSize );

	/* sort the surfaces info list */
	qsort( sortSurfaces, numBSPDrawSurfaces, sizeof( int ), CompareSurfaceInfo );

	/* allocate a list of surfaces that would go into raw lightmaps */
	numLightSurfaces = 0;
	lightSurfaces = safe_malloc( numSurfsLightmapped * sizeof( int ) );
	memset( lightSurfaces, 0, numSurfsLightmapped * sizeof( int ) );

	/* allocate a list of raw lightmaps */
	numRawSuperLuxels = 0;
	numRawLightmaps = 0;
	rawLightmaps = safe_malloc( numSurfsLightmapped * sizeof( *rawLightmaps ) );
	memset( rawLightmaps, 0, numSurfsLightmapped * sizeof( *rawLightmaps ) );

	/* walk the list of sorted surfaces */
	for ( i = 0; i < numBSPDrawSurfaces; i++ )
	{
		/* get info and attempt early out */
		num = sortSurfaces[ i ];
		ds = &bspDrawSurfaces[ num ];
		info = &surfaceInfos[ num ];
		if ( info->hasLightmap == qfalse || info->lm != NULL || info->parentSurfaceNum >= 0 ) {
			continue;
		}

		/* allocate a new raw lightmap */
		lm = &rawLightmaps[ numRawLightmaps ];
		numRawLightmaps++;

		/* set it up */
		lm->splotchFix = info->si->splotchFix;
		lm->firstLightSurface = numLightSurfaces;
		lm->numLightSurfaces = 0;
		lm->sampleSize = info->sampleSize;
		lm->actualSampleSize = info->sampleSize;
		lm->entityNum = info->entityNum;
		lm->recvShadows = info->recvShadows;
		lm->brightness = info->si->lmBrightness;
		lm->filterRadius = info->si->lmFilterRadius;
		VectorCopy( info->axis, lm->axis );
		lm->plane = info->plane;
		VectorCopy( info->mins, lm->mins );
		VectorCopy( info->maxs, lm->maxs );

		lm->customWidth = info->si->lmCustomWidth;
		lm->customHeight = info->si->lmCustomHeight;

		/* add the surface to the raw lightmap */
		AddSurfaceToRawLightmap( num, lm );
		info->lm = lm;

		/* do an exhaustive merge */
		added = qtrue;
		while ( added )
		{
			/* walk the list of surfaces again */
			added = qfalse;
			for ( j = i + 1; j < numBSPDrawSurfaces && lm->finished == qfalse; j++ )
			{
				/* get info and attempt early out */
				num2 = sortSurfaces[ j ];
				ds2 = &bspDrawSurfaces[ num2 ];
				info2 = &surfaceInfos[ num2 ];
				if ( info2->hasLightmap == qfalse || info2->lm != NULL ) {
					continue;
				}

				/* add the surface to the raw lightmap */
				if ( AddSurfaceToRawLightmap( num2, lm ) ) {
					info2->lm = lm;
					added = qtrue;
				}
				else
				{
					/* back up one */
					lm->numLightSurfaces--;
					numLightSurfaces--;
				}
			}
		}

		/* finish the lightmap and allocate the various buffers */
		FinishRawLightmap( lm );
	}

	/* allocate vertex luxel storage */
	for ( k = 0; k < MAX_LIGHTMAPS; k++ )
	{
		vertexLuxels[ k ] = safe_malloc( numBSPDrawVerts * VERTEX_LUXEL_SIZE * sizeof( float ) );
		memset( vertexLuxels[ k ], 0, numBSPDrawVerts * VERTEX_LUXEL_SIZE * sizeof( float ) );
		radVertexLuxels[ k ] = safe_malloc( numBSPDrawVerts * VERTEX_LUXEL_SIZE * sizeof( float ) );
		memset( radVertexLuxels[ k ], 0, numBSPDrawVerts * VERTEX_LUXEL_SIZE * sizeof( float ) );
	}

	/* emit some stats */
	Sys_FPrintf( SYS_VRB, "%9d surfaces\n", numBSPDrawSurfaces );
	Sys_FPrintf( SYS_VRB, "%9d raw lightmaps\n", numRawLightmaps );
	Sys_FPrintf( SYS_VRB, "%9d surfaces vertex lit\n", numSurfsVertexLit );
	Sys_FPrintf( SYS_VRB, "%9d surfaces lightmapped\n", numSurfsLightmapped );
	Sys_FPrintf( SYS_VRB, "%9d planar surfaces lightmapped\n", numPlanarsLightmapped );
	Sys_FPrintf( SYS_VRB, "%9d non-planar surfaces lightmapped\n", numNonPlanarsLightmapped );
	Sys_FPrintf( SYS_VRB, "%9d patches lightmapped\n", numPatchesLightmapped );
	Sys_FPrintf( SYS_VRB, "%9d planar patches lightmapped\n", numPlanarPatchesLightmapped );
}



/*
   StitchSurfaceLightmaps()
   stitches lightmap edges
   2002-11-20 update: use this func only for stitching nonplanar patch lightmap seams
 */

#define MAX_STITCH_CANDIDATES   32
#define MAX_STITCH_LUXELS       64

void StitchSurfaceLightmaps( void ){
	int i, j, x, y, x2, y2, *cluster, *cluster2,
		numStitched, numCandidates, numLuxels, f, fOld, start;
	rawLightmap_t   *lm, *a, *b, *c[ MAX_STITCH_CANDIDATES ];
	float           *luxel, *luxel2, *origin, *origin2, *normal, *normal2,
					 sampleSize, average[ 3 ], totalColor, ootc, *luxels[ MAX_STITCH_LUXELS ];


	/* disabled for now */
	return;

	/* note it */
	Sys_Printf( "--- StitchSurfaceLightmaps ---\n" );

	/* init pacifier */
	fOld = -1;
	start = I_FloatTime();

	/* walk the list of raw lightmaps */
	numStitched = 0;
	for ( i = 0; i < numRawLightmaps; i++ )
	{
		/* print pacifier */
		f = 10 * i / numRawLightmaps;
		if ( f != fOld ) {
			fOld = f;
			Sys_Printf( "%i...", f );
		}

		/* get lightmap a */
		a = &rawLightmaps[ i ];

		/* walk rest of lightmaps */
		numCandidates = 0;
		for ( j = i + 1; j < numRawLightmaps && numCandidates < MAX_STITCH_CANDIDATES; j++ )
		{
			/* get lightmap b */
			b = &rawLightmaps[ j ];

			/* test bounding box */
			if ( a->mins[ 0 ] > b->maxs[ 0 ] || a->maxs[ 0 ] < b->mins[ 0 ] ||
				 a->mins[ 1 ] > b->maxs[ 1 ] || a->maxs[ 1 ] < b->mins[ 1 ] ||
				 a->mins[ 2 ] > b->maxs[ 2 ] || a->maxs[ 2 ] < b->mins[ 2 ] ) {
				continue;
			}

			/* add candidate */
			c[ numCandidates++ ] = b;
		}

		/* walk luxels */
		for ( y = 0; y < a->sh; y++ )
		{
			for ( x = 0; x < a->sw; x++ )
			{
				/* ignore unmapped/unlit luxels */
				lm = a;
				cluster = SUPER_CLUSTER( x, y );
				if ( *cluster == CLUSTER_UNMAPPED ) {
					continue;
				}
				luxel = SUPER_LUXEL( 0, x, y );
				if ( luxel[ 3 ] <= 0.0f ) {
					continue;
				}

				/* get particulars */
				origin = SUPER_ORIGIN( x, y );
				normal = SUPER_NORMAL( x, y );

				/* walk candidate list */
				for ( j = 0; j < numCandidates; j++ )
				{
					/* get candidate */
					b = c[ j ];
					lm = b;

					/* set samplesize to the smaller of the pair */
					sampleSize = 0.5f * ( a->actualSampleSize < b->actualSampleSize ? a->actualSampleSize : b->actualSampleSize );

					/* test bounding box */
					if ( origin[ 0 ] < ( b->mins[ 0 ] - sampleSize ) || ( origin[ 0 ] > b->maxs[ 0 ] + sampleSize ) ||
						 origin[ 1 ] < ( b->mins[ 1 ] - sampleSize ) || ( origin[ 1 ] > b->maxs[ 1 ] + sampleSize ) ||
						 origin[ 2 ] < ( b->mins[ 2 ] - sampleSize ) || ( origin[ 2 ] > b->maxs[ 2 ] + sampleSize ) ) {
						continue;
					}

					/* walk candidate luxels */
					VectorClear( average );
					numLuxels = 0;
					totalColor = 0.0f;
					for ( y2 = 0; y2 < b->sh && numLuxels < MAX_STITCH_LUXELS; y2++ )
					{
						for ( x2 = 0; x2 < b->sw && numLuxels < MAX_STITCH_LUXELS; x2++ )
						{
							/* ignore same luxels */
							if ( a == b && abs( x - x2 ) <= 1 && abs( y - y2 ) <= 1 ) {
								continue;
							}

							/* ignore unmapped/unlit luxels */
							cluster2 = SUPER_CLUSTER( x2, y2 );
							if ( *cluster2 == CLUSTER_UNMAPPED ) {
								continue;
							}
							luxel2 = SUPER_LUXEL( 0, x2, y2 );
							if ( luxel2[ 3 ] <= 0.0f ) {
								continue;
							}

							/* get particulars */
							origin2 = SUPER_ORIGIN( x2, y2 );
							normal2 = SUPER_NORMAL( x2, y2 );

							/* test normal */
							if ( DotProduct( normal, normal2 ) < 0.5f ) {
								continue;
							}

							/* test bounds */
							if ( fabs( origin[ 0 ] - origin2[ 0 ] ) > sampleSize ||
								 fabs( origin[ 1 ] - origin2[ 1 ] ) > sampleSize ||
								 fabs( origin[ 2 ] - origin2[ 2 ] ) > sampleSize ) {
								continue;
							}

							/* add luxel */
							//%	VectorSet( luxel2, 255, 0, 255 );
							luxels[ numLuxels++ ] = luxel2;
							VectorAdd( average, luxel2, average );
							totalColor += luxel2[ 3 ];
						}
					}

					/* early out */
					if ( numLuxels == 0 ) {
						continue;
					}

					/* scale average */
					ootc = 1.0f / totalColor;
					VectorScale( average, ootc, luxel );
					luxel[ 3 ] = 1.0f;
					numStitched++;
				}
			}
		}
	}

	/* emit statistics */
	Sys_Printf( " (%i)\n", (int) ( I_FloatTime() - start ) );
	Sys_FPrintf( SYS_VRB, "%9d luxels stitched\n", numStitched );
}



/*
   CompareBSPLuxels()
   compares two surface lightmaps' bsp luxels, ignoring occluded luxels
 */

#define SOLID_EPSILON       0.0625
#define LUXEL_TOLERANCE     0.0025
#define LUXEL_COLOR_FRAC    0.001302083 /* 1 / 3 / 256 */

static qboolean CompareBSPLuxels( rawLightmap_t *a, int aNum, rawLightmap_t *b, int bNum ){
	rawLightmap_t   *lm;
	int x, y;
	double delta, total, rd, gd, bd;
	float           *aLuxel, *bLuxel;


	/* styled lightmaps will never be collapsed to non-styled lightmaps when there is _minlight */
	if ( ( minLight[ 0 ] || minLight[ 1 ] || minLight[ 2 ] ) &&
		 ( ( aNum == 0 && bNum != 0 ) || ( aNum != 0 && bNum == 0 ) ) ) {
		return qfalse;
	}

	/* basic tests */
	if ( a->customWidth != b->customWidth || a->customHeight != b->customHeight ||
		 a->brightness != b->brightness ||
		 a->solid[ aNum ] != b->solid[ bNum ] ||
		 a->bspLuxels[ aNum ] == NULL || b->bspLuxels[ bNum ] == NULL ) {
		return qfalse;
	}

	/* compare solid color lightmaps */
	if ( a->solid[ aNum ] && b->solid[ bNum ] ) {
		/* get deltas */
		rd = fabs( a->solidColor[ aNum ][ 0 ] - b->solidColor[ bNum ][ 0 ] );
		gd = fabs( a->solidColor[ aNum ][ 1 ] - b->solidColor[ bNum ][ 1 ] );
		bd = fabs( a->solidColor[ aNum ][ 2 ] - b->solidColor[ bNum ][ 2 ] );

		/* compare color */
		if ( rd > SOLID_EPSILON || gd > SOLID_EPSILON || bd > SOLID_EPSILON ) {
			return qfalse;
		}

		/* okay */
		return qtrue;
	}

	/* compare nonsolid lightmaps */
	if ( a->w != b->w || a->h != b->h ) {
		return qfalse;
	}

	/* compare luxels */
	delta = 0.0;
	total = 0.0;
	for ( y = 0; y < a->h; y++ )
	{
		for ( x = 0; x < a->w; x++ )
		{
			/* increment total */
			total += 1.0;

			/* get luxels */
			lm = a; aLuxel = BSP_LUXEL( aNum, x, y );
			lm = b; bLuxel = BSP_LUXEL( bNum, x, y );

			/* ignore unused luxels */
			if ( aLuxel[ 0 ] < 0 || bLuxel[ 0 ] < 0 ) {
				continue;
			}

			/* get deltas */
			rd = fabs( aLuxel[ 0 ] - bLuxel[ 0 ] );
			gd = fabs( aLuxel[ 1 ] - bLuxel[ 1 ] );
			bd = fabs( aLuxel[ 2 ] - bLuxel[ 2 ] );

			/* 2003-09-27: compare individual luxels */
			if ( rd > 3.0 || gd > 3.0 || bd > 3.0 ) {
				return qfalse;
			}

			/* compare (fixme: take into account perceptual differences) */
			delta += rd * LUXEL_COLOR_FRAC;
			delta += gd * LUXEL_COLOR_FRAC;
			delta += bd * LUXEL_COLOR_FRAC;

			/* is the change too high? */
			if ( total > 0.0 && ( ( delta / total ) > LUXEL_TOLERANCE ) ) {
				return qfalse;
			}
		}
	}

	/* made it this far, they must be identical (or close enough) */
	return qtrue;
}



/*
   MergeBSPLuxels()
   merges two surface lightmaps' bsp luxels, overwriting occluded luxels
 */

static qboolean MergeBSPLuxels( rawLightmap_t *a, int aNum, rawLightmap_t *b, int bNum ){
	rawLightmap_t   *lm;
	int x, y;
	float luxel[ 3 ], *aLuxel, *bLuxel;


	/* basic tests */
	if ( a->customWidth != b->customWidth || a->customHeight != b->customHeight ||
		 a->brightness != b->brightness ||
		 a->solid[ aNum ] != b->solid[ bNum ] ||
		 a->bspLuxels[ aNum ] == NULL || b->bspLuxels[ bNum ] == NULL ) {
		return qfalse;
	}

	/* compare solid lightmaps */
	if ( a->solid[ aNum ] && b->solid[ bNum ] ) {
		/* average */
		VectorAdd( a->solidColor[ aNum ], b->solidColor[ bNum ], luxel );
		VectorScale( luxel, 0.5f, luxel );

		/* copy to both */
		VectorCopy( luxel, a->solidColor[ aNum ] );
		VectorCopy( luxel, b->solidColor[ bNum ] );

		/* return to sender */
		return qtrue;
	}

	/* compare nonsolid lightmaps */
	if ( a->w != b->w || a->h != b->h ) {
		return qfalse;
	}

	/* merge luxels */
	for ( y = 0; y < a->h; y++ )
	{
		for ( x = 0; x < a->w; x++ )
		{
			/* get luxels */
			lm = a; aLuxel = BSP_LUXEL( aNum, x, y );
			lm = b; bLuxel = BSP_LUXEL( bNum, x, y );

			/* handle occlusion mismatch */
			if ( aLuxel[ 0 ] < 0.0f ) {
				VectorCopy( bLuxel, aLuxel );
			}
			else if ( bLuxel[ 0 ] < 0.0f ) {
				VectorCopy( aLuxel, bLuxel );
			}
			else
			{
				/* average */
				VectorAdd( aLuxel, bLuxel, luxel );
				VectorScale( luxel, 0.5f, luxel );

				/* debugging code */
				//%	luxel[ 2 ] += 64.0f;

				/* copy to both */
				VectorCopy( luxel, aLuxel );
				VectorCopy( luxel, bLuxel );
			}
		}
	}

	/* done */
	return qtrue;
}



/*
   ApproximateLuxel()
   determines if a single luxel is can be approximated with the interpolated vertex rgba
 */

static qboolean ApproximateLuxel( rawLightmap_t *lm, bspDrawVert_t *dv ){
	int i, x, y, d, lightmapNum;
	float   *luxel;
	vec3_t color, vertexColor;
	byte cb[ 4 ], vcb[ 4 ];


	/* find luxel xy coords */
	x = dv->lightmap[ 0 ][ 0 ] / superSample;
	y = dv->lightmap[ 0 ][ 1 ] / superSample;
	if ( x < 0 ) {
		x = 0;
	}
	else if ( x >= lm->w ) {
		x = lm->w - 1;
	}
	if ( y < 0 ) {
		y = 0;
	}
	else if ( y >= lm->h ) {
		y = lm->h - 1;
	}

	/* walk list */
	for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
	{
		/* early out */
		if ( lm->styles[ lightmapNum ] == LS_NONE ) {
			continue;
		}

		/* get luxel */
		luxel = BSP_LUXEL( lightmapNum, x, y );

		/* ignore occluded luxels */
		if ( luxel[ 0 ] < 0.0f || luxel[ 1 ] < 0.0f || luxel[ 2 ] < 0.0f ) {
			return qtrue;
		}

		/* copy, set min color and compare */
		VectorCopy( luxel, color );
		VectorCopy( dv->color[ 0 ], vertexColor );

		/* styles are not affected by minlight */
		if ( lightmapNum == 0 ) {
			for ( i = 0; i < 3; i++ )
			{
				/* set min color */
				if ( color[ i ] < minLight[ i ] ) {
					color[ i ] = minLight[ i ];
				}
				if ( vertexColor[ i ] < minLight[ i ] ) { /* note NOT minVertexLight */
					vertexColor[ i ] = minLight[ i ];
				}
			}
		}

		/* set to bytes */
		ColorToBytes( color, cb, 1.0f );
		ColorToBytes( vertexColor, vcb, 1.0f );

		/* compare */
		for ( i = 0; i < 3; i++ )
		{
			d = cb[ i ] - vcb[ i ];
			if ( d < 0 ) {
				d *= -1;
			}
			if ( d > approximateTolerance ) {
				return qfalse;
			}
		}
	}

	/* close enough for the girls i date */
	return qtrue;
}



/*
   ApproximateTriangle()
   determines if a single triangle can be approximated with vertex rgba
 */

static qboolean ApproximateTriangle_r( rawLightmap_t *lm, bspDrawVert_t *dv[ 3 ] ){
	bspDrawVert_t mid, *dv2[ 3 ];
	int max;


	/* approximate the vertexes */
	if ( ApproximateLuxel( lm, dv[ 0 ] ) == qfalse ) {
		return qfalse;
	}
	if ( ApproximateLuxel( lm, dv[ 1 ] ) == qfalse ) {
		return qfalse;
	}
	if ( ApproximateLuxel( lm, dv[ 2 ] ) == qfalse ) {
		return qfalse;
	}

	/* subdivide calc */
	{
		int i;
		float dx, dy, dist, maxDist;


		/* find the longest edge and split it */
		max = -1;
		maxDist = 0;
		for ( i = 0; i < 3; i++ )
		{
			dx = dv[ i ]->lightmap[ 0 ][ 0 ] - dv[ ( i + 1 ) % 3 ]->lightmap[ 0 ][ 0 ];
			dy = dv[ i ]->lightmap[ 0 ][ 1 ] - dv[ ( i + 1 ) % 3 ]->lightmap[ 0 ][ 1 ];
			dist = sqrt( ( dx * dx ) + ( dy * dy ) );
			if ( dist > maxDist ) {
				maxDist = dist;
				max = i;
			}
		}

		/* try to early out */
		if ( i < 0 || maxDist < subdivideThreshold ) {
			return qtrue;
		}
	}

	/* split the longest edge and map it */
	LerpDrawVert( dv[ max ], dv[ ( max + 1 ) % 3 ], &mid );
	if ( ApproximateLuxel( lm, &mid ) == qfalse ) {
		return qfalse;
	}

	/* recurse to first triangle */
	VectorCopy( dv, dv2 );
	dv2[ max ] = &mid;
	if ( ApproximateTriangle_r( lm, dv2 ) == qfalse ) {
		return qfalse;
	}

	/* recurse to second triangle */
	VectorCopy( dv, dv2 );
	dv2[ ( max + 1 ) % 3 ] = &mid;
	return ApproximateTriangle_r( lm, dv2 );
}



/*
   ApproximateLightmap()
   determines if a raw lightmap can be approximated sufficiently with vertex colors
 */

static qboolean ApproximateLightmap( rawLightmap_t *lm ){
	int n, num, i, x, y, pw[ 5 ], r;
	bspDrawSurface_t    *ds;
	surfaceInfo_t       *info;
	mesh_t src, *subdivided, *mesh;
	bspDrawVert_t       *verts, *dv[ 3 ];
	qboolean approximated;


	/* approximating? */
	if ( approximateTolerance <= 0 ) {
		return qfalse;
	}

	/* test for jmonroe */
	#if 0
	/* don't approx lightmaps with styled twins */
	if ( lm->numStyledTwins > 0 ) {
		return qfalse;
	}

	/* don't approx lightmaps with styles */
	for ( i = 1; i < MAX_LIGHTMAPS; i++ )
	{
		if ( lm->styles[ i ] != LS_NONE ) {
			return qfalse;
		}
	}
	#endif

	/* assume reduced until shadow detail is found */
	approximated = qtrue;

	/* walk the list of surfaces on this raw lightmap */
	for ( n = 0; n < lm->numLightSurfaces; n++ )
	{
		/* get surface */
		num = lightSurfaces[ lm->firstLightSurface + n ];
		ds = &bspDrawSurfaces[ num ];
		info = &surfaceInfos[ num ];

		/* assume not-reduced initially */
		info->approximated = qfalse;

		/* bail if lightmap doesn't match up */
		if ( info->lm != lm ) {
			continue;
		}

		/* bail if not vertex lit */
		if ( info->si->noVertexLight ) {
			continue;
		}

		/* assume that surfaces whose bounding boxes is smaller than 2x samplesize will be forced to vertex */
		if ( ( info->maxs[ 0 ] - info->mins[ 0 ] ) <= ( 2.0f * info->sampleSize ) &&
			 ( info->maxs[ 1 ] - info->mins[ 1 ] ) <= ( 2.0f * info->sampleSize ) &&
			 ( info->maxs[ 2 ] - info->mins[ 2 ] ) <= ( 2.0f * info->sampleSize ) ) {
			info->approximated = qtrue;
			numSurfsVertexForced++;
			continue;
		}

		/* handle the triangles */
		switch ( ds->surfaceType )
		{
		case MST_PLANAR:
			/* get verts */
			verts = yDrawVerts + ds->firstVert;

			/* map the triangles */
			info->approximated = qtrue;
			for ( i = 0; i < ds->numIndexes && info->approximated; i += 3 )
			{
				dv[ 0 ] = &verts[ bspDrawIndexes[ ds->firstIndex + i ] ];
				dv[ 1 ] = &verts[ bspDrawIndexes[ ds->firstIndex + i + 1 ] ];
				dv[ 2 ] = &verts[ bspDrawIndexes[ ds->firstIndex + i + 2 ] ];
				info->approximated = ApproximateTriangle_r( lm, dv );
			}
			break;

		case MST_PATCH:
			/* make a mesh from the drawsurf */
			src.width = ds->patchWidth;
			src.height = ds->patchHeight;
			src.verts = &yDrawVerts[ ds->firstVert ];
			//%	subdivided = SubdivideMesh( src, 8, 512 );
			subdivided = SubdivideMesh2( src, info->patchIterations );

			/* fit it to the curve and remove colinear verts on rows/columns */
			PutMeshOnCurve( *subdivided );
			mesh = RemoveLinearMeshColumnsRows( subdivided );
			FreeMesh( subdivided );

			/* get verts */
			verts = mesh->verts;

			/* map the mesh quads */
			info->approximated = qtrue;
			for ( y = 0; y < ( mesh->height - 1 ) && info->approximated; y++ )
			{
				for ( x = 0; x < ( mesh->width - 1 ) && info->approximated; x++ )
				{
					/* set indexes */
					pw[ 0 ] = x + ( y * mesh->width );
					pw[ 1 ] = x + ( ( y + 1 ) * mesh->width );
					pw[ 2 ] = x + 1 + ( ( y + 1 ) * mesh->width );
					pw[ 3 ] = x + 1 + ( y * mesh->width );
					pw[ 4 ] = x + ( y * mesh->width );      /* same as pw[ 0 ] */

					/* set radix */
					r = ( x + y ) & 1;

					/* get drawverts and map first triangle */
					dv[ 0 ] = &verts[ pw[ r + 0 ] ];
					dv[ 1 ] = &verts[ pw[ r + 1 ] ];
					dv[ 2 ] = &verts[ pw[ r + 2 ] ];
					info->approximated = ApproximateTriangle_r( lm, dv );

					/* get drawverts and map second triangle */
					dv[ 0 ] = &verts[ pw[ r + 0 ] ];
					dv[ 1 ] = &verts[ pw[ r + 2 ] ];
					dv[ 2 ] = &verts[ pw[ r + 3 ] ];
					if ( info->approximated ) {
						info->approximated = ApproximateTriangle_r( lm, dv );
					}
				}
			}

			/* free the mesh */
			FreeMesh( mesh );
			break;

		default:
			break;
		}

		/* reduced? */
		if ( info->approximated == qfalse ) {
			approximated = qfalse;
		}
		else{
			numSurfsVertexApproximated++;
		}
	}

	/* return */
	return approximated;
}



/*
   TestOutLightmapStamp()
   tests a stamp on a given lightmap for validity
 */

static qboolean TestOutLightmapStamp( rawLightmap_t *lm, int lightmapNum, outLightmap_t *olm, int x, int y ){
	int sx, sy, ox, oy, offset;
	float       *luxel;


	/* bounds check */
	if ( x < 0 || y < 0 || ( x + lm->w ) > olm->customWidth || ( y + lm->h ) > olm->customHeight ) {
		return qfalse;
	}

	/* solid lightmaps test a 1x1 stamp */
	if ( lm->solid[ lightmapNum ] ) {
		offset = ( y * olm->customWidth ) + x;
		if ( olm->lightBits[ offset >> 3 ] & ( 1 << ( offset & 7 ) ) ) {
			return qfalse;
		}
		return qtrue;
	}

	/* test the stamp */
	for ( sy = 0; sy < lm->h; sy++ )
	{
		for ( sx = 0; sx < lm->w; sx++ )
		{
			/* get luxel */
			luxel = BSP_LUXEL( lightmapNum, sx, sy );
			if ( luxel[ 0 ] < 0.0f ) {
				continue;
			}

			/* get bsp lightmap coords and test */
			ox = x + sx;
			oy = y + sy;
			offset = ( oy * olm->customWidth ) + ox;
			if ( olm->lightBits[ offset >> 3 ] & ( 1 << ( offset & 7 ) ) ) {
				return qfalse;
			}
		}
	}

	/* stamp is empty */
	return qtrue;
}



/*
   SetupOutLightmap()
   sets up an output lightmap
 */

static void SetupOutLightmap( rawLightmap_t *lm, outLightmap_t *olm ){
	/* dummy check */
	if ( lm == NULL || olm == NULL ) {
		return;
	}

	/* is this a "normal" bsp-stored lightmap? */
	if ( ( lm->customWidth == game->lightmapSize && lm->customHeight == game->lightmapSize ) || externalLightmaps ) {
		olm->lightmapNum = numBSPLightmaps;
		numBSPLightmaps++;

		/* lightmaps are interleaved with light direction maps */
		if ( deluxemap ) {
			numBSPLightmaps++;
		}
	}
	else{
		olm->lightmapNum = -3;
	}

	/* set external lightmap number */
	olm->extLightmapNum = -1;

	/* set it up */
	olm->numLightmaps = 0;
	olm->customWidth = lm->customWidth;
	olm->customHeight = lm->customHeight;
	olm->freeLuxels = olm->customWidth * olm->customHeight;
	olm->numShaders = 0;

	/* allocate buffers */
	olm->lightBits = safe_malloc( ( olm->customWidth * olm->customHeight / 8 ) + 8 );
	memset( olm->lightBits, 0, ( olm->customWidth * olm->customHeight / 8 ) + 8 );
	olm->bspLightBytes = safe_malloc( olm->customWidth * olm->customHeight * 3 );
	memset( olm->bspLightBytes, 0, olm->customWidth * olm->customHeight * 3 );
	if ( deluxemap ) {
		olm->bspDirBytes = safe_malloc( olm->customWidth * olm->customHeight * 3 );
		memset( olm->bspDirBytes, 0, olm->customWidth * olm->customHeight * 3 );
	}
}



/*
   FindOutLightmaps()
   for a given surface lightmap, find output lightmap pages and positions for it
 */

static void FindOutLightmaps( rawLightmap_t *lm ){
	int i, j, lightmapNum, xMax, yMax, x, y, sx, sy, ox, oy, offset, temp;
	outLightmap_t       *olm;
	surfaceInfo_t       *info;
	float               *luxel, *deluxel;
	vec3_t color, direction;
	byte                *pixel;
	qboolean ok;


	/* set default lightmap number (-3 = LIGHTMAP_BY_VERTEX) */
	for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
		lm->outLightmapNums[ lightmapNum ] = -3;

	/* can this lightmap be approximated with vertex color? */
	if ( ApproximateLightmap( lm ) ) {
		return;
	}

	/* walk list */
	for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
	{
		/* early out */
		if ( lm->styles[ lightmapNum ] == LS_NONE ) {
			continue;
		}

		/* don't store twinned lightmaps */
		if ( lm->twins[ lightmapNum ] != NULL ) {
			continue;
		}

		/* if this is a styled lightmap, try some normalized locations first */
		ok = qfalse;
		if ( lightmapNum > 0 && outLightmaps != NULL ) {
			/* loop twice */
			for ( j = 0; j < 2; j++ )
			{
				/* try identical position */
				for ( i = 0; i < numOutLightmaps; i++ )
				{
					/* get the output lightmap */
					olm = &outLightmaps[ i ];

					/* simple early out test */
					if ( olm->freeLuxels < lm->used ) {
						continue;
					}

					/* don't store non-custom raw lightmaps on custom bsp lightmaps */
					if ( olm->customWidth != lm->customWidth ||
						 olm->customHeight != lm->customHeight ) {
						continue;
					}

					/* try identical */
					if ( j == 0 ) {
						x = lm->lightmapX[ 0 ];
						y = lm->lightmapY[ 0 ];
						ok = TestOutLightmapStamp( lm, lightmapNum, olm, x, y );
					}

					/* try shifting */
					else
					{
						for ( sy = -1; sy <= 1; sy++ )
						{
							for ( sx = -1; sx <= 1; sx++ )
							{
								x = lm->lightmapX[ 0 ] + sx * ( olm->customWidth >> 1 );  //%	lm->w;
								y = lm->lightmapY[ 0 ] + sy * ( olm->customHeight >> 1 ); //%	lm->h;
								ok = TestOutLightmapStamp( lm, lightmapNum, olm, x, y );

								if ( ok ) {
									break;
								}
							}

							if ( ok ) {
								break;
							}
						}
					}

					if ( ok ) {
						break;
					}
				}

				if ( ok ) {
					break;
				}
			}
		}

		/* try normal placement algorithm */
		if ( ok == qfalse ) {
			/* reset origin */
			x = 0;
			y = 0;

			/* walk the list of lightmap pages */
			for ( i = 0; i < numOutLightmaps; i++ )
			{
				/* get the output lightmap */
				olm = &outLightmaps[ i ];

				/* simple early out test */
				if ( olm->freeLuxels < lm->used ) {
					continue;
				}

				/* don't store non-custom raw lightmaps on custom bsp lightmaps */
				if ( olm->customWidth != lm->customWidth ||
					 olm->customHeight != lm->customHeight ) {
					continue;
				}

				/* set maxs */
				if ( lm->solid[ lightmapNum ] ) {
					xMax = olm->customWidth;
					yMax = olm->customHeight;
				}
				else
				{
					xMax = ( olm->customWidth - lm->w ) + 1;
					yMax = ( olm->customHeight - lm->h ) + 1;
				}

				/* walk the origin around the lightmap */
				for ( y = 0; y < yMax; y++ )
				{
					for ( x = 0; x < xMax; x++ )
					{
						/* find a fine tract of lauhnd */
						ok = TestOutLightmapStamp( lm, lightmapNum, olm, x, y );

						if ( ok ) {
							break;
						}
					}

					if ( ok ) {
						break;
					}
				}

				if ( ok ) {
					break;
				}

				/* reset x and y */
				x = 0;
				y = 0;
			}
		}

		/* no match? */
		if ( ok == qfalse ) {
			/* allocate two new output lightmaps */
			numOutLightmaps += 2;
			olm = safe_malloc( numOutLightmaps * sizeof( outLightmap_t ) );
			if ( !olm )
			{
				Error( "FindOutLightmaps: Failed to allocate memory.\n" );
			}

			if ( outLightmaps != NULL && numOutLightmaps > 2 ) {
				memcpy( olm, outLightmaps, ( numOutLightmaps - 2 ) * sizeof( outLightmap_t ) );
				free( outLightmaps );
			}
			outLightmaps = olm;

			/* initialize both out lightmaps */
			SetupOutLightmap( lm, &outLightmaps[ numOutLightmaps - 2 ] );
			SetupOutLightmap( lm, &outLightmaps[ numOutLightmaps - 1 ] );

			/* set out lightmap */
			i = numOutLightmaps - 2;
			olm = &outLightmaps[ i ];

			/* set stamp xy origin to the first surface lightmap */
			if ( lightmapNum > 0 ) {
				x = lm->lightmapX[ 0 ];
				y = lm->lightmapY[ 0 ];
			}
		}

		/* if this is a style-using lightmap, it must be exported */
		if ( lightmapNum > 0 && game->load != LoadRBSPFile ) {
			olm->extLightmapNum = 0;
		}

		/* add the surface lightmap to the bsp lightmap */
		lm->outLightmapNums[ lightmapNum ] = i;
		lm->lightmapX[ lightmapNum ] = x;
		lm->lightmapY[ lightmapNum ] = y;
		olm->numLightmaps++;

		/* add shaders */
		for ( i = 0; i < lm->numLightSurfaces; i++ )
		{
			/* get surface info */
			info = &surfaceInfos[ lightSurfaces[ lm->firstLightSurface + i ] ];

			/* test for shader */
			for ( j = 0; j < olm->numShaders; j++ )
			{
				if ( olm->shaders[ j ] == info->si ) {
					break;
				}
			}

			/* if it doesn't exist, add it */
			if ( j >= olm->numShaders && olm->numShaders < MAX_LIGHTMAP_SHADERS ) {
				olm->shaders[ olm->numShaders ] = info->si;
				olm->numShaders++;
				numLightmapShaders++;
			}
		}

		/* set maxs */
		if ( lm->solid[ lightmapNum ] ) {
			xMax = 1;
			yMax = 1;
		}
		else
		{
			xMax = lm->w;
			yMax = lm->h;
		}

		/* mark the bits used */
		for ( y = 0; y < yMax; y++ )
		{
			for ( x = 0; x < xMax; x++ )
			{
				/* get luxel */
				luxel = BSP_LUXEL( lightmapNum, x, y );
				deluxel = BSP_DELUXEL( x, y );
				if ( luxel[ 0 ] < 0.0f && !lm->solid[ lightmapNum ] ) {
					continue;
				}

				/* set minimum light */
				if ( lm->solid[ lightmapNum ] ) {
					if ( debug ) {
						VectorSet( color, 255.0f, 0.0f, 0.0f );
					}
					else{
						VectorCopy( lm->solidColor[ lightmapNum ], color );
					}
				}
				else{
					VectorCopy( luxel, color );
				}

				/* styles are not affected by minlight */
				if ( lightmapNum == 0 ) {
					for ( i = 0; i < 3; i++ )
					{
						if ( color[ i ] < minLight[ i ] ) {
							color[ i ] = minLight[ i ];
						}
					}
				}

				/* get bsp lightmap coords  */
				ox = x + lm->lightmapX[ lightmapNum ];
				oy = y + lm->lightmapY[ lightmapNum ];
				offset = ( oy * olm->customWidth ) + ox;

				/* flag pixel as used */
				olm->lightBits[ offset >> 3 ] |= ( 1 << ( offset & 7 ) );
				olm->freeLuxels--;

				/* store color */
				pixel = olm->bspLightBytes + ( ( ( oy * olm->customWidth ) + ox ) * 3 );
				ColorToBytes( color, pixel, lm->brightness );

				/* store direction */
				if ( deluxemap ) {
					/* normalize average light direction */
					if ( VectorNormalize( deluxel, direction ) ) {
						/* encode [-1,1] in [0,255] */
						pixel = olm->bspDirBytes + ( ( ( oy * olm->customWidth ) + ox ) * 3 );
						for ( i = 0; i < 3; i++ )
						{
							temp = ( direction[ i ] + 1.0f ) * 127.5f;
							if ( temp < 0 ) {
								pixel[ i ] = 0;
							}
							else if ( temp > 255 ) {
								pixel[ i ] = 255;
							}
							else{
								pixel[ i ] = temp;
							}
						}
					}
				}
			}
		}
	}
}



/*
   CompareRawLightmap()
   compare function for qsort()
 */

static int CompareRawLightmap( const void *a, const void *b ){
	rawLightmap_t   *alm, *blm;
	surfaceInfo_t   *aInfo, *bInfo;
	int i, min, diff;


	/* get lightmaps */
	alm = &rawLightmaps[ *( (int*) a ) ];
	blm = &rawLightmaps[ *( (int*) b ) ];

	/* get min number of surfaces */
	min = ( alm->numLightSurfaces < blm->numLightSurfaces ? alm->numLightSurfaces : blm->numLightSurfaces );

	/* iterate */
	for ( i = 0; i < min; i++ )
	{
		/* get surface info */
		aInfo = &surfaceInfos[ lightSurfaces[ alm->firstLightSurface + i ] ];
		bInfo = &surfaceInfos[ lightSurfaces[ blm->firstLightSurface + i ] ];

		/* compare shader names */
		diff = strcmp( aInfo->si->shader, bInfo->si->shader );
		if ( diff != 0 ) {
			return diff;
		}
	}

	/* test style count */
	diff = 0;
	for ( i = 0; i < MAX_LIGHTMAPS; i++ )
		diff += blm->styles[ i ] - alm->styles[ i ];
	if ( diff ) {
		return diff;
	}

	/* compare size */
	diff = ( blm->w * blm->h ) - ( alm->w * alm->h );
	if ( diff != 0 ) {
		return diff;
	}

	/* must be equivalent */
	return 0;
}



/*
   StoreSurfaceLightmaps()
   stores the surface lightmaps into the bsp as byte rgb triplets
 */

void StoreSurfaceLightmaps( void ){
	int i, j, k, x, y, lx, ly, sx, sy, *cluster, mappedSamples;
	int style, size, lightmapNum, lightmapNum2;
	float               *normal, *luxel, *bspLuxel, *bspLuxel2, *radLuxel, samples, occludedSamples;
	vec3_t sample, occludedSample, dirSample, colorMins, colorMaxs;
	float               *deluxel, *bspDeluxel, *bspDeluxel2;
	byte                *lb;
	int numUsed, numTwins, numTwinLuxels, numStored;
	float lmx, lmy, efficiency;
	vec3_t color;
	bspDrawSurface_t    *ds, *parent, dsTemp;
	surfaceInfo_t       *info;
	rawLightmap_t       *lm, *lm2;
	outLightmap_t       *olm;
	bspDrawVert_t       *dv, *ydv, *dvParent;
	char dirname[ 1024 ], filename[ 1024 ];
	shaderInfo_t        *csi;
	char lightmapName[ 128 ];
	char                *rgbGenValues[ 256 ];
	char                *alphaGenValues[ 256 ];


	/* note it */
	Sys_Printf( "--- StoreSurfaceLightmaps ---\n" );

	/* setup */
	strcpy( dirname, source );
	StripExtension( dirname );
	memset( rgbGenValues, 0, sizeof( rgbGenValues ) );
	memset( alphaGenValues, 0, sizeof( alphaGenValues ) );

	/* -----------------------------------------------------------------
	   average the sampled luxels into the bsp luxels
	   ----------------------------------------------------------------- */

	/* note it */
	Sys_FPrintf( SYS_VRB, "Subsampling..." );

	/* walk the list of raw lightmaps */
	numUsed = 0;
	numTwins = 0;
	numTwinLuxels = 0;
	numSolidLightmaps = 0;
	for ( i = 0; i < numRawLightmaps; i++ )
	{
		/* get lightmap */
		lm = &rawLightmaps[ i ];

		/* walk individual lightmaps */
		for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
		{
			/* early outs */
			if ( lm->superLuxels[ lightmapNum ] == NULL ) {
				continue;
			}

			/* allocate bsp luxel storage */
			if ( lm->bspLuxels[ lightmapNum ] == NULL ) {
				size = lm->w * lm->h * BSP_LUXEL_SIZE * sizeof( float );
				lm->bspLuxels[ lightmapNum ] = safe_malloc( size );
				memset( lm->bspLuxels[ lightmapNum ], 0, size );
			}

			/* allocate radiosity lightmap storage */
			if ( bounce ) {
				size = lm->w * lm->h * RAD_LUXEL_SIZE * sizeof( float );
				if ( lm->radLuxels[ lightmapNum ] == NULL ) {
					lm->radLuxels[ lightmapNum ] = safe_malloc( size );
				}
				memset( lm->radLuxels[ lightmapNum ], 0, size );
			}

			/* average supersampled luxels */
			for ( y = 0; y < lm->h; y++ )
			{
				for ( x = 0; x < lm->w; x++ )
				{
					/* subsample */
					samples = 0.0f;
					occludedSamples = 0.0f;
					mappedSamples = 0;
					VectorClear( sample );
					VectorClear( occludedSample );
					VectorClear( dirSample );
					for ( ly = 0; ly < superSample; ly++ )
					{
						for ( lx = 0; lx < superSample; lx++ )
						{
							/* sample luxel */
							sx = x * superSample + lx;
							sy = y * superSample + ly;
							luxel = SUPER_LUXEL( lightmapNum, sx, sy );
							deluxel = SUPER_DELUXEL( sx, sy );
							normal = SUPER_NORMAL( sx, sy );
							cluster = SUPER_CLUSTER( sx, sy );

							/* sample deluxemap */
							if ( deluxemap && lightmapNum == 0 ) {
								VectorAdd( dirSample, deluxel, dirSample );
							}

							/* keep track of used/occluded samples */
							if ( *cluster != CLUSTER_UNMAPPED ) {
								mappedSamples++;
							}

							/* handle lightmap border? */
							if ( lightmapBorder && ( sx == 0 || sx == ( lm->sw - 1 ) || sy == 0 || sy == ( lm->sh - 1 ) ) && luxel[ 3 ] > 0.0f ) {
								VectorSet( sample, 255.0f, 0.0f, 0.0f );
								samples += 1.0f;
							}

							/* handle debug */
							else if ( debug && *cluster < 0 ) {
								if ( *cluster == CLUSTER_UNMAPPED ) {
									VectorSet( luxel, 255, 204, 0 );
								}
								else if ( *cluster == CLUSTER_OCCLUDED ) {
									VectorSet( luxel, 255, 0, 255 );
								}
								else if ( *cluster == CLUSTER_FLOODED ) {
									VectorSet( luxel, 0, 32, 255 );
								}
								VectorAdd( occludedSample, luxel, occludedSample );
								occludedSamples += 1.0f;
							}

							/* normal luxel handling */
							else if ( luxel[ 3 ] > 0.0f ) {
								/* handle lit or flooded luxels */
								if ( *cluster > 0 || *cluster == CLUSTER_FLOODED ) {
									VectorAdd( sample, luxel, sample );
									samples += luxel[ 3 ];
								}

								/* handle occluded or unmapped luxels */
								else
								{
									VectorAdd( occludedSample, luxel, occludedSample );
									occludedSamples += luxel[ 3 ];
								}

								/* handle style debugging */
								if ( debug && lightmapNum > 0 && x < 2 && y < 2 ) {
									VectorCopy( debugColors[ 0 ], sample );
									samples = 1;
								}
							}
						}
					}

					/* only use occluded samples if necessary */
					if ( samples <= 0.0f ) {
						VectorCopy( occludedSample, sample );
						samples = occludedSamples;
					}

					/* get luxels */
					luxel = SUPER_LUXEL( lightmapNum, x, y );
					deluxel = SUPER_DELUXEL( x, y );

					/* store light direction */
					if ( deluxemap && lightmapNum == 0 ) {
						VectorCopy( dirSample, deluxel );
					}

					/* store the sample back in super luxels */
					if ( samples > 0.01f ) {
						VectorScale( sample, ( 1.0f / samples ), luxel );
						luxel[ 3 ] = 1.0f;
					}

					/* if any samples were mapped in any way, store ambient color */
					else if ( mappedSamples > 0 ) {
						if ( lightmapNum == 0 ) {
							VectorCopy( ambientColor, luxel );
						}
						else{
							VectorClear( luxel );
						}
						luxel[ 3 ] = 1.0f;
					}

					/* store a bogus value to be fixed later */
					else
					{
						VectorClear( luxel );
						luxel[ 3 ] = -1.0f;
					}
				}
			}

			/* setup */
			lm->used = 0;
			ClearBounds( colorMins, colorMaxs );

			/* clean up and store into bsp luxels */
			for ( y = 0; y < lm->h; y++ )
			{
				for ( x = 0; x < lm->w; x++ )
				{
					/* get luxels */
					luxel = SUPER_LUXEL( lightmapNum, x, y );
					deluxel = SUPER_DELUXEL( x, y );

					/* copy light direction */
					if ( deluxemap && lightmapNum == 0 ) {
						VectorCopy( deluxel, dirSample );
					}

					/* is this a valid sample? */
					if ( luxel[ 3 ] > 0.0f ) {
						VectorCopy( luxel, sample );
						samples = luxel[ 3 ];
						numUsed++;
						lm->used++;

						/* fix negative samples */
						for ( j = 0; j < 3; j++ )
						{
							if ( sample[ j ] < 0.0f ) {
								sample[ j ] = 0.0f;
							}
						}
					}
					else
					{
						/* nick an average value from the neighbors */
						VectorClear( sample );
						VectorClear( dirSample );
						samples = 0.0f;

						/* fixme: why is this disabled?? */
						for ( sy = ( y - 1 ); sy <= ( y + 1 ); sy++ )
						{
							if ( sy < 0 || sy >= lm->h ) {
								continue;
							}

							for ( sx = ( x - 1 ); sx <= ( x + 1 ); sx++ )
							{
								if ( sx < 0 || sx >= lm->w || ( sx == x && sy == y ) ) {
									continue;
								}

								/* get neighbor's particulars */
								luxel = SUPER_LUXEL( lightmapNum, sx, sy );
								if ( luxel[ 3 ] < 0.0f ) {
									continue;
								}
								VectorAdd( sample, luxel, sample );
								samples += luxel[ 3 ];
							}
						}

						/* no samples? */
						if ( samples == 0.0f ) {
							VectorSet( sample, -1.0f, -1.0f, -1.0f );
							samples = 1.0f;
						}
						else
						{
							numUsed++;
							lm->used++;

							/* fix negative samples */
							for ( j = 0; j < 3; j++ )
							{
								if ( sample[ j ] < 0.0f ) {
									sample[ j ] = 0.0f;
								}
							}
						}
					}

					/* scale the sample */
					VectorScale( sample, ( 1.0f / samples ), sample );

					/* store the sample in the radiosity luxels */
					if ( bounce > 0 ) {
						radLuxel = RAD_LUXEL( lightmapNum, x, y );
						VectorCopy( sample, radLuxel );

						/* if only storing bounced light, early out here */
						if ( bounceOnly && !bouncing ) {
							continue;
						}
					}

					/* store the sample in the bsp luxels */
					bspLuxel = BSP_LUXEL( lightmapNum, x, y );
					bspDeluxel = BSP_DELUXEL( x, y );

					VectorAdd( bspLuxel, sample, bspLuxel );
					if ( deluxemap && lightmapNum == 0 ) {
						VectorAdd( bspDeluxel, dirSample, bspDeluxel );
					}

					/* add color to bounds for solid checking */
					if ( samples > 0.0f ) {
						AddPointToBounds( bspLuxel, colorMins, colorMaxs );
					}
				}
			}

			/* set solid color */
			lm->solid[ lightmapNum ] = qfalse;
			VectorAdd( colorMins, colorMaxs, lm->solidColor[ lightmapNum ] );
			VectorScale( lm->solidColor[ lightmapNum ], 0.5f, lm->solidColor[ lightmapNum ] );

			/* nocollapse prevents solid lightmaps */
			if ( noCollapse == qfalse ) {
				/* check solid color */
				VectorSubtract( colorMaxs, colorMins, sample );
				if ( ( sample[ 0 ] <= SOLID_EPSILON && sample[ 1 ] <= SOLID_EPSILON && sample[ 2 ] <= SOLID_EPSILON ) ||
					 ( lm->w <= 2 && lm->h <= 2 ) ) { /* small lightmaps get forced to solid color */
					/* set to solid */
					VectorCopy( colorMins, lm->solidColor[ lightmapNum ] );
					lm->solid[ lightmapNum ] = qtrue;
					numSolidLightmaps++;
				}

				/* if all lightmaps aren't solid, then none of them are solid */
				if ( lm->solid[ lightmapNum ] != lm->solid[ 0 ] ) {
					for ( y = 0; y < MAX_LIGHTMAPS; y++ )
					{
						if ( lm->solid[ y ] ) {
							numSolidLightmaps--;
						}
						lm->solid[ y ] = qfalse;
					}
				}
			}

			/* wrap bsp luxels if necessary */
			if ( lm->wrap[ 0 ] ) {
				for ( y = 0; y < lm->h; y++ )
				{
					bspLuxel = BSP_LUXEL( lightmapNum, 0, y );
					bspLuxel2 = BSP_LUXEL( lightmapNum, lm->w - 1, y );
					VectorAdd( bspLuxel, bspLuxel2, bspLuxel );
					VectorScale( bspLuxel, 0.5f, bspLuxel );
					VectorCopy( bspLuxel, bspLuxel2 );
					if ( deluxemap && lightmapNum == 0 ) {
						bspDeluxel = BSP_DELUXEL( 0, y );
						bspDeluxel2 = BSP_DELUXEL( lm->w - 1, y );
						VectorAdd( bspDeluxel, bspDeluxel2, bspDeluxel );
						VectorScale( bspDeluxel, 0.5f, bspDeluxel );
						VectorCopy( bspDeluxel, bspDeluxel2 );
					}
				}
			}
			if ( lm->wrap[ 1 ] ) {
				for ( x = 0; x < lm->w; x++ )
				{
					bspLuxel = BSP_LUXEL( lightmapNum, x, 0 );
					bspLuxel2 = BSP_LUXEL( lightmapNum, x, lm->h - 1 );
					VectorAdd( bspLuxel, bspLuxel2, bspLuxel );
					VectorScale( bspLuxel, 0.5f, bspLuxel );
					VectorCopy( bspLuxel, bspLuxel2 );
					if ( deluxemap && lightmapNum == 0 ) {
						bspDeluxel = BSP_DELUXEL( x, 0 );
						bspDeluxel2 = BSP_DELUXEL( x, lm->h - 1 );
						VectorAdd( bspDeluxel, bspDeluxel2, bspDeluxel );
						VectorScale( bspDeluxel, 0.5f, bspDeluxel );
						VectorCopy( bspDeluxel, bspDeluxel2 );
					}
				}
			}
		}
	}

	/* -----------------------------------------------------------------
	   collapse non-unique lightmaps
	   ----------------------------------------------------------------- */

	if ( noCollapse == qfalse && deluxemap == qfalse ) {
		/* note it */
		Sys_FPrintf( SYS_VRB, "collapsing..." );

		/* set all twin refs to null */
		for ( i = 0; i < numRawLightmaps; i++ )
		{
			for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
			{
				rawLightmaps[ i ].twins[ lightmapNum ] = NULL;
				rawLightmaps[ i ].twinNums[ lightmapNum ] = -1;
				rawLightmaps[ i ].numStyledTwins = 0;
			}
		}

		/* walk the list of raw lightmaps */
		for ( i = 0; i < numRawLightmaps; i++ )
		{
			/* get lightmap */
			lm = &rawLightmaps[ i ];

			/* walk lightmaps */
			for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
			{
				/* early outs */
				if ( lm->bspLuxels[ lightmapNum ] == NULL ||
					 lm->twins[ lightmapNum ] != NULL ) {
					continue;
				}

				/* find all lightmaps that are virtually identical to this one */
				for ( j = i + 1; j < numRawLightmaps; j++ )
				{
					/* get lightmap */
					lm2 = &rawLightmaps[ j ];

					/* walk lightmaps */
					for ( lightmapNum2 = 0; lightmapNum2 < MAX_LIGHTMAPS; lightmapNum2++ )
					{
						/* early outs */
						if ( lm2->bspLuxels[ lightmapNum2 ] == NULL ||
							 lm2->twins[ lightmapNum2 ] != NULL ) {
							continue;
						}

						/* compare them */
						if ( CompareBSPLuxels( lm, lightmapNum, lm2, lightmapNum2 ) ) {
							/* merge and set twin */
							if ( MergeBSPLuxels( lm, lightmapNum, lm2, lightmapNum2 ) ) {
								lm2->twins[ lightmapNum2 ] = lm;
								lm2->twinNums[ lightmapNum2 ] = lightmapNum;
								numTwins++;
								numTwinLuxels += ( lm->w * lm->h );

								/* count styled twins */
								if ( lightmapNum > 0 ) {
									lm->numStyledTwins++;
								}
							}
						}
					}
				}
			}
		}
	}

	/* -----------------------------------------------------------------
	   sort raw lightmaps by shader
	   ----------------------------------------------------------------- */

	/* note it */
	Sys_FPrintf( SYS_VRB, "sorting..." );

	/* allocate a new sorted list */
	if ( sortLightmaps == NULL ) {
		sortLightmaps = safe_malloc( numRawLightmaps * sizeof( int ) );
	}

	/* fill it out and sort it */
	for ( i = 0; i < numRawLightmaps; i++ )
		sortLightmaps[ i ] = i;
	qsort( sortLightmaps, numRawLightmaps, sizeof( int ), CompareRawLightmap );

	/* -----------------------------------------------------------------
	   allocate output lightmaps
	   ----------------------------------------------------------------- */

	/* note it */
	Sys_FPrintf( SYS_VRB, "allocating..." );

	/* kill all existing output lightmaps */
	if ( outLightmaps != NULL ) {
		for ( i = 0; i < numOutLightmaps; i++ )
		{
			free( outLightmaps[ i ].lightBits );
			free( outLightmaps[ i ].bspLightBytes );
		}
		free( outLightmaps );
		outLightmaps = NULL;
	}

	numLightmapShaders = 0;
	numOutLightmaps = 0;
	numBSPLightmaps = 0;
	numExtLightmaps = 0;

	/* find output lightmap */
	for ( i = 0; i < numRawLightmaps; i++ )
	{
		lm = &rawLightmaps[ sortLightmaps[ i ] ];
		FindOutLightmaps( lm );
	}

	/* set output numbers in twinned lightmaps */
	for ( i = 0; i < numRawLightmaps; i++ )
	{
		/* get lightmap */
		lm = &rawLightmaps[ sortLightmaps[ i ] ];

		/* walk lightmaps */
		for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
		{
			/* get twin */
			lm2 = lm->twins[ lightmapNum ];
			if ( lm2 == NULL ) {
				continue;
			}
			lightmapNum2 = lm->twinNums[ lightmapNum ];

			/* find output lightmap from twin */
			lm->outLightmapNums[ lightmapNum ] = lm2->outLightmapNums[ lightmapNum2 ];
			lm->lightmapX[ lightmapNum ] = lm2->lightmapX[ lightmapNum2 ];
			lm->lightmapY[ lightmapNum ] = lm2->lightmapY[ lightmapNum2 ];
		}
	}

	/* -----------------------------------------------------------------
	   store output lightmaps
	   ----------------------------------------------------------------- */

	/* note it */
	Sys_FPrintf( SYS_VRB, "storing..." );

	/* count the bsp lightmaps and allocate space */
	if ( bspLightBytes != NULL ) {
		free( bspLightBytes );
	}
	if ( numBSPLightmaps == 0 || externalLightmaps ) {
		numBSPLightBytes = 0;
		bspLightBytes = NULL;
	}
	else
	{
		numBSPLightBytes = ( numBSPLightmaps * game->lightmapSize * game->lightmapSize * 3 );
		bspLightBytes = safe_malloc( numBSPLightBytes );
		memset( bspLightBytes, 0, numBSPLightBytes );
	}

	/* walk the list of output lightmaps */
	for ( i = 0; i < numOutLightmaps; i++ )
	{
		/* get output lightmap */
		olm = &outLightmaps[ i ];

		/* is this a valid bsp lightmap? */
		if ( olm->lightmapNum >= 0 && !externalLightmaps ) {
			/* copy lighting data */
			lb = bspLightBytes + ( olm->lightmapNum * game->lightmapSize * game->lightmapSize * 3 );
			memcpy( lb, olm->bspLightBytes, game->lightmapSize * game->lightmapSize * 3 );

			/* copy direction data */
			if ( deluxemap ) {
				lb = bspLightBytes + ( ( olm->lightmapNum + 1 ) * game->lightmapSize * game->lightmapSize * 3 );
				memcpy( lb, olm->bspDirBytes, game->lightmapSize * game->lightmapSize * 3 );
			}
		}

		/* external lightmap? */
		if ( olm->lightmapNum < 0 || olm->extLightmapNum >= 0 || externalLightmaps ) {
			/* make a directory for the lightmaps */
			Q_mkdir( dirname );

			/* set external lightmap number */
			olm->extLightmapNum = numExtLightmaps;

			/* write lightmap */
			sprintf( filename, "%s/" EXTERNAL_LIGHTMAP, dirname, numExtLightmaps );
			Sys_FPrintf( SYS_VRB, "\nwriting %s", filename );
			WriteTGA24( filename, olm->bspLightBytes, olm->customWidth, olm->customHeight, qtrue );
			numExtLightmaps++;

			/* write deluxemap */
			if ( deluxemap ) {
				sprintf( filename, "%s/" EXTERNAL_LIGHTMAP, dirname, numExtLightmaps );
				Sys_FPrintf( SYS_VRB, "\nwriting %s", filename );
				WriteTGA24( filename, olm->bspDirBytes, olm->customWidth, olm->customHeight, qtrue );
				numExtLightmaps++;

				if ( debugDeluxemap ) {
					olm->extLightmapNum++;
				}
			}
		}
	}

	if ( numExtLightmaps > 0 ) {
		Sys_FPrintf( SYS_VRB, "\n" );
	}

	/* delete unused external lightmaps */
	for ( i = numExtLightmaps; i; i++ )
	{
		/* determine if file exists */
		sprintf( filename, "%s/" EXTERNAL_LIGHTMAP, dirname, i );
		if ( !FileExists( filename ) ) {
			break;
		}

		/* delete it */
		remove( filename );
	}

	/* -----------------------------------------------------------------
	   project the lightmaps onto the bsp surfaces
	   ----------------------------------------------------------------- */

	/* note it */
	Sys_FPrintf( SYS_VRB, "projecting..." );

	/* walk the list of surfaces */
	for ( i = 0; i < numBSPDrawSurfaces; i++ )
	{
		/* get the surface and info */
		ds = &bspDrawSurfaces[ i ];
		info = &surfaceInfos[ i ];
		lm = info->lm;
		olm = NULL;

		/* handle surfaces with identical parent */
		if ( info->parentSurfaceNum >= 0 ) {
			/* preserve original data and get parent */
			parent = &bspDrawSurfaces[ info->parentSurfaceNum ];
			memcpy( &dsTemp, ds, sizeof( *ds ) );

			/* overwrite child with parent data */
			memcpy( ds, parent, sizeof( *ds ) );

			/* restore key parts */
			ds->fogNum = dsTemp.fogNum;
			ds->firstVert = dsTemp.firstVert;
			ds->firstIndex = dsTemp.firstIndex;
			memcpy( ds->lightmapVecs, dsTemp.lightmapVecs, sizeof( dsTemp.lightmapVecs ) );

			/* set vertex data */
			dv = &bspDrawVerts[ ds->firstVert ];
			dvParent = &bspDrawVerts[ parent->firstVert ];
			for ( j = 0; j < ds->numVerts; j++ )
			{
				memcpy( dv[ j ].lightmap, dvParent[ j ].lightmap, sizeof( dv[ j ].lightmap ) );
				memcpy( dv[ j ].color, dvParent[ j ].color, sizeof( dv[ j ].color ) );
			}

			/* skip the rest */
			continue;
		}

		/* handle vertex lit or approximated surfaces */
		else if ( lm == NULL || lm->outLightmapNums[ 0 ] < 0 ) {
			for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
			{
				ds->lightmapNum[ lightmapNum ] = -3;
				ds->lightmapStyles[ lightmapNum ] = ds->vertexStyles[ lightmapNum ];
			}
		}

		/* handle lightmapped surfaces */
		else
		{
			/* walk lightmaps */
			for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
			{
				/* set style */
				ds->lightmapStyles[ lightmapNum ] = lm->styles[ lightmapNum ];

				/* handle unused style */
				if ( lm->styles[ lightmapNum ] == LS_NONE || lm->outLightmapNums[ lightmapNum ] < 0 ) {
					ds->lightmapNum[ lightmapNum ] = -3;
					continue;
				}

				/* get output lightmap */
				olm = &outLightmaps[ lm->outLightmapNums[ lightmapNum ] ];

				/* set bsp lightmap number */
				ds->lightmapNum[ lightmapNum ] = olm->lightmapNum;

				/* deluxemap debugging makes the deluxemap visible */
				if ( deluxemap && debugDeluxemap && lightmapNum == 0 ) {
					ds->lightmapNum[ lightmapNum ]++;
				}

				/* calc lightmap origin in texture space */
				lmx = (float) lm->lightmapX[ lightmapNum ] / (float) olm->customWidth;
				lmy = (float) lm->lightmapY[ lightmapNum ] / (float) olm->customHeight;

				/* calc lightmap st coords */
				dv = &bspDrawVerts[ ds->firstVert ];
				ydv = &yDrawVerts[ ds->firstVert ];
				for ( j = 0; j < ds->numVerts; j++ )
				{
					if ( lm->solid[ lightmapNum ] ) {
						dv[ j ].lightmap[ lightmapNum ][ 0 ] = lmx + ( 0.5f / (float) olm->customWidth );
						dv[ j ].lightmap[ lightmapNum ][ 1 ] = lmy + ( 0.5f / (float) olm->customWidth );
					}
					else
					{
						dv[ j ].lightmap[ lightmapNum ][ 0 ] = lmx + ( ydv[ j ].lightmap[ 0 ][ 0 ] / ( superSample * olm->customWidth ) );
						dv[ j ].lightmap[ lightmapNum ][ 1 ] = lmy + ( ydv[ j ].lightmap[ 0 ][ 1 ] / ( superSample * olm->customHeight ) );
					}
				}
			}
		}

		/* store vertex colors */
		dv = &bspDrawVerts[ ds->firstVert ];
		for ( j = 0; j < ds->numVerts; j++ )
		{
			/* walk lightmaps */
			for ( lightmapNum = 0; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
			{
				/* handle unused style */
				if ( ds->vertexStyles[ lightmapNum ] == LS_NONE ) {
					VectorClear( color );
				}
				else
				{
					/* get vertex color */
					luxel = VERTEX_LUXEL( lightmapNum, ds->firstVert + j );
					VectorCopy( luxel, color );

					/* set minimum light */
					if ( lightmapNum == 0 ) {
						for ( k = 0; k < 3; k++ )
							if ( color[ k ] < minVertexLight[ k ] ) {
								color[ k ] = minVertexLight[ k ];
							}
					}
				}

				/* store to bytes */
				if ( !info->si->noVertexLight ) {
					ColorToBytes( color, dv[ j ].color[ lightmapNum ], info->si->vertexScale );
				}
			}
		}

		/* surfaces with styled lightmaps and a style marker get a custom generated shader (fixme: make this work with external lightmaps) */
		if ( olm != NULL && lm != NULL && lm->styles[ 1 ] != LS_NONE && game->load != LoadRBSPFile ) { //%	info->si->styleMarker > 0 )
			qboolean dfEqual;
			char key[ 32 ], styleStage[ 512 ], styleStages[ 4096 ], rgbGen[ 128 ], alphaGen[ 128 ];


			/* setup */
			sprintf( styleStages, "\n\t// Q3Map2 custom lightstyle stage(s)\n" );
			dv = &bspDrawVerts[ ds->firstVert ];

			/* depthFunc equal? */
			if ( info->si->styleMarker == 2 || info->si->implicitMap == IM_MASKED ) {
				dfEqual = qtrue;
			}
			else{
				dfEqual = qfalse;
			}

			/* generate stages for styled lightmaps */
			for ( lightmapNum = 1; lightmapNum < MAX_LIGHTMAPS; lightmapNum++ )
			{
				/* early out */
				style = lm->styles[ lightmapNum ];
				if ( style == LS_NONE || lm->outLightmapNums[ lightmapNum ] < 0 ) {
					continue;
				}

				/* get output lightmap */
				olm = &outLightmaps[ lm->outLightmapNums[ lightmapNum ] ];

				/* lightmap name */
				if ( lm->outLightmapNums[ lightmapNum ] == lm->outLightmapNums[ 0 ] ) {
					strcpy( lightmapName, "$lightmap" );
				}
				else{
					sprintf( lightmapName, "maps/%s/" EXTERNAL_LIGHTMAP, mapName, olm->extLightmapNum );
				}

				/* get rgbgen string */
				if ( rgbGenValues[ style ] == NULL ) {
					sprintf( key, "_style%drgbgen", style );
					rgbGenValues[ style ] = (char*) ValueForKey( &entities[ 0 ], key );
					if ( rgbGenValues[ style ][ 0 ] == '\0' ) {
						rgbGenValues[ style ] = "wave noise 0.5 1 0 5.37";
					}
				}
				rgbGen[ 0 ] = '\0';
				if ( rgbGenValues[ style ][ 0 ] != '\0' ) {
					sprintf( rgbGen, "\t\trgbGen %s // style %d\n", rgbGenValues[ style ], style );
				}
				else{
					rgbGen[ 0 ] = '\0';
				}

				/* get alphagen string */
				if ( alphaGenValues[ style ] == NULL ) {
					sprintf( key, "_style%dalphagen", style );
					alphaGenValues[ style ] = (char*) ValueForKey( &entities[ 0 ], key );
				}
				if ( alphaGenValues[ style ][ 0 ] != '\0' ) {
					sprintf( alphaGen, "\t\talphaGen %s // style %d\n", alphaGenValues[ style ], style );
				}
				else{
					alphaGen[ 0 ] = '\0';
				}

				/* calculate st offset */
				lmx = dv[ 0 ].lightmap[ lightmapNum ][ 0 ] - dv[ 0 ].lightmap[ 0 ][ 0 ];
				lmy = dv[ 0 ].lightmap[ lightmapNum ][ 1 ] - dv[ 0 ].lightmap[ 0 ][ 1 ];

				/* create additional stage */
				if ( lmx == 0.0f && lmy == 0.0f ) {
					sprintf( styleStage,    "\t{\n"
											"\t\tmap %s\n"                                      /* lightmap */
											"\t\tblendFunc GL_SRC_ALPHA GL_ONE\n"
											"%s"                                                /* depthFunc equal */
											"%s"                                                /* rgbGen */
											"%s"                                                /* alphaGen */
											"\t\ttcGen lightmap\n"
											"\t}\n",
							 lightmapName,
							 ( dfEqual ? "\t\tdepthFunc equal\n" : "" ),
							 rgbGen,
							 alphaGen );
				}
				else
				{
					sprintf( styleStage,    "\t{\n"
											"\t\tmap %s\n"                                      /* lightmap */
											"\t\tblendFunc GL_SRC_ALPHA GL_ONE\n"
											"%s"                                                /* depthFunc equal */
											"%s"                                                /* rgbGen */
											"%s"                                                /* alphaGen */
											"\t\ttcGen lightmap\n"
											"\t\ttcMod transform 1 0 0 1 %1.5f %1.5f\n"         /* st offset */
											"\t}\n",
							 lightmapName,
							 ( dfEqual ? "\t\tdepthFunc equal\n" : "" ),
							 rgbGen,
							 alphaGen,
							 lmx, lmy );

				}

				/* concatenate */
				strcat( styleStages, styleStage );
			}

			/* create custom shader */
			if ( info->si->styleMarker == 2 ) {
				csi = CustomShader( info->si, "q3map_styleMarker2", styleStages );
			}
			else{
				csi = CustomShader( info->si, "q3map_styleMarker", styleStages );
			}

			/* emit remap command */
			//%	EmitVertexRemapShader( csi->shader, info->si->shader );

			/* store it */
			//%	Sys_Printf( "Emitting: %s (%d", csi->shader, strlen( csi->shader ) );
			ds->shaderNum = EmitShader( csi->shader, &bspShaders[ ds->shaderNum ].contentFlags, &bspShaders[ ds->shaderNum ].surfaceFlags );
			//%	Sys_Printf( ")\n" );
		}

		/* devise a custom shader for this surface (fixme: make this work with light styles) */
		else if ( olm != NULL && lm != NULL && !externalLightmaps &&
				  ( olm->customWidth != game->lightmapSize || olm->customHeight != game->lightmapSize ) ) {
			/* get output lightmap */
			olm = &outLightmaps[ lm->outLightmapNums[ 0 ] ];

			/* do some name mangling */
			sprintf( lightmapName, "maps/%s/" EXTERNAL_LIGHTMAP, mapName, olm->extLightmapNum );

			/* create custom shader */
			csi = CustomShader( info->si, "$lightmap", lightmapName );

			/* store it */
			//%	Sys_Printf( "Emitting: %s (%d", csi->shader, strlen( csi->shader ) );
			ds->shaderNum = EmitShader( csi->shader, &bspShaders[ ds->shaderNum ].contentFlags, &bspShaders[ ds->shaderNum ].surfaceFlags );
			//%	Sys_Printf( ")\n" );
		}

		/* use the normal plain-jane shader */
		else{
			ds->shaderNum = EmitShader( info->si->shader, &bspShaders[ ds->shaderNum ].contentFlags, &bspShaders[ ds->shaderNum ].surfaceFlags );
		}
	}

	/* finish */
	Sys_FPrintf( SYS_VRB, "done.\n" );

	/* calc num stored */
	numStored = numBSPLightBytes / 3;
	efficiency = ( numStored <= 0 )
				 ? 0
				 : (float) numUsed / (float) numStored;

	/* print stats */
	Sys_Printf( "%9d luxels used\n", numUsed );
	Sys_Printf( "%9d luxels stored (%3.2f percent efficiency)\n", numStored, efficiency * 100.0f );
	Sys_Printf( "%9d solid surface lightmaps\n", numSolidLightmaps );
	Sys_Printf( "%9d identical surface lightmaps, using %d luxels\n", numTwins, numTwinLuxels );
	Sys_Printf( "%9d vertex forced surfaces\n", numSurfsVertexForced );
	Sys_Printf( "%9d vertex approximated surfaces\n", numSurfsVertexApproximated );
	Sys_Printf( "%9d BSP lightmaps\n", numBSPLightmaps );
	Sys_Printf( "%9d total lightmaps\n", numOutLightmaps );
	Sys_Printf( "%9d unique lightmap/shader combinations\n", numLightmapShaders );

	/* write map shader file */
	WriteMapShaderFile();
}
