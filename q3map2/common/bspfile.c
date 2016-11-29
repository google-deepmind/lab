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


#include "cmdlib.h"
#include "mathlib.h"
#include "inout.h"
#include "bspfile.h"
#include "scriplib.h"

void GetLeafNums( void );

//=============================================================================

int bsp_version = Q3_BSP_VERSION;

int nummodels;
dmodel_t dmodels[MAX_MAP_MODELS];

int numShaders;
dshader_t dshaders[MAX_MAP_SHADERS];

int entdatasize;
char dentdata[MAX_MAP_ENTSTRING];

int numleafs;
dleaf_t dleafs[MAX_MAP_LEAFS];

int numplanes;
dplane_t dplanes[MAX_MAP_PLANES];

int numnodes;
dnode_t dnodes[MAX_MAP_NODES];

int numleafsurfaces;
int dleafsurfaces[MAX_MAP_LEAFFACES];

int numleafbrushes;
int dleafbrushes[MAX_MAP_LEAFBRUSHES];

int numbrushes;
dbrush_t dbrushes[MAX_MAP_BRUSHES];

int numbrushsides;
dbrushside_t dbrushsides[MAX_MAP_BRUSHSIDES];

int numLightBytes;
byte        *lightBytes;

int numGridPoints;
byte        *gridData;

int numVisBytes;
byte visBytes[MAX_MAP_VISIBILITY];

int numDrawVerts = 0;
int numDrawVertsBuffer = 0;
drawVert_t  *drawVerts = NULL;

int numDrawIndexes;
int drawIndexes[MAX_MAP_DRAW_INDEXES];

int numDrawSurfaces;
int numDrawSurfacesBuffer = 0;
dsurface_t  *drawSurfaces = NULL;

int numFogs;
dfog_t dfogs[MAX_MAP_FOGS];

void SetLightBytes( int n ){
	if ( lightBytes != 0 ) {
		free( lightBytes );
	}

	numLightBytes = n;

	if ( n == 0 ) {
		return;
	}

	lightBytes = safe_malloc_info( numLightBytes, "SetLightBytes" );

	memset( lightBytes, 0, numLightBytes );
}

void SetGridPoints( int n ){
	if ( gridData != 0 ) {
		free( gridData );
	}

	numGridPoints = n;

	if ( n == 0 ) {
		return;
	}

	gridData = safe_malloc_info( numGridPoints * 8, "SetGridPoints" );

	memset( gridData, 0, numGridPoints * 8 );
}

void IncDrawVerts(){
	numDrawVerts++;

	if ( drawVerts == 0 ) {
		numDrawVertsBuffer = MAX_MAP_DRAW_VERTS / 37;

		drawVerts = safe_malloc_info( sizeof( drawVert_t ) * numDrawVertsBuffer, "IncDrawVerts" );

	}
	else if ( numDrawVerts > numDrawVertsBuffer ) {
		numDrawVertsBuffer *= 3; // multiply by 1.5
		numDrawVertsBuffer /= 2;

		if ( numDrawVertsBuffer > MAX_MAP_DRAW_VERTS ) {
			numDrawVertsBuffer = MAX_MAP_DRAW_VERTS;
		}

		drawVerts = realloc( drawVerts, sizeof( drawVert_t ) * numDrawVertsBuffer );

		if ( !drawVerts ) {
			Error( "realloc() failed (IncDrawVerts)" );
		}
	}

	memset( drawVerts + ( numDrawVerts - 1 ), 0, sizeof( drawVert_t ) );
}

void SetDrawVerts( int n ){
	if ( drawVerts != 0 ) {
		free( drawVerts );
	}

	numDrawVerts = n;
	numDrawVertsBuffer = numDrawVerts;

	drawVerts = safe_malloc_info( sizeof( drawVert_t ) * numDrawVertsBuffer, "IncDrawVerts" );

	memset( drawVerts, 0, n * sizeof( drawVert_t ) );
}

