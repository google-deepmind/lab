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

#include "inv.h"


//health
#define FPH				4 
//armor
#define FPA				3
//without the weapons
//shotgun
#define SGW				100
//machinegun
#define MGW				50
//grenadelauncher
#define GLW				200
//rocket launcher
#define RLW				320
//rail gun
#define RGW				170
//bfg10k
#define BFW				300
//lightninggun
#define LGW				185
//plasmagun
#define PGW				260
//prox-launcher
#define PXW				380
//nailgun
#define NGW				150
//chaingun
#define CGW				200




//with the weapons
//shotgun
#define GSGW				50
//machinegun
#define GMGW				50
//grenadelauncher
#define GGLW				100
//rocketlauncher
#define GRLW				110
//railgun
#define GRGW				50
//bfg10k
#define GBFW				100
//lightninggun
#define GLGW			120
//plasmagun
#define GPGW			50
//prox-launcher
#define GPXW			200
//failgun
#define GNGW			100
//chaingun
#define GCGW			125
//individual powerups
//teleport
#define TELW				190
//medkit
#define MEDW				150
//quad-damage
#define QW				200
//envirosuit
#define ENVW				50
//haste
#define HAW				50
//invisibility
#define INW				140
//regeneration
#define REGW				50
//flight
#define FLW				160
//kamikaze
#define KAMW				290
//invulnerability
#define IBW				150
//portal
#define PORW				300
//scout
#define SCW				190
//guard
#define GUW				280
//doubler
#define DUBW				100
//ammo-regen
#define AMRW				140
//red-cube
#define REDCW				200
//blue-cube
#define BLCW				200
//ctf flag weight
#define FGW				450

//
#include "fuzi.c"
