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

#ifndef __INOUT__
#define __INOUT__

// inout is the only stuff relying on xml, include the headers there
#if defined( _WIN32 )
  // required for static linking libxml on Windows
  #define LIBXML_STATIC
#endif
#include "libxml/tree.h"
#include "mathlib.h"

// some useful xml routines
xmlNodePtr xml_NodeForVec( vec3_t v );
void xml_SendNode( xmlNodePtr node );
// print a message in q3map output and send the corresponding select information down the xml stream
// bError: do we end with an error on this one or do we go ahead?
void xml_Select( char *msg, int entitynum, int brushnum, qboolean bError );
// end q3map with an error message and send a point information in the xml stream
// note: we might want to add a boolean to use this as a warning or an error thing..
void xml_Winding( char *msg, vec3_t p[], int numpoints, qboolean die );
void xml_Point( char *msg, vec3_t pt );

extern qboolean bNetworkBroadcast;
void Broadcast_Setup( const char *dest );
void Broadcast_Shutdown();

#define SYS_VRB 0 // verbose support (on/off)
#define SYS_STD 1 // standard print level
#define SYS_WRN 2 // warnings
#define SYS_ERR 3 // error
#define SYS_NOXML 4 // don't send that down the XML stream

extern qboolean verbose;
void Sys_Printf( const char *text, ... );
void Sys_FPrintf( int flag, const char *text, ... );

#ifdef _DEBUG
#define DBG_XML 1
#endif

#ifdef DBG_XML
void DumpXML();
#endif

#endif
