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



/* marker */
#define PICOMODEL_C



/* dependencies */
#include "picointernal.h"



/*
   PicoInit()
   initializes the picomodel library
 */

int PicoInit( void ){
	/* successfully initialized -sea */
	return 1;
}



/*
   PicoShutdown()
   shuts the pico model library down
 */

void PicoShutdown( void ){
	/* do something interesting here in the future */
	return;
}



/*
   PicoError()
   returns last picomodel error code (see PME_* defines)
 */

int PicoError( void ){
	/* todo: do something here */
	return 0;
}



/*
   PicoSetMallocFunc()
   sets the ptr to the malloc function
 */

void PicoSetMallocFunc( void *( *func )( size_t ) ){
	if ( func != NULL ) {
		_pico_ptr_malloc = func;
	}
}



/*
   PicoSetFreeFunc()
   sets the ptr to the free function
 */

void PicoSetFreeFunc( void ( *func )( void* ) ){
	if ( func != NULL ) {
		_pico_ptr_free = func;
	}
}



/*
   PicoSetLoadFileFunc()
   sets the ptr to the file load function
 */

void PicoSetLoadFileFunc( void ( *func )( char*, unsigned char**, int* ) ){
	if ( func != NULL ) {
		_pico_ptr_load_file = func;
	}
}



/*
   PicoSetFreeFileFunc()
   sets the ptr to the free function
 */

void PicoSetFreeFileFunc( void ( *func )( void* ) ){
	if ( func != NULL ) {
		_pico_ptr_free_file = func;
	}
}



/*
   PicoSetPrintFunc()
   sets the ptr to the print function
 */

void PicoSetPrintFunc( void ( *func )( int, const char* ) ){
	if ( func != NULL ) {
		_pico_ptr_print = func;
	}
}

picoModel_t *PicoModuleLoadModel( const picoModule_t* pm, char* fileName, picoByte_t* buffer, int bufSize, int frameNum ){
	char    *modelFileName, *remapFileName;

	/* see whether this module can load the model file or not */
	if ( pm->canload( fileName, buffer, bufSize ) == PICO_PMV_OK ) {
		/* use loader provided by module to read the model data */
		picoModel_t* model = pm->load( fileName, frameNum, buffer, bufSize );
		if ( model == NULL ) {
			return NULL;
		}

		/* assign pointer to file format module */
		model->module = pm;

		/* get model file name */
		modelFileName = PicoGetModelFileName( model );

		/* apply model remappings from <model>.remap */
		if ( strlen( modelFileName ) ) {
			/* alloc copy of model file name */
			remapFileName = _pico_alloc( strlen( modelFileName ) + 20 );
			if ( remapFileName != NULL ) {
				/* copy model file name and change extension */
				strcpy( remapFileName, modelFileName );
				_pico_setfext( remapFileName, "remap" );

				/* try to remap model; we don't handle the result */
				PicoRemapModel( model, remapFileName );

				/* free the remap file name string */
				_pico_free( remapFileName );
			}
		}

		return model;
	}

	return NULL;
}

/*
   PicoLoadModel()
   the meat and potatoes function
 */

picoModel_t *PicoLoadModel( char *fileName, int frameNum ){
	const picoModule_t  **modules, *pm;
	picoModel_t         *model;
	picoByte_t          *buffer;
	int bufSize;


	/* init */
	model = NULL;

	/* make sure we've got a file name */
	if ( fileName == NULL ) {
		_pico_printf( PICO_ERROR, "PicoLoadModel: No filename given (fileName == NULL)" );
		return NULL;
	}

	/* load file data (buffer is allocated by host app) */
	_pico_load_file( fileName, &buffer, &bufSize );
	if ( bufSize < 0 ) {
		_pico_printf( PICO_ERROR, "PicoLoadModel: Failed loading model %s", fileName );
		return NULL;
	}

	/* get ptr to list of supported modules */
	modules = PicoModuleList( NULL );

	/* run it through the various loader functions and try */
	/* to find a loader that fits the given file data */
	for ( ; *modules != NULL; modules++ )
	{
		/* get module */
		pm = *modules;

		/* sanity check */
		if ( pm == NULL ) {
			break;
		}

		/* module must be able to load */
		if ( pm->canload == NULL || pm->load == NULL ) {
			continue;
		}

		model = PicoModuleLoadModel( pm, fileName, buffer, bufSize, frameNum );
		if ( model != NULL ) {
			/* model was loaded, so break out of loop */
			break;
		}
	}

	/* free memory used by file buffer */
	if ( buffer ) {
		_pico_free_file( buffer );
	}

	return model;
}

/*
	FIXME: From 1.5; Unused yet
*/

picoModel_t	*PicoModuleLoadModelStream( const picoModule_t* module, void* inputStream, PicoInputStreamReadFunc inputStreamRead, size_t streamLength, int frameNum ) {
	picoModel_t			*model;
	picoByte_t			*buffer;
	int					bufSize;

	/* init */
	model = NULL;

	if ( inputStream == NULL ) {
		_pico_printf( PICO_ERROR, "PicoLoadModel: invalid input stream (inputStream == NULL)" );
		return NULL;
	}

	if ( inputStreamRead == NULL ) {
		_pico_printf( PICO_ERROR, "PicoLoadModel: invalid input stream (inputStreamRead == NULL) ");
		return NULL;
	}

	buffer = _pico_alloc( streamLength + 1 );

	bufSize = (int)inputStreamRead( inputStream, buffer, streamLength );
	buffer[ bufSize ] = '\0';

	{
		// dummy filename
		char fileName[128];
		fileName[0] = '.';
		strncpy( fileName + 1, module->defaultExts[ 0 ], 126 );
		fileName[127] = '\0';
		model = PicoModuleLoadModel( module, fileName, buffer, bufSize, frameNum );
	}

	/* free memory used by file buffer */
	if ( model != 0 ) {
		_pico_free( buffer );
	}

	return model;
}



/* ----------------------------------------------------------------------------
   models
   ---------------------------------------------------------------------------- */

/*
   PicoNewModel()
   creates a new pico model
 */

picoModel_t *PicoNewModel( void ){
	picoModel_t *model;

	/* allocate */
	model = _pico_alloc( sizeof( picoModel_t ) );
	if ( model == NULL ) {
		return NULL;
	}

	/* clear */
	memset( model, 0, sizeof( picoModel_t ) );

	/* model set up */
	_pico_zero_bounds( model->mins, model->maxs );

	/* set initial frame count to 1 -sea */
	model->numFrames = 1;

	/* return ptr to new model */
	return model;
}


/*
   PicoFreeModel()
   frees a model and all associated data
 */

void PicoFreeModel( picoModel_t *model ){
	int i;

	/* sanity check */
	if ( model == NULL ) {
		return;
	}

	/* free bits */
	if ( model->name ) {
		_pico_free( model->name );
	}

	if ( model->fileName ) {
		_pico_free( model->fileName );
	}

	/* free shaders */
	for ( i = 0; i < model->numShaders; i++ )
		PicoFreeShader( model->shader[ i ] );
	free( model->shader );

	/* free surfaces */
	for ( i = 0; i < model->numSurfaces; i++ )
		PicoFreeSurface( model->surface[ i ] );
	free( model->surface );

	/* free the model */
	_pico_free( model );
}


/*
   PicoAdjustModel()
   adjusts a models's memory allocations to handle the requested sizes.
   will always grow, never shrink
 */

int PicoAdjustModel( picoModel_t *model, int numShaders, int numSurfaces ){
	/* dummy check */
	if ( model == NULL ) {
		return 0;
	}

	/* bare minimums */
	/* sea: null surface/shader fix (1s=>0s) */
	if ( numShaders < 0 ) {
		numShaders = 0;
	}
	if ( numSurfaces < 0 ) {
		numSurfaces = 0;
	}

	/* additional shaders? */
	while ( numShaders > model->maxShaders )
	{
		model->maxShaders += PICO_GROW_SHADERS;
		if ( !_pico_realloc( (void *) &model->shader, model->numShaders * sizeof( *model->shader ), model->maxShaders * sizeof( *model->shader ) ) ) {
			return 0;
		}
	}

	/* set shader count to higher */
	if ( numShaders > model->numShaders ) {
		model->numShaders = numShaders;
	}

	/* additional surfaces? */
	while ( numSurfaces > model->maxSurfaces )
	{
		model->maxSurfaces += PICO_GROW_SURFACES;
		if ( !_pico_realloc( (void *) &model->surface, model->numSurfaces * sizeof( *model->surface ), model->maxSurfaces * sizeof( *model->surface ) ) ) {
			return 0;
		}
	}

	/* set shader count to higher */
	if ( numSurfaces > model->numSurfaces ) {
		model->numSurfaces = numSurfaces;
	}

	/* return ok */
	return 1;
}


