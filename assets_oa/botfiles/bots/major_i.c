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
#define FPH				5 
//armor
#define FPA				3
//without the weapons
//shotgun
#define SGW				350
//machinegun
#define MGW				50
//grenadelauncher
#define GLW				300
//rocket launcher
#define RLW				350
//rail gun
#define RGW				500
//bfg10k
#define BFW				200
//lightninggun
#define LGW				150
//plasmagun
#define PGW				375
//prox-launcher
#define PXW				150
//nailgun
#define NGW				275
//chaingun
#define CGW				350




//with the weapons
//shotgun
#define GSGW				100
//machinegun
#define GMGW				50
//grenadelauncher
#define GGLW				300
//rocketlauncher
#define GRLW				110
//railgun
#define GRGW				350
//bfg10k
#define GBFW				100
//lightninggun
#define GLGW			75
//plasmagun
#define GPGW			250
//prox-launcher
#define GPXW			100
//failgun
#define GNGW			190
//chaingun
#define GCGW			300
//individual powerups
//teleport
#define TELW				190
//medkit
#define MEDW				150
//quad-damage
#define QW				400
//envirosuit
#define ENVW				150
//haste
#define HAW				350
//invisibility
#define INW				150
//regeneration
#define REGW				450
//flight
#define FLW				125
//kamikaze
#define KAMW				150
//invulnerability
#define IBW				190
//portal
#define PORW				200
//scout
#define SCW				250
//guard
#define GUW				190
//doubler
#define DUBW				150
//ammo-regen
#define AMRW				150
//red-cube
#define REDCW				200
//blue-cube
#define BLCW				200
//ctf flag weight
#define FGW				500

//
#include "fuzi.c"