void SetDrawSurfacesBuffer(){
	if ( drawSurfaces != 0 ) {
		free( drawSurfaces );
	}

	numDrawSurfacesBuffer = MAX_MAP_DRAW_SURFS;

	drawSurfaces = safe_malloc_info( sizeof( dsurface_t ) * numDrawSurfacesBuffer, "IncDrawSurfaces" );

	memset( drawSurfaces, 0, MAX_MAP_DRAW_SURFS * sizeof( drawVert_t ) );
}

void SetDrawSurfaces( int n ){
	if ( drawSurfaces != 0 ) {
		free( drawSurfaces );
	}

	numDrawSurfaces = n;
	numDrawSurfacesBuffer = numDrawSurfaces;

	drawSurfaces = safe_malloc_info( sizeof( dsurface_t ) * numDrawSurfacesBuffer, "IncDrawSurfaces" );

	memset( drawSurfaces, 0, n * sizeof( drawVert_t ) );
}

void BspFilesCleanup(){
	if ( drawVerts != 0 ) {
		free( drawVerts );
	}
	if ( drawSurfaces != 0 ) {
		free( drawSurfaces );
	}
	if ( lightBytes != 0 ) {
		free( lightBytes );
	}
	if ( gridData != 0 ) {
		free( gridData );
	}
}

//=============================================================================

/*
   =============
   SwapBlock

   If all values are 32 bits, this can be used to swap everything
   =============
 */
void SwapBlock( int *block, int sizeOfBlock ) {
	int i;

	sizeOfBlock >>= 2;
	for ( i = 0 ; i < sizeOfBlock ; i++ ) {
		block[i] = LittleLong( block[i] );
	}
}

/*
   =============
   SwapBSPFile

   Byte swaps all data in a bsp file.
   =============
 */
void SwapBSPFile( void ) {
	int i;

	// models
	SwapBlock( (int *)dmodels, nummodels * sizeof( dmodels[0] ) );

	// shaders (don't swap the name)
	for ( i = 0 ; i < numShaders ; i++ ) {
		dshaders[i].contentFlags = LittleLong( dshaders[i].contentFlags );
		dshaders[i].surfaceFlags = LittleLong( dshaders[i].surfaceFlags );
	}

	// planes
	SwapBlock( (int *)dplanes, numplanes * sizeof( dplanes[0] ) );

	// nodes
	SwapBlock( (int *)dnodes, numnodes * sizeof( dnodes[0] ) );

	// leafs
	SwapBlock( (int *)dleafs, numleafs * sizeof( dleafs[0] ) );

	// leaffaces
	SwapBlock( (int *)dleafsurfaces, numleafsurfaces * sizeof( dleafsurfaces[0] ) );

	// leafbrushes
	SwapBlock( (int *)dleafbrushes, numleafbrushes * sizeof( dleafbrushes[0] ) );

	// brushes
	SwapBlock( (int *)dbrushes, numbrushes * sizeof( dbrushes[0] ) );

	// brushsides
	SwapBlock( (int *)dbrushsides, numbrushsides * sizeof( dbrushsides[0] ) );

	// vis
	( (int *)&visBytes )[0] = LittleLong( ( (int *)&visBytes )[0] );
	( (int *)&visBytes )[1] = LittleLong( ( (int *)&visBytes )[1] );

	// drawverts (don't swap colors )
	for ( i = 0 ; i < numDrawVerts ; i++ ) {
		drawVerts[i].lightmap[0] = LittleFloat( drawVerts[i].lightmap[0] );
		drawVerts[i].lightmap[1] = LittleFloat( drawVerts[i].lightmap[1] );
		drawVerts[i].st[0] = LittleFloat( drawVerts[i].st[0] );
		drawVerts[i].st[1] = LittleFloat( drawVerts[i].st[1] );
		drawVerts[i].xyz[0] = LittleFloat( drawVerts[i].xyz[0] );
		drawVerts[i].xyz[1] = LittleFloat( drawVerts[i].xyz[1] );
		drawVerts[i].xyz[2] = LittleFloat( drawVerts[i].xyz[2] );
		drawVerts[i].normal[0] = LittleFloat( drawVerts[i].normal[0] );
		drawVerts[i].normal[1] = LittleFloat( drawVerts[i].normal[1] );
		drawVerts[i].normal[2] = LittleFloat( drawVerts[i].normal[2] );
	}

	// drawindexes
	SwapBlock( (int *)drawIndexes, numDrawIndexes * sizeof( drawIndexes[0] ) );

	// drawsurfs
	SwapBlock( (int *)drawSurfaces, numDrawSurfaces * sizeof( drawSurfaces[0] ) );

	// fogs
	for ( i = 0 ; i < numFogs ; i++ ) {
		dfogs[i].brushNum = LittleLong( dfogs[i].brushNum );
		dfogs[i].visibleSide = LittleLong( dfogs[i].visibleSide );
	}
}