/* ----------------------------------------------------------------------------
   shaders
   ---------------------------------------------------------------------------- */

/*
   PicoNewShader()
   creates a new pico shader and returns its index. -sea
 */

picoShader_t *PicoNewShader( picoModel_t *model ){
	picoShader_t    *shader;

	/* allocate and clear */
	shader = _pico_alloc( sizeof( picoShader_t ) );
	if ( shader == NULL ) {
		return NULL;
	}
	memset( shader, 0, sizeof( picoShader_t ) );

	/* attach it to the model */
	if ( model != NULL ) {
		/* adjust model */
		if ( !PicoAdjustModel( model, model->numShaders + 1, 0 ) ) {
			_pico_free( shader );
			return NULL;
		}
		/* attach */
		model->shader[ model->numShaders - 1 ] = shader;
		shader->model = model;
	}
	/* setup default shader colors */
	_pico_set_color( shader->ambientColor, 0, 0, 0, 0 );
	_pico_set_color( shader->diffuseColor, 255, 255, 255, 1 );
	_pico_set_color( shader->specularColor, 0, 0, 0, 0 );

	/* no need to do this, but i do it anyway */
	shader->transparency = 0;
	shader->shininess = 0;

	/* return the newly created shader */
	return shader;
}


/*
   PicoFreeShader()
   frees a shader and all associated data -sea
 */

void PicoFreeShader( picoShader_t *shader ){
	/* dummy check */
	if ( shader == NULL ) {
		return;
	}

	/* free bits */
	if ( shader->name ) {
		_pico_free( shader->name );
	}
	if ( shader->mapName ) {
		_pico_free( shader->mapName );
	}

	/* free the shader */
	_pico_free( shader );
}


/*
   PicoFindShader()
   finds a named shader in a model
 */

picoShader_t *PicoFindShader( picoModel_t *model, char *name, int caseSensitive ){
	int i;

	/* sanity checks */
	if ( model == NULL || name == NULL ) { /* sea: null name fix */
		return NULL;
	}

	/* walk list */
	for ( i = 0; i < model->numShaders; i++ )
	{
		/* skip null shaders or shaders with null names */
		if ( model->shader[ i ] == NULL ||
			 model->shader[ i ]->name == NULL ) {
			continue;
		}

		/* compare the shader name with name we're looking for */
		if ( caseSensitive ) {
			if ( !strcmp( name, model->shader[ i ]->name ) ) {
				return model->shader[ i ];
			}
		}
		else if ( !_pico_stricmp( name, model->shader[ i ]->name ) ) {
			return model->shader[ i ];
		}
	}

	/* named shader not found */
	return NULL;
}


/* ----------------------------------------------------------------------------
   surfaces
   ---------------------------------------------------------------------------- */

/*
   PicoNewSurface()
   creates a new pico surface
 */

picoSurface_t *PicoNewSurface( picoModel_t *model ){
	picoSurface_t   *surface;
	char surfaceName[64];

	/* allocate and clear */
	surface = _pico_alloc( sizeof( *surface ) );
	if ( surface == NULL ) {
		return NULL;
	}
	memset( surface, 0, sizeof( *surface ) );

	/* attach it to the model */
	if ( model != NULL ) {
		/* adjust model */
		if ( !PicoAdjustModel( model, 0, model->numSurfaces + 1 ) ) {
			_pico_free( surface );
			return NULL;
		}

		/* attach */
		model->surface[ model->numSurfaces - 1 ] = surface;
		surface->model = model;

		/* set default name */
		sprintf( surfaceName, "Unnamed_%d", model->numSurfaces );
		PicoSetSurfaceName( surface, surfaceName );
	}

	/* return */
	return surface;
}


/*
   PicoFreeSurface()
   frees a surface and all associated data
 */
void PicoFreeSurface( picoSurface_t *surface ){
	int i;

	/* dummy check */
	if ( surface == NULL ) {
		return;
	}

	/* free bits */
	_pico_free( surface->xyz );
	_pico_free( surface->normal );
	_pico_free( surface->smoothingGroup );
	_pico_free( surface->index );
	_pico_free( surface->faceNormal );

	if ( surface->name ) {
		_pico_free( surface->name );
	}

	/* free arrays */
	for ( i = 0; i < surface->numSTArrays; i++ )
		_pico_free( surface->st[ i ] );
	free( surface->st );
	for ( i = 0; i < surface->numColorArrays; i++ )
		_pico_free( surface->color[ i ] );
	free( surface->color );

	/* free the surface */
	_pico_free( surface );
}


/*
   PicoAdjustSurface()
   adjusts a surface's memory allocations to handle the requested sizes.
   will always grow, never shrink
 */

int PicoAdjustSurface( picoSurface_t *surface, int numVertexes, int numSTArrays, int numColorArrays, int numIndexes, int numFaceNormals ){
	int i;

	/* dummy check */
	if ( surface == NULL ) {
		return 0;
	}

	/* bare minimums */
	if ( numVertexes < 1 ) {
		numVertexes = 1;
	}
	if ( numSTArrays < 1 ) {
		numSTArrays = 1;
	}
	if ( numColorArrays < 1 ) {
		numColorArrays = 1;
	}
	if ( numIndexes < 1 ) {
		numIndexes = 1;
	}

	/* additional vertexes? */
	while ( numVertexes > surface->maxVertexes ) /* fix */
	{
		surface->maxVertexes += PICO_GROW_VERTEXES;
		if ( !_pico_realloc( (void *) &surface->xyz, surface->numVertexes * sizeof( *surface->xyz ), surface->maxVertexes * sizeof( *surface->xyz ) ) ) {
			return 0;
		}
		if ( !_pico_realloc( (void *) &surface->normal, surface->numVertexes * sizeof( *surface->normal ), surface->maxVertexes * sizeof( *surface->normal ) ) ) {
			return 0;
		}
		if ( !_pico_realloc( (void *) &surface->smoothingGroup, surface->numVertexes * sizeof( *surface->smoothingGroup ), surface->maxVertexes * sizeof( *surface->smoothingGroup ) ) ) {
			return 0;
		}
		for ( i = 0; i < surface->numSTArrays; i++ )
			if ( !_pico_realloc( (void*) &surface->st[ i ], surface->numVertexes * sizeof( *surface->st[ i ] ), surface->maxVertexes * sizeof( *surface->st[ i ] ) ) ) {
				return 0;
			}
		for ( i = 0; i < surface->numColorArrays; i++ )
			if ( !_pico_realloc( (void*) &surface->color[ i ], surface->numVertexes * sizeof( *surface->color[ i ] ), surface->maxVertexes * sizeof( *surface->color[ i ] ) ) ) {
				return 0;
			}
	}

	/* set vertex count to higher */
	if ( numVertexes > surface->numVertexes ) {
		surface->numVertexes = numVertexes;
	}

	/* additional st arrays? */
	while ( numSTArrays > surface->maxSTArrays ) /* fix */
	{
		surface->maxSTArrays += PICO_GROW_ARRAYS;
		if ( !_pico_realloc( (void*) &surface->st, surface->numSTArrays * sizeof( *surface->st ), surface->maxSTArrays * sizeof( *surface->st ) ) ) {
			return 0;
		}
		while ( surface->numSTArrays < numSTArrays )
		{
			surface->st[ surface->numSTArrays ] = _pico_alloc( surface->maxVertexes * sizeof( *surface->st[ 0 ] ) );
			memset( surface->st[ surface->numSTArrays ], 0, surface->maxVertexes * sizeof( *surface->st[ 0 ] ) );
			surface->numSTArrays++;
		}
	}

	/* additional color arrays? */
	while ( numColorArrays > surface->maxColorArrays ) /* fix */
	{
		surface->maxColorArrays += PICO_GROW_ARRAYS;
		if ( !_pico_realloc( (void*) &surface->color, surface->numColorArrays * sizeof( *surface->color ), surface->maxColorArrays * sizeof( *surface->color ) ) ) {
			return 0;
		}
		while ( surface->numColorArrays < numColorArrays )
		{
			surface->color[ surface->numColorArrays ] = _pico_alloc( surface->maxVertexes * sizeof( *surface->color[ 0 ] ) );
			memset( surface->color[ surface->numColorArrays ], 0, surface->maxVertexes * sizeof( *surface->color[ 0 ] ) );
			surface->numColorArrays++;
		}
	}

	/* additional indexes? */
	while ( numIndexes > surface->maxIndexes ) /* fix */
	{
		surface->maxIndexes += PICO_GROW_INDEXES;
		if ( !_pico_realloc( (void*) &surface->index, surface->numIndexes * sizeof( *surface->index ), surface->maxIndexes * sizeof( *surface->index ) ) ) {
			return 0;
		}
	}

	/* set index count to higher */
	if ( numIndexes > surface->numIndexes ) {
		surface->numIndexes = numIndexes;
	}

	/* additional face normals? */
	while ( numFaceNormals > surface->maxFaceNormals ) /* fix */
	{
		surface->maxFaceNormals += PICO_GROW_FACES;
		if ( !_pico_realloc( (void *) &surface->faceNormal, surface->numFaceNormals * sizeof( *surface->faceNormal ), surface->maxFaceNormals * sizeof( *surface->faceNormal ) ) ) {
			return 0;
		}
	}

	/* set face normal count to higher */
	if ( numFaceNormals > surface->numFaceNormals ) {
		surface->numFaceNormals = numFaceNormals;
	}

	/* return ok */
	return 1;
}


/* PicoFindSurface:
 *   Finds first matching named surface in a model.
 */
picoSurface_t *PicoFindSurface(
	picoModel_t *model, char *name, int caseSensitive ){
	int i;

	/* sanity check */
	if ( model == NULL || name == NULL ) {
		return NULL;
	}

	/* walk list */
	for ( i = 0; i < model->numSurfaces; i++ )
	{
		/* skip null surfaces or surfaces with null names */
		if ( model->surface[ i ] == NULL ||
			 model->surface[ i ]->name == NULL ) {
			continue;
		}

		/* compare the surface name with name we're looking for */
		if ( caseSensitive ) {
			if ( !strcmp( name,model->surface[ i ]->name ) ) {
				return model->surface[ i ];
			}
		}
		else {
			if ( !_pico_stricmp( name,model->surface[ i ]->name ) ) {
				return model->surface[ i ];
			}
		}
	}
	/* named surface not found */
	return NULL;
}



/*----------------------------------------------------------------------------
   PicoSet*() Setter Functions
   ----------------------------------------------------------------------------*/

void PicoSetModelName( picoModel_t *model, char *name ){
	if ( model == NULL || name == NULL ) {
		return;
	}
	if ( model->name != NULL ) {
		_pico_free( model->name );
	}

	model->name = _pico_clone_alloc( name );
}



void PicoSetModelFileName( picoModel_t *model, char *fileName ){
	if ( model == NULL || fileName == NULL ) {
		return;
	}
	if ( model->fileName != NULL ) {
		_pico_free( model->fileName );
	}

	model->fileName = _pico_clone_alloc( fileName );
}



void PicoSetModelFrameNum( picoModel_t *model, int frameNum ){
	if ( model == NULL ) {
		return;
	}
	model->frameNum = frameNum;
}



void PicoSetModelNumFrames( picoModel_t *model, int numFrames ){
	if ( model == NULL ) {
		return;
	}
	model->numFrames = numFrames;
}



void PicoSetModelData( picoModel_t *model, void *data ){
	if ( model == NULL ) {
		return;
	}
	model->data = data;
}



void PicoSetShaderName( picoShader_t *shader, char *name ){
	if ( shader == NULL || name == NULL ) {
		return;
	}
	if ( shader->name != NULL ) {
		_pico_free( shader->name );
	}

	shader->name = _pico_clone_alloc( name );
}



void PicoSetShaderMapName( picoShader_t *shader, char *mapName ){
	if ( shader == NULL || mapName == NULL ) {
		return;
	}
	if ( shader->mapName != NULL ) {
		_pico_free( shader->mapName );
	}

	shader->mapName = _pico_clone_alloc( mapName );
}



void PicoSetShaderAmbientColor( picoShader_t *shader, picoColor_t color ){
	if ( shader == NULL || color == NULL ) {
		return;
	}
	shader->ambientColor[ 0 ] = color[ 0 ];
	shader->ambientColor[ 1 ] = color[ 1 ];
	shader->ambientColor[ 2 ] = color[ 2 ];
	shader->ambientColor[ 3 ] = color[ 3 ];
}



void PicoSetShaderDiffuseColor( picoShader_t *shader, picoColor_t color ){
	if ( shader == NULL || color == NULL ) {
		return;
	}
	shader->diffuseColor[ 0 ] = color[ 0 ];
	shader->diffuseColor[ 1 ] = color[ 1 ];
	shader->diffuseColor[ 2 ] = color[ 2 ];
	shader->diffuseColor[ 3 ] = color[ 3 ];
}



void PicoSetShaderSpecularColor( picoShader_t *shader, picoColor_t color ){
	if ( shader == NULL || color == NULL ) {
		return;
	}
	shader->specularColor[ 0 ] = color[ 0 ];
	shader->specularColor[ 1 ] = color[ 1 ];
	shader->specularColor[ 2 ] = color[ 2 ];
	shader->specularColor[ 3 ] = color[ 3 ];
}



void PicoSetShaderTransparency( picoShader_t *shader, float value ){
	if ( shader == NULL ) {
		return;
	}
	shader->transparency = value;

	/* cap to 0..1 range */
	if ( shader->transparency < 0.0 ) {
		shader->transparency = 0.0;
	}
	if ( shader->transparency > 1.0 ) {
		shader->transparency = 1.0;
	}
}



void PicoSetShaderShininess( picoShader_t *shader, float value ){
	if ( shader == NULL ) {
		return;
	}
	shader->shininess = value;

	/* cap to 0..127 range */
	if ( shader->shininess < 0.0 ) {
		shader->shininess = 0.0;
	}
	if ( shader->shininess > 127.0 ) {
		shader->shininess = 127.0;
	}
}



void PicoSetSurfaceData( picoSurface_t *surface, void *data ){
	if ( surface == NULL ) {
		return;
	}
	surface->data = data;
}



void PicoSetSurfaceType( picoSurface_t *surface, picoSurfaceType_t type ){
	if ( surface == NULL ) {
		return;
	}
	surface->type = type;
}



void PicoSetSurfaceName( picoSurface_t *surface, char *name ){
	if ( surface == NULL || name == NULL ) {
		return;
	}
	if ( surface->name != NULL ) {
		_pico_free( surface->name );
	}

	surface->name = _pico_clone_alloc( name );
}



void PicoSetSurfaceShader( picoSurface_t *surface, picoShader_t *shader ){
	if ( surface == NULL ) {
		return;
	}
	surface->shader = shader;
}



void PicoSetSurfaceXYZ( picoSurface_t *surface, int num, picoVec3_t xyz ){
	if ( surface == NULL || num < 0 || xyz == NULL ) {
		return;
	}
	if ( !PicoAdjustSurface( surface, num + 1, 0, 0, 0, 0 ) ) {
		return;
	}
	_pico_copy_vec( xyz, surface->xyz[ num ] );
	if ( surface->model != NULL ) {
		_pico_expand_bounds( xyz, surface->model->mins, surface->model->maxs );
	}
}



void PicoSetSurfaceNormal( picoSurface_t *surface, int num, picoVec3_t normal ){
	if ( surface == NULL || num < 0 || normal == NULL ) {
		return;
	}
	if ( !PicoAdjustSurface( surface, num + 1, 0, 0, 0, 0 ) ) {
		return;
	}
	_pico_copy_vec( normal, surface->normal[ num ] );
}



void PicoSetSurfaceST( picoSurface_t *surface, int array, int num, picoVec2_t st ){
	if ( surface == NULL || num < 0 || st == NULL ) {
		return;
	}
	if ( !PicoAdjustSurface( surface, num + 1, array + 1, 0, 0, 0 ) ) {
		return;
	}
	surface->st[ array ][ num ][ 0 ] = st[ 0 ];
	surface->st[ array ][ num ][ 1 ] = st[ 1 ];
}



void PicoSetSurfaceColor( picoSurface_t *surface, int array, int num, picoColor_t color ){
	if ( surface == NULL || num < 0 || color == NULL ) {
		return;
	}
	if ( !PicoAdjustSurface( surface, num + 1, 0, array + 1, 0, 0 ) ) {
		return;
	}
	surface->color[ array ][ num ][ 0 ] = color[ 0 ];
	surface->color[ array ][ num ][ 1 ] = color[ 1 ];
	surface->color[ array ][ num ][ 2 ] = color[ 2 ];
	surface->color[ array ][ num ][ 3 ] = color[ 3 ];
}



void PicoSetSurfaceIndex( picoSurface_t *surface, int num, picoIndex_t index ){
	if ( surface == NULL || num < 0 ) {
		return;
	}
	if ( !PicoAdjustSurface( surface, 0, 0, 0, num + 1, 0 ) ) {
		return;
	}
	surface->index[ num ] = index;
}



void PicoSetSurfaceIndexes( picoSurface_t *surface, int num, picoIndex_t *index, int count ){
	if ( num < 0 || index == NULL || count < 1 ) {
		return;
	}
	if ( !PicoAdjustSurface( surface, 0, 0, 0, num + count, 0 ) ) {
		return;
	}
	memcpy( &surface->index[ num ], index, count * sizeof( surface->index[ num ] ) );
}



void PicoSetFaceNormal( picoSurface_t *surface, int num, picoVec3_t normal ){
	if ( surface == NULL || num < 0 || normal == NULL ) {
		return;
	}
	if ( !PicoAdjustSurface( surface, 0, 0, 0, 0, num + 1 ) ) {
		return;
	}
	_pico_copy_vec( normal, surface->faceNormal[ num ] );
}

void PicoSetSurfaceSmoothingGroup( picoSurface_t *surface, int num, picoIndex_t smoothingGroup ){
	if ( num < 0 ) {
		return;
	}
	if ( !PicoAdjustSurface( surface, num + 1, 0, 0, 0, 0 ) ) {
		return;
	}
	surface->smoothingGroup[ num ] = smoothingGroup;
}

void PicoSetSurfaceSpecial( picoSurface_t *surface, int num, int special ){
	if ( surface == NULL || num < 0 || num >= PICO_MAX_SPECIAL ) {
		return;
	}
	surface->special[ num ] = special;
}



/*----------------------------------------------------------------------------
   PicoGet*() Getter Functions
   ----------------------------------------------------------------------------*/

char *PicoGetModelName( picoModel_t *model ){
	if ( model == NULL ) {
		return NULL;
	}
	if ( model->name == NULL ) {
		return (char*) "";
	}
	return model->name;
}



char *PicoGetModelFileName( picoModel_t *model ){
	if ( model == NULL ) {
		return NULL;
	}
	if ( model->fileName == NULL ) {
		return (char*) "";
	}
	return model->fileName;
}



int PicoGetModelFrameNum( picoModel_t *model ){
	if ( model == NULL ) {
		return 0;
	}
	return model->frameNum;
}



int PicoGetModelNumFrames( picoModel_t *model ){
	if ( model == NULL ) {
		return 0;
	}
	return model->numFrames;
}



void *PicoGetModelData( picoModel_t *model ){
	if ( model == NULL ) {
		return NULL;
	}
	return model->data;
}



int PicoGetModelNumShaders( picoModel_t *model ){
	if ( model == NULL ) {
		return 0;
	}
	return model->numShaders;
}



picoShader_t *PicoGetModelShader( picoModel_t *model, int num ){
	/* a few sanity checks */
	if ( model == NULL ) {
		return NULL;
	}
	if ( model->shader == NULL ) {
		return NULL;
	}
	if ( num < 0 || num >= model->numShaders ) {
		return NULL;
	}

	/* return the shader */
	return model->shader[ num ];
}



int PicoGetModelNumSurfaces( picoModel_t *model ){
	if ( model == NULL ) {
		return 0;
	}
	return model->numSurfaces;
}



picoSurface_t *PicoGetModelSurface( picoModel_t *model, int num ){
	/* a few sanity checks */
	if ( model == NULL ) {
		return NULL;
	}
	if ( model->surface == NULL ) {
		return NULL;
	}
	if ( num < 0 || num >= model->numSurfaces ) {
		return NULL;
	}

	/* return the surface */
	return model->surface[ num ];
}



int PicoGetModelTotalVertexes( picoModel_t *model ){
	int i, count;


	if ( model == NULL ) {
		return 0;
	}
	if ( model->surface == NULL ) {
		return 0;
	}

	count = 0;
	for ( i = 0; i < model->numSurfaces; i++ )
		count += PicoGetSurfaceNumVertexes( model->surface[ i ] );

	return count;
}



int PicoGetModelTotalIndexes( picoModel_t *model ){
	int i, count;


	if ( model == NULL ) {
		return 0;
	}
	if ( model->surface == NULL ) {
		return 0;
	}

	count = 0;
	for ( i = 0; i < model->numSurfaces; i++ )
		count += PicoGetSurfaceNumIndexes( model->surface[ i ] );

	return count;
}



char *PicoGetShaderName( picoShader_t *shader ){
	if ( shader == NULL ) {
		return NULL;
	}
	if ( shader->name == NULL ) {
		return (char*) "";
	}
	return shader->name;
}



char *PicoGetShaderMapName( picoShader_t *shader ){
	if ( shader == NULL ) {
		return NULL;
	}
	if ( shader->mapName == NULL ) {
		return (char*) "";
	}
	return shader->mapName;
}



picoByte_t *PicoGetShaderAmbientColor( picoShader_t *shader ){
	if ( shader == NULL ) {
		return NULL;
	}
	return shader->ambientColor;
}



picoByte_t *PicoGetShaderDiffuseColor( picoShader_t *shader ){
	if ( shader == NULL ) {
		return NULL;
	}
	return shader->diffuseColor;
}



picoByte_t *PicoGetShaderSpecularColor( picoShader_t *shader ){
	if ( shader == NULL ) {
		return NULL;
	}
	return shader->specularColor;
}



float PicoGetShaderTransparency( picoShader_t *shader ){
	if ( shader == NULL ) {
		return 0.0f;
	}
	return shader->transparency;
}



float PicoGetShaderShininess( picoShader_t *shader ){
	if ( shader == NULL ) {
		return 0.0f;
	}
	return shader->shininess;
}



void *PicoGetSurfaceData( picoSurface_t *surface ){
	if ( surface == NULL ) {
		return NULL;
	}
	return surface->data;
}



picoSurfaceType_t PicoGetSurfaceType( picoSurface_t *surface ){
	if ( surface == NULL ) {
		return PICO_BAD;
	}
	return surface->type;
}



char *PicoGetSurfaceName( picoSurface_t *surface ){
	if ( surface == NULL ) {
		return NULL;
	}
	if ( surface->name == NULL ) {
		return (char*) "";
	}
	return surface->name;
}



picoShader_t *PicoGetSurfaceShader( picoSurface_t *surface ){
	if ( surface == NULL ) {
		return NULL;
	}
	return surface->shader;
}



int PicoGetSurfaceNumVertexes( picoSurface_t *surface ){
	if ( surface == NULL ) {
		return 0;
	}
	return surface->numVertexes;
}



picoVec_t *PicoGetSurfaceXYZ( picoSurface_t *surface, int num ){
	if ( surface == NULL || num < 0 || num > surface->numVertexes ) {
		return NULL;
	}
	return surface->xyz[ num ];
}



picoVec_t *PicoGetSurfaceNormal( picoSurface_t *surface, int num ){
	if ( surface == NULL || num < 0 || num > surface->numVertexes ) {
		return NULL;
	}
	return surface->normal[ num ];
}



picoVec_t *PicoGetSurfaceST( picoSurface_t *surface, int array, int num ){
	if ( surface == NULL || array < 0 || array > surface->numSTArrays || num < 0 || num > surface->numVertexes ) {
		return NULL;
	}
	return surface->st[ array ][ num ];
}



picoByte_t *PicoGetSurfaceColor( picoSurface_t *surface, int array, int num ){
	if ( surface == NULL || array < 0 || array > surface->numColorArrays || num < 0 || num > surface->numVertexes ) {
		return NULL;
	}
	return surface->color[ array ][ num ];
}



int PicoGetSurfaceNumIndexes( picoSurface_t *surface ){
	if ( surface == NULL ) {
		return 0;
	}
	return surface->numIndexes;
}



picoIndex_t PicoGetSurfaceIndex( picoSurface_t *surface, int num ){
	if ( surface == NULL || num < 0 || num > surface->numIndexes ) {
		return 0;
	}
	return surface->index[ num ];
}



picoIndex_t *PicoGetSurfaceIndexes( picoSurface_t *surface, int num ){
	if ( surface == NULL || num < 0 || num > surface->numIndexes ) {
		return NULL;
	}
	return &surface->index[ num ];
}


picoVec_t *PicoGetFaceNormal( picoSurface_t *surface, int num ){
	if ( surface == NULL || num < 0 || num > surface->numFaceNormals ) {
		return NULL;
	}
	return surface->faceNormal[ num ];
}

picoIndex_t PicoGetSurfaceSmoothingGroup( picoSurface_t *surface, int num ){
	if ( surface == NULL || num < 0 || num > surface->numVertexes ) {
		return -1;
	}
	return surface->smoothingGroup[ num ];
}

int PicoGetSurfaceSpecial( picoSurface_t *surface, int num ){
	if ( surface == NULL || num < 0 || num >= PICO_MAX_SPECIAL ) {
		return 0;
	}
	return surface->special[ num ];
}



/* ----------------------------------------------------------------------------
   hashtable related functions
   ---------------------------------------------------------------------------- */

/* hashtable code for faster vertex lookups */
//#define HASHTABLE_SIZE 32768 // 2048			/* power of 2, use & */
#define HASHTABLE_SIZE 7919 // 32749 // 2039    /* prime, use % */

int PicoGetHashTableSize( void ){
	return HASHTABLE_SIZE;
}

#define HASH_USE_EPSILON

#ifdef HASH_USE_EPSILON
#define HASH_XYZ_EPSILON                    0.01f
#define HASH_XYZ_EPSILONSPACE_MULTIPLIER    1.f / HASH_XYZ_EPSILON
#define HASH_ST_EPSILON                     0.0001f
#define HASH_NORMAL_EPSILON                 0.02f
#endif

