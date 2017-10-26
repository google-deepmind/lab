/* -----------------------------------------------------------------------------

   PicoModel Library

   Copyright (c) 2002, Randy Reddig & seaw0lf
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the names of the copyright holders nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   ----------------------------------------------------------------------------- */

/*
   Nurail: Used pm_md3.c (Randy Reddig) as a template.
 */

/* marker */
#define PM_FM_C

/* dependencies */
#include "pm_fm.h"

//#define FM_VERBOSE_DBG	0
#undef FM_VERBOSE_DBG
#undef FM_DBG

typedef struct index_LUT_s
{
	short Vert;
	short ST;
	struct  index_LUT_s *next;

} index_LUT_t;

typedef struct index_DUP_LUT_s
{
	short ST;
	short OldVert;

} index_DUP_LUT_t;


// _fm_canload()
static int _fm_canload( PM_PARAMS_CANLOAD ){
	fm_t fm;
	unsigned char   *bb;
	int fm_file_pos;

	bb = (unsigned char *) buffer;

	// Header
	fm.fm_header_hdr = (fm_chunk_header_t *) bb;
	fm_file_pos = sizeof( fm_chunk_header_t ) + fm.fm_header_hdr->size;
#ifdef FM_VERBOSE_DBG
	_pico_printf( PICO_VERBOSE, "IDENT: %s\n", (unsigned char *) fm.fm_header_hdr->ident );
#endif
	if ( ( strcmp( fm.fm_header_hdr->ident, FM_HEADERCHUNKNAME ) )  ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM Header Ident incorrect\n" );
#endif
		return PICO_PMV_ERROR_IDENT;
	}

	// check fm
	if ( _pico_little_long( fm.fm_header_hdr->version ) != FM_HEADERCHUNKVER ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM Header Version incorrect\n" );
#endif
		return PICO_PMV_ERROR_VERSION;
	}

	// Skin
	fm.fm_skin_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_skin_hdr->size;
#ifdef FM_VERBOSE_DBG
	_pico_printf( PICO_VERBOSE, "SKIN: %s\n", (unsigned char *) fm.fm_skin_hdr->ident );
#endif
	if ( ( strcmp( fm.fm_skin_hdr->ident, FM_SKINCHUNKNAME ) ) ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM Skin Ident incorrect\n" );
#endif
		return PICO_PMV_ERROR_IDENT;
	}

	// check fm
	if ( _pico_little_long( fm.fm_skin_hdr->version ) != FM_SKINCHUNKVER ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM Skin Version incorrect\n" );
#endif
		return PICO_PMV_ERROR_VERSION;
	}

	// st
	fm.fm_st_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_st_hdr->size;
#ifdef FM_VERBOSE_DBG
	_pico_printf( PICO_VERBOSE, "ST: %s\n", (unsigned char *) fm.fm_st_hdr->ident );
#endif
	if ( ( strcmp( fm.fm_st_hdr->ident, FM_STCOORDCHUNKNAME ) ) ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM ST Ident incorrect\n" );
#endif
		return PICO_PMV_ERROR_IDENT;
	}

	// check fm
	if ( _pico_little_long( fm.fm_st_hdr->version ) != FM_STCOORDCHUNKVER ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM ST Version incorrect\n" );
#endif
		return PICO_PMV_ERROR_VERSION;
	}

	// tri
	fm.fm_tri_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_tri_hdr->size;
#ifdef FM_VERBOSE_DBG
	_pico_printf( PICO_VERBOSE, "TRI: %s\n", (unsigned char *) fm.fm_tri_hdr->ident );
#endif
	if ( ( strcmp( fm.fm_tri_hdr->ident, FM_TRISCHUNKNAME ) ) ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM Tri Ident incorrect\n" );
#endif
		return PICO_PMV_ERROR_IDENT;
	}

	// check fm
	if ( _pico_little_long( fm.fm_tri_hdr->version ) != FM_TRISCHUNKVER ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM Tri Version incorrect\n" );
#endif
		return PICO_PMV_ERROR_VERSION;
	}

	// frame
	fm.fm_frame_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t );
#ifdef FM_VERBOSE_DBG
	_pico_printf( PICO_VERBOSE, "FRAME: %s\n", (unsigned char *) fm.fm_frame_hdr->ident );
#endif
	if ( ( strcmp( fm.fm_frame_hdr->ident, FM_FRAMESCHUNKNAME ) ) ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM Frame Ident incorrect\n" );
#endif
		return PICO_PMV_ERROR_IDENT;
	}

	// check fm
	if ( _pico_little_long( fm.fm_frame_hdr->version ) != FM_FRAMESCHUNKVER ) {
#ifdef FM_DBG
		_pico_printf( PICO_WARNING, "FM Frame Version incorrect\n" );
#endif
		return PICO_PMV_ERROR_VERSION;
	}

	// file seems to be a valid fm
	return PICO_PMV_OK;
}



// _fm_load() loads a Heretic 2 model file.
static picoModel_t *_fm_load( PM_PARAMS_LOAD ){
	int i, j, dups, dup_index;
	int fm_file_pos;
	short tot_numVerts;
	index_LUT_t     *p_index_LUT, *p_index_LUT2, *p_index_LUT3;
	index_DUP_LUT_t *p_index_LUT_DUPS;

	fm_vert_normal_t    *vert;

	char skinname[FM_SKINPATHSIZE];
	fm_t fm;
	fm_header_t     *fm_head;
	fm_st_t         *texCoord;
	fm_xyz_st_t     *tri_verts;
	fm_xyz_st_t     *triangle;
	fm_frame_t      *frame;

	picoByte_t      *bb;
	picoModel_t *picoModel;
	picoSurface_t   *picoSurface;
	picoShader_t    *picoShader;
	picoVec3_t xyz, normal;
	picoVec2_t st;
	picoColor_t color;


	// fm loading
	_pico_printf( PICO_NORMAL, "Loading \"%s\"", fileName );

	bb = (picoByte_t*) buffer;

	// Header Header
	fm.fm_header_hdr = (fm_chunk_header_t *) bb;
	fm_file_pos = sizeof( fm_chunk_header_t ) + fm.fm_header_hdr->size;
	if ( ( strcmp( fm.fm_header_hdr->ident, FM_HEADERCHUNKNAME ) )  ) {
		_pico_printf( PICO_WARNING, "FM Header Ident incorrect\n" );
		return NULL;
	}

	if ( _pico_little_long( fm.fm_header_hdr->version ) != FM_HEADERCHUNKVER ) {
		_pico_printf( PICO_WARNING, "FM Header Version incorrect\n" );
		return NULL;
	}

	// Skin Header
	fm.fm_skin_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_skin_hdr->size;
	if ( ( strcmp( fm.fm_skin_hdr->ident, FM_SKINCHUNKNAME ) ) ) {
		_pico_printf( PICO_WARNING, "FM Skin Ident incorrect\n" );
		return NULL;
	}

	if ( _pico_little_long( fm.fm_skin_hdr->version ) != FM_SKINCHUNKVER ) {
		_pico_printf( PICO_WARNING, "FM Skin Version incorrect\n" );
		return NULL;
	}

	// ST Header
	fm.fm_st_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_st_hdr->size;
	if ( ( strcmp( fm.fm_st_hdr->ident, FM_STCOORDCHUNKNAME ) ) ) {
		_pico_printf( PICO_WARNING, "FM ST Ident incorrect\n" );
		return NULL;
	}

	if ( _pico_little_long( fm.fm_st_hdr->version ) != FM_STCOORDCHUNKVER ) {
		_pico_printf( PICO_WARNING, "FM ST Version incorrect\n" );
		return NULL;
	}

	// Tris Header
	fm.fm_tri_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t ) + fm.fm_tri_hdr->size;
	if ( ( strcmp( fm.fm_tri_hdr->ident, FM_TRISCHUNKNAME ) ) ) {
		_pico_printf( PICO_WARNING, "FM Tri Ident incorrect\n" );
		return NULL;
	}

	if ( _pico_little_long( fm.fm_tri_hdr->version ) != FM_TRISCHUNKVER ) {
		_pico_printf( PICO_WARNING, "FM Tri Version incorrect\n" );
		return NULL;
	}

	// Frame Header
	fm.fm_frame_hdr = (fm_chunk_header_t *) ( bb + fm_file_pos );
	fm_file_pos += sizeof( fm_chunk_header_t );
	if ( ( strcmp( fm.fm_frame_hdr->ident, FM_FRAMESCHUNKNAME ) ) ) {
		_pico_printf( PICO_WARNING, "FM Frame Ident incorrect\n" );
		return NULL;
	}

	if ( _pico_little_long( fm.fm_frame_hdr->version ) != FM_FRAMESCHUNKVER ) {
		_pico_printf( PICO_WARNING, "FM Frame Version incorrect\n" );
		return NULL;
	}

	// Header
	fm_file_pos = sizeof( fm_chunk_header_t );
	fm_head = fm.fm_header = (fm_header_t *) ( bb + fm_file_pos );
	fm_file_pos += fm.fm_header_hdr->size;

	// Skin
	fm_file_pos += sizeof( fm_chunk_header_t );
	fm.fm_skin = (fm_skinpath_t *) ( bb + fm_file_pos );
	fm_file_pos += fm.fm_skin_hdr->size;

	// ST
	fm_file_pos += sizeof( fm_chunk_header_t );
	texCoord = fm.fm_st = (fm_st_t *) ( bb + fm_file_pos );
	fm_file_pos += fm.fm_st_hdr->size;

	// Tri
	fm_file_pos += sizeof( fm_chunk_header_t );
	tri_verts = fm.fm_tri = (fm_xyz_st_t *) ( bb + fm_file_pos );
	fm_file_pos += fm.fm_tri_hdr->size;

	// Frame
	fm_file_pos += sizeof( fm_chunk_header_t );
	frame = fm.fm_frame = (fm_frame_t *) ( bb + fm_file_pos );

	// do frame check
	if ( fm_head->numFrames < 1 ) {
		_pico_printf( PICO_ERROR, "%s has 0 frames!", fileName );
		return NULL;
	}

	if ( frameNum < 0 || frameNum >= fm_head->numFrames ) {
		_pico_printf( PICO_ERROR, "Invalid or out-of-range FM frame specified" );
		return NULL;
	}

	// swap fm
	fm_head->skinWidth = _pico_little_long( fm_head->skinWidth );
	fm_head->skinHeight = _pico_little_long( fm_head->skinHeight );
	fm_head->frameSize = _pico_little_long( fm_head->frameSize );

	fm_head->numSkins = _pico_little_long( fm_head->numSkins );
	fm_head->numXYZ = _pico_little_long( fm_head->numXYZ );
	fm_head->numST = _pico_little_long( fm_head->numST );
	fm_head->numTris = _pico_little_long( fm_head->numTris );
	fm_head->numGLCmds = _pico_little_long( fm_head->numGLCmds );
	fm_head->numFrames = _pico_little_long( fm_head->numFrames );

	// swap frame scale and translation
	for ( i = 0; i < 3; i++ )
	{
		frame->header.scale[ i ] = _pico_little_float( frame->header.scale[ i ] );
		frame->header.translate[ i ] = _pico_little_float( frame->header.translate[ i ] );
	}

	// swap triangles
	triangle = tri_verts;
	for ( i = 0; i < fm_head->numTris; i++, triangle++ )
	{
		for ( j = 0; j < 3; j++ )
		{
			triangle->index_xyz[ j ] = _pico_little_short( triangle->index_xyz[ j ] );
			triangle->index_st[ j ] = _pico_little_short( triangle->index_st[ j ] );
		}
	}

	// swap st coords
	for ( i = 0; i < fm_head->numST; i++ )
	{
		texCoord->s = _pico_little_short( texCoord[i].s );
		texCoord->t = _pico_little_short( texCoord[i].t );
	}
	// set Skin Name
	strncpy( skinname, fm.fm_skin->path, FM_SKINPATHSIZE );

#ifdef FM_VERBOSE_DBG
	// Print out md2 values
	_pico_printf( PICO_VERBOSE,"numSkins->%d  numXYZ->%d  numST->%d  numTris->%d  numFrames->%d\nSkin Name \"%s\"\n", fm_head->numSkins, fm_head->numXYZ, fm_head->numST, fm_head->numTris, fm_head->numFrames, &skinname );
#endif

	// detox Skin name
	_pico_setfext( skinname, "" );
	_pico_unixify( skinname );

	/* create new pico model */
	picoModel = PicoNewModel();
	if ( picoModel == NULL ) {
		_pico_printf( PICO_ERROR, "Unable to allocate a new model" );
		return NULL;
	}

	/* do model setup */
	PicoSetModelFrameNum( picoModel, frameNum );
	PicoSetModelNumFrames( picoModel, fm_head->numFrames ); /* sea */
	PicoSetModelName( picoModel, fileName );
	PicoSetModelFileName( picoModel, fileName );

	// allocate new pico surface
	picoSurface = PicoNewSurface( picoModel );
	if ( picoSurface == NULL ) {
		_pico_printf( PICO_ERROR, "Unable to allocate a new model surface" );
		PicoFreeModel( picoModel );
		return NULL;
	}


	PicoSetSurfaceType( picoSurface, PICO_TRIANGLES );
	PicoSetSurfaceName( picoSurface, frame->header.name );
	picoShader = PicoNewShader( picoModel );
	if ( picoShader == NULL ) {
		_pico_printf( PICO_ERROR, "Unable to allocate a new model shader" );
		PicoFreeModel( picoModel );
		return NULL;
	}

	PicoSetShaderName( picoShader, skinname );

	// associate current surface with newly created shader
	PicoSetSurfaceShader( picoSurface, picoShader );

	// Init LUT for Verts
	p_index_LUT = (index_LUT_t *)_pico_alloc( sizeof( index_LUT_t ) * fm_head->numXYZ );
	for ( i = 0; i < fm_head->numXYZ; i++ )
	{
		p_index_LUT[i].Vert = -1;
		p_index_LUT[i].ST = -1;
		p_index_LUT[i].next = NULL;
	}

	// Fill in Look Up Table, and allocate/fill Linked List from vert array as needed for dup STs per Vert.
	tot_numVerts = fm_head->numXYZ;
	dups = 0;
	triangle = tri_verts;

	for ( i = 0; i < fm_head->numTris; i++ )
	{
		for ( j = 0; j < 3; j++ )
		{
			if ( p_index_LUT[triangle->index_xyz[j]].ST == -1 ) { // No Main Entry
				p_index_LUT[triangle->index_xyz[j]].ST = triangle->index_st[j];
			}

			else if ( triangle->index_st[j] == p_index_LUT[triangle->index_xyz[j]].ST ) { // Equal to Main Entry
#ifdef FM_VERBOSE_DBG
				_pico_printf( PICO_NORMAL, "-> Tri #%d, Vert %d:\t XYZ:%d   ST:%d\n", i, j, triangle->index_xyz[j], triangle->index_st[j] );
#endif
				continue;
			}
			else if ( p_index_LUT[triangle->index_xyz[j]].next == NULL ) { // Not equal to Main entry, and no LL entry
				// Add first entry of LL from Main
				p_index_LUT2 = (index_LUT_t *)_pico_alloc( sizeof( index_LUT_t ) );
				if ( p_index_LUT2 == NULL ) {
					_pico_printf( PICO_NORMAL, " Couldn't allocate memory!\n" );
				}
				p_index_LUT[triangle->index_xyz[j]].next = (index_LUT_t *)p_index_LUT2;
				p_index_LUT2->Vert = dups;
				p_index_LUT2->ST = triangle->index_st[j];
				p_index_LUT2->next = NULL;
#ifdef FM_VERBOSE_DBG
				_pico_printf( PICO_NORMAL, " ADDING first LL XYZ:%d DUP:%d ST:%d\n", triangle->index_xyz[j], dups, triangle->index_st[j] );
#endif
				triangle->index_xyz[j] = dups + fm_head->numXYZ; // Make change in Tri hunk
				dups++;
			}
			else // Try to find in LL from Main Entry
			{
				p_index_LUT3 = p_index_LUT2 = p_index_LUT[triangle->index_xyz[j]].next;
				while ( ( p_index_LUT2 != NULL ) && ( triangle->index_xyz[j] != p_index_LUT2->Vert ) ) // Walk down LL
				{
					p_index_LUT3 = p_index_LUT2;
					p_index_LUT2 = p_index_LUT2->next;
				}
				p_index_LUT2 = p_index_LUT3;

				if ( triangle->index_st[j] == p_index_LUT2->ST ) { // Found it
					triangle->index_xyz[j] = p_index_LUT2->Vert + fm_head->numXYZ; // Make change in Tri hunk
#ifdef FM_VERBOSE_DBG
					_pico_printf( PICO_NORMAL, "--> Tri #%d, Vert %d:\t XYZ:%d   ST:%d\n", i, j, triangle->index_xyz[j], triangle->index_st[j] );
#endif
					continue;
				}

				if ( p_index_LUT2->next == NULL ) { // Didn't find it. Add entry to LL.
					// Add the Entry
					p_index_LUT3 = (index_LUT_t *)_pico_alloc( sizeof( index_LUT_t ) );
					if ( p_index_LUT3 == NULL ) {
						_pico_printf( PICO_NORMAL, " Couldn't allocate memory!\n" );
					}
					p_index_LUT2->next = (index_LUT_t *)p_index_LUT3;
					p_index_LUT3->Vert = dups;
					p_index_LUT3->ST = triangle->index_st[j];
					p_index_LUT3->next = NULL;
#ifdef FM_VERBOSE_DBG
					_pico_printf( PICO_NORMAL, " ADDING additional LL XYZ:%d DUP:%d NewXYZ:%d ST:%d\n", triangle->index_xyz[j], dups, dups + ( fm_head->numXYZ ), triangle->index_st[j] );
#endif
					triangle->index_xyz[j] = dups + fm_head->numXYZ; // Make change in Tri hunk
					dups++;
				}
			}
#ifdef FM_VERBOSE_DBG
			_pico_printf( PICO_NORMAL, "---> Tri #%d, Vert %d:\t XYZ:%d   ST:%d\n", i, j, triangle->index_xyz[j], triangle->index_st[j] );
#endif
		}
		triangle++;
	}

	// malloc and build array for Dup STs
	p_index_LUT_DUPS = (index_DUP_LUT_t *)_pico_alloc( sizeof( index_DUP_LUT_t ) * dups );
	if ( p_index_LUT_DUPS == NULL ) {
		_pico_printf( PICO_NORMAL, " Couldn't allocate memory!\n" );
	}

	dup_index = 0;
	for ( i = 0; i < fm_head->numXYZ; i++ )
	{
		p_index_LUT2 = p_index_LUT[i].next;
		while ( p_index_LUT2 != NULL )
		{
			p_index_LUT_DUPS[p_index_LUT2->Vert].OldVert = i;
			p_index_LUT_DUPS[p_index_LUT2->Vert].ST = p_index_LUT2->ST;
			dup_index++;
			p_index_LUT2 = p_index_LUT2->next;
		}
	}
