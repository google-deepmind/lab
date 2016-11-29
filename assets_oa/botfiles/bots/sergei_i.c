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
#define FPH				2 
//armor
#define FPA				2
//without the weapons
//shotgun
#define SGW				90
//machinegun
#define MGW				50
//grenadelauncher
#define GLW				150
//rocket launcher
#define RLW				220
//rail gun
#define RGW				160
//bfg10k
#define BFW				150
//lightninggun
#define LGW				195
//plasmagun
#define PGW				260
//prox-launcher
#define PXW				180
//nailgun
#define NGW				350
//chaingun
#define CGW				100




//with the weapons
//shotgun
#define GSGW				50
//machinegun
#define GMGW				50
//grenadelauncher
#define GGLW				110
//rocketlauncher
#define GRLW				120
//railgun
#define GRGW				150
//bfg10k
#define GBFW				50
//lightninggun
#define GLGW			100
//plasmagun
#define GPGW			150
//prox-launcher
#define GPXW			100
//nailgun
#define GNGW			120
//chaingun
#define GCGW			125
//individual powerups
//teleport
#define TELW				140
//medkit
#define MEDW				190
//quad-damage
#define QW				200
//envirosuit
#define ENVW				150
//haste
#define HAW				125
//invisibility
#define INW				130
//regeneration
#define REGW				150
//flight
#define FLW				150
//kamikaze
#define KAMW				390
//invulnerability
#define IBW				150
//portal
#define PORW				130
//scout
#define SCW				190
//guard
#define GUW				180
//doubler
#define DUBW				110
//ammo-regen
#define AMRW				160
//red-cube
#define REDCW				200
//blue-cube
#define BLCW				200
//ctf flag weight
#define FGW				450

//
#include "fuzi.c"
