/*
   Copyright (C) 1999-2007 id Software, Inc., 2017 Google Inc. and contributors.
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

#include <stdint.h>

#ifndef _WIN32
// The below define is necessary to use
// pthreads extensions like pthread_mutexattr_settype
#define _GNU_SOURCE
#include <pthread.h>
#endif

#include "cmdlib.h"
#include "mathlib.h"
#include "inout.h"
#include "qthreads.h"

#define MAX_THREADS 64

int dispatch;
int workcount;
int oldf;
qboolean pacifier;

qboolean threaded;

/*
   =============
   GetThreadWork

   =============
 */
int GetThreadWork( void ){
	int r;
	int f;

	ThreadLock();

	if ( dispatch == workcount ) {
		ThreadUnlock();
		return -1;
	}

	f = 10 * dispatch / workcount;
	if ( f != oldf ) {
		oldf = f;
		if ( pacifier ) {
			Sys_Printf( "%i...", f );
			fflush( stdout );   /* ydnar */
		}
	}

	r = dispatch;
	dispatch++;
	ThreadUnlock();

	return r;
}


void ( *workfunction )( int );

void ThreadWorkerFunction( int threadnum ){
	int work;

	while ( 1 )
	{
		work = GetThreadWork();
		if ( work == -1 ) {
			break;
		}
//Sys_Printf ("thread %i, work %i\n", threadnum, work);
		workfunction( work );
	}
}

void RunThreadsOnIndividual( int workcnt, qboolean showpacifier, void ( *func )( int ) ){
	if ( numthreads == -1 ) {
		ThreadSetDefault();
	}
	workfunction = func;
	RunThreadsOn( workcnt, showpacifier, ThreadWorkerFunction );
}


/*
   ===================================================================

   WIN32

   ===================================================================
 */
#ifdef WIN32

#define USED

#include <windows.h>

int numthreads = -1;
CRITICAL_SECTION crit;
static int enter;

void ThreadSetDefault( void ){
	SYSTEM_INFO info;

	if ( numthreads == -1 ) { // not set manually
		GetSystemInfo( &info );
		numthreads = info.dwNumberOfProcessors;
		if ( numthreads < 1 || numthreads > 32 ) {
			numthreads = 1;
		}
	}

	Sys_Printf( "%i threads\n", numthreads );
}


void ThreadLock( void ){
	if ( !threaded ) {
		return;
	}
	EnterCriticalSection( &crit );
	if ( enter ) {
		Error( "Recursive ThreadLock\n" );
	}
	enter = 1;
}

void ThreadUnlock( void ){
	if ( !threaded ) {
		return;
	}
	if ( !enter ) {
		Error( "ThreadUnlock without lock\n" );
	}
	enter = 0;
	LeaveCriticalSection( &crit );
}

/*
   =============
   RunThreadsOn
   =============
 */
void RunThreadsOn( int workcnt, qboolean showpacifier, void ( *func )( int ) ){
	int threadid[MAX_THREADS];
	HANDLE threadhandle[MAX_THREADS];
	int i;
	int start, end;

	start = I_FloatTime();
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	threaded = qtrue;

	//
	// run threads in parallel
	//
	InitializeCriticalSection( &crit );

	if ( numthreads == 1 ) { // use same thread
		func( 0 );
	}
	else
	{
		for ( i = 0 ; i < numthreads ; i++ )
		{
			threadhandle[i] = CreateThread(
				NULL,   // LPSECURITY_ATTRIBUTES lpsa,
			    //0,		// DWORD cbStack,

			    /* ydnar: cranking stack size to eliminate radiosity crash with 1MB stack on win32 */
				( 4096 * 1024 ),

				(LPTHREAD_START_ROUTINE)func,   // LPTHREAD_START_ROUTINE lpStartAddr,
				(LPVOID)i,  // LPVOID lpvThreadParm,
				0,          //   DWORD fdwCreate,
				&threadid[i] );
		}

		for ( i = 0 ; i < numthreads ; i++ )
			WaitForSingleObject( threadhandle[i], INFINITE );
	}
	DeleteCriticalSection( &crit );

	threaded = qfalse;
	end = I_FloatTime();
	if ( pacifier ) {
		Sys_Printf( " (%i)\n", end - start );
	}
}


#endif

/*
   ===================================================================

   OSF1

   ===================================================================
 */

#ifdef __osf__
#define USED

int numthreads = 4;

void ThreadSetDefault( void ){
	if ( numthreads == -1 ) { // not set manually
		numthreads = 4;
	}
}


#include <pthread.h>

pthread_mutex_t *my_mutex;

void ThreadLock( void ){
	if ( my_mutex ) {
		pthread_mutex_lock( my_mutex );
	}
}

