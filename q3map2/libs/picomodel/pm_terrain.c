/* -----------------------------------------------------------------------------

   PicoModel Library

   Copyright (c) 2003, Randy Reddig & seaw0lf
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



/* marker */
#define PM_TERRAIN_C



/* dependencies */
#include "picointernal.h"



typedef struct tga_s
{
	unsigned char id_length, colormap_type, image_type;
	unsigned short colormap_index, colormap_length;
	unsigned char colormap_size;
	unsigned short x_origin, y_origin, width, height;
	unsigned char pixel_size, attributes;
}
tga_t;



/*
   _terrain_load_tga_buffer()
   loads a tga image into a newly allocated image buffer
   fixme: replace/clean this function
 */

void _terrain_load_tga_buffer( unsigned char *buffer, unsigned char **pic, int *width, int *height ) {
	int row, column;
	int columns, rows, numPixels;
	unsigned char   *pixbuf;
	unsigned char   *buf_p;
	tga_t targa_header;
	unsigned char   *targa_rgba;


	*pic = NULL;

	if ( buffer == NULL ) {
		return;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;

	targa_header.colormap_index = _pico_little_short( *(short*)buf_p );
	buf_p += 2;
	targa_header.colormap_length = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.y_origin = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.width = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.height = _pico_little_short( *(short*) buf_p );
	buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if ( targa_header.image_type != 2 && targa_header.image_type != 10 && targa_header.image_type != 3 ) {
		_pico_printf( PICO_ERROR, "Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n" );
		pic = NULL;
		return;
	}

	if ( targa_header.colormap_type != 0 ) {
		_pico_printf( PICO_ERROR, "Indexed color TGA images not supported\n" );
		return;
	}

	if ( targa_header.pixel_size != 32 && targa_header.pixel_size != 24 && targa_header.image_type != 3 ) {
		_pico_printf( PICO_ERROR, "Only 32 or 24 bit TGA images supported (not indexed color)\n" );
		pic = NULL;
		return;
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if ( width ) {
		*width = columns;
	}
	if ( height ) {
		*height = rows;
	}

	targa_rgba = _pico_alloc( numPixels * 4 );
	*pic = targa_rgba;

	if ( targa_header.id_length != 0 ) {
		buf_p += targa_header.id_length;  // skip TARGA image comment

	}
	if ( targa_header.image_type == 2 || targa_header.image_type == 3 ) {
		// Uncompressed RGB or gray scale image
		for ( row = rows - 1; row >= 0; row-- )
		{
			pixbuf = targa_rgba + row * columns * 4;
			for ( column = 0; column < columns; column++ )
			{
				unsigned char red,green,blue,alphabyte;
				switch ( targa_header.pixel_size )
				{

				case 8:
					blue = *buf_p++;
					green = blue;
					red = blue;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;

				case 24:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				default:
					break;
				}
			}
		}
	}

	/* rle encoded pixels */
	else if ( targa_header.image_type == 10 ) {
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;

		red = 0;
		green = 0;
		blue = 0;
		alphabyte = 0xff;

		for ( row = rows - 1; row >= 0; row-- ) {
			pixbuf = targa_rgba + row * columns * 4;
			for ( column = 0; column < columns; ) {
				packetHeader = *buf_p++;
				packetSize = 1 + ( packetHeader & 0x7f );
				if ( packetHeader & 0x80 ) {        // run-length packet
					switch ( targa_header.pixel_size ) {
					case 24:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						break;
					default:
						//Error("LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
						break;
					}

					for ( j = 0; j < packetSize; j++ ) {
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						column++;
						if ( column == columns ) { // run spans across rows
							column = 0;
							if ( row > 0 ) {
								row--;
							}
							else{
								goto breakOut;
							}
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
				else {                            // non run-length packet
					for ( j = 0; j < packetSize; j++ ) {
						switch ( targa_header.pixel_size ) {
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						default:
							//Sysprintf("LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
							break;
						}
						column++;
						if ( column == columns ) { // pixel packet run spans across rows
							column = 0;
							if ( row > 0 ) {
								row--;
							}
							else{
								goto breakOut;
							}
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
			}
breakOut:;
		}
	}

	/* fix vertically flipped image */
	if ( ( targa_header.attributes & ( 1 << 5 ) ) ) {
		int flip;
		for ( row = 0; row < .5f * rows; row++ )
		{
			for ( column = 0; column < columns; column++ )
			{
				flip = *( (int*)targa_rgba + row * columns + column );
				*( (int*)targa_rgba + row * columns + column ) = *( (int*)targa_rgba + ( ( rows - 1 ) - row ) * columns + column );
				*( (int*)targa_rgba + ( ( rows - 1 ) - row ) * columns + column ) = flip;
			}
		}
	}
}



/*
   _terrain_canload()
   validates a picoterrain file
 */

static int _terrain_canload( PM_PARAMS_CANLOAD ) {
	picoParser_t    *p;


	/* keep the friggin compiler happy */
	/**fileName = *fileName*/;

	/* create pico parser */
	p = _pico_new_parser( (picoByte_t*) buffer, bufSize );
	if ( p == NULL ) {
		return PICO_PMV_ERROR_MEMORY;
	}

	/* get first token */
	if ( _pico_parse_first( p ) == NULL ) {
		return PICO_PMV_ERROR_IDENT;
	}

	/* check first token */
	if ( _pico_stricmp( p->token, "picoterrain" ) ) {
		_pico_free_parser( p );
		return PICO_PMV_ERROR_IDENT;
	}

	/* free the pico parser object */
	_pico_free_parser( p );

	/* file seems to be a valid picoterrain file */
	return PICO_PMV_OK;
}



/*
   _terrain_load()
   loads a picoterrain file
 */

static picoModel_t *_terrain_load( PM_PARAMS_LOAD ) {
	int i, j, v, pw[ 5 ], r;
	picoParser_t    *p;

	char            *shader, *heightmapFile, *colormapFile;
	picoVec3_t scale, origin;

	unsigned char   *imageBuffer;
	int imageBufSize, w, h, cw, ch;
	unsigned char   *heightmap, *colormap, *heightPixel, *colorPixel;

	picoModel_t     *picoModel;
	picoSurface_t   *picoSurface;
	picoShader_t    *picoShader;
	picoVec3_t xyz, normal;
	picoVec2_t st;
	picoColor_t color;


	/* keep the friggin compiler happy */
	/**fileName = *fileName*/;

	/* create pico parser */
	p = _pico_new_parser( (picoByte_t*) buffer, bufSize );
	if ( p == NULL ) {
		return NULL;
	}

	/* get first token */
	if ( _pico_parse_first( p ) == NULL ) {
		return NULL;
	}

	/* check first token */
	if ( _pico_stricmp( p->token, "picoterrain" ) ) {
		_pico_printf( PICO_ERROR, "Invalid PicoTerrain model" );
		_pico_free_parser( p );
		return NULL;
	}

	/* setup */
	shader = heightmapFile = colormapFile = NULL;
	_pico_set_vec( scale, 512, 512, 32 );

	/* parse ase model file */
	while ( 1 )
	{
		/* get first token on line */
		if ( !_pico_parse_first( p ) ) {
			break;
		}

		/* skip empty lines */
		if ( !p->token || !p->token[ 0 ] ) {
			continue;
		}

		/* shader */
		if ( !_pico_stricmp( p->token, "shader" ) ) {
			if ( _pico_parse( p, 0 ) && p->token[ 0 ] ) {
				if ( shader != NULL ) {
					_pico_free( shader );
				}
				shader = _pico_clone_alloc( p->token );
			}
		}

		/* heightmap */
		else if ( !_pico_stricmp( p->token, "heightmap" ) ) {
			if ( _pico_parse( p, 0 ) && p->token[ 0 ] ) {
				if ( heightmapFile != NULL ) {
					_pico_free( heightmapFile );
				}
				heightmapFile = _pico_clone_alloc( p->token );
			}
		}

		/* colormap */
		else if ( !_pico_stricmp( p->token, "colormap" ) ) {
			if ( _pico_parse( p, 0 ) && p->token[ 0 ] ) {
				if ( colormapFile != NULL ) {
					_pico_free( colormapFile );
				}
				colormapFile = _pico_clone_alloc( p->token );
			}
		}

		/* scale */
		else if ( !_pico_stricmp( p->token, "scale" ) ) {
			_pico_parse_vec( p, scale );
		}

		/* skip unparsed rest of line and continue */
		_pico_parse_skip_rest( p );
	}

	/* ----------------------------------------------------------------- */

	/* load heightmap */
	heightmap = imageBuffer = NULL;
	_pico_load_file( heightmapFile, &imageBuffer, &imageBufSize );
	_terrain_load_tga_buffer( imageBuffer, &heightmap, &w, &h );
	_pico_free( heightmapFile );
	_pico_free_file( imageBuffer );

	if ( heightmap == NULL || w < 2 || h < 2 ) {
		_pico_printf( PICO_ERROR, "PicoTerrain model with invalid heightmap" );
		if ( shader != NULL ) {
			_pico_free( shader );
		}
		if ( colormapFile != NULL ) {
			_pico_free( colormapFile );
		}
		_pico_free_parser( p );
		return NULL;
	}

	/* set origin (bottom lowest corner of terrain mesh) */
	_pico_set_vec( origin, ( w / -2 ) * scale[ 0 ], ( h / -2 ) * scale[ 1 ], -128 * scale[ 2 ] );

	/* load colormap */
	colormap = imageBuffer = NULL;
	_pico_load_file( colormapFile, &imageBuffer, &imageBufSize );
	_terrain_load_tga_buffer( imageBuffer, &colormap, &cw, &ch );
	_pico_free( colormapFile );
	_pico_free_file( imageBuffer );

	if ( cw != w || ch != h ) {
		_pico_printf( PICO_WARNING, "PicoTerrain colormap/heightmap size mismatch" );
		_pico_free( colormap );
		colormap = NULL;
	}

	/* ----------------------------------------------------------------- */

	/* create new pico model */
	picoModel = PicoNewModel();
	if ( picoModel == NULL ) {
		_pico_printf( PICO_ERROR, "Unable to allocate a new model" );
		return NULL;
	}

	/* do model setup */
	PicoSetModelFrameNum( picoModel, frameNum );
	PicoSetModelNumFrames( picoModel, 1 ); /* sea */
	PicoSetModelName( picoModel, fileName );
	PicoSetModelFileName( picoModel, fileName );

	/* allocate new pico surface */
	picoSurface = PicoNewSurface( picoModel );
	if ( picoSurface == NULL ) {
		_pico_printf( PICO_ERROR, "Unable to allocate a new model surface" );
		PicoFreeModel( picoModel ); /* sea */
		return NULL;
	}

	/* terrain surfaces are triangle meshes */
	PicoSetSurfaceType( picoSurface, PICO_TRIANGLES );

	/* set surface name */
	PicoSetSurfaceName( picoSurface, "picoterrain" );

	/* create new pico shader */
	picoShader = PicoNewShader( picoModel );
	if ( picoShader == NULL ) {
		_pico_printf( PICO_ERROR, "Unable to allocate a new model shader" );
		PicoFreeModel( picoModel );
		_pico_free( shader );
		return NULL;
	}

	/* detox and set shader name */
	_pico_setfext( shader, "" );
	_pico_unixify( shader );
	PicoSetShaderName( picoShader, shader );
	_pico_free( shader );

	/* associate current surface with newly created shader */
	PicoSetSurfaceShader( picoSurface, picoShader );

	/* make bogus normal */
	_pico_set_vec( normal, 0.0f, 0.0f, 0.0f );

	/* create mesh */
	for ( j = 0; j < h; j++ )
	{
		for ( i = 0; i < w; i++ )
		{
			/* get pointers */
			v = i + ( j * w );
			heightPixel = heightmap + v * 4;
			colorPixel = colormap
						 ? colormap + v * 4
						 : NULL;

			/* set xyz */
			_pico_set_vec( xyz, origin[ 0 ] + scale[ 0 ] * i,
						   origin[ 1 ] + scale[ 1 ] * j,
						   origin[ 2 ] + scale[ 2 ] * heightPixel[ 0 ] );
			PicoSetSurfaceXYZ( picoSurface, v, xyz );

			/* set normal */
			PicoSetSurfaceNormal( picoSurface, v, normal );

			/* set st */
			st[ 0 ] = (float) i;
			st[ 1 ] = (float) j;
			PicoSetSurfaceST( picoSurface, 0, v, st );

			/* set color */
			if ( colorPixel != NULL ) {
				_pico_set_color( color, colorPixel[ 0 ], colorPixel[ 1 ], colorPixel[ 2 ], colorPixel[ 3 ] );
			}
			else{
				_pico_set_color( color, 255, 255, 255, 255 );
			}
			PicoSetSurfaceColor( picoSurface, 0, v, color );

			/* set triangles (zero alpha in heightmap suppresses this quad) */
			if ( i < ( w - 1 ) && j < ( h - 1 ) && heightPixel[ 3 ] >= 128 ) {
				/* set indexes */
				pw[ 0 ] = i + ( j * w );
				pw[ 1 ] = i + ( ( j + 1 ) * w );
				pw[ 2 ] = i + 1 + ( ( j + 1 ) * w );
				pw[ 3 ] = i + 1 + ( j * w );
				pw[ 4 ] = i + ( j * w );  /* same as pw[ 0 ] */

				/* set radix */
				r = ( i + j ) & 1;

				/* make first triangle */
				PicoSetSurfaceIndex( picoSurface, ( v * 6 + 0 ), (picoIndex_t) pw[ r + 0 ] );
				PicoSetSurfaceIndex( picoSurface, ( v * 6 + 1 ), (picoIndex_t) pw[ r + 1 ] );
				PicoSetSurfaceIndex( picoSurface, ( v * 6 + 2 ), (picoIndex_t) pw[ r + 2 ] );

				/* make second triangle */
				PicoSetSurfaceIndex( picoSurface, ( v * 6 + 3 ), (picoIndex_t) pw[ r + 0 ] );
				PicoSetSurfaceIndex( picoSurface, ( v * 6 + 4 ), (picoIndex_t) pw[ r + 2 ] );
				PicoSetSurfaceIndex( picoSurface, ( v * 6 + 5 ), (picoIndex_t) pw[ r + 3 ] );
			}
		}
	}

	/* free stuff */
	_pico_free_parser( p );
	_pico_free( heightmap );
	_pico_free( colormap );

	/* return the new pico model */
	return picoModel;
}



/* pico file format module definition */
const picoModule_t picoModuleTerrain =
{
	"1.3",                       /* module version string */
	"PicoTerrain",               /* module display name */
	"Randy Reddig",              /* author's name */
	"2003 Randy Reddig",     /* module copyright */
	{
		"picoterrain", NULL, NULL, NULL  /* default extensions to use */
	},
	_terrain_canload,           /* validation routine */
	_terrain_load,              /* load routine */
	NULL,                       /* save validation routine */
	NULL                        /* save routine */
};