/*
   =============
   GetLumpElements
   =============
 */
int GetLumpElements( dheader_t  *header, int lump, int size ) {
	int length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;

	if ( length % size ) {
		Error( "LoadBSPFile: odd lump size" );
	}

	return length / size;
}

/*
   =============
   CopyLump
   =============
 */
int CopyLump( dheader_t *header, int lump, void *dest, int size ) {
	int length, ofs;

	length = header->lumps[lump].filelen;
	ofs = header->lumps[lump].fileofs;

	if ( length == 0 ) {
		return 0;
	}

	if ( length % size ) {
		Error( "LoadBSPFile: odd lump size" );
	}

	memcpy( dest, (byte *)header + ofs, length );

	return length / size;
}

/*
   =============
   LoadBSPFile
   =============
 */
void    LoadBSPFile( const char *filename ) {
	dheader_t   *header;

	// load the file header
	LoadFile( filename, (void **)&header );

	// swap the header
	SwapBlock( (int *)header, sizeof( *header ) );

	if ( header->ident != BSP_IDENT ) {
		Error( "%s is not a IBSP file", filename );
	}
	if ( header->version != bsp_version ) {
		Error( "%s is version %i, not %i", filename, header->version, bsp_version );
	}

	numShaders = CopyLump( header, LUMP_SHADERS, dshaders, sizeof( dshader_t ) );
	nummodels = CopyLump( header, LUMP_MODELS, dmodels, sizeof( dmodel_t ) );
	numplanes = CopyLump( header, LUMP_PLANES, dplanes, sizeof( dplane_t ) );
	numleafs = CopyLump( header, LUMP_LEAFS, dleafs, sizeof( dleaf_t ) );
	numnodes = CopyLump( header, LUMP_NODES, dnodes, sizeof( dnode_t ) );
	numleafsurfaces = CopyLump( header, LUMP_LEAFSURFACES, dleafsurfaces, sizeof( dleafsurfaces[0] ) );
	numleafbrushes = CopyLump( header, LUMP_LEAFBRUSHES, dleafbrushes, sizeof( dleafbrushes[0] ) );
	numbrushes = CopyLump( header, LUMP_BRUSHES, dbrushes, sizeof( dbrush_t ) );
	numbrushsides = CopyLump( header, LUMP_BRUSHSIDES, dbrushsides, sizeof( dbrushside_t ) );
	numDrawVerts = GetLumpElements( header, LUMP_DRAWVERTS, sizeof( drawVert_t ) );
	SetDrawVerts( numDrawVerts );
	CopyLump( header, LUMP_DRAWVERTS, drawVerts, sizeof( drawVert_t ) );
	numDrawSurfaces = GetLumpElements( header, LUMP_SURFACES, sizeof( dsurface_t ) );
	SetDrawSurfaces( numDrawSurfaces );
	numDrawSurfaces = CopyLump( header, LUMP_SURFACES, drawSurfaces, sizeof( dsurface_t ) );
	numFogs = CopyLump( header, LUMP_FOGS, dfogs, sizeof( dfog_t ) );
	numDrawIndexes = CopyLump( header, LUMP_DRAWINDEXES, drawIndexes, sizeof( drawIndexes[0] ) );

	numVisBytes = CopyLump( header, LUMP_VISIBILITY, visBytes, 1 );
	numLightBytes = GetLumpElements( header, LUMP_LIGHTMAPS, 1 );
	SetLightBytes( numLightBytes );
	CopyLump( header, LUMP_LIGHTMAPS, lightBytes, 1 );
	entdatasize = CopyLump( header, LUMP_ENTITIES, dentdata, 1 );

	numGridPoints = GetLumpElements( header, LUMP_LIGHTGRID, 8 );
	SetGridPoints( numGridPoints );
	CopyLump( header, LUMP_LIGHTGRID, gridData, 8 );


	free( header );     // everything has been copied out

	// swap everything
	SwapBSPFile();
}