void ThreadUnlock( void ){
	if ( my_mutex ) {
		pthread_mutex_unlock( my_mutex );
	}
}


/*
   =============
   RunThreadsOn
   =============
 */
void RunThreadsOn( int workcnt, qboolean showpacifier, void ( *func )( int ) ){
	int i;
	pthread_t work_threads[MAX_THREADS];
	pthread_addr_t status;
	pthread_attr_t attrib;
	pthread_mutexattr_t mattrib;
	int start, end;

	start = I_FloatTime();
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	threaded = qtrue;

	if ( pacifier ) {
		setbuf( stdout, NULL );
	}

	if ( !my_mutex ) {
		my_mutex = safe_malloc( sizeof( *my_mutex ) );
		if ( pthread_mutexattr_create( &mattrib ) == -1 ) {
			Error( "pthread_mutex_attr_create failed" );
		}
		if ( pthread_mutexattr_setkind_np( &mattrib, MUTEX_FAST_NP ) == -1 ) {
			Error( "pthread_mutexattr_setkind_np failed" );
		}
		if ( pthread_mutex_init( my_mutex, mattrib ) == -1 ) {
			Error( "pthread_mutex_init failed" );
		}
	}

	if ( pthread_attr_create( &attrib ) == -1 ) {
		Error( "pthread_attr_create failed" );
	}
	if ( pthread_attr_setstacksize( &attrib, 0x100000 ) == -1 ) {
		Error( "pthread_attr_setstacksize failed" );
	}

	for ( i = 0 ; i < numthreads ; i++ )
	{
		if ( pthread_create( &work_threads[i], attrib
							 , (pthread_startroutine_t)func, (pthread_addr_t)i ) == -1 ) {
			Error( "pthread_create failed" );
		}
	}

	for ( i = 0 ; i < numthreads ; i++ )
	{
		if ( pthread_join( work_threads[i], &status ) == -1 ) {
			Error( "pthread_join failed" );
		}
	}

	threaded = qfalse;

	end = I_FloatTime();
	if ( pacifier ) {
		Sys_Printf( " (%i)\n", end - start );
	}
}


#endif

/*
   ===================================================================

   IRIX

   ===================================================================
 */

#ifdef _MIPS_ISA
#define USED

#include <task.h>
#include <abi_mutex.h>
#include <sys/types.h>
#include <sys/prctl.h>


int numthreads = -1;
abilock_t lck;

void ThreadSetDefault( void ){
	if ( numthreads == -1 ) {
		numthreads = prctl( PR_MAXPPROCS );
	}
	Sys_Printf( "%i threads\n", numthreads );
	usconfig( CONF_INITUSERS, numthreads );
}


void ThreadLock( void ){
	spin_lock( &lck );
}

void ThreadUnlock( void ){
	release_lock( &lck );
}


/*
   =============
   RunThreadsOn
   =============
 */
void RunThreadsOn( int workcnt, qboolean showpacifier, void ( *func )( int ) ){
	int i;
	int pid[MAX_THREADS];
	int start, end;

	start = I_FloatTime();
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	threaded = qtrue;

	if ( pacifier ) {
		setbuf( stdout, NULL );
	}

	init_lock( &lck );

	for ( i = 0 ; i < numthreads - 1 ; i++ )
	{
		pid[i] = sprocsp( ( void ( * )( void *, size_t ) )func, PR_SALL, (void *)i
						  , NULL, 0x200000 ); // 2 meg stacks
		if ( pid[i] == -1 ) {
			perror( "sproc" );
			Error( "sproc failed" );
		}
	}

	func( i );

	for ( i = 0 ; i < numthreads - 1 ; i++ )
		wait( NULL );

	threaded = qfalse;

	end = I_FloatTime();
	if ( pacifier ) {
		Sys_Printf( " (%i)\n", end - start );
	}
}


#endif


/*
   =======================================================================

   Linux pthreads

   =======================================================================
 */

#if defined( __linux__ ) || defined( __FreeBSD__ )
#define USED

int numthreads = 4;

void ThreadSetDefault( void ){
	if ( numthreads == -1 ) { // not set manually
		/* default to one thread, only multi-thread when specifically told to */
		numthreads = 1;
	}
	if ( numthreads > 1 ) {
		Sys_Printf( "threads: %d\n", numthreads );
	}
}

#include <pthread.h>

typedef struct pt_mutex_s
{
	pthread_t       *owner;
	pthread_mutex_t a_mutex;
	pthread_cond_t cond;
	unsigned int lock;
} pt_mutex_t;

pt_mutex_t global_lock;

