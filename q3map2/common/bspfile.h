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

#include "qfiles.h"
#include "surfaceflags.h"

extern int bsp_version;

extern int nummodels;
extern dmodel_t dmodels[MAX_MAP_MODELS];

extern int numShaders;
extern dshader_t dshaders[MAX_MAP_MODELS];

extern int entdatasize;
extern char dentdata[MAX_MAP_ENTSTRING];

extern int numleafs;
extern dleaf_t dleafs[MAX_MAP_LEAFS];

extern int numplanes;
extern dplane_t dplanes[MAX_MAP_PLANES];

extern int numnodes;
extern dnode_t dnodes[MAX_MAP_NODES];

extern int numleafsurfaces;
extern int dleafsurfaces[MAX_MAP_LEAFFACES];

extern int numleafbrushes;
extern int dleafbrushes[MAX_MAP_LEAFBRUSHES];

extern int numbrushes;
extern dbrush_t dbrushes[MAX_MAP_BRUSHES];

extern int numbrushsides;
extern dbrushside_t dbrushsides[MAX_MAP_BRUSHSIDES];

void SetLightBytes( int n );
extern int numLightBytes;
extern byte        *lightBytes;

void SetGridPoints( int n );
extern int numGridPoints;
extern byte        *gridData;

extern int numVisBytes;
extern byte visBytes[MAX_MAP_VISIBILITY];

void SetDrawVerts( int n );
void IncDrawVerts();
extern int numDrawVerts;
extern drawVert_t  *drawVerts;

extern int numDrawIndexes;
extern int drawIndexes[MAX_MAP_DRAW_INDEXES];

void SetDrawSurfaces( int n );
void SetDrawSurfacesBuffer();
extern int numDrawSurfaces;
extern dsurface_t  *drawSurfaces;

extern int numFogs;
extern dfog_t dfogs[MAX_MAP_FOGS];

void    LoadBSPFile( const char *filename );
void    WriteBSPFile( const char *filename );
void    PrintBSPFileSizes( void );

//===============


typedef struct epair_s {
	struct epair_s  *next;
	char    *key;
	char    *value;
} epair_t;

typedef struct {
	vec3_t origin;
	struct bspbrush_s   *brushes;
	struct parseMesh_s  *patches;
	int firstDrawSurf;
	epair_t     *epairs;
} entity_t;

extern int num_entities;
extern entity_t entities[MAX_MAP_ENTITIES];

void    ParseEntities( void );
void    UnparseEntities( void );

void    SetKeyValue( entity_t *ent, const char *key, const char *value );
const char  *ValueForKey( const entity_t *ent, const char *key );
// will return "" if not present

vec_t   FloatForKey( const entity_t *ent, const char *key );
void    GetVectorForKey( const entity_t *ent, const char *key, vec3_t vec );

epair_t *ParseEpair( void );

void    PrintEntity( const entity_t *ent );