unsigned int PicoVertexCoordGenerateHash( picoVec3_t xyz ){
	unsigned int hash = 0;

#ifndef HASH_USE_EPSILON
	hash += ~( *( (unsigned int*) &xyz[ 0 ] ) << 15 );
	hash ^= ( *( (unsigned int*) &xyz[ 0 ] ) >> 10 );
	hash += ( *( (unsigned int*) &xyz[ 1 ] ) << 3 );
	hash ^= ( *( (unsigned int*) &xyz[ 1 ] ) >> 6 );
	hash += ~( *( (unsigned int*) &xyz[ 2 ] ) << 11 );
	hash ^= ( *( (unsigned int*) &xyz[ 2 ] ) >> 16 );
#else
	picoVec3_t xyz_epsilonspace;

	_pico_scale_vec( xyz, HASH_XYZ_EPSILONSPACE_MULTIPLIER, xyz_epsilonspace );
	xyz_epsilonspace[ 0 ] = (float)floor( xyz_epsilonspace[ 0 ] );
	xyz_epsilonspace[ 1 ] = (float)floor( xyz_epsilonspace[ 1 ] );
	xyz_epsilonspace[ 2 ] = (float)floor( xyz_epsilonspace[ 2 ] );

	hash += ~( *( (unsigned int*) &xyz_epsilonspace[ 0 ] ) << 15 );
	hash ^= ( *( (unsigned int*) &xyz_epsilonspace[ 0 ] ) >> 10 );
	hash += ( *( (unsigned int*) &xyz_epsilonspace[ 1 ] ) << 3 );
	hash ^= ( *( (unsigned int*) &xyz_epsilonspace[ 1 ] ) >> 6 );
	hash += ~( *( (unsigned int*) &xyz_epsilonspace[ 2 ] ) << 11 );
	hash ^= ( *( (unsigned int*) &xyz_epsilonspace[ 2 ] ) >> 16 );
#endif

	//hash = hash & (HASHTABLE_SIZE-1);
	hash = hash % ( HASHTABLE_SIZE );
	return hash;
}

picoVertexCombinationHash_t **PicoNewVertexCombinationHashTable( void ){
	picoVertexCombinationHash_t **hashTable = _pico_alloc( HASHTABLE_SIZE * sizeof( picoVertexCombinationHash_t* ) );

	memset( hashTable, 0, HASHTABLE_SIZE * sizeof( picoVertexCombinationHash_t* ) );

	return hashTable;
}

void PicoFreeVertexCombinationHashTable( picoVertexCombinationHash_t **hashTable ){
	int i;
	picoVertexCombinationHash_t *vertexCombinationHash;
	picoVertexCombinationHash_t *nextVertexCombinationHash;

	/* dummy check */
	if ( hashTable == NULL ) {
		return;
	}

	for ( i = 0; i < HASHTABLE_SIZE; i++ )
	{
		if ( hashTable[ i ] ) {
			nextVertexCombinationHash = NULL;

			for ( vertexCombinationHash = hashTable[ i ]; vertexCombinationHash; vertexCombinationHash = nextVertexCombinationHash )
			{
				nextVertexCombinationHash = vertexCombinationHash->next;
				if ( vertexCombinationHash->data != NULL ) {
					_pico_free( vertexCombinationHash->data );
				}
				_pico_free( vertexCombinationHash );
			}
		}
	}

	_pico_free( hashTable );
}

picoVertexCombinationHash_t *PicoFindVertexCombinationInHashTable( picoVertexCombinationHash_t **hashTable, picoVec3_t xyz, picoVec3_t normal, picoVec3_t st, picoColor_t color ){
	unsigned int hash;
	picoVertexCombinationHash_t *vertexCombinationHash;

	/* dumy check */
	if ( hashTable == NULL || xyz == NULL || normal == NULL || st == NULL || color == NULL ) {
		return NULL;
	}

	hash = PicoVertexCoordGenerateHash( xyz );

	for ( vertexCombinationHash = hashTable[ hash ]; vertexCombinationHash; vertexCombinationHash = vertexCombinationHash->next )
	{
#ifndef HASH_USE_EPSILON
		/* check xyz */
		if ( ( vertexCombinationHash->vcd.xyz[ 0 ] != xyz[ 0 ] || vertexCombinationHash->vcd.xyz[ 1 ] != xyz[ 1 ] || vertexCombinationHash->vcd.xyz[ 2 ] != xyz[ 2 ] ) ) {
			continue;
		}

		/* check normal */
		if ( ( vertexCombinationHash->vcd.normal[ 0 ] != normal[ 0 ] || vertexCombinationHash->vcd.normal[ 1 ] != normal[ 1 ] || vertexCombinationHash->vcd.normal[ 2 ] != normal[ 2 ] ) ) {
			continue;
		}

		/* check st */
		if ( vertexCombinationHash->vcd.st[ 0 ] != st[ 0 ] || vertexCombinationHash->vcd.st[ 1 ] != st[ 1 ] ) {
			continue;
		}
#else
		/* check xyz */
		if ( ( fabs( xyz[ 0 ] - vertexCombinationHash->vcd.xyz[ 0 ] ) ) > HASH_XYZ_EPSILON ||
			 ( fabs( xyz[ 1 ] - vertexCombinationHash->vcd.xyz[ 1 ] ) ) > HASH_XYZ_EPSILON ||
			 ( fabs( xyz[ 2 ] - vertexCombinationHash->vcd.xyz[ 2 ] ) ) > HASH_XYZ_EPSILON ) {
			continue;
		}

		/* check normal */
		if ( ( fabs( normal[ 0 ] - vertexCombinationHash->vcd.normal[ 0 ] ) ) > HASH_NORMAL_EPSILON ||
			 ( fabs( normal[ 1 ] - vertexCombinationHash->vcd.normal[ 1 ] ) ) > HASH_NORMAL_EPSILON ||
			 ( fabs( normal[ 2 ] - vertexCombinationHash->vcd.normal[ 2 ] ) ) > HASH_NORMAL_EPSILON ) {
			continue;
		}

		/* check st */
		if ( ( fabs( st[ 0 ] - vertexCombinationHash->vcd.st[ 0 ] ) ) > HASH_ST_EPSILON ||
			 ( fabs( st[ 1 ] - vertexCombinationHash->vcd.st[ 1 ] ) ) > HASH_ST_EPSILON ) {
			continue;
		}
#endif

		/* check color */
		if ( *( (int*) vertexCombinationHash->vcd.color ) != *( (int*) color ) ) {
			continue;
		}

		/* gotcha */
		return vertexCombinationHash;
	}

	return NULL;
}

picoVertexCombinationHash_t *PicoAddVertexCombinationToHashTable( picoVertexCombinationHash_t **hashTable, picoVec3_t xyz, picoVec3_t normal, picoVec3_t st, picoColor_t color, picoIndex_t index ){
	unsigned int hash;
	picoVertexCombinationHash_t *vertexCombinationHash;

	/* dumy check */
	if ( hashTable == NULL || xyz == NULL || normal == NULL || st == NULL || color == NULL ) {
		return NULL;
	}

	vertexCombinationHash = _pico_alloc( sizeof( picoVertexCombinationHash_t ) );

	if ( !vertexCombinationHash ) {
		return NULL;
	}

	hash = PicoVertexCoordGenerateHash( xyz );

	_pico_copy_vec( xyz, vertexCombinationHash->vcd.xyz );
	_pico_copy_vec( normal, vertexCombinationHash->vcd.normal );
	_pico_copy_vec2( st, vertexCombinationHash->vcd.st );
	_pico_copy_color( color, vertexCombinationHash->vcd.color );
	vertexCombinationHash->index = index;
	vertexCombinationHash->data = NULL;
	vertexCombinationHash->next = hashTable[ hash ];
	hashTable[ hash ] = vertexCombinationHash;

	return vertexCombinationHash;
}

/* ----------------------------------------------------------------------------
   specialized routines
   ---------------------------------------------------------------------------- */

/*
   PicoFindSurfaceVertex()
   finds a vertex matching the set parameters
   fixme: needs non-naive algorithm
 */

