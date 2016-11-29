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

//===========================================================================
//
// Name:         l_net_wins.c
// Function:     WinSock
// Programmer:   MrElusive
// Last update:  TTimo: cross-platform version, l_net library
// Tab Size:     2
// Notes:
//===========================================================================

//#include <windows.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "l_net.h"
#include "l_net_wins.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <netdb.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

extern void WinPrint( char *str, ... );

#define WinError WinPrint

#define qtrue   1
#define qfalse  0

#define ioctlsocket ioctl
#define closesocket close

int WSAGetLastError(){
	return errno;
}

/*
   typedef struct tag_error_struct
   {
    int     errnum;
    LPSTR   errstr;
   } ERROR_STRUCT;
 */

typedef struct tag_error_struct
{
	int errnum;
	const char *errstr;
} ERROR_STRUCT;

#define NET_NAMELEN         64

static char my_tcpip_address[NET_NAMELEN];

#define DEFAULTnet_hostport 26000

#define MAXHOSTNAMELEN      256

static int net_acceptsocket = -1;       // socket for fielding new connections
static int net_controlsocket;
static int net_hostport;                // udp port number for acceptsocket
static int net_broadcastsocket = 0;
//static qboolean ifbcastinit = qfalse;
//static struct sockaddr_s broadcastaddr;
static struct sockaddr_s broadcastaddr;

static unsigned long myAddr;