#ifdef FM_VERBOSE_DBG
	_pico_printf( PICO_NORMAL, " Dups = %d\n", dups );
	_pico_printf( PICO_NORMAL, " Dup Index = %d\n", dup_index );
#endif
	for ( i = 0; i < fm_head->numXYZ; i++ )
	{
#ifdef FM_VERBOSE_DBG
		_pico_printf( PICO_NORMAL, "Vert: %4d\t%4d",i, p_index_LUT[i].ST );
#endif
		if ( p_index_LUT[i].next != NULL ) {

			p_index_LUT2 = p_index_LUT[i].next;
			do {
#ifdef FM_VERBOSE_DBG
				_pico_printf( PICO_NORMAL, " %4d %4d", p_index_LUT2->Vert, p_index_LUT2->ST );
#endif
				p_index_LUT2 = p_index_LUT2->next;
			} while ( p_index_LUT2 != NULL );

		}
#ifdef FM_VERBOSE_DBG
		_pico_printf( PICO_NORMAL, "\n" );
#endif
	}


#ifdef FM_VERBOSE_DBG
	for ( i = 0; i < dup_index; i++ )
		_pico_printf( PICO_NORMAL, " Dup Index #%d  OldVert: %d  ST: %d\n", i, p_index_LUT_DUPS[i].OldVert, p_index_LUT_DUPS[i].ST );

	triangle = tri_verts;
	for ( i = 0; i < fm_head->numTris; i++ )
	{
		for ( j = 0; j < 3; j++ )
			_pico_printf( PICO_NORMAL, "Tri #%d, Vert %d:\t XYZ:%d   ST:%d\n", i, j, triangle->index_xyz[j], triangle->index_st[j] );
		_pico_printf( PICO_NORMAL, "\n" );
		triangle++;
	}
