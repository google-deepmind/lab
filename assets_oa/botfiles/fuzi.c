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


#define ARMOR				20
#define AMMO				20
#define HEALTH				20
#define POWERUP				20
#define WEAPON				20

//breaks compatibility with the existing q3a bots, sorry.
#define FP(x)	(x < 0 ? 1 : x)
#define ARS(x) 	balance($evalfloat(FP(FPA/x)), $evalfloat(FP(FPA/x-ARMOR)), $evalfloat(FP(FPA/x+ARMOR)))
#define AS(x)	balance($evalfloat(FP(x)), $evalfloat(FP(x/AMMO)), $evalfloat(FP(x*AMMO)))
#define HS(x)	balance($evalfloat(FP(FPH/x)), $evalfloat(FP(FPH/x-HEALTH)), $evalfloat(FP(FPH/x+HEALTH)))
#define PS(x)	balance($evalfloat(FP(x)), $evalfloat(FP(x/POWERUP)), $evalfloat(FP(x*POWERUP)))
#define WS(x)	balance($evalfloat(FP(x)), $evalfloat(FP(x/WEAPON)), $evalfloat(FP(x*WEAPON)))


weight "item_armor_shard"
{
switch(INVENTORY_SCOUT)
	{
	case 1:
	{
	switch(INVENTORY_ARMOR)
	{
	case 10: return ARS(20);
	case 20: return ARS(20);
	case 30: return ARS(20);
	case 40: return ARS(19);
	case 50: return ARS(19);
	case 60: return ARS(19);
	case 70: return ARS(19);
	case 80: return ARS(18);
	case 90: return ARS(18);
	case 100: return ARS(15);
	case 110: return ARS(15);
	case 120: return ARS(15);
	case 130: return ARS(15);
	case 140: return ARS(12);
	case 150: return ARS(12);
	case 160: return ARS(12);
	case 170: return ARS(12);
	case 180: return ARS(12);
	case 190: return ARS(11);
	case 200: return ARS(11);
	default: return ARS(1);
	} 
	} 
	default: return 0;
	} 
}

weight "item_armor_combat"
{
	switch(INVENTORY_SCOUT) 
	{
	case 1:
	{
	switch(INVENTORY_ARMOR)
	{
	case 10: return ARS(80);
	case 20: return ARS(70);
	case 30: return ARS(65);
	case 40: return ARS(65);
	case 50: return ARS(65);
	case 60: return ARS(60);
	case 70: return ARS(60);
	case 80: return ARS(40);
	case 90: return ARS(40);
	case 100: return ARS(40);
	case 110: return ARS(30);
	case 120: return ARS(30);
	case 130: return ARS(30);
	case 140: return ARS(25);
	case 150: return ARS(25);
	case 160: return ARS(25);
	case 170: return ARS(25);
	case 180: return ARS(20);
	case 190: return ARS(20);
	case 200: return ARS(20);
	default: return ARS(1);
	}
	}
	default: return 0;
	} 
}

weight "item_armor_body"
{
	switch(INVENTORY_SCOUT) 
	{
	case 1:
	{
	switch(INVENTORY_ARMOR)
	{
	case 10: return ARS(100);
	case 20: return ARS(95);
	case 30: return ARS(95);
	case 40: return ARS(95);
	case 50: return ARS(90);
	case 60: return ARS(60);
	case 70: return ARS(60);
	case 80: return ARS(55);
	case 90: return ARS(55);
	case 100: return ARS(50);
	case 110: return ARS(40);
	case 120: return ARS(40);
	case 130: return ARS(35);
	case 140: return ARS(25);
	case 150: return ARS(15);
	case 160: return ARS(15);
	case 170: return ARS(15);
	case 180: return ARS(10);
	case 190: return ARS(10);
	case 200: return ARS(10);
	default: return ARS(1);
	}
	}
	default: return 0;
	} 
}

weight "item_health_small"
{
	switch(INVENTORY_HEALTH)
	{
	case 10: return HS(100);
	case 20: return HS(90);
	case 30: return HS(85);
	case 40: return HS(80);
	case 50: return HS(75);
	case 60: return HS(70);
	case 70: return HS(65);
	case 80: return HS(60);
	case 90: return HS(55);
	case 100: return HS(50);
	default: return 0;
	}
}

