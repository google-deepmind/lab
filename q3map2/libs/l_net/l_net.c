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

//====================================================================
//
// Name:			l_net.c
// Function:		-
// Programmer:		MrElusive
// Last update:		-
// Tab size:		3
// Notes:
//====================================================================

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "l_net.h"
#include "l_net_wins.h"

#define GetMemory malloc
#define FreeMemory free

#define qtrue   1
#define qfalse  0

#ifdef _DEBUG
void WinPrint( char *str, ... ){
	va_list argptr;
	char text[4096];

	va_start( argptr,str );
	vsprintf( text, str, argptr );
	va_end( argptr );

	printf("%s", text );
}
#else
void WinPrint( char *str, ... ){
}
#endif

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Net_SetAddressPort( address_t *address, int port ){
	sockaddr_t addr;

	WINS_StringToAddr( address->ip, &addr );
	WINS_SetSocketPort( &addr, port );
	strcpy( address->ip, WINS_AddrToString( &addr ) );
} //end of the function Net_SetAddressPort
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Net_AddressCompare( address_t *addr1, address_t *addr2 ){
#ifdef _WIN32
	return stricmp( addr1->ip, addr2->ip );
#elif defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	return strcasecmp( addr1->ip, addr2->ip );
#else
	#error unknown architecture
#endif
} //end of the function Net_AddressCompare
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Net_SocketToAddress( socket_t *sock, address_t *address ){
	strcpy( address->ip, WINS_AddrToString( &sock->addr ) );
} //end of the function Net_SocketToAddress
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Net_Send( socket_t *sock, netmessage_t *msg ){
	int size;

	size = msg->size;
	msg->size = 0;
	NMSG_WriteLong( msg, size - 4 );
	msg->size = size;
	//WinPrint("Net_Send: message of size %d\n", sendmsg.size);
	return WINS_Write( sock->socket, msg->data, msg->size, NULL );
} //end of the function Net_SendSocketReliable
//===========================================================================
// returns the number of bytes recieved
// -1 on error
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Net_Receive( socket_t *sock, netmessage_t *msg ){
	int curread;

	if ( sock->remaining > 0 ) {
		curread = WINS_Read( sock->socket, &sock->msg.data[sock->msg.size], sock->remaining, NULL );
		if ( curread == -1 ) {
			WinPrint( "Net_Receive: read error\n" );
			return -1;
		} //end if
		sock->remaining -= curread;
		sock->msg.size += curread;
		if ( sock->remaining <= 0 ) {
			sock->remaining = 0;
			memcpy( msg, &sock->msg, sizeof( netmessage_t ) );
			sock->msg.size = 0;
			return msg->size - 4;
		} //end if
		return 0;
	} //end if
	sock->msg.size = WINS_Read( sock->socket, sock->msg.data, 4, NULL );
	if ( sock->msg.size == 0 ) {
		return 0;
	}
	if ( sock->msg.size == -1 ) {
		WinPrint( "Net_Receive: size header read error\n" );
		return -1;
	} //end if
	  //WinPrint("Net_Receive: message size header %d\n", msg->size);
	sock->msg.read = 0;
	sock->remaining = NMSG_ReadLong( &sock->msg );
	if ( sock->remaining == 0 ) {
		return 0;
	}
	if ( sock->remaining < 0 || sock->remaining > MAX_NETMESSAGE ) {
		WinPrint( "Net_Receive: invalid message size %d\n", sock->remaining );
		return -1;
	} //end if
	  //try to read the message
	curread = WINS_Read( sock->socket, &sock->msg.data[sock->msg.size], sock->remaining, NULL );
	if ( curread == -1 ) {
		WinPrint( "Net_Receive: read error\n" );
		return -1;
	} //end if
	sock->remaining -= curread;
	sock->msg.size += curread;
	if ( sock->remaining <= 0 ) {
		sock->remaining = 0;
		memcpy( msg, &sock->msg, sizeof( netmessage_t ) );
		sock->msg.size = 0;
		return msg->size - 4;
	} //end if
	  //the message has not been completely read yet
#ifdef _DEBUG
	printf( "++timo TODO: debug the Net_Receive on big size messages\n" );
#endif
	return 0;
} //end of the function Net_Receive
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
socket_t *Net_AllocSocket( void ){
	socket_t *sock;

	sock = (socket_t *) GetMemory( sizeof( socket_t ) );
	memset( sock, 0, sizeof( socket_t ) );
	return sock;
} //end of the function Net_AllocSocket
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Net_FreeSocket( socket_t *sock ){
	FreeMemory( sock );
} //end of the function Net_FreeSocket
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
socket_t *Net_Connect( address_t *address, int port ){
	int newsock;
	socket_t *sock;
	sockaddr_t sendaddr;

	// see if we can resolve the host name
	WINS_StringToAddr( address->ip, &sendaddr );

	newsock = WINS_OpenReliableSocket( port );
	if ( newsock == -1 ) {
		return NULL;
	}

	sock = Net_AllocSocket();
	if ( sock == NULL ) {
		WINS_CloseSocket( newsock );
		return NULL;
	} //end if
	sock->socket = newsock;

	//connect to the host
	if ( WINS_Connect( newsock, &sendaddr ) == -1 ) {
		Net_FreeSocket( sock );
		WINS_CloseSocket( newsock );
		WinPrint( "Net_Connect: error connecting\n" );
		return NULL;
	} //end if

	memcpy( &sock->addr, &sendaddr, sizeof( sockaddr_t ) );
	//now we can send messages
	//
	return sock;
} //end of the function Net_Connect

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
socket_t *Net_ListenSocket( int port ){
	int newsock;
	socket_t *sock;

	newsock = WINS_OpenReliableSocket( port );
	if ( newsock == -1 ) {
		return NULL;
	}

	if ( WINS_Listen( newsock ) == -1 ) {
		WINS_CloseSocket( newsock );
		return NULL;
	} //end if
	sock = Net_AllocSocket();
	if ( sock == NULL ) {
		WINS_CloseSocket( newsock );
		return NULL;
	} //end if
	sock->socket = newsock;
	WINS_GetSocketAddr( newsock, &sock->addr );
	WinPrint( "listen socket opened at %s\n", WINS_AddrToString( &sock->addr ) );
	//
	return sock;
} //end of the function Net_ListenSocket
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
socket_t *Net_Accept( socket_t *sock ){
	int newsocket;
	sockaddr_t sendaddr;
	socket_t *newsock;

	newsocket = WINS_Accept( sock->socket, &sendaddr );
	if ( newsocket == -1 ) {
		return NULL;
	}

	newsock = Net_AllocSocket();
	if ( newsock == NULL ) {
		WINS_CloseSocket( newsocket );
		return NULL;
	} //end if
	newsock->socket = newsocket;
	memcpy( &newsock->addr, &sendaddr, sizeof( sockaddr_t ) );
	//
	return newsock;
} //end of the function Net_Accept
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Net_Disconnect( socket_t *sock ){
	WINS_CloseSocket( sock->socket );
	Net_FreeSocket( sock );
} //end of the function Net_Disconnect
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Net_StringToAddress( char *string, address_t *address ){
	strcpy( address->ip, string );
} //end of the function Net_StringToAddress
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Net_MyAddress( address_t *address ){
	strcpy( address->ip, WINS_MyAddress() );
} //end of the function Net_MyAddress
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Net_Setup( void ){
	if ( !WINS_Init() ) {
		return qfalse;
	}

	//
	WinPrint( "my address is %s\n", WINS_MyAddress() );
	//
	return qtrue;
} //end of the function Net_Setup
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Net_Shutdown( void ){
	WINS_Shutdown();
} //end of the function Net_Shutdown
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void NMSG_Clear( netmessage_t *msg ){
	msg->size = 4;
} //end of the function NMSG_Clear
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void NMSG_WriteChar( netmessage_t *msg, int c ){
	if ( c < -128 || c > 127 ) {
		WinPrint( "NMSG_WriteChar: range error\n" );
	}

	if ( msg->size >= MAX_NETMESSAGE ) {
		WinPrint( "NMSG_WriteChar: overflow\n" );
		return;
	} //end if
	msg->data[msg->size] = c;
	msg->size++;
} //end of the function NMSG_WriteChar
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void NMSG_WriteByte( netmessage_t *msg, int c ){
	if ( c < -128 || c > 127 ) {
		WinPrint( "NMSG_WriteByte: range error\n" );
	}

	if ( msg->size + 1 >= MAX_NETMESSAGE ) {
		WinPrint( "NMSG_WriteByte: overflow\n" );
		return;
	} //end if
	msg->data[msg->size] = c;
	msg->size++;
} //end of the function NMSG_WriteByte
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void NMSG_WriteShort( netmessage_t *msg, int c ){
	if ( c < ( (short)0x8000 ) || c > (short)0x7fff ) {
		WinPrint( "NMSG_WriteShort: range error" );
	}

	if ( msg->size + 2 >= MAX_NETMESSAGE ) {
		WinPrint( "NMSG_WriteShort: overflow\n" );
		return;
	} //end if
	msg->data[msg->size] = c & 0xff;
	msg->data[msg->size + 1] = c >> 8;
	msg->size += 2;
} //end of the function NMSG_WriteShort
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void NMSG_WriteLong( netmessage_t *msg, int c ){
	if ( msg->size + 4 >= MAX_NETMESSAGE ) {
		WinPrint( "NMSG_WriteLong: overflow\n" );
		return;
	} //end if
	msg->data[msg->size] = c & 0xff;
	msg->data[msg->size + 1] = ( c >> 8 ) & 0xff;
	msg->data[msg->size + 2] = ( c >> 16 ) & 0xff;
	msg->data[msg->size + 3] = c >> 24;
	msg->size += 4;
} //end of the function NMSG_WriteLong
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void NMSG_WriteFloat( netmessage_t *msg, float c ){
	if ( msg->size + 4 >= MAX_NETMESSAGE ) {
		WinPrint( "NMSG_WriteLong: overflow\n" );
		return;
	} //end if
	msg->data[msg->size] = *( (int *)&c ) & 0xff;
	msg->data[msg->size + 1] = ( *( (int *)&c ) >> 8 ) & 0xff;
	msg->data[msg->size + 2] = ( *( (int *)&c ) >> 16 ) & 0xff;
	msg->data[msg->size + 3] = *( (int *)&c ) >> 24;
	msg->size += 4;
} //end of the function NMSG_WriteFloat
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void NMSG_WriteString( netmessage_t *msg, char *string ){
	if ( msg->size + strlen( string ) + 1 >= MAX_NETMESSAGE ) {
		WinPrint( "NMSG_WriteString: overflow\n" );
		return;
	} //end if
	memcpy( &msg->data[msg->size], string, strlen( string ) + 1 );
	msg->size += strlen( string ) + 1;
} //end of the function NMSG_WriteString
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void NMSG_ReadStart( netmessage_t *msg ){
	msg->readoverflow = qfalse;
	msg->read = 4;
} //end of the function NMSG_ReadStart
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int NMSG_ReadChar( netmessage_t *msg ){
	if ( msg->read + 1 > msg->size ) {
		msg->readoverflow = qtrue;
		WinPrint( "NMSG_ReadChar: read overflow\n" );
		return 0;
	} //end if
	msg->read++;
	return msg->data[msg->read - 1];
} //end of the function NMSG_ReadChar
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int NMSG_ReadByte( netmessage_t *msg ){
	if ( msg->read + 1 > msg->size ) {
		msg->readoverflow = qtrue;
		WinPrint( "NMSG_ReadByte: read overflow\n" );
		return 0;
	} //end if
	msg->read++;
	return msg->data[msg->read - 1];
} //end of the function NMSG_ReadByte
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int NMSG_ReadShort( netmessage_t *msg ){
	int c;

	if ( msg->read + 2 > msg->size ) {
		msg->readoverflow = qtrue;
		WinPrint( "NMSG_ReadShort: read overflow\n" );
		return 0;
	} //end if
	c = (short)( msg->data[msg->read] + ( msg->data[msg->read + 1] << 8 ) );
	msg->read += 2;
	return c;
} //end of the function NMSG_ReadShort
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int NMSG_ReadLong( netmessage_t *msg ){
	int c;

	if ( msg->read + 4 > msg->size ) {
		msg->readoverflow = qtrue;
		WinPrint( "NMSG_ReadLong: read overflow\n" );
		return 0;
	} //end if
	c = msg->data[msg->read]
		+ ( msg->data[msg->read + 1] << 8 )
		+ ( msg->data[msg->read + 2] << 16 )
		+ ( msg->data[msg->read + 3] << 24 );
	msg->read += 4;
	return c;
} //end of the function NMSG_ReadLong
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
float NMSG_ReadFloat( netmessage_t *msg ){
	int c;

	if ( msg->read + 4 > msg->size ) {
		msg->readoverflow = qtrue;
		WinPrint( "NMSG_ReadLong: read overflow\n" );
		return 0;
	} //end if
	c = msg->data[msg->read]
		+ ( msg->data[msg->read + 1] << 8 )
		+ ( msg->data[msg->read + 2] << 16 )
		+ ( msg->data[msg->read + 3] << 24 );
	msg->read += 4;
	return *(float *)&c;
} //end of the function NMSG_ReadFloat
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *NMSG_ReadString( netmessage_t *msg ){
	static char string[2048];
	int l, c;

	l = 0;
	do
	{
		if ( msg->read + 1 > msg->size ) {
			msg->readoverflow = qtrue;
			WinPrint( "NMSG_ReadString: read overflow\n" );
			string[l] = 0;
			return string;
		} //end if
		c = msg->data[msg->read];
		msg->read++;
		if ( c == 0 ) {
			break;
		}
		string[l] = c;
		l++;
	} while ( l < sizeof( string ) - 1 );
	string[l] = 0;
	return string;
} //end of the function NMSG_ReadString
