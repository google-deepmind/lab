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

// piclib.h


void LoadLBM( const char *filename, byte **picture, byte **palette );
void WriteLBMfile( const char *filename, byte *data, int width, int height
				   , byte *palette );
void LoadPCX( const char *filename, byte **picture, byte **palette, int *width, int *height );
void WritePCXfile( const char *filename, byte *data, int width, int height
				   , byte *palette );

// loads / saves either lbm or pcx, depending on extension
void Load256Image( const char *name, byte **pixels, byte **palette,
				   int *width, int *height );
void Save256Image( const char *name, byte *pixels, byte *palette,
				   int width, int height );


void LoadTGA( const char *filename, byte **pixels, int *width, int *height );
void LoadTGABuffer( const byte *buffer, const byte* enddata, byte **pic, int *width, int *height );
void WriteTGA( const char *filename, byte *data, int width, int height );
int LoadJPGBuff( void *src_buffer, int src_size, unsigned char **pic, int *width, int *height );

void Load32BitImage( const char *name, unsigned **pixels, int *width, int *height );
