/* -------------------------------------------------------------------------------

   Copyright (C) 1999-2013 id Software, Inc. and contributors.
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
#define EXPORTENTS_C



/* dependencies */
#include "q3map2.h"




/* -------------------------------------------------------------------------------

   this file contains code that exports entities to a .ent file.

   ------------------------------------------------------------------------------- */

/*
   ExportEntities()
   exports the entities to a text file (.ent)
 */

void ExportEntities( void ){
        char filename[ 1024 ];
        FILE *file;
		
        /* note it */
        Sys_FPrintf( SYS_VRB, "--- ExportEntities ---\n" );
		
        /* do some path mangling */
        strcpy( filename, source );
        StripExtension( filename );
        strcat( filename, ".ent" );
		
        /* sanity check */
        if ( bspEntDataSize == 0 ) {
			Sys_FPrintf( SYS_WRN, "WARNING: No BSP entity data. aborting...\n" );
			return;
        }
		
        /* write it */
        Sys_Printf( "Writing %s\n", filename );
        Sys_FPrintf( SYS_VRB, "(%d bytes)\n", bspEntDataSize );
        file = fopen( filename, "w" );
		
        if ( file == NULL ) {
                Error( "Unable to open %s for writing", filename );
        }
		
        fprintf( file, "%s\n", bspEntData );
        fclose( file );
}



/*
   ExportEntitiesMain()
   exports the entities to a text file (.ent)
 */

int ExportEntitiesMain( int argc, char **argv ){
        /* arg checking */
        if ( argc < 1 ) {
                Sys_Printf( "Usage: q3map -exportents [-v] <mapname>\n" );
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
        ExportEntities();
		
        /* return to sender */
        return 0;
}
