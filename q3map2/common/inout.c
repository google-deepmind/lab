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

//-----------------------------------------------------------------------------
//
//
// DESCRIPTION:
// deal with in/out tasks, for either stdin/stdout or network/XML stream
//

#include "cmdlib.h"
#include "mathlib.h"
#include "polylib.h"
#include "inout.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <direct.h>
#include <windows.h>
#endif

// network broadcasting
#include "l_net/l_net.h"
#if defined( _WIN32 )
  // required for static linking libxml on Windows
  #define LIBXML_STATIC
#endif
#include "libxml/tree.h"

// utf8 conversion
#include <glib.h>

#ifdef WIN32
HWND hwndOut = NULL;
qboolean lookedForServer = qfalse;
UINT wm_BroadcastCommand = -1;
#endif

socket_t *brdcst_socket;
netmessage_t msg;

// our main document
// is streamed through the network to Radiant
// possibly written to disk at the end of the run
//++timo FIXME: need to be global, required when creating nodes?
xmlDocPtr doc;
xmlNodePtr tree;

// some useful stuff
xmlNodePtr xml_NodeForVec( vec3_t v ){
	xmlNodePtr ret;
	char buf[1024];

	sprintf( buf, "%f %f %f", v[0], v[1], v[2] );
	ret = xmlNewNode( NULL, BAD_CAST "point" );
	xmlNodeSetContent( ret, BAD_CAST buf );
	return ret;
}

// send a node down the stream, add it to the document
void xml_SendNode( xmlNodePtr node ){
	xmlBufferPtr xml_buf;
	char xmlbuf[MAX_NETMESSAGE]; // we have to copy content from the xmlBufferPtr into an aux buffer .. that sucks ..
	// this index loops through the node buffer
	int pos = 0;
	int size;

	xmlAddChild( doc->children, node );

	if ( brdcst_socket ) {
		xml_buf = xmlBufferCreate();
		xmlNodeDump( xml_buf, doc, node, 0, 0 );

		// the XML node might be too big to fit in a single network message
		// l_net library defines an upper limit of MAX_NETMESSAGE
		// there are some size check errors, so we use MAX_NETMESSAGE-10 to be safe
		// if the size of the buffer exceeds MAX_NETMESSAGE-10 we'll send in several network messages
		while ( pos < xml_buf->use )
		{
			// what size are we gonna send now?
			( xml_buf->use - pos < MAX_NETMESSAGE - 10 ) ? ( size = xml_buf->use - pos ) : ( size = MAX_NETMESSAGE - 10 );
			//++timo just a debug thing
			if ( size == MAX_NETMESSAGE - 10 ) {
				Sys_FPrintf( SYS_NOXML, "Got to split the buffer\n" );
			}
			memcpy( xmlbuf, xml_buf->content + pos, size );
			xmlbuf[size] = '\0';
			NMSG_Clear( &msg );
			NMSG_WriteString( &msg, xmlbuf );
			Net_Send( brdcst_socket, &msg );
			// now that the thing is sent prepare to loop again
			pos += size;
		}

#if 0
		// NOTE: the NMSG_WriteString is limited to MAX_NETMESSAGE
		// we will need to split into chunks
		// (we could also go lower level, in the end it's using send and receiv which are not size limited)
		//++timo FIXME: MAX_NETMESSAGE is not exactly the max size we can stick in the message
		//  there's some tweaking to do in l_net for that .. so let's give us a margin for now

		//++timo we need to handle the case of a buffer too big to fit in a single message
		// try without checks for now
		if ( xml_buf->use > MAX_NETMESSAGE - 10 ) {
			// if we send that we are probably gonna break the stream at the other end..
			// and Error will call right there
			//Error( "MAX_NETMESSAGE exceeded for XML feedback stream in FPrintf (%d)\n", xml_buf->use);
			Sys_FPrintf( SYS_NOXML, "MAX_NETMESSAGE exceeded for XML feedback stream in FPrintf (%d)\n", xml_buf->use );
			xml_buf->content[xml_buf->use] = '\0'; //++timo this corrupts the buffer but we don't care it's for printing
			Sys_FPrintf( SYS_NOXML, xml_buf->content );

		}

		size = xml_buf->use;
		memcpy( xmlbuf, xml_buf->content, size );
		xmlbuf[size] = '\0';
		NMSG_Clear( &msg );
		NMSG_WriteString( &msg, xmlbuf );
		Net_Send( brdcst_socket, &msg );
#endif

		xmlBufferFree( xml_buf );
	}
}