int PicoFindSurfaceVertexNum( picoSurface_t *surface, picoVec3_t xyz, picoVec3_t normal, int numSTs, picoVec2_t *st, int numColors, picoColor_t *color, picoIndex_t smoothingGroup ){
	int i, j;


	/* dummy check */
	if ( surface == NULL || surface->numVertexes <= 0 ) {
		return -1;
	}

	/* walk vertex list */
	for ( i = 0; i < surface->numVertexes; i++ )
	{
		/* check xyz */
		if ( xyz != NULL && ( surface->xyz[ i ][ 0 ] != xyz[ 0 ] || surface->xyz[ i ][ 1 ] != xyz[ 1 ] || surface->xyz[ i ][ 2 ] != xyz[ 2 ] ) ) {
			continue;
		}

		/* check normal */
		if ( normal != NULL && ( surface->normal[ i ][ 0 ] != normal[ 0 ] || surface->normal[ i ][ 1 ] != normal[ 1 ] || surface->normal[ i ][ 2 ] != normal[ 2 ] ) ) {
			continue;
		}

		/* check normal */
		if ( surface->smoothingGroup[ i ] != smoothingGroup ) {
			continue;
		}

		/* check st */
		if ( numSTs > 0 && st != NULL ) {
			for ( j = 0; j < numSTs; j++ )
			{
				if ( surface->st[ j ][ i ][ 0 ] != st[ j ][ 0 ] || surface->st[ j ][ i ][ 1 ] != st[ j ][ 1 ] ) {
					break;
				}
			}
			if ( j != numSTs ) {
				continue;
			}
		}

		/* check color */
		if ( numColors > 0 && color != NULL ) {
			for ( j = 0; j < numSTs; j++ )
			{
				if ( *( (int*) surface->color[ j ] ) != *( (int*) color[ j ] ) ) {
					break;
				}
			}
			if ( j != numColors ) {
				continue;
			}
		}

		/* vertex matches */
		return i;
	}

	/* nada */
	return -1;
}



/*
   PicoFixSurfaceNormals()
   fixes broken normals (certain formats bork normals)
 */

//#define MAX_NORMAL_VOTES        128
//#define EQUAL_NORMAL_EPSILON    0.01
//#define BAD_NORMAL_EPSILON      0.5
//
//void PicoFixSurfaceNormals( picoSurface_t *surface ){
//	int i, j, k, a, b, c, numVotes, faceIndex;
//	picoVec3_t votes[ MAX_NORMAL_VOTES ];
//	picoVec3_t      *normals, diff;
//	picoVec4_t plane;
//
//
//	/* dummy check */
//	if ( surface == NULL || surface->numVertexes == 0 ) {
//		return;
//	}
//
//	/* fixme: handle other surface types */
//	if ( surface->type != PICO_TRIANGLES ) {
//		return;
//	}
//
//	/* allocate normal storage */
//	normals = _pico_alloc( surface->numVertexes * sizeof( *normals ) );
//	if ( normals == NULL ) {
//		_pico_printf( PICO_ERROR, "PicoFixSurfaceNormals: Unable to allocate memory for temporary normal storage" );
//		return;
//	}
//
//	/* zero it out */
//	memset( normals, 0, surface->numVertexes * sizeof( *normals ) );
//
//	/* walk vertex list */
//	for ( i = 0; i < surface->numVertexes; i++ )
//	{
//		/* zero out votes */
//		numVotes = 0;
//
//		/* find all the triangles that reference this vertex */
//		for ( j = 0, faceIndex = 0; j < surface->numIndexes; j += 3, faceIndex++ )
//		{
//			/* get triangle */
//			a = surface->index[ j ];
//			b = surface->index[ j + 1 ];
//			c = surface->index[ j + 2 ];
//
//			/* ignore degenerate triangles */
//			if ( a == b || b == c || c == a ) {
//				continue;
//			}
//
//			/* ignore indexes out of range */
//			if ( a < 0 || a >= surface->numVertexes ||
//				 b < 0 || b >= surface->numVertexes ||
//				 c < 0 || c >= surface->numVertexes ) {
//				continue;
//			}
//
//			/* test triangle */
//			if ( a == i || b == i || c == i ) {
//				/* if this surface has face normals */
//				if ( surface->numFaceNormals && faceIndex < surface->numFaceNormals ) {
//					_pico_copy_vec( surface->faceNormal[ faceIndex ], plane );
//					if ( plane[ 0 ] == 0.f && plane[ 1 ] == 0.f && plane[ 2 ] == 0.f ) {
//						/* if null normal, make plane from the 3 points */
//						if ( _pico_calc_plane( plane, surface->xyz[ a ], surface->xyz[ b ], surface->xyz[ c ] ) == 0 ) {
//							continue;
//						}
//					}
//				}
//				/* make a plane from the 3 points */
//				else if ( _pico_calc_plane( plane, surface->xyz[ a ], surface->xyz[ b ], surface->xyz[ c ] ) == 0 ) {
//					continue;
//				}
//
//				/* see if this normal has already been voted */
//				for ( k = 0; k < numVotes; k++ )
//				{
//					_pico_subtract_vec( plane, votes[ k ], diff );
//					if ( fabs( diff[ 0 ] ) < EQUAL_NORMAL_EPSILON &&
//						 fabs( diff[ 1 ] ) < EQUAL_NORMAL_EPSILON &&
//						 fabs( diff[ 2 ] ) < EQUAL_NORMAL_EPSILON ) {
//						break;
//					}
//				}
//
//				/* add a new vote? */
//				if ( k == numVotes && numVotes < MAX_NORMAL_VOTES ) {
//					_pico_copy_vec( plane, votes[ numVotes ] );
//					numVotes++;
//				}
//			}
//		}
//
//		/* tally votes */
//		if ( numVotes > 0 ) {
//			/* create average normal */
//			_pico_zero_vec( normals[ i ] );
//			for ( k = 0; k < numVotes; k++ )
//				_pico_add_vec( normals[ i ], votes[ k ], normals[ i ] );
//
//			/* normalize it */
//			if ( _pico_normalize_vec( normals[ i ] ) ) {
//				/* test against actual normal */
//				if ( fabs( _pico_dot_vec( normals[ i ], surface->normal[ i ] ) - 1 ) > BAD_NORMAL_EPSILON ) {
//					//%	printf( "Normal %8d: (%f %f %f) -> (%f %f %f)\n", i,
//					//%		surface->normal[ i ][ 0 ], surface->normal[ i ][ 1 ], surface->normal[ i ][ 2 ],
//					//%		normals[ i ][ 0 ], normals[ i ][ 1 ], normals[ i ][ 2 ] );
//					_pico_copy_vec( normals[ i ], surface->normal[ i ] );
//				}
//			}
//		}
//	}
//
//	/* free normal storage */
//	_pico_free( normals );
//}

typedef struct _IndexArray IndexArray;
struct _IndexArray
{
	picoIndex_t* data;
	picoIndex_t* last;
};

void indexarray_push_back(IndexArray* self, picoIndex_t value) {
	*self->last++ = value;
}

size_t indexarray_size(IndexArray* self) {
	return self->last - self->data;
}

void indexarray_reserve(IndexArray* self, size_t size) {
	self->data = self->last = _pico_calloc(size, sizeof(picoIndex_t));
}

void indexarray_clear(IndexArray* self) {
	_pico_free(self->data);
}

typedef struct _BinaryTreeNode BinaryTreeNode;
struct _BinaryTreeNode
{
	picoIndex_t left;
	picoIndex_t right;
};

typedef struct _BinaryTree BinaryTree;
struct _BinaryTree
{
	BinaryTreeNode* data;
	BinaryTreeNode* last;
};

void binarytree_extend(BinaryTree* self) {
	self->last->left = 0;
	self->last->right = 0;
	++self->last;
}

size_t binarytree_size(BinaryTree* self) {
	return self->last - self->data;
}

void binarytree_reserve(BinaryTree* self, size_t size) {
	self->data = self->last = _pico_calloc(size, sizeof(BinaryTreeNode));
}

void binarytree_clear(BinaryTree* self) {
	_pico_free(self->data);
}

typedef int(*LessFunc)(void*, picoIndex_t, picoIndex_t);

typedef struct _UniqueIndices UniqueIndices;
struct _UniqueIndices
{
	BinaryTree tree;
	IndexArray indices;
	LessFunc lessFunc;
	void* lessData;
};

size_t UniqueIndices_size(UniqueIndices* self) {
	return binarytree_size(&self->tree);
}

void UniqueIndices_reserve(UniqueIndices* self, size_t size) {
	binarytree_reserve(&self->tree, size);
	indexarray_reserve(&self->indices, size);
}

void UniqueIndices_init(UniqueIndices* self, LessFunc lessFunc, void* lessData) {
	self->lessFunc = lessFunc;
	self->lessData = lessData;
}

void UniqueIndices_destroy(UniqueIndices* self) {
	binarytree_clear(&self->tree);
	indexarray_clear(&self->indices);
}

