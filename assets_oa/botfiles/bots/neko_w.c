/*
===========================================================================
Copyright (C) 2006 Dmn_clown (aka: Bob Isaac (rjisaac@gmail.com))

This file is part of Open Arena and is based upon Mr. Elusive's fuzzy logic
system found in Quake 3 Arena.

Open Arena is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Open Arena is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*weapon defines    		weights 10-500
			    try to keep it balanced*/

#include "inv.h"
//gauntlet
#define GTW				10
//shotgun
#define SGW				180
//machinegun
#define MGW				75
//grenade launcher
#define GLW				170
//rocket launcher
#define RLW				160
//railgun
#define RGW				250
//bfg10k
#define BFW				90
//lightninggun
#define LGW				200
//plasmagun
#define PGW				110
//grapplinghook
#define GRW				10
//prox-launcher
#define PXW				160
//nailgun
#define NGW				200
//chaingun
#define CGW				175

#include "fuzw.c"