void xml_Select( char *msg, int entitynum, int brushnum, qboolean bError ){
	xmlNodePtr node, select;
	char buf[1024];
	char level[2];

	// now build a proper "select" XML node
	sprintf( buf, "Entity %i, Brush %i: %s", entitynum, brushnum, msg );
	node = xmlNewNode( NULL, BAD_CAST "select" );
	xmlNodeSetContent( node, BAD_CAST buf );
	level[0] = (int)'0' + ( bError ? SYS_ERR : SYS_WRN )  ;
	level[1] = 0;
	xmlSetProp( node, BAD_CAST "level", BAD_CAST (char *)&level );
	// a 'select' information
	sprintf( buf, "%i %i", entitynum, brushnum );
	select = xmlNewNode( NULL, BAD_CAST "brush" );
	xmlNodeSetContent( select, BAD_CAST buf );
	xmlAddChild( node, select );
	xml_SendNode( node );

	sprintf( buf, "Entity %i, Brush %i: %s", entitynum, brushnum, msg );
	if ( bError ) {
		Error( buf );
	}
	else{
		Sys_FPrintf( SYS_NOXML, "%s\n", buf );
	}

}

void xml_Point( char *msg, vec3_t pt ){
	xmlNodePtr node, point;
	char buf[1024];
	char level[2];

	node = xmlNewNode( NULL, BAD_CAST "pointmsg" );
	xmlNodeSetContent( node, BAD_CAST msg );
	level[0] = (int)'0' + SYS_ERR;
	level[1] = 0;
	xmlSetProp( node, BAD_CAST "level", BAD_CAST (char *)&level );
	// a 'point' node
	sprintf( buf, "%g %g %g", pt[0], pt[1], pt[2] );
	point = xmlNewNode( NULL, BAD_CAST "point" );
	xmlNodeSetContent( point, BAD_CAST buf );
	xmlAddChild( node, point );
	xml_SendNode( node );

	sprintf( buf, "%s (%g %g %g)", msg, pt[0], pt[1], pt[2] );
	Error( buf );
}

#define WINDING_BUFSIZE 2048
void xml_Winding( char *msg, vec3_t p[], int numpoints, qboolean die ){
	xmlNodePtr node, winding;
	char buf[WINDING_BUFSIZE];
	char smlbuf[128];
	char level[2];
	int i;

	node = xmlNewNode( NULL, BAD_CAST "windingmsg" );
	xmlNodeSetContent( node, BAD_CAST msg );
	level[0] = (int)'0' + SYS_ERR;
	level[1] = 0;
	xmlSetProp( node, BAD_CAST "level", BAD_CAST (char *)&level );
	// a 'winding' node
	sprintf( buf, "%i ", numpoints );
	for ( i = 0; i < numpoints; i++ )
	{
		sprintf( smlbuf, "(%g %g %g)", p[i][0], p[i][1], p[i][2] );
		// don't overflow
		if ( strlen( buf ) + strlen( smlbuf ) > WINDING_BUFSIZE ) {
			break;
		}
		strcat( buf, smlbuf );
	}

	winding = xmlNewNode( NULL, BAD_CAST "winding" );
	xmlNodeSetContent( winding, BAD_CAST buf );
	xmlAddChild( node, winding );
	xml_SendNode( node );

	if ( die ) {
		Error( msg );
	}
	else
	{
		Sys_Printf( msg );
		Sys_Printf( "\n" );
	}
}