//============================================================================

/*
   =============
   AddLump
   =============
 */
void AddLump( FILE *bspfile, dheader_t *header, int lumpnum, const void *data, int len ) {
	lump_t *lump;

	lump = &header->lumps[lumpnum];

	lump->fileofs = LittleLong( ftell( bspfile ) );
	lump->filelen = LittleLong( len );
	SafeWrite( bspfile, data, ( len + 3 ) & ~3 );
}

/*
   =============
   WriteBSPFile

   Swaps the bsp file in place, so it should not be referenced again
   =============
 */
void    WriteBSPFile( const char *filename ) {
	dheader_t outheader, *header;
	FILE        *bspfile;

	header = &outheader;
	memset( header, 0, sizeof( dheader_t ) );

	SwapBSPFile();

	header->ident = LittleLong( BSP_IDENT );
	header->version = LittleLong( bsp_version );

	bspfile = SafeOpenWrite( filename );
	SafeWrite( bspfile, header, sizeof( dheader_t ) );    // overwritten later

	AddLump( bspfile, header, LUMP_SHADERS, dshaders, numShaders * sizeof( dshader_t ) );
	AddLump( bspfile, header, LUMP_PLANES, dplanes, numplanes * sizeof( dplane_t ) );
	AddLump( bspfile, header, LUMP_LEAFS, dleafs, numleafs * sizeof( dleaf_t ) );
	AddLump( bspfile, header, LUMP_NODES, dnodes, numnodes * sizeof( dnode_t ) );
	AddLump( bspfile, header, LUMP_BRUSHES, dbrushes, numbrushes * sizeof( dbrush_t ) );
	AddLump( bspfile, header, LUMP_BRUSHSIDES, dbrushsides, numbrushsides * sizeof( dbrushside_t ) );
	AddLump( bspfile, header, LUMP_LEAFSURFACES, dleafsurfaces, numleafsurfaces * sizeof( dleafsurfaces[0] ) );
	AddLump( bspfile, header, LUMP_LEAFBRUSHES, dleafbrushes, numleafbrushes * sizeof( dleafbrushes[0] ) );
	AddLump( bspfile, header, LUMP_MODELS, dmodels, nummodels * sizeof( dmodel_t ) );
	AddLump( bspfile, header, LUMP_DRAWVERTS, drawVerts, numDrawVerts * sizeof( drawVert_t ) );
	AddLump( bspfile, header, LUMP_SURFACES, drawSurfaces, numDrawSurfaces * sizeof( dsurface_t ) );
	AddLump( bspfile, header, LUMP_VISIBILITY, visBytes, numVisBytes );
	AddLump( bspfile, header, LUMP_LIGHTMAPS, lightBytes, numLightBytes );
	AddLump( bspfile, header, LUMP_LIGHTGRID, gridData, 8 * numGridPoints );
	AddLump( bspfile, header, LUMP_ENTITIES, dentdata, entdatasize );
	AddLump( bspfile, header, LUMP_FOGS, dfogs, numFogs * sizeof( dfog_t ) );
	AddLump( bspfile, header, LUMP_DRAWINDEXES, drawIndexes, numDrawIndexes * sizeof( drawIndexes[0] ) );

	fseek( bspfile, 0, SEEK_SET );
	SafeWrite( bspfile, header, sizeof( dheader_t ) );
	fclose( bspfile );
}

