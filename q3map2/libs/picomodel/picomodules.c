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
#define PICOMODULES_C



/* dependencies */
#include "picointernal.h"



/* external modules */
extern const picoModule_t picoModuleMD3;
extern const picoModule_t picoModule3DS;
extern const picoModule_t picoModuleASE;
extern const picoModule_t picoModuleOBJ;
extern const picoModule_t picoModuleMS3D;
extern const picoModule_t picoModuleMDC;
extern const picoModule_t picoModuleMD2;
extern const picoModule_t picoModuleFM;
extern const picoModule_t picoModuleLWO;
extern const picoModule_t picoModuleTerrain;



/* list of all supported file format modules */
const picoModule_t *picoModules[] =
{
	&picoModuleMD3,     /* quake3 arena md3 */
	&picoModule3DS,     /* autodesk 3ds */
	&picoModuleASE,     /* autodesk ase */
	&picoModuleMS3D,    /* milkshape3d */
	&picoModuleMDC,     /* return to castle wolfenstein mdc */
	&picoModuleMD2,     /* quake2 md2 */
	&picoModuleFM,      /* heretic2 fm */
	&picoModuleLWO,     /* lightwave object */
	&picoModuleTerrain,	/* picoterrain object */
	&picoModuleOBJ,     /* wavefront object */
	NULL                /* arnold */
};



/*
   PicoModuleList()
   returns a pointer to the module list and optionally stores
   the number of supported modules in 'numModules'. Note that
   this param can be NULL when the count is not needed.
 */

const picoModule_t **PicoModuleList( int *numModules ) {
	/* get module count */
	if ( numModules != NULL ) {
		for ( ( *numModules ) = 0; picoModules[ *numModules ] != NULL; ( *numModules )++ ) ;
	}

	/* return list of modules */
	return (const picoModule_t**) picoModules;
}