#endif
	// Build Picomodel
	triangle = tri_verts;
	for ( j = 0; j < fm_head->numTris; j++, triangle++ )
	{
		PicoSetSurfaceIndex( picoSurface, j * 3, triangle->index_xyz[0] );
		PicoSetSurfaceIndex( picoSurface, j * 3 + 1, triangle->index_xyz[1] );
		PicoSetSurfaceIndex( picoSurface, j * 3 + 2, triangle->index_xyz[2] );
	}

	vert = (fm_vert_normal_t*) ( (picoByte_t*) ( frame->verts ) );
	for ( i = 0; i < fm_head->numXYZ; i++, vert++ )
	{
		/* set vertex origin */
		xyz[ 0 ] = vert->v[0] * frame->header.scale[0] + frame->header.translate[0];
		xyz[ 1 ] = vert->v[1] * frame->header.scale[1] + frame->header.translate[1];
		xyz[ 2 ] = vert->v[2] * frame->header.scale[2] + frame->header.translate[2];
		PicoSetSurfaceXYZ( picoSurface, i, xyz );

		/* set normal */
		normal[ 0 ] = fm_normals[vert->lightnormalindex][0];
		normal[ 1 ] = fm_normals[vert->lightnormalindex][1];
		normal[ 2 ] = fm_normals[vert->lightnormalindex][2];
		PicoSetSurfaceNormal( picoSurface, i, normal );

		/* set st coords */
		st[ 0 ] =  ( ( texCoord[p_index_LUT[i].ST].s ) / ( (float)fm_head->skinWidth ) );
		st[ 1 ] =  ( texCoord[p_index_LUT[i].ST].t / ( (float)fm_head->skinHeight ) );
		PicoSetSurfaceST( picoSurface, 0, i, st );
	}

	if ( dups ) {
		for ( i = 0; i < dups; i++ )
		{
			j = p_index_LUT_DUPS[i].OldVert;
			/* set vertex origin */
			xyz[ 0 ] = frame->verts[j].v[0] * frame->header.scale[0] + frame->header.translate[0];
			xyz[ 1 ] = frame->verts[j].v[1] * frame->header.scale[1] + frame->header.translate[1];
			xyz[ 2 ] = frame->verts[j].v[2] * frame->header.scale[2] + frame->header.translate[2];
			PicoSetSurfaceXYZ( picoSurface, i + fm_head->numXYZ, xyz );

			/* set normal */
			normal[ 0 ] = fm_normals[frame->verts[j].lightnormalindex][0];
			normal[ 1 ] = fm_normals[frame->verts[j].lightnormalindex][1];
			normal[ 2 ] = fm_normals[frame->verts[j].lightnormalindex][2];
			PicoSetSurfaceNormal( picoSurface, i + fm_head->numXYZ, normal );

			/* set st coords */
			st[ 0 ] =  ( ( texCoord[p_index_LUT_DUPS[i].ST].s ) / ( (float)fm_head->skinWidth ) );
			st[ 1 ] =  ( texCoord[p_index_LUT_DUPS[i].ST].t / ( (float)fm_head->skinHeight ) );
			PicoSetSurfaceST( picoSurface, 0, i + fm_head->numXYZ, st );
		}
	}

	/* set color */
	PicoSetSurfaceColor( picoSurface, 0, 0, color );

	// Free up malloc'ed LL entries
	for ( i = 0; i < fm_head->numXYZ; i++ )
	{
		if ( p_index_LUT[i].next != NULL ) {
			p_index_LUT2 = p_index_LUT[i].next;
			do {
				p_index_LUT3 = p_index_LUT2->next;
				_pico_free( p_index_LUT2 );
				p_index_LUT2 = p_index_LUT3;
				dups--;
			} while ( p_index_LUT2 != NULL );
		}
	}

	if ( dups ) {
		_pico_printf( PICO_WARNING, " Not all LL mallocs freed\n" );
	}

	// Free malloc'ed LUTs
	_pico_free( p_index_LUT );
	_pico_free( p_index_LUT_DUPS );

	/* return the new pico model */
	return picoModel;

}



/* pico file format module definition */
const picoModule_t picoModuleFM =
{
	"0.85",                     /* module version string */
	"Heretic 2 FM",             /* module display name */
	"Nurail",                   /* author's name */
	"2003 Nurail",              /* module copyright */
	{
		"fm", NULL, NULL, NULL  /* default extensions to use */
	},
	_fm_canload,                /* validation routine */
	_fm_load,                   /* load routine */
	NULL,                       /* save validation routine */
	NULL                        /* save routine */
};