void ThreadLock( void ){
	pt_mutex_t *pt_mutex = &global_lock;

	if ( !threaded ) {
		return;
	}

	pthread_mutex_lock( &pt_mutex->a_mutex );
	if ( pthread_equal( pthread_self(), (pthread_t)&pt_mutex->owner ) ) {
		pt_mutex->lock++;
	}
	else
	{
		if ( ( !pt_mutex->owner ) && ( pt_mutex->lock == 0 ) ) {
			pt_mutex->owner = (pthread_t *)pthread_self();
			pt_mutex->lock  = 1;
		}
		else
		{
			while ( 1 )
			{
				pthread_cond_wait( &pt_mutex->cond, &pt_mutex->a_mutex );
				if ( ( !pt_mutex->owner ) && ( pt_mutex->lock == 0 ) ) {
					pt_mutex->owner = (pthread_t *)pthread_self();
					pt_mutex->lock  = 1;
					break;
				}
			}
		}
	}
	pthread_mutex_unlock( &pt_mutex->a_mutex );
}

void ThreadUnlock( void ){
	pt_mutex_t *pt_mutex = &global_lock;

	if ( !threaded ) {
		return;
	}

	pthread_mutex_lock( &pt_mutex->a_mutex );
	pt_mutex->lock--;

	if ( pt_mutex->lock == 0 ) {
		pt_mutex->owner = NULL;
		pthread_cond_signal( &pt_mutex->cond );
	}

	pthread_mutex_unlock( &pt_mutex->a_mutex );
}

void recursive_mutex_init( pthread_mutexattr_t attribs ){
	pt_mutex_t *pt_mutex = &global_lock;

	pt_mutex->owner = NULL;
	if ( pthread_mutex_init( &pt_mutex->a_mutex, &attribs ) != 0 ) {
		Error( "pthread_mutex_init failed\n" );
	}
	if ( pthread_cond_init( &pt_mutex->cond, NULL ) != 0 ) {
		Error( "pthread_cond_init failed\n" );
	}

	pt_mutex->lock = 0;
}

/*
   =============
   RunThreadsOn
   =============
 */
void RunThreadsOn( int workcnt, qboolean showpacifier, void ( *func )( int ) ){
	pthread_mutexattr_t mattrib;
	pthread_t work_threads[MAX_THREADS];

	int start, end;
	int i = 0;
	void *exit_value;

	start     = I_FloatTime();
	pacifier  = showpacifier;

	dispatch  = 0;
	oldf      = -1;
	workcount = workcnt;

	if ( numthreads == 1 ) {
		func( 0 );
	}
	else
	{
		threaded  = qtrue;

		if ( pacifier ) {
			setbuf( stdout, NULL );
		}

		if ( pthread_mutexattr_init( &mattrib ) != 0 ) {
			Error( "pthread_mutexattr_init failed" );
		}
#if __GLIBC_MINOR__ == 1
		if ( pthread_mutexattr_settype( &mattrib, PTHREAD_MUTEX_FAST_NP ) != 0 )
#else
		if ( pthread_mutexattr_settype( &mattrib, PTHREAD_MUTEX_ADAPTIVE_NP ) != 0 )
#endif
		{ Error( "pthread_mutexattr_settype failed" ); }
		recursive_mutex_init( mattrib );

		for ( i = 0 ; i < numthreads ; i++ )
		{
			/* Default pthread attributes: joinable & non-realtime scheduling */
			if ( pthread_create( &work_threads[i], NULL, (void*)func, (void*)(uintptr_t)i ) != 0 ) {
				Error( "pthread_create failed" );
			}
		}
		for ( i = 0 ; i < numthreads ; i++ )
		{
			if ( pthread_join( work_threads[i], &exit_value ) != 0 ) {
				Error( "pthread_join failed" );
			}
		}
		pthread_mutexattr_destroy( &mattrib );
		threaded = qfalse;
	}

	end = I_FloatTime();
	if ( pacifier ) {
		Sys_Printf( " (%i)\n", end - start );
	}
}
#endif // if defined( __linux__ ) || defined( __FreeBSD__ )


/*
   =======================================================================

   SINGLE THREAD

   =======================================================================
 */

#ifndef USED

int numthreads = 1;

void ThreadSetDefault( void ){
	numthreads = 1;
}

void ThreadLock( void ){
}

void ThreadUnlock( void ){
}

/*
   =============
   RunThreadsOn
   =============
 */
void RunThreadsOn( int workcnt, qboolean showpacifier, void ( *func )( int ) ){
	int start, end;

	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	start = I_FloatTime();
	func( 0 );

	end = I_FloatTime();
	if ( pacifier ) {
		Sys_Printf( " (%i)\n", end - start );
	}
}

#endif