picoIndex_t UniqueIndices_find_or_insert(UniqueIndices* self, picoIndex_t value) {
	picoIndex_t index = 0;

	for (;; )
	{
		if (self->lessFunc(self->lessData, value, self->indices.data[index])) {
			BinaryTreeNode* node = self->tree.data + index;
			if (node->left != 0) {
				index = node->left;
				continue;
			}
			else
			{
				node->left = (picoIndex_t)binarytree_size(&self->tree);
				binarytree_extend(&self->tree);
				indexarray_push_back(&self->indices, value);
				return node->left;
			}
		}
		if (self->lessFunc(self->lessData, self->indices.data[index], value)) {
			BinaryTreeNode* node = self->tree.data + index;
			if (node->right != 0) {
				index = node->right;
				continue;
			}
			else
			{
				node->right = (picoIndex_t)binarytree_size(&self->tree);
				binarytree_extend(&self->tree);
				indexarray_push_back(&self->indices, value);
				return node->right;
			}
		}

		return index;
	}
}

picoIndex_t UniqueIndices_insert(UniqueIndices* self, picoIndex_t value) {
	if (self->tree.data == self->tree.last) {
		binarytree_extend(&self->tree);
		indexarray_push_back(&self->indices, value);
		return 0;
	}
	else
	{
		return UniqueIndices_find_or_insert(self, value);
	}
}

typedef struct picoSmoothVertices_s picoSmoothVertices_t;
struct picoSmoothVertices_s
{
	picoVec3_t* xyz;
	picoIndex_t* smoothingGroups;
};

int lessSmoothVertex(void* data, picoIndex_t first, picoIndex_t second) {
	picoSmoothVertices_t* smoothVertices = data;

	if (smoothVertices->xyz[first][0] != smoothVertices->xyz[second][0]) {
		return smoothVertices->xyz[first][0] < smoothVertices->xyz[second][0];
	}
	if (smoothVertices->xyz[first][1] != smoothVertices->xyz[second][1]) {
		return smoothVertices->xyz[first][1] < smoothVertices->xyz[second][1];
	}
	if (smoothVertices->xyz[first][2] != smoothVertices->xyz[second][2]) {
		return smoothVertices->xyz[first][2] < smoothVertices->xyz[second][2];
	}
	if (smoothVertices->smoothingGroups[first] != smoothVertices->smoothingGroups[second]) {
		return smoothVertices->smoothingGroups[first] < smoothVertices->smoothingGroups[second];
	}
	return 0;
}

void _pico_vertices_combine_shared_normals(picoVec3_t* xyz, picoIndex_t* smoothingGroups, picoVec3_t* normals, picoIndex_t numVertices) {
	UniqueIndices vertices;
	IndexArray indices;
	picoSmoothVertices_t smoothVertices = { xyz, smoothingGroups };
	UniqueIndices_init(&vertices, lessSmoothVertex, &smoothVertices);
	UniqueIndices_reserve(&vertices, numVertices);
	indexarray_reserve(&indices, numVertices);

	{
		picoIndex_t i = 0;
		for (; i < numVertices; ++i)
		{
			size_t size = UniqueIndices_size(&vertices);
			picoIndex_t index = UniqueIndices_insert(&vertices, i);
			if ((size_t)index != size) {
				float* normal = normals[vertices.indices.data[index]];
				_pico_add_vec(normal, normals[i], normal);
			}
			indexarray_push_back(&indices, index);
		}
	}

	{
		picoIndex_t maxIndex = 0;
		picoIndex_t* i = indices.data;
		for (; i != indices.last; ++i)
		{
			if (*i <= maxIndex) {
				_pico_copy_vec(normals[vertices.indices.data[*i]], normals[i - indices.data]);
			}
			else
			{
				maxIndex = *i;
			}
		}
	}

	UniqueIndices_destroy(&vertices);
	indexarray_clear(&indices);
}

typedef picoVec3_t* picoNormalIter_t;
typedef picoIndex_t* picoIndexIter_t;

#define THE_CROSSPRODUCTS_OF_ANY_PAIR_OF_EDGES_OF_A_GIVEN_TRIANGLE_ARE_EQUAL 1

void _pico_triangles_generate_weighted_normals(picoIndexIter_t first, picoIndexIter_t end, picoVec3_t* xyz, picoVec3_t* normals) {
	for (; first != end; first += 3)
	{
#if (THE_CROSSPRODUCTS_OF_ANY_PAIR_OF_EDGES_OF_A_GIVEN_TRIANGLE_ARE_EQUAL)
		picoVec3_t weightedNormal;
		{
			float* a = xyz[*(first + 0)];
			float* b = xyz[*(first + 1)];
			float* c = xyz[*(first + 2)];
			picoVec3_t ba, ca;
			_pico_subtract_vec(b, a, ba);
			_pico_subtract_vec(c, a, ca);
			_pico_cross_vec(ca, ba, weightedNormal);
		}
#endif
		{
			int j = 0;
			for (; j < 3; ++j)
			{
				float* normal = normals[*(first + j)];
#if ( !THE_CROSSPRODUCTS_OF_ANY_PAIR_OF_EDGES_OF_A_GIVEN_TRIANGLE_ARE_EQUAL )
				picoVec3_t weightedNormal;
				{
					float* a = xyz[*(first + ((j + 0) % 3))];
					float* b = xyz[*(first + ((j + 1) % 3))];
					float* c = xyz[*(first + ((j + 2) % 3))];
					picoVec3_t ba, ca;
					_pico_subtract_vec(b, a, ba);
					_pico_subtract_vec(c, a, ca);
					_pico_cross_vec(ca, ba, weightedNormal);
				}
#endif
				_pico_add_vec(weightedNormal, normal, normal);
			}
		}
	}
}

void _pico_normals_zero(picoNormalIter_t first, picoNormalIter_t last) {
	for (; first != last; ++first)
	{
		_pico_zero_vec(*first);
	}
}

void _pico_normals_normalize(picoNormalIter_t first, picoNormalIter_t last) {
	for (; first != last; ++first)
	{
		_pico_normalize_vec(*first);
	}
}