//============================================================================

/*
   =============
   PrintBSPFileSizes

   Dumps info about current file
   =============
 */
void PrintBSPFileSizes( void ) {
	if ( !num_entities ) {
		ParseEntities();
	}

	Sys_Printf( "%6i models       %7i\n"
				,nummodels, (int)( nummodels * sizeof( dmodel_t ) ) );
	Sys_Printf( "%6i shaders      %7i\n"
				,numShaders, (int)( numShaders * sizeof( dshader_t ) ) );
	Sys_Printf( "%6i brushes      %7i\n"
				,numbrushes, (int)( numbrushes * sizeof( dbrush_t ) ) );
	Sys_Printf( "%6i brushsides   %7i\n"
				,numbrushsides, (int)( numbrushsides * sizeof( dbrushside_t ) ) );
	Sys_Printf( "%6i fogs         %7i\n"
				,numFogs, (int)( numFogs * sizeof( dfog_t ) ) );
	Sys_Printf( "%6i planes       %7i\n"
				,numplanes, (int)( numplanes * sizeof( dplane_t ) ) );
	Sys_Printf( "%6i entdata      %7i\n", num_entities, entdatasize );

	Sys_Printf( "\n" );

	Sys_Printf( "%6i nodes        %7i\n"
				,numnodes, (int)( numnodes * sizeof( dnode_t ) ) );
	Sys_Printf( "%6i leafs        %7i\n"
				,numleafs, (int)( numleafs * sizeof( dleaf_t ) ) );
	Sys_Printf( "%6i leafsurfaces %7i\n"
				,numleafsurfaces, (int)( numleafsurfaces * sizeof( dleafsurfaces[0] ) ) );
	Sys_Printf( "%6i leafbrushes  %7i\n"
				,numleafbrushes, (int)( numleafbrushes * sizeof( dleafbrushes[0] ) ) );
	Sys_Printf( "%6i drawverts    %7i\n"
				,numDrawVerts, (int)( numDrawVerts * sizeof( drawVerts[0] ) ) );
	Sys_Printf( "%6i drawindexes  %7i\n"
				,numDrawIndexes, (int)( numDrawIndexes * sizeof( drawIndexes[0] ) ) );
	Sys_Printf( "%6i drawsurfaces %7i\n"
				,numDrawSurfaces, (int)( numDrawSurfaces * sizeof( drawSurfaces[0] ) ) );

	Sys_Printf( "%6i lightmaps    %7i\n"
				,numLightBytes / ( LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT * 3 ), numLightBytes );
	Sys_Printf( "       visibility   %7i\n"
				, numVisBytes );
}


//============================================

int num_entities;
entity_t entities[MAX_MAP_ENTITIES];

void StripTrailing( char *e ) {
	char    *s;

	s = e + strlen( e ) - 1;
	while ( s >= e && *s <= 32 )
	{
		*s = 0;
		s--;
	}
}

/*
   =================
   ParseEpair
   =================
 */
epair_t *ParseEpair( void ) {
	epair_t *e;

	e = safe_malloc( sizeof( epair_t ) );
	memset( e, 0, sizeof( epair_t ) );

	if ( strlen( token ) >= MAX_KEY - 1 ) {
		Error( "ParseEpar: token too long" );
	}
	e->key = copystring( token );
	GetToken( qfalse );
	if ( strlen( token ) >= MAX_VALUE - 1 ) {
		Error( "ParseEpar: token too long" );
	}
	e->value = copystring( token );

	// strip trailing spaces that sometimes get accidentally
	// added in the editor
	StripTrailing( e->key );
	StripTrailing( e->value );

	return e;
}


/*
   ================
   ParseEntity
   ================
 */
