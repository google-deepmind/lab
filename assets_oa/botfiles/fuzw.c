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

weight "BFG10K"
{
	switch(INVENTORY_BFG10K)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_BFGAMMO)
			{
			case 1: return 0;
			default: return BFW;
			}
		}
	} 
} 

weight "Chaingun"
{
	switch(INVENTORY_CHAINGUN)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_BELT)
			{
			case 1: return 0;
			default: 
				{
				switch(ENEMY_HORIZONTAL_DIST)
					{
					case 900: return CGW;
					default: return $evalint(CGW*0.1);
					}
				}
			}
		}
	} 
} 


weight "Gauntlet"
{
	switch(INVENTORY_GAUNTLET)
		{
		case 1: return 0;
		default: return GTW;
		} 
} 

weight "Grappling Hook"
{
	switch(INVENTORY_GRAPPLINGHOOK)
	{
		case 1: return 0;
		default: return GRW;
	} 
} 


weight "Grenade Launcher"
{
	switch(INVENTORY_GRENADELAUNCHER)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_GRENADES)
			{
			case 1: return 0;
			default: 
				{
				switch(ENEMY_HORIZONTAL_DIST)
					{
					case 600: return GLW;
					default: return $evalint(GLW*0.1);
					}
				}
			}
		}
	} 
} 

weight "Lightning Gun"
{
	switch(INVENTORY_LIGHTNING)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_LIGHTNINGAMMO)
			{
			case 1: return 0;
			default: 
				{
				switch(ENEMY_HORIZONTAL_DIST)
					{
					case 768: return LGW;
					default: return $evalint(LGW*0.1);
					}
				}
			}
		}
	} 
}

weight "Machinegun"
{
	switch(INVENTORY_MACHINEGUN)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_BULLETS)
			{
			case 1: return 0;
			default: 
				{
				switch(ENEMY_HORIZONTAL_DIST)
					{
					case 800: return MGW;
					default: return $evalint(MGW*0.1);
					}
				}
			}
		}
	} 
} 

weight "Nailgun"
{
	switch(INVENTORY_NAILGUN)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_NAILS)
			{
			case 1: return 0;
			default: 
				{
				switch(ENEMY_HORIZONTAL_DIST)
					{
					case 800: return NGW;
					default: return $evalint(NGW*0.1);
					}
				}
			}
		}
	} 
} 

weight "Plasma Gun"
{
	switch(INVENTORY_PLASMAGUN)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_CELLS)
			{
			case 1: return 0;
			default: 
				{
				switch(ENEMY_HORIZONTAL_DIST)
					{
					case 2000: return PGW;
					default: return $evalint(PGW*0.1);
					}
				}
			}
		}
	} 
} 

weight "Prox Launcher"
{
	switch(INVENTORY_PROXLAUNCHER)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_MINES)
			{
			case 1: return 0;
			default: 
				{
				switch(ENEMY_HORIZONTAL_DIST)
					{
					case 200: return PXW;
					default: return $evalint(PXW*0.1);
					}
				}
			}
		}
	}
}

weight "Railgun"
{
	switch(INVENTORY_RAILGUN)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_SLUGS)
			{
			case 1: return 0;
			default: return RGW;
			}
		}
	} 
} 


weight "Rocket Launcher"
{
	switch(INVENTORY_ROCKETLAUNCHER)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_ROCKETS)
			{
			case 1: return 0;
			default: 
				{
				switch(ENEMY_HORIZONTAL_DIST)
					{
					case 4000: return RLW;
					default: return $evalint(RLW*0.1);
					}
				}
			}
		}
	} 
} 

weight "Shotgun"
{
	switch(INVENTORY_SHOTGUN)
	{
		case 1: return 0;
		default: 
		{
		switch(INVENTORY_SHELLS)
			{
			case 1: return 0;
			default: 
				{
				switch(ENEMY_HORIZONTAL_DIST)
					{
					case 600: return SGW;
					default: return $evalint(SGW*0.1);
					}
				}			
			}
		}
	} 
}