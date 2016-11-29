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
// Name:			l_net.h
// Function:		-
// Programmer:		MrElusive
// Last update:		TTimo: cross-platform version, l_net library
// Tab size:		2
// Notes:
//====================================================================

//++timo FIXME: the l_net code understands that as the max size for the netmessage_s structure
//  we have defined unsigned char data[MAX_NETMESSAGE] in netmessage_s but actually it cannot be filled completely
//  we need to introduce a new #define and adapt to data[MAX_NETBUFFER]
#define MAX_NETMESSAGE      1024
#define MAX_NETADDRESS      32

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef enum { qfalse, qtrue } qboolean;
typedef unsigned char byte;
#endif

typedef struct address_s
{
	char ip[MAX_NETADDRESS];
} address_t;

typedef struct sockaddr_s
{
	short sa_family;
	unsigned char sa_data[14];
} sockaddr_t;

typedef struct netmessage_s
{
	unsigned char data[MAX_NETMESSAGE];
	int size;
	int read;
	int readoverflow;
} netmessage_t;

typedef struct socket_s
{
	int socket;                         //socket number
	sockaddr_t addr;                    //socket address
	netmessage_t msg;                   //current message being read
	int remaining;                      //remaining bytes to read for the current message
	struct socket_s *prev, *next;   //prev and next socket in a list
} socket_t;

//compare addresses
int Net_AddressCompare( address_t *addr1, address_t *addr2 );
//gives the address of a socket
void Net_SocketToAddress( socket_t *sock, address_t *address );
//converts a string to an address
void Net_StringToAddress( char *string, address_t *address );
//set the address ip port
void Net_SetAddressPort( address_t *address, int port );
//send a message to the given socket
int Net_Send( socket_t *sock, netmessage_t *msg );
//recieve a message from the given socket
int Net_Receive( socket_t *sock, netmessage_t *msg );
//connect to a host
// NOTE: port is the localhost port, usually 0
// ex: Net_Connect( "192.168.0.1:39000", 0 )
socket_t *Net_Connect( address_t *address, int port );
//disconnect from a host
void Net_Disconnect( socket_t *sock );
//returns the local address
void Net_MyAddress( address_t *address );
//listen at the given port
socket_t *Net_ListenSocket( int port );
//accept new connections at the given socket
socket_t *Net_Accept( socket_t *sock );
//setup networking
int Net_Setup( void );
//shutdown networking
void Net_Shutdown( void );
//message handling
void  NMSG_Clear( netmessage_t *msg );
void  NMSG_WriteChar( netmessage_t *msg, int c );
void  NMSG_WriteByte( netmessage_t *msg, int c );
void  NMSG_WriteShort( netmessage_t *msg, int c );
void  NMSG_WriteLong( netmessage_t *msg, int c );
void  NMSG_WriteFloat( netmessage_t *msg, float c );
void  NMSG_WriteString( netmessage_t *msg, char *string );
void  NMSG_ReadStart( netmessage_t *msg );
int   NMSG_ReadChar( netmessage_t *msg );
int   NMSG_ReadByte( netmessage_t *msg );
int   NMSG_ReadShort( netmessage_t *msg );
int   NMSG_ReadLong( netmessage_t *msg );
float NMSG_ReadFloat( netmessage_t *msg );
char *NMSG_ReadString( netmessage_t *msg );

//++timo FIXME: the WINS_ things are not necessary, they can be made portable arther easily
char *WINS_ErrorMessage( int error );

#ifdef __cplusplus
}
#endif
