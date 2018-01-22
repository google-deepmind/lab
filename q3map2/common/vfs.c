/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// Rules:
//
// - Directories should be searched in the following order: ~/.q3a/baseq3,
//   install dir (/usr/local/games/quake3/baseq3) and cd_path (/mnt/cdrom/baseq3).
//
// - Pak files are searched first inside the directories.
// - Case insensitive.
// - Unix-style slashes (/) (windows is backwards .. everyone knows that)
//
// Leonardo Zide (leo@lokigames.com)
//

#include <stdio.h>

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#include <dirent.h>
#include <unistd.h>
#else
#include <wtypes.h>
#include <io.h>
#define R_OK 04
#define S_ISDIR( mode ) ( mode & _S_IFDIR )
#define PATH_MAX 260
#endif

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "cmdlib.h"
#include "mathlib.h"
#include <glib.h>
#include "inout.h"
#include "vfs.h"
#include "unzip.h"

typedef struct
{
	char*   name;
	unz_s zipinfo;
	unzFile zipfile;
	guint32 size;
} VFS_PAKFILE;

// =============================================================================
// Global variables

static GSList*  g_unzFiles;
static GSList*  g_pakFiles;
static char g_strDirs[VFS_MAXDIRS][PATH_MAX];
static int g_numDirs;
static gboolean g_bUsePak = TRUE;

// =============================================================================
// Static functions

static void vfsAddSlash( char *str ){
	int n = strlen( str );
	if ( n > 0 ) {
		if ( str[n - 1] != '\\' && str[n - 1] != '/' ) {
			strcat( str, "/" );
		}
	}
}

static void vfsFixDOSName( char *src ){
	if ( src == NULL ) {
		return;
	}

	while ( *src )
	{
		if ( *src == '\\' ) {
			*src = '/';
		}
		src++;
	}
}

//!\todo Define globally or use heap-allocated string.
#undef NAME_MAX
#define NAME_MAX 4096

static void vfsInitPakFile( const char *filename ){
	unz_global_info gi;
	unzFile uf;
	guint32 i;
	int err;

	uf = unzOpen( filename );
	if ( uf == NULL ) {
		return;
	}

	g_unzFiles = g_slist_append( g_unzFiles, uf );

	err = unzGetGlobalInfo( uf,&gi );
	if ( err != UNZ_OK ) {
		return;
	}
	unzGoToFirstFile( uf );

	for ( i = 0; i < gi.number_entry; i++ )
	{
		char filename_inzip[NAME_MAX];
		char *filename_lower;
		unz_file_info file_info;
		VFS_PAKFILE* file;

		err = unzGetCurrentFileInfo( uf, &file_info, filename_inzip, sizeof( filename_inzip ), NULL, 0, NULL, 0 );
		if ( err != UNZ_OK ) {
			break;
		}

		file = (VFS_PAKFILE*)safe_malloc( sizeof( VFS_PAKFILE ) );
		g_pakFiles = g_slist_append( g_pakFiles, file );

		vfsFixDOSName( filename_inzip );
		filename_lower = g_ascii_strdown( filename_inzip, -1 );//-1 null terminated string

		file->name = strdup( filename_lower );
		file->size = file_info.uncompressed_size;
		file->zipfile = uf;
		memcpy( &file->zipinfo, uf, sizeof( unz_s ) );

		if ( ( i + 1 ) < gi.number_entry ) {
			err = unzGoToNextFile( uf );
			if ( err != UNZ_OK ) {
				break;
			}
		}
		g_free( filename_lower );
	}
}

// =============================================================================
// Global functions

// reads all pak files from a dir
void vfsInitDirectory( const char *path ){
	char filename[PATH_MAX];
	char *dirlist;
	GDir *dir;

	if ( g_numDirs == ( VFS_MAXDIRS - 1 ) ) {
		return;
	}

	Sys_Printf( "VFS Init: %s\n", path );

	strcpy( g_strDirs[g_numDirs], path );
	vfsFixDOSName( g_strDirs[g_numDirs] );
	vfsAddSlash( g_strDirs[g_numDirs] );
	g_numDirs++;

	if ( g_bUsePak ) {
		dir = g_dir_open( path, 0, NULL );

		if ( dir != NULL ) {
			while ( 1 )
			{
				const char* name = g_dir_read_name( dir );
				if ( name == NULL ) {
					break;
				}

				dirlist = g_strdup( name );

				{

					char *ext = strrchr( dirlist, '.' );

					if ( ext != NULL && ( !Q_stricmp( ext, ".pk3dir" ) || !Q_stricmp( ext, ".dpkdir" ) ) ) {
						if ( g_numDirs == VFS_MAXDIRS ) {
							continue;
						}
						snprintf( g_strDirs[g_numDirs], PATH_MAX, "%s/%s", path, name );
						g_strDirs[g_numDirs][PATH_MAX-1] = '\0';
						vfsFixDOSName( g_strDirs[g_numDirs] );
						vfsAddSlash( g_strDirs[g_numDirs] );
						++g_numDirs;
					}

					if ( ext == NULL || ( Q_stricmp( ext, ".pk3" ) != 0 && Q_stricmp( ext, ".dpk" ) != 0 ) ) {
						continue;
					}
				}

				sprintf( filename, "%s/%s", path, dirlist );
				vfsInitPakFile( filename );

				g_free( dirlist );
			}
			g_dir_close( dir );
		}
	}
}