weight "item_health"
{
	switch(INVENTORY_HEALTH)
	{
	case 10: return HS(110);
	case 20: return HS(100);
	case 30: return HS(90);
	case 40: return HS(80);
	case 50: return HS(70);
	case 60: return HS(60);
	case 70: return HS(50);
	case 80: return HS(40);
	case 90: return HS(30);
	case 100: return HS(30);
	default: return 0;
	}
} 

weight "item_health_large"
{
	
	switch(INVENTORY_HEALTH)
	{
	case 10: return HS(120);
	case 20: return HS(110);
	case 30: return HS(100);
	case 40: return HS(90);
	case 50: return HS(80);
	case 60: return HS(70);
	case 70: return HS(60);
	case 80: return HS(50);
	case 90: return HS(40);
	case 100: return HS(30);
	default: return 0;
	}
}

weight "item_health_mega"
{
	switch(INVENTORY_HEALTH)
	{
	case 10: return HS(150);
	case 20: return HS(120);
	case 30: return HS(110);
	case 40: return HS(100);
	case 50: return HS(90);
	case 60: return HS(90);
	case 70: return HS(80);
	case 80: return HS(80);
	case 90: return HS(70);
	case 100: return HS(60);
	default: return 0;
	}
}

weight "weapon_shotgun"
{
	switch(INVENTORY_SHOTGUN)
	{
	case 1: return WS(SGW);
	default: return 1;
	}
}

weight "weapon_machinegun"
{
	switch(INVENTORY_MACHINEGUN)
	{
	case 1: return WS(MGW);
	default: return 1;
	} 
}

weight "weapon_grenadelauncher"
{
	switch(INVENTORY_GRENADELAUNCHER)
	{
	case 1: return WS(GLW);
	default: return 1;
	} 
}

weight "weapon_rocketlauncher"
{
	switch(INVENTORY_ROCKETLAUNCHER)
	{
	case 1: return WS(RLW);
	default: return 1;
	} 
}

weight "weapon_lightning"
{
	switch(INVENTORY_LIGHTNING)
	{
	case 1: return WS(LGW);
	default: return 1;
	}
} 

weight "weapon_railgun"
{
	switch(INVENTORY_RAILGUN)
	{
	case 1: return WS(RGW);
	default: return 1;
	} 
}

weight "weapon_plasmagun"
{
	switch(INVENTORY_PLASMAGUN)
	{
	case 1: return WS(PGW);
	default: return 1;
	} 
}

weight "weapon_bfg"
{
	switch(INVENTORY_BFG10K)
	{
	case 1: return WS(BFW);
	default: return 1;
	} 
}
 
weight "weapon_nailgun"
{
	switch(INVENTORY_NAILGUN)
	{
	case 1: return WS(NGW);
	default: return 1;
	} 
}

weight "weapon_prox_launcher"
{
	switch(INVENTORY_PROXLAUNCHER)
	{
	case 1: return WS(PXW);
	default: return 1;
	}
}

weight "weapon_chaingun"
{
	switch(INVENTORY_CHAINGUN)
	{
	case 1: return WS(CGW);
	default: return 1;
	} 
}