double _pico_length_vec(picoVec3_t vec) {
	return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

#define NORMAL_UNIT_LENGTH_EPSILON 0.01
#define FLOAT_EQUAL_EPSILON( f, other, epsilon ) ( fabs( f - other ) < epsilon )

int _pico_normal_is_unit_length(picoVec3_t normal) {
	return FLOAT_EQUAL_EPSILON(_pico_length_vec(normal), 1.0, NORMAL_UNIT_LENGTH_EPSILON);
}

int _pico_normal_within_tolerance(picoVec3_t normal, picoVec3_t other) {
	return _pico_dot_vec(normal, other) > 0.0f;
}

void _pico_normals_assign_generated_normals(picoNormalIter_t first, picoNormalIter_t last, picoNormalIter_t generated) {
	for (; first != last; ++first, ++generated)
	{
		if (!_pico_normal_is_unit_length(*first) || !_pico_normal_within_tolerance(*first, *generated)) {
			_pico_copy_vec(*generated, *first);
		}
	}
}

void PicoFixSurfaceNormals(picoSurface_t* surface) {
	picoVec3_t* normals = (picoVec3_t*)_pico_calloc(surface->numVertexes, sizeof(picoVec3_t));

	_pico_normals_zero(normals, normals + surface->numVertexes);

	_pico_triangles_generate_weighted_normals(surface->index, surface->index + surface->numIndexes, surface->xyz, normals);
	_pico_vertices_combine_shared_normals(surface->xyz, surface->smoothingGroup, normals, surface->numVertexes);

	_pico_normals_normalize(normals, normals + surface->numVertexes);

	_pico_normals_assign_generated_normals(surface->normal, surface->normal + surface->numVertexes, normals);

	_pico_free(normals);
}

/*
   PicoRemapModel() - sea
   remaps model material/etc. information using the remappings
   contained in the given 'remapFile' (full path to the ascii file to open)
   returns 1 on success or 0 on error
 */

#define _prm_error_return \
	{ \
		_pico_free_parser( p ); \
		_pico_free_file( remapBuffer ); \
		return 0; \
	}

int PicoRemapModel( picoModel_t *model, char *remapFile ){
	picoParser_t    *p;
	picoByte_t      *remapBuffer;
	int remapBufSize;


	/* sanity checks */
	if ( model == NULL || remapFile == NULL ) {
		return 0;
	}

	/* load remap file contents */
	_pico_load_file( remapFile, &remapBuffer, &remapBufSize );

	/* check result */
	if ( remapBufSize == 0 ) {
		return 1;   /* file is empty: no error */
	}
	if ( remapBufSize < 0 ) {
		return 0;   /* load failed: error */

	}
	/* create a new pico parser */
	p = _pico_new_parser( remapBuffer, remapBufSize );
	if ( p == NULL ) {
		/* ram is really cheap nowadays... */
		_prm_error_return;
	}

	/* doo teh parse */
	while ( 1 )
	{
		/* get next token in remap file */
		if ( !_pico_parse( p, 1 ) ) {
			break;
		}

		/* skip over c++ style comment lines */
		if ( !_pico_stricmp( p->token, "//" ) ) {
			_pico_parse_skip_rest( p );
			continue;
		}

		/* block for quick material shader name remapping */
		/* materials { "m" (=>|->|=) "s" } */
		if ( !_pico_stricmp( p->token, "materials" ) ) {
			int level = 1;

			/* check bracket */
			if ( !_pico_parse_check( p, 1, "{" ) ) {
				_prm_error_return;
			}

			/* process assignments */
			while ( 1 )
			{
				picoShader_t    *shader;
				char            *materialName;

				/* get material name */
				if ( _pico_parse( p, 1 ) == NULL ) {
					break;
				}
				if ( !strlen( p->token ) ) {
					continue;
				}
				materialName = _pico_clone_alloc( p->token );
				if ( materialName == NULL ) {
					_prm_error_return;
				}

				/* handle levels */
				if ( p->token[0] == '{' ) {
					level++;
				}
				if ( p->token[0] == '}' ) {
					level--;
				}
				if ( !level ) {
					break;
				}

				/* get next token (assignment token or shader name) */
				if ( !_pico_parse( p, 0 ) ) {
					_pico_free( materialName );
					_prm_error_return;
				}
				/* skip assignment token (if present) */
				if ( !strcmp( p->token, "=>" ) ||
					 !strcmp( p->token, "->" ) ||
					 !strcmp( p->token, "=" ) ) {
					/* simply grab the next token */
					if ( !_pico_parse( p, 0 ) ) {
						_pico_free( materialName );
						_prm_error_return;
					}
				}
				/* try to find material by name */
				shader = PicoFindShader( model, materialName, 0 );

				/* we've found a material matching the name */
				if ( shader != NULL ) {
					PicoSetShaderName( shader, p->token );
				}
				/* free memory used by material name */
				_pico_free( materialName );

				/* skip rest */
				_pico_parse_skip_rest( p );
			}
		}
		/* block for detailed single material remappings */
		/* materials[ "m" ] { key data... } */
		else if ( !_pico_stricmp( p->token, "materials[" ) ) {
			picoShader_t *shader;
			char *tempMaterialName;
			int level = 1;

			/* get material name */
			if ( !_pico_parse( p, 0 ) ) {
				_prm_error_return;
			}

			/* temporary copy of material name */
			tempMaterialName = _pico_clone_alloc( p->token );
			if ( tempMaterialName == NULL ) {
				_prm_error_return;
			}

			/* check square closing bracket */
			if ( !_pico_parse_check( p, 0, "]" ) ) {
				_prm_error_return;
			}

			/* try to find material by name */
			shader = PicoFindShader( model, tempMaterialName, 0 );

			/* free memory used by temporary material name */
			_pico_free( tempMaterialName );

			/* we haven't found a material matching the name */
			/* so we simply skip the braced section now and */
			/* continue parsing with the next main token */
			if ( shader == NULL ) {
				_pico_parse_skip_braced( p );
				continue;
			}
			/* check opening bracket */
			if ( !_pico_parse_check( p, 1, "{" ) ) {
				_prm_error_return;
			}

			/* process material info keys */
			while ( 1 )
			{
				/* get key name */
				if ( _pico_parse( p, 1 ) == NULL ) {
					break;
				}
				if ( !strlen( p->token ) ) {
					continue;
				}

				/* handle levels */
				if ( p->token[0] == '{' ) {
					level++;
				}
				if ( p->token[0] == '}' ) {
					level--;
				}
				if ( !level ) {
					break;
				}

				/* remap shader name */
				if ( !_pico_stricmp( p->token, "shader" ) ) {
					if ( !_pico_parse( p, 0 ) ) {
						_prm_error_return;
					}
					PicoSetShaderName( shader, p->token );
				}
				/* remap shader map name */
				else if ( !_pico_stricmp( p->token, "mapname" ) ) {
					if ( !_pico_parse( p, 0 ) ) {
						_prm_error_return;
					}
					PicoSetShaderMapName( shader, p->token );
				}
				/* remap shader's ambient color */
				else if ( !_pico_stricmp( p->token, "ambient" ) ) {
					picoColor_t color;
					picoVec3_t v;

					/* get vector from parser */
					if ( !_pico_parse_vec( p, v ) ) {
						_prm_error_return;
					}

					/* store as color */
					color[ 0 ] = (picoByte_t)v[ 0 ];
					color[ 1 ] = (picoByte_t)v[ 1 ];
					color[ 2 ] = (picoByte_t)v[ 2 ];
					color[ 3 ] = 1;

					/* set new ambient color */
					PicoSetShaderAmbientColor( shader, color );
				}
				/* remap shader's diffuse color */
				else if ( !_pico_stricmp( p->token, "diffuse" ) ) {
					picoColor_t color;
					picoVec3_t v;

					/* get vector from parser */
					if ( !_pico_parse_vec( p, v ) ) {
						_prm_error_return;
					}

					/* store as color */
					color[ 0 ] = (picoByte_t)v[ 0 ];
					color[ 1 ] = (picoByte_t)v[ 1 ];
					color[ 2 ] = (picoByte_t)v[ 2 ];
					color[ 3 ] = 1;

					/* set new ambient color */
					PicoSetShaderDiffuseColor( shader, color );
				}
				/* remap shader's specular color */
				else if ( !_pico_stricmp( p->token, "specular" ) ) {
					picoColor_t color;
					picoVec3_t v;

					/* get vector from parser */
					if ( !_pico_parse_vec( p,v ) ) {
						_prm_error_return;
					}

					/* store as color */
					color[ 0 ] = (picoByte_t)v[ 0 ];
					color[ 1 ] = (picoByte_t)v[ 1 ];
					color[ 2 ] = (picoByte_t)v[ 2 ];
					color[ 3 ] = 1;

					/* set new ambient color */
					PicoSetShaderSpecularColor( shader, color );
				}
				/* skip rest */
				_pico_parse_skip_rest( p );
			}
		}
		/* end 'materials[' */
	}

	/* free both parser and file buffer */
	_pico_free_parser( p );
	_pico_free_file( remapBuffer );

	/* return with success */
	return 1;
}


/*
   PicoAddTriangleToModel() - jhefty
   A nice way to add individual triangles to the model.
   Chooses an appropriate surface based on the shader, or adds a new surface if necessary
 */

void PicoAddTriangleToModel( picoModel_t *model, picoVec3_t** xyz, picoVec3_t** normals,
							 int numSTs, picoVec2_t **st, int numColors, picoColor_t **colors,
							 picoShader_t* shader, picoIndex_t* smoothingGroup ){
	int i, j;
	int vertDataIndex;
	picoSurface_t* workSurface = NULL;

	/* see if a surface already has the shader */
	for ( i = 0 ; i < model->numSurfaces ; i++ )
	{
		workSurface = model->surface[i];
		if ( workSurface->shader == shader ) {
			break;
		}
	}

	/* no surface uses this shader yet, so create a new surface */
	if ( !workSurface || i >= model->numSurfaces ) {
		/* create a new surface in the model for the unique shader */
		workSurface = PicoNewSurface( model );
		if ( !workSurface ) {
			_pico_printf( PICO_ERROR, "Could not allocate a new surface!\n" );
			return;
		}

		/* do surface setup */
		PicoSetSurfaceType( workSurface, PICO_TRIANGLES );
		PicoSetSurfaceName( workSurface, shader->name );
		PicoSetSurfaceShader( workSurface, shader );
	}

	/* add the triangle data to the surface */
	for ( i = 0 ; i < 3 ; i++ )
	{
		/* get the next free spot in the index array */
		int newVertIndex = PicoGetSurfaceNumIndexes( workSurface );

		/* get the index of the vertex that we're going to store at newVertIndex */
		vertDataIndex = PicoFindSurfaceVertexNum( workSurface, *xyz[i], *normals[i], numSTs, st[i], numColors, colors[i], smoothingGroup[i] );

		/* the vertex wasn't found, so create a new vertex in the pool from the data we have */
		if ( vertDataIndex == -1 ) {
			/* find the next spot for a new vertex */
			vertDataIndex = PicoGetSurfaceNumVertexes( workSurface );

			/* assign the data to it */
			PicoSetSurfaceXYZ( workSurface,vertDataIndex, *xyz[i] );
			PicoSetSurfaceNormal( workSurface, vertDataIndex, *normals[i] );

			/* make sure to copy over all available ST's and colors for the vertex */
			for ( j = 0 ; j < numColors ; j++ )
			{
				PicoSetSurfaceColor( workSurface, j, vertDataIndex, colors[i][j] );
			}
			for ( j = 0 ; j < numSTs ; j++ )
			{
				PicoSetSurfaceST( workSurface, j, vertDataIndex, st[i][j] );
			}

			PicoSetSurfaceSmoothingGroup( workSurface, vertDataIndex, smoothingGroup[ i ] );
		}

		/* add this vertex to the triangle */
		PicoSetSurfaceIndex( workSurface, newVertIndex, vertDataIndex );
	}
}