// frees all memory that we allocated
void vfsShutdown(){
	while ( g_unzFiles )
	{
		unzClose( (unzFile)g_unzFiles->data );
		g_unzFiles = g_slist_remove( g_unzFiles, g_unzFiles->data );
	}

	while ( g_pakFiles )
	{
		VFS_PAKFILE* file = (VFS_PAKFILE*)g_pakFiles->data;
		free( file->name );
		free( file );
		g_pakFiles = g_slist_remove( g_pakFiles, file );
	}
}

// return the number of files that match
int vfsGetFileCount( const char *filename ){
	int i, count = 0;
	char fixed[NAME_MAX], tmp[NAME_MAX];
	char *lower;
	GSList *lst;

	strcpy( fixed, filename );
	vfsFixDOSName( fixed );
	lower = g_ascii_strdown( fixed, -1 );

	for ( lst = g_pakFiles; lst != NULL; lst = g_slist_next( lst ) )
	{
		VFS_PAKFILE* file = (VFS_PAKFILE*)lst->data;

		if ( strcmp( file->name, lower ) == 0 ) {
			count++;
		}
	}

	for ( i = 0; i < g_numDirs; i++ )
	{
		strcpy( tmp, g_strDirs[i] );
		strcat( tmp, lower );
		if ( access( tmp, R_OK ) == 0 ) {
			count++;
		}
	}
	g_free( lower );
	return count;
}

// NOTE: when loading a file, you have to allocate one extra byte and set it to \0
int vfsLoadFile( const char *filename, void **bufferptr, int index ){
	int i, count = 0;
	char tmp[NAME_MAX], fixed[NAME_MAX];
	char *lower;
	GSList *lst;

	// filename is a full path
	if ( index == -1 ) {
		long len;
		FILE *f;

		f = fopen( filename, "rb" );
		if ( f == NULL ) {
			return -1;
		}

		fseek( f, 0, SEEK_END );
		len = ftell( f );
		rewind( f );

		*bufferptr = safe_malloc( len + 1 );
		if ( *bufferptr == NULL ) {
			return -1;
		}

		fread( *bufferptr, 1, len, f );
		fclose( f );

		// we need to end the buffer with a 0
		( (char*) ( *bufferptr ) )[len] = 0;

		return len;
	}

	*bufferptr = NULL;
	strcpy( fixed, filename );
	vfsFixDOSName( fixed );
	lower = g_ascii_strdown( fixed, -1 );

	for ( i = 0; i < g_numDirs; i++ )
	{
		strcpy( tmp, g_strDirs[i] );
		strcat( tmp, filename );
		if ( access( tmp, R_OK ) == 0 ) {
			if ( count == index ) {
				long len;
				FILE *f;

				f = fopen( tmp, "rb" );
				if ( f == NULL ) {
					return -1;
				}

				fseek( f, 0, SEEK_END );
				len = ftell( f );
				rewind( f );

				*bufferptr = safe_malloc( len + 1 );
				if ( *bufferptr == NULL ) {
					return -1;
				}

				fread( *bufferptr, 1, len, f );
				fclose( f );

				// we need to end the buffer with a 0
				( (char*) ( *bufferptr ) )[len] = 0;

				return len;
			}

			count++;
		}
	}

	for ( lst = g_pakFiles; lst != NULL; lst = g_slist_next( lst ) )
	{
		VFS_PAKFILE* file = (VFS_PAKFILE*)lst->data;

		if ( strcmp( file->name, lower ) != 0 ) {
			continue;
		}

		if ( count == index ) {
			memcpy( file->zipfile, &file->zipinfo, sizeof( unz_s ) );

			if ( unzOpenCurrentFile( file->zipfile ) != UNZ_OK ) {
				return -1;
			}

			*bufferptr = safe_malloc( file->size + 1 );
			// we need to end the buffer with a 0
			( (char*) ( *bufferptr ) )[file->size] = 0;

			i = unzReadCurrentFile( file->zipfile, *bufferptr, file->size );
			unzCloseCurrentFile( file->zipfile );
			if ( i < 0 ) {
				return -1;
			}
			else{
				g_free( lower );
				return file->size;
			}
		}

		count++;
	}
	g_free( lower );
	return -1;
}