weight "ammo_shells"
{
	switch(INVENTORY_SHELLS)
	{
	case 20: return AS(30);
	case 40: return AS(30);
	case 60: return AS(11);
	case 80: return AS(11);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
} 

weight "ammo_bullets"
{
	switch(INVENTORY_BULLETS)
	{
	case 20: return AS(30);
	case 40: return AS(30);
	case 60: return AS(30);
	case 80: return AS(11);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
} 

weight "ammo_grenades"
{
	switch(INVENTORY_GRENADES)
	{
	case 20: return AS(30);
	case 40: return AS(30);
	case 60: return AS(11);
	case 80: return AS(11);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
} 

weight "ammo_cells"
{
	switch(INVENTORY_CELLS)
	{
	case 20: return AS(30);
	case 40: return AS(30);
	case 60: return AS(30);
	case 80: return AS(30);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
} 

weight "ammo_lightning"
{
	switch(INVENTORY_LIGHTNINGAMMO)
	{
	case 20: return AS(30);
	case 40: return AS(30);
	case 60: return AS(30);
	case 80: return AS(30);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
} 

weight "ammo_rockets"
{
	switch(INVENTORY_ROCKETS)
	{
	case 20: return AS(30);
	case 40: return AS(30);
	case 60: return AS(11);
	case 80: return AS(11);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
} 

weight "ammo_slugs"
{
	switch(INVENTORY_SLUGS)
	{
	case 20: return AS(30);
	case 40: return AS(30);
	case 60: return AS(11);
	case 80: return AS(11);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
} 

weight "ammo_bfg"
{
	switch(INVENTORY_BFGAMMO)
	{
	case 20: return AS(30);
	case 40: return AS(30);
	case 60: return AS(11);
	case 80: return AS(11);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	}
} 


weight "ammo_nails"
{
	switch(INVENTORY_NAILS)
	{
	case 20: return AS(30);
	case 40: return AS(30);
	case 60: return AS(11);
	case 80: return AS(11);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
}

weight "ammo_mines"
{
	switch(INVENTORY_MINES)
	{
	case 20: return AS(30);
	case 40: return AS(11);
	case 60: return AS(11);
	case 80: return AS(11);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
}

weight "ammo_belt"
{
	switch(INVENTORY_BELT)
	{
	case 20: return AS(30);
	case 40: return AS(11);
	case 60: return AS(11);
	case 80: return AS(11);
	case 100: return AS(11);
	case 120: return AS(11);
	case 140: return AS(11);
	case 160: return AS(11);
	case 180: return AS(11);
	case 200: return AS(11);
	default: return 0;
	} 
}


//needs work

weight "holdable_teleporter"
{
	switch(INVENTORY_TELEPORTER)
	{
	case 1: return PS(TELW);
	default: return 0;
	} 
} 

weight "holdable_medkit"
{
	switch(INVENTORY_TELEPORTER)
	{
	case 1: return PS(MEDW);
	default: return 0; 
	} 
} 

weight "holdable_kamikaze"
{
	switch(INVENTORY_TELEPORTER)
	{
	case 1: return PS(KAMW);
	default: return 0; 
	} 
} 

weight "holdable_portal"
{
	switch(INVENTORY_TELEPORTER)
	{
	case 1: return PS(PORW);
	default: return 0; 
	} 
} 

weight "holdable_invulnerability"
{
	switch(INVENTORY_TELEPORTER)
	{
	case 1: return PS(IBW);
	default: return 0; 
	} 
} 

weight "item_quad"
{
	return PS(QW);
} 

weight "item_enviro"
{
	return PS(ENVW);
} 

weight "item_haste"
{
	return PS(HAW);
} 

weight "item_invisibility"
{
	return PS(INW);
} 

weight "item_regen"
{
	return PS(REGW);
} 

weight "item_flight"
{
	return PS(FLW);
} 

weight "item_scout"
{
	switch(INVENTORY_SCOUT)
	{
	case 1: return PS(SCW);
	default: return 0; 
	}  
} 

weight "item_guard"
{
	switch(INVENTORY_SCOUT)
	{
	case 1: return PS(GUW);
	default: return 0; 
	} 
} 

weight "item_doubler"
{
	switch(INVENTORY_SCOUT)
	{
	case 1: return PS(DUBW);
	default: return 0; 
	}
} 

weight "item_ammoregen"
{
	switch(INVENTORY_SCOUT)
	{
	case 1: return PS(AMRW);
	default: return 0; 
	} 
} 


//=============================================
// This is only used to pickup dropped CTF
// flags now. The logic in here makes no
// sense since the bot has specific CTF AI.
// yet without it the bots barely move towards 
// the opposing flags due to low flag weights.
//=============================================

weight "team_CTF_redflag" 
{
	switch(INVENTORY_REDFLAG)
	{
	case 1: return FGW;
	default: return 300;
	} 
} 

weight "team_CTF_blueflag" 
{
	switch(INVENTORY_BLUEFLAG)
	{
	case 1: return FGW;
	default: return 300;
	} 
} 


weight "team_CTF_neutralflag" 
{
	return FGW;
} 

weight "item_redcube" 
{
	return REDCW;
} 

weight "item_bluecube" 
{
	return BLCW;
} 

weight "item_botroam"
{
	return 1;
} 

weight "team_dom_pointWhite"
{
	return 1000;
}

weight "team_dom_pointRed"
{
	return 700;
}

weight "team_dom_pointBlue"
{
	return 700;
}