qboolean    ParseEntity( void ) {
	epair_t     *e;
	entity_t    *mapent;

	if ( !GetToken( qtrue ) ) {
		return qfalse;
	}

	if ( strcmp( token, "{" ) ) {
		Error( "ParseEntity: { not found" );
	}
	if ( num_entities == MAX_MAP_ENTITIES ) {
		Error( "num_entities == MAX_MAP_ENTITIES" );
	}
	mapent = &entities[num_entities];
	num_entities++;

	do {
		if ( !GetToken( qtrue ) ) {
			Error( "ParseEntity: EOF without closing brace" );
		}
		if ( !strcmp( token, "}" ) ) {
			break;
		}
		e = ParseEpair();
		e->next = mapent->epairs;
		mapent->epairs = e;
	} while ( 1 );

	return qtrue;
}

/*
   ================
   ParseEntities

   Parses the dentdata string into entities
   ================
 */
void ParseEntities( void ) {
	num_entities = 0;
	ParseFromMemory( dentdata, entdatasize );

	while ( ParseEntity() ) {
	}
}


/*
   ================
   UnparseEntities

   Generates the dentdata string from all the entities
   This allows the utilities to add or remove key/value pairs
   to the data created by the map editor.
   ================
 */
void UnparseEntities( void ) {
	char    *buf, *end;
	epair_t *ep;
	char line[2048];
	int i;
	char key[1024], value[1024];

	buf = dentdata;
	end = buf;
	*end = 0;

	for ( i = 0 ; i < num_entities ; i++ ) {
		ep = entities[i].epairs;
		if ( !ep ) {
			continue;   // ent got removed
		}

		strcat( end,"{\n" );
		end += 2;

		for ( ep = entities[i].epairs ; ep ; ep = ep->next ) {
			strcpy( key, ep->key );
			StripTrailing( key );
			strcpy( value, ep->value );
			StripTrailing( value );

			sprintf( line, "\"%s\" \"%s\"\n", key, value );
			strcat( end, line );
			end += strlen( line );
		}
		strcat( end,"}\n" );
		end += 2;

		if ( end > buf + MAX_MAP_ENTSTRING ) {
			Error( "Entity text too long" );
		}
	}
	entdatasize = end - buf + 1;
}

void PrintEntity( const entity_t *ent ) {
	epair_t *ep;

	Sys_Printf( "------- entity %p -------\n", ent );
	for ( ep = ent->epairs ; ep ; ep = ep->next ) {
		Sys_Printf( "%s = %s\n", ep->key, ep->value );
	}

}

void    SetKeyValue( entity_t *ent, const char *key, const char *value ) {
	epair_t *ep;

	for ( ep = ent->epairs ; ep ; ep = ep->next ) {
		if ( !strcmp( ep->key, key ) ) {
			free( ep->value );
			ep->value = copystring( value );
			return;
		}
	}
	ep = safe_malloc( sizeof( *ep ) );
	ep->next = ent->epairs;
	ent->epairs = ep;
	ep->key = copystring( key );
	ep->value = copystring( value );
}

const char  *ValueForKey( const entity_t *ent, const char *key ) {
	epair_t *ep;

	for ( ep = ent->epairs ; ep ; ep = ep->next ) {
		if ( !strcmp( ep->key, key ) ) {
			return ep->value;
		}
	}
	return "";
}

vec_t   FloatForKey( const entity_t *ent, const char *key ) {
	const char  *k;

	k = ValueForKey( ent, key );
	return atof( k );
}

void    GetVectorForKey( const entity_t *ent, const char *key, vec3_t vec ) {
	const char  *k;
	double v1, v2, v3;

	k = ValueForKey( ent, key );

	// scanf into doubles, then assign, so it is vec_t size independent
	v1 = v2 = v3 = 0;
	sscanf( k, "%lf %lf %lf", &v1, &v2, &v3 );
	vec[0] = v1;
	vec[1] = v2;
	vec[2] = v3;
}
