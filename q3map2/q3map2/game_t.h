
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

/* ydnar: for -game support */
typedef struct game_s
{
	char        *arg;           /* -game matches this */
	char        *gamePath;      /* main game data dir */
	char        *homeBasePath;  /* home sub-dir on unix */
	char        *magic;         /* magic word for figuring out base path */
	qboolean wolfLight;         /* when true, lights work like wolf q3map  */
	int bspVersion;             /* BSP version to use */
}
game_t;