// in include
#include "stream_version.h"

void Broadcast_Setup( const char *dest ){
	address_t address;
	char sMsg[1024];

	Net_Setup();
	Net_StringToAddress( (char *)dest, &address );
	brdcst_socket = Net_Connect( &address, 0 );
	if ( brdcst_socket ) {
		// send in a header
		sprintf( sMsg, "<?xml version=\"1.0\"?><q3map_feedback version=\"" Q3MAP_STREAM_VERSION "\">" );
		NMSG_Clear( &msg );
		NMSG_WriteString( &msg, sMsg );
		Net_Send( brdcst_socket, &msg );
	}
}

void Broadcast_Shutdown(){
	if ( brdcst_socket ) {
		Sys_Printf( "Disconnecting\n" );
		Net_Disconnect( brdcst_socket );
		brdcst_socket = NULL;
	}
}

// all output ends up through here
void FPrintf( int flag, char *buf ){
	xmlNodePtr node;
	static qboolean bGotXML = qfalse;
	char level[2];

	printf( "%s", buf );

	// the following part is XML stuff only.. but maybe we don't want that message to go down the XML pipe?
	if ( flag == SYS_NOXML ) {
		return;
	}

	// ouput an XML file of the run
	// use the DOM interface to build a tree
	/*
	   <message level='flag'>
	   message string
	   .. various nodes to describe corresponding geometry ..
	   </message>
	 */
	if ( !bGotXML ) {
		// initialize
		doc = xmlNewDoc( BAD_CAST "1.0" );
		doc->children = xmlNewDocRawNode( doc, NULL, BAD_CAST "q3map_feedback", NULL );
		bGotXML = qtrue;
	}
	node = xmlNewNode( NULL, BAD_CAST "message" );
	{
		gchar* utf8 = g_locale_to_utf8( buf, -1, NULL, NULL, NULL );
		xmlNodeSetContent( node, BAD_CAST utf8 );
		g_free( utf8 );
	}
	level[0] = (int)'0' + flag;
	level[1] = 0;
	xmlSetProp( node, BAD_CAST "level", BAD_CAST (char *)&level );

	xml_SendNode( node );
}

#ifdef DBG_XML
void DumpXML(){
	xmlSaveFile( "XMLDump.xml", doc );
}
#endif

void Sys_FPrintf( int flag, const char *format, ... ){
	char out_buffer[4096];
	va_list argptr;

	if ( ( flag == SYS_VRB ) && ( verbose == qfalse ) ) {
		return;
	}

	va_start( argptr, format );
	vsprintf( out_buffer, format, argptr );
	va_end( argptr );

	FPrintf( flag, out_buffer );
}

void Sys_Printf( const char *format, ... ){
	char out_buffer[4096];
	va_list argptr;

	va_start( argptr, format );
	vsprintf( out_buffer, format, argptr );
	va_end( argptr );

	FPrintf( SYS_STD, out_buffer );
}

/*
   =================
   Error

   For abnormal program terminations
   =================
 */
void Error( const char *error, ... ){
	char out_buffer[4096];
	char tmp[4096];
	va_list argptr;

	va_start( argptr,error );
	vsprintf( tmp, error, argptr );
	va_end( argptr );

	sprintf( out_buffer, "************ ERROR ************\n%s\n", tmp );

	FPrintf( SYS_ERR, out_buffer );

#ifdef DBG_XML
	DumpXML();
#endif

	//++timo HACK ALERT .. if we shut down too fast the xml stream won't reach the listener.
	// a clean solution is to send a sync request node in the stream and wait for an answer before exiting
	Sys_Sleep( 1000 );

	Broadcast_Shutdown();

	exit( 1 );
}