ERROR_STRUCT errlist[] = {
	{EACCES,"EACCES - The address is protected, user is not root"},
	{EAGAIN,"EAGAIN - Operation on non-blocking socket that cannot return immediatly"},
	{EBADF, "EBADF - sockfd is not a valid descriptor"},
	{EFAULT, "EFAULT - The parameter is not in a writable part of the user address space"},
	{EINVAL,"EINVAL - The socket is already bound to an address"},
	{ENOBUFS,"ENOBUFS - not enough memory"},
	{ENOMEM, "ENOMEM - not enough memory"},
	{ENOTCONN, "ENOTCONN - not connected"},
	{ENOTSOCK,"ENOTSOCK - Argument is file descriptor not a socket"},
	{EOPNOTSUPP,"ENOTSUPP - The referenced socket is not of type SOCK_STREAM"},
	{EPERM, "EPERM - Firewall rules forbid connection"},
	{-1, NULL}
};

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *WINS_ErrorMessage( int error ){
	int search = 0;

	if ( !error ) {
		return "No error occurred";
	}

	for ( search = 0; errlist[search].errstr; search++ )
	{
		if ( error == errlist[search].errnum ) {
			return (char *)errlist[search].errstr;
		}
	}  //end for

	return "Unknown error";
} //end of the function WINS_ErrorMessage
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_Init( void ){
	int i;
	struct hostent *local;
	char buff[MAXHOSTNAMELEN];
	struct sockaddr_s addr;
	char    *p;
/*
   linux doesn't have anything to initialize for the net
   "Windows .. built for the internet .. the internet .. built with unix"
 */
#if 0
	int r;
	WORD wVersionRequested;

	wVersionRequested = MAKEWORD( 2, 2 );

	r = WSAStartup( wVersionRequested, &winsockdata );

	if ( r ) {
		WinPrint( "Winsock initialization failed.\n" );
		return -1;
	}
#endif
	/*
	   i = COM_CheckParm ("-udpport");
	   if (i == 0)*/
	net_hostport = DEFAULTnet_hostport;
	/*
	   else if (i < com_argc-1)
	    net_hostport = Q_atoi (com_argv[i+1]);
	   else
	    Sys_Error ("WINS_Init: you must specify a number after -udpport");
	 */

	// determine my name & address
	gethostname( buff, MAXHOSTNAMELEN );
	local = gethostbyname( buff );

	// When hostname can not be resolved, return gracefully
	if ( local == 0 ) {
		WinError( "WINS_Init: Unable to resolve hostname\n" );
		return 0;
	}

	myAddr = *(int *)local->h_addr_list[0];

	// if the quake hostname isn't set, set it to the machine name
//	if (Q_strcmp(hostname.string, "UNNAMED") == 0)
	{
		// see if it's a text IP address (well, close enough)
		for ( p = buff; *p; p++ )
			if ( ( *p < '0' || *p > '9' ) && *p != '.' ) {
				break;
			}

		// if it is a real name, strip off the domain; we only want the host
		if ( *p ) {
			for ( i = 0; i < 15; i++ )
				if ( buff[i] == '.' ) {
					break;
				}
			buff[i] = 0;
		}
//		Cvar_Set ("hostname", buff);
	}

	//++timo WTF is that net_controlsocket? it's sole purpose is to retrieve the local IP?
	if ( ( net_controlsocket = WINS_OpenSocket( 0 ) ) == SOCKET_ERROR ) {
		WinError( "WINS_Init: Unable to open control socket\n" );
	}

	( (struct sockaddr_in *)&broadcastaddr )->sin_family = AF_INET;
	( (struct sockaddr_in *)&broadcastaddr )->sin_addr.s_addr = INADDR_BROADCAST;
	( (struct sockaddr_in *)&broadcastaddr )->sin_port = htons( (u_short)net_hostport );

	WINS_GetSocketAddr( net_controlsocket, &addr );
	strcpy( my_tcpip_address,  WINS_AddrToString( &addr ) );
	p = strrchr( my_tcpip_address, ':' );
	if ( p ) {
		*p = 0;
	}
	WinPrint( "Winsock Initialized\n" );

	return net_controlsocket;
} //end of the function WINS_Init
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *WINS_MyAddress( void ){
	return my_tcpip_address;
} //end of the function WINS_MyAddress
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void WINS_Shutdown( void ){
	//WINS_Listen(0);
	WINS_CloseSocket( net_controlsocket );
//	WSACleanup();
	//
	WinPrint( "Winsock Shutdown\n" );
} //end of the function WINS_Shutdown
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
   void WINS_Listen(int state)
   {
    // enable listening
    if (state)
    {
        if (net_acceptsocket != -1)
            return;
        if ((net_acceptsocket = WINS_OpenSocket (net_hostport)) == -1)
            WinError ("WINS_Listen: Unable to open accept socket\n");
        return;
    }

    // disable listening
    if (net_acceptsocket == -1)
        return;
    WINS_CloseSocket (net_acceptsocket);
    net_acceptsocket = -1;
   } //end of the function WINS_Listen*/
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_OpenSocket( int port ){
	int newsocket;
	struct sockaddr_in address;
	u_long _true = 1;

	if ( ( newsocket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) == SOCKET_ERROR ) {
		WinPrint( "WINS_OpenSocket: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		return -1;
	} //end if

	if ( ioctlsocket( newsocket, FIONBIO, &_true ) == SOCKET_ERROR ) {
		WinPrint( "WINS_OpenSocket: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		closesocket( newsocket );
		return -1;
	} //end if

	memset( (char *) &address, 0, sizeof( address ) );
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( (u_short)port );
	if ( bind( newsocket, (void *)&address, sizeof( address ) ) == -1 ) {
		WinPrint( "WINS_OpenSocket: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		closesocket( newsocket );
		return -1;
	} //end if

	return newsocket;
} //end of the function WINS_OpenSocket
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_OpenReliableSocket( int port ){
	int newsocket;
	struct sockaddr_in address;
	qboolean _true = 0xFFFFFFFF;

	//IPPROTO_TCP
	//
	if ( ( newsocket = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		WinPrint( "WINS_OpenReliableSocket: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		return -1;
	} //end if

	memset( (char *) &address, 0, sizeof( address ) );
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl( INADDR_ANY );
	address.sin_port = htons( (u_short)port );
	if ( bind( newsocket, (void *)&address, sizeof( address ) ) == -1 ) {
		WinPrint( "WINS_OpenReliableSocket: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		closesocket( newsocket );
		return -1;
	} //end if

	//
	if ( setsockopt( newsocket, IPPROTO_TCP, TCP_NODELAY, (void *) &_true, sizeof( int ) ) == -1 ) {
		WinPrint( "WINS_OpenReliableSocket: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		WinPrint( "setsockopt error\n" );
	} //end if

	return newsocket;
} //end of the function WINS_OpenReliableSocket
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_Listen( int socket ){
	u_long _true = 1;

	if ( ioctlsocket( socket, FIONBIO, &_true ) == -1 ) {
		WinPrint( "WINS_Listen: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		return -1;
	} //end if
	if ( listen( socket, SOMAXCONN ) == SOCKET_ERROR ) {
		WinPrint( "WINS_Listen: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		return -1;
	} //end if
	return 0;
} //end of the function WINS_Listen
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_Accept( int socket, struct sockaddr_s *addr ){
	socklen_t addrlen = sizeof( struct sockaddr_s );
	int newsocket;
	qboolean _true = 1;

	newsocket = accept( socket, (struct sockaddr *)addr, &addrlen );
	if ( newsocket == INVALID_SOCKET ) {
		if ( errno == EAGAIN ) {
			return -1;
		}
		WinPrint( "WINS_Accept: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		return -1;
	} //end if
	  //
	if ( setsockopt( newsocket, IPPROTO_TCP, TCP_NODELAY, (void *) &_true, sizeof( int ) ) == SOCKET_ERROR ) {
		WinPrint( "WINS_Accept: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		WinPrint( "setsockopt error\n" );
	} //end if
	return newsocket;
} //end of the function WINS_Accept
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_CloseSocket( int socket ){
	/*
	   if (socket == net_broadcastsocket)
	    net_broadcastsocket = 0;
	 */
//	shutdown(socket, SD_SEND);

	if ( closesocket( socket ) == SOCKET_ERROR ) {
		WinPrint( "WINS_CloseSocket: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		return SOCKET_ERROR;
	} //end if
	return 0;
} //end of the function WINS_CloseSocket
//===========================================================================
// this lets you type only as much of the net address as required, using
// the local network components to fill in the rest
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
static int PartialIPAddress( char *in, struct sockaddr_s *hostaddr ){
	char buff[256];
	char *b;
	int addr;
	int num;
	int mask;

	buff[0] = '.';
	b = buff;
	strcpy( buff + 1, in );
	if ( buff[1] == '.' ) {
		b++;
	}

	addr = 0;
	mask = -1;
	while ( *b == '.' )
	{
		num = 0;
		if ( *++b < '0' || *b > '9' ) {
			return -1;
		}
		while ( !( *b < '0' || *b > '9' ) )
			num = num * 10 + *( b++ ) - '0';
		mask <<= 8;
		addr = ( addr << 8 ) + num;
	}

	hostaddr->sa_family = AF_INET;
	( (struct sockaddr_in *)hostaddr )->sin_port = htons( (u_short)net_hostport );
	( (struct sockaddr_in *)hostaddr )->sin_addr.s_addr = ( myAddr & htonl( mask ) ) | htonl( addr );

	return 0;
} //end of the function PartialIPAddress
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_Connect( int socket, struct sockaddr_s *addr ){
	int ret;
	u_long _true2 = 0xFFFFFFFF;

	ret = connect( socket, (struct sockaddr *)addr, sizeof( struct sockaddr_s ) );
	if ( ret == SOCKET_ERROR ) {
		WinPrint( "WINS_Connect: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		return -1;
	} //end if
	if ( ioctlsocket( socket, FIONBIO, &_true2 ) == -1 ) {
		WinPrint( "WINS_Connect: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
		return -1;
	} //end if
	return 0;
} //end of the function WINS_Connect
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_CheckNewConnections( void ){
	char buf[4];

	if ( net_acceptsocket == -1 ) {
		return -1;
	}

	if ( recvfrom( net_acceptsocket, buf, 4, MSG_PEEK, NULL, NULL ) > 0 ) {
		return net_acceptsocket;
	}
	return -1;
} //end of the function WINS_CheckNewConnections
//===========================================================================
// returns the number of bytes read
// 0 if no bytes available
// -1 on failure
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_Read( int socket, byte *buf, int len, struct sockaddr_s *addr ){
	socklen_t addrlen = sizeof( struct sockaddr_s );
	int ret;

	if ( addr ) {
		ret = recvfrom( socket, buf, len, 0, (struct sockaddr *)addr, &addrlen );
		if ( ret == -1 ) {
//			errno = WSAGetLastError();

			if ( errno == EAGAIN || errno == ENOTCONN ) {
				return 0;
			}
		} //end if
	} //end if
	else
	{
		ret = recv( socket, buf, len, 0 );
		// if there's no data on the socket ret == -1 and errno == EAGAIN
		// MSDN states that if ret == 0 the socket has been closed
		// man recv doesn't say anything
		if ( ret == 0 ) {
			return -1;
		}
		if ( ret == SOCKET_ERROR ) {
//			errno = WSAGetLastError();

			if ( errno == EAGAIN || errno == ENOTCONN ) {
				return 0;
			}
		} //end if
	} //end else
	if ( ret == SOCKET_ERROR ) {
		WinPrint( "WINS_Read: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
	} //end if
	return ret;
} //end of the function WINS_Read
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_MakeSocketBroadcastCapable( int socket ){
	int i = 1;

	// make this socket broadcast capable
	if ( setsockopt( socket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof( i ) ) < 0 ) {
		return -1;
	}
	net_broadcastsocket = socket;

	return 0;
} //end of the function WINS_MakeSocketBroadcastCapable
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_Broadcast( int socket, byte *buf, int len ){
	int ret;

	if ( socket != net_broadcastsocket ) {
		if ( net_broadcastsocket != 0 ) {
			WinError( "Attempted to use multiple broadcasts sockets\n" );
		}
		ret = WINS_MakeSocketBroadcastCapable( socket );
		if ( ret == -1 ) {
			WinPrint( "Unable to make socket broadcast capable\n" );
			return ret;
		}
	}

	return WINS_Write( socket, buf, len, &broadcastaddr );
} //end of the function WINS_Broadcast
//===========================================================================
// returns qtrue on success or qfalse on failure
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_Write( int socket, byte *buf, int len, struct sockaddr_s *addr ){
	int ret = 0, written;

	if ( addr ) {
		written = 0;
		while ( written < len )
		{
			ret = sendto( socket, &buf[written], len - written, 0, (struct sockaddr *)addr, sizeof( struct sockaddr_s ) );
			if ( ret == SOCKET_ERROR ) {
				if ( WSAGetLastError() != EAGAIN ) {
					return qfalse;
				}
				//++timo FIXME: what is this used for?
//				Sleep(1000);
			} //end if
			else
			{
				written += ret;
			}
		}
	} //end if
	else
	{
		written = 0;
		while ( written < len )
		{
			ret = send( socket, buf, len, 0 );
			if ( ret == SOCKET_ERROR ) {
				if ( WSAGetLastError() != EAGAIN ) {
					return qfalse;
				}
				//++timo FIXME: what is this used for?
//				Sleep(1000);
			} //end if
			else
			{
				written += ret;
			}
		}
	} //end else
	if ( ret == SOCKET_ERROR ) {
		WinPrint( "WINS_Write: %s\n", WINS_ErrorMessage( WSAGetLastError() ) );
	} //end if
	return ( ret == len );
} //end of the function WINS_Write
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *WINS_AddrToString( struct sockaddr_s *addr ){
	static char buffer[22];
	int haddr;

	haddr = ntohl( ( (struct sockaddr_in *)addr )->sin_addr.s_addr );
	sprintf( buffer, "%d.%d.%d.%d:%d", ( haddr >> 24 ) & 0xff, ( haddr >> 16 ) & 0xff, ( haddr >> 8 ) & 0xff, haddr & 0xff, ntohs( ( (struct sockaddr_in *)addr )->sin_port ) );
	return buffer;
} //end of the function WINS_AddrToString
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_StringToAddr( char *string, struct sockaddr_s *addr ){
	int ha1, ha2, ha3, ha4, hp;
	int ipaddr;

	sscanf( string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp );
	ipaddr = ( ha1 << 24 ) | ( ha2 << 16 ) | ( ha3 << 8 ) | ha4;

	addr->sa_family = AF_INET;
	( (struct sockaddr_in *)addr )->sin_addr.s_addr = htonl( ipaddr );
	( (struct sockaddr_in *)addr )->sin_port = htons( (u_short)hp );
	return 0;
} //end of the function WINS_StringToAddr
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_GetSocketAddr( int socket, struct sockaddr_s *addr ){
	socklen_t addrlen = sizeof( struct sockaddr_s );
	unsigned int a;

	memset( addr, 0, sizeof( struct sockaddr_s ) );
	getsockname( socket, (struct sockaddr *)addr, &addrlen );
	a = ( (struct sockaddr_in *)addr )->sin_addr.s_addr;
	if ( a == 0 || a == inet_addr( "127.0.0.1" ) ) {
		( (struct sockaddr_in *)addr )->sin_addr.s_addr = myAddr;
	}

	return 0;
} //end of the function WINS_GetSocketAddr
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_GetNameFromAddr( struct sockaddr_s *addr, char *name ){
	struct hostent *hostentry;

	hostentry = gethostbyaddr( (char *)&( (struct sockaddr_in *)addr )->sin_addr, sizeof( struct in_addr ), AF_INET );
	if ( hostentry ) {
		strncpy( name, (char *)hostentry->h_name, NET_NAMELEN - 1 );
		return 0;
	}

	strcpy( name, WINS_AddrToString( addr ) );
	return 0;
} //end of the function WINS_GetNameFromAddr
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_GetAddrFromName( char *name, struct sockaddr_s *addr ){
	struct hostent *hostentry;

	if ( name[0] >= '0' && name[0] <= '9' ) {
		return PartialIPAddress( name, addr );
	}

	hostentry = gethostbyname( name );
	if ( !hostentry ) {
		return -1;
	}

	addr->sa_family = AF_INET;
	( (struct sockaddr_in *)addr )->sin_port = htons( (u_short)net_hostport );
	( (struct sockaddr_in *)addr )->sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];

	return 0;
} //end of the function WINS_GetAddrFromName
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_AddrCompare( struct sockaddr_s *addr1, struct sockaddr_s *addr2 ){
	if ( addr1->sa_family != addr2->sa_family ) {
		return -1;
	}

	if ( ( (struct sockaddr_in *)addr1 )->sin_addr.s_addr != ( (struct sockaddr_in *)addr2 )->sin_addr.s_addr ) {
		return -1;
	}

	if ( ( (struct sockaddr_in *)addr1 )->sin_port != ( (struct sockaddr_in *)addr2 )->sin_port ) {
		return 1;
	}

	return 0;
} //end of the function WINS_AddrCompare
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_GetSocketPort( struct sockaddr_s *addr ){
	return ntohs( ( (struct sockaddr_in *)addr )->sin_port );
} //end of the function WINS_GetSocketPort
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WINS_SetSocketPort( struct sockaddr_s *addr, int port ){
	( (struct sockaddr_in *)addr )->sin_port = htons( (u_short)port );
	return 0;
} //end of the function WINS_SetSocketPort
