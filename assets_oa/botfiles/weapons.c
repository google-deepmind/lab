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

#define IMPACT_DAMAGE			1		//straight impact damage
#define SPLASH_DAMAGE			2		//splash damage

projectileinfo 
{
name				"bfgexploision"
damage				40
radius				100
damagetype			$evalint(IMPACT_DAMAGE|SPLASH_DAMAGE)
}

weaponinfo 
{
name				"BFG10K"
number				WEAPONINDEX_BFG
projectile			"bfgexploision"
numprojectiles			1
speed				0
} 

projectileinfo 
{
name				"gauntletdamage"
damage				50
damagetype			IMPACT_DAMAGE
}

weaponinfo 
{
name				"Gauntlet"
number				WEAPONINDEX_GAUNTLET
projectile			"gauntletdamage"
numprojectiles			1
speed				0
}

projectileinfo 
{
name				"grenade"
damage				120
radius				160
damagetype			$evalint(IMPACT_DAMAGE|SPLASH_DAMAGE)
}

weaponinfo 
{
name				"Grenade Launcher"
number				WEAPONINDEX_GRENADE_LAUNCHER
projectile			"grenade"
numprojectiles			1
speed				700
}


projectileinfo
{
name				"lightning"
damage				24
damagetype			IMPACT_DAMAGE
}

weaponinfo
{
name				"Lightning Gun"
number				WEAPONINDEX_LIGHTNING
projectile			"lightning"
numprojectiles			1
speed				0
} 


projectileinfo 
{
name				"machinegunbullet"
damage				8
damagetype			IMPACT_DAMAGE
}

weaponinfo 
{
name				"Machinegun"
number				WEAPONINDEX_MACHINEGUN
projectile			"machinegunbullet"
numprojectiles			1
speed				0
}

projectileinfo 
{
name				"plasma"
damage				20
radius				20
damagetype			$evalint(IMPACT_DAMAGE|SPLASH_DAMAGE)
}

weaponinfo 
{
name				"Plasma Gun"
number				WEAPONINDEX_PLASMAGUN
projectile			"plasma"
numprojectiles			1
speed				2000
} 

projectileinfo 
{
name				"rail"
damage				100
damagetype			IMPACT_DAMAGE
}

weaponinfo 
{
name				"Railgun"
number				WEAPONINDEX_RAILGUN
projectile			"rail"
numprojectiles			1
speed				0
} 

projectileinfo
{
name				"rocket"
damage				100
radius				120
damagetype			$evalint(IMPACT_DAMAGE|SPLASH_DAMAGE)
}

weaponinfo 
{
name				"Rocket Launcher"
number				WEAPONINDEX_ROCKET_LAUNCHER
projectile			"rocket"
numprojectiles			1
speed				900
}


projectileinfo
{
name				"shotgunbullet"
damage				10
damagetype			IMPACT_DAMAGE
}

weaponinfo
{
name				"Shotgun"
number				WEAPONINDEX_SHOTGUN
projectile			"shotgunbullet"
numprojectiles			11
speed				0
}

projectileinfo 
{
name				"chaingunbullet"
damage				7
damagetype			$evalint(IMPACT_DAMAGE)
}

weaponinfo 
{
name				"Chaingun"
number				WEAPONINDEX_CHAINGUN
projectile			"chaingunbullet"
numprojectiles			1
speed				0
} 

projectileinfo 
{
name				"nail"
damage				30
damagetype			$evalint(IMPACT_DAMAGE)
}

weaponinfo 
{
name				"Nailgun"
number				WEAPONINDEX_NAILGUN
projectile			"nail"
numprojectiles			13
speed				0
} 

projectileinfo 
{
name				"mine"
damage				0
damagetype			$evalint(SPLASH_DAMAGE)
}

weaponinfo 
{
name				"Prox Launcher"
number				WEAPONINDEX_PROXLAUNCHER
projectile			"mine"
numprojectiles		1
speed				0
}
