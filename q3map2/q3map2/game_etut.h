/* -------------------------------------------------------------------------------

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

   ----------------------------------------------------------------------------------

   This code has been altered significantly from its original form, to support
   several games based on the Quake III Arena engine, in the form of "Q3Map2."

   ------------------------------------------------------------------------------- */



/* marker */
#ifndef GAME_ETUT_H
#define GAME_ETUT_H



/* -------------------------------------------------------------------------------

   content and surface flags

   ------------------------------------------------------------------------------- */

/* game flags */
#define U_CONT_SOLID                1           /* an eye is never valid in a solid */
#define U_CONT_LAVA                 8
#define U_CONT_SLIME                16
#define U_CONT_WATER                32
#define U_CONT_FOG                  64

#define U_CONT_AREAPORTAL           0x8000

#define U_CONT_PLAYERCLIP           0x10000
#define U_CONT_MONSTERCLIP          0x20000
#define U_CONT_TELEPORTER           0x40000
#define U_CONT_JUMPPAD              0x80000
#define U_CONT_CLUSTERPORTAL        0x100000
#define U_CONT_DONOTENTER           0x200000
#define U_CONT_BOTCLIP              0x400000

#define U_CONT_ORIGIN               0x1000000   /* removed before bsping an entity */

#define U_CONT_BODY                 0x2000000   /* should never be on a brush, only in game */
#define U_CONT_CORPSE               0x4000000
#define U_CONT_DETAIL               0x8000000   /* brushes not used for the bsp */
#define U_CONT_STRUCTURAL           0x10000000  /* brushes used for the bsp */
#define U_CONT_TRANSLUCENT          0x20000000  /* don't consume surface fragments inside */
#define U_CONT_TRIGGER              0x40000000
#define U_CONT_NODROP               0x80000000  /* don't leave bodies or items (death fog, lava) */

#define U_SURF_NODAMAGE             0x1         /* never give falling damage */
#define U_SURF_SLICK                0x2         /* effects game physics */
#define U_SURF_SKY                  0x4         /* lighting from environment map */
#define U_SURF_LADDER               0x8
#define U_SURF_NOIMPACT             0x10        /* don't make missile explosions */
#define U_SURF_NOMARKS              0x20        /* don't leave missile marks */
#define U_SURF_FLESH                0x40        /* make flesh sounds and effects */
#define U_SURF_NODRAW               0x80        /* don't generate a drawsurface at all */
#define U_SURF_HINT                 0x100       /* make a primary bsp splitter */
#define U_SURF_SKIP                 0x200       /* completely ignore, allowing non-closed brushes */
#define U_SURF_NOLIGHTMAP           0x400       /* surface doesn't need a lightmap */
#define U_SURF_POINTLIGHT           0x800       /* generate lighting info at vertexes */
#define U_SURF_METALSTEPS           0x1000      /* clanking footsteps */
#define U_SURF_NOSTEPS              0x2000      /* no footstep sounds */
#define U_SURF_NONSOLID             0x4000      /* don't collide against curves with this set */
#define U_SURF_LIGHTFILTER          0x8000      /* act as a light filter during q3map -light */
#define U_SURF_ALPHASHADOW          0x10000     /* do per-pixel light shadow casting in q3map */
#define U_SURF_NODLIGHT             0x20000     /* don't dlight even if solid (solid lava, skies) */
#define U_SURF_DUST                 0x40000     /* leave a dust trail when walking on this surface */

/* ydnar flags */
#define U_SURF_VERTEXLIT            ( U_SURF_POINTLIGHT | U_SURF_NOLIGHTMAP )

/* materials */
#define U_MAT_MASK                  0xFFF00000  /* mask to get the material type */

#define U_MAT_NONE                  0x00000000
#define U_MAT_TIN                   0x00100000
#define U_MAT_ALUMINUM              0x00200000
#define U_MAT_IRON                  0x00300000
#define U_MAT_TITANIUM              0x00400000
#define U_MAT_STEEL                 0x00500000
#define U_MAT_BRASS                 0x00600000
#define U_MAT_COPPER                0x00700000
#define U_MAT_CEMENT                0x00800000
#define U_MAT_ROCK                  0x00900000
#define U_MAT_GRAVEL                0x00A00000
#define U_MAT_PAVEMENT              0x00B00000
#define U_MAT_BRICK                 0x00C00000
#define U_MAT_CLAY                  0x00D00000
#define U_MAT_GRASS                 0x00E00000
#define U_MAT_DIRT                  0x00F00000
#define U_MAT_MUD                   0x01000000
#define U_MAT_SNOW                  0x01100000
#define U_MAT_ICE                   0x01200000
#define U_MAT_SAND                  0x01300000
#define U_MAT_CERAMICTILE           0x01400000
#define U_MAT_LINOLEUM              0x01500000
#define U_MAT_RUG                   0x01600000
#define U_MAT_PLASTER               0x01700000
#define U_MAT_PLASTIC               0x01800000
#define U_MAT_CARDBOARD             0x01900000
#define U_MAT_HARDWOOD              0x01A00000
#define U_MAT_SOFTWOOD              0x01B00000
#define U_MAT_PLANK                 0x01C00000
#define U_MAT_GLASS                 0x01D00000
#define U_MAT_WATER                 0x01E00000
#define U_MAT_STUCCO                0x01F00000



/* -------------------------------------------------------------------------------

   game_t struct

   ------------------------------------------------------------------------------- */

{
	"etut",             /* -game x */
	"etut",             /* default base game data dir */
	".etwolf",          /* unix home sub-dir */
	"et",               /* magic path word */
	"scripts",          /* shader directory */
	1024,               /* max lightmapped surface verts */
	1024,               /* max surface verts */
	6144,               /* max surface indexes */
	qfalse,             /* flares */
	"flareshader",      /* default flare shader */
	qfalse,             /* wolf lighting model? */
	128,                /* lightmap width/height */
	2.2f,               /* lightmap gamma */
	1.0f,               /* lightmap compensate */
	"IBSP",             /* bsp file prefix */
	47,                 /* bsp file version */
	qfalse,             /* cod-style lump len/ofs order */
	LoadIBSPFile,       /* bsp load function */
	WriteIBSPFile,      /* bsp write function */

	{
		/* name				contentFlags				contentFlagsClear			surfaceFlags				surfaceFlagsClear			compileFlags				compileFlagsClear */

		/* default */
		{ "default",        U_CONT_SOLID,               -1,                         0,                          -1,                         C_SOLID,                    -1 },


		/* ydnar */
		{ "lightgrid",      0,                          0,                          0,                          0,                          C_LIGHTGRID,                0 },
		{ "antiportal",     0,                          0,                          0,                          0,                          C_ANTIPORTAL,               0 },
		{ "skip",           0,                          0,                          0,                          0,                          C_SKIP,                     0 },


		/* compiler */
		{ "origin",         U_CONT_ORIGIN,              U_CONT_SOLID,               0,                          0,                          C_ORIGIN | C_TRANSLUCENT,   C_SOLID },
		{ "areaportal",     U_CONT_AREAPORTAL,          U_CONT_SOLID,               0,                          0,                          C_AREAPORTAL | C_TRANSLUCENT,   C_SOLID },
		{ "trans",          U_CONT_TRANSLUCENT,         0,                          0,                          0,                          C_TRANSLUCENT,              0 },
		{ "detail",         U_CONT_DETAIL,              0,                          0,                          0,                          C_DETAIL,                   0 },
		{ "structural",     U_CONT_STRUCTURAL,          0,                          0,                          0,                          C_STRUCTURAL,               0 },
		{ "hint",           0,                          0,                          U_SURF_HINT,                0,                          C_HINT,                     0 },
		{ "nodraw",         0,                          0,                          U_SURF_NODRAW,              0,                          C_NODRAW,                   0 },

		{ "alphashadow",    0,                          0,                          U_SURF_ALPHASHADOW,         0,                          C_ALPHASHADOW | C_TRANSLUCENT,  0 },
		{ "lightfilter",    0,                          0,                          U_SURF_LIGHTFILTER,         0,                          C_LIGHTFILTER | C_TRANSLUCENT,  0 },
		{ "nolightmap",     0,                          0,                          U_SURF_VERTEXLIT,           0,                          C_VERTEXLIT,                0 },
		{ "pointlight",     0,                          0,                          U_SURF_VERTEXLIT,           0,                          C_VERTEXLIT,                0 },


		/* game */
		{ "nonsolid",       0,                          U_CONT_SOLID,               U_SURF_NONSOLID,            0,                          0,                          C_SOLID },

		{ "trigger",        U_CONT_TRIGGER,             U_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		{ "water",          U_CONT_WATER,               U_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },
		{ "slime",          U_CONT_SLIME,               U_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },
		{ "lava",           U_CONT_LAVA,                U_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },

		{ "playerclip",     U_CONT_PLAYERCLIP,          U_CONT_SOLID,               0,                          0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "monsterclip",    U_CONT_MONSTERCLIP,         U_CONT_SOLID,               0,                          0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "nodrop",         U_CONT_NODROP,              U_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		{ "clusterportal",  U_CONT_CLUSTERPORTAL,       U_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },
		{ "donotenter",     U_CONT_DONOTENTER,          U_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },
		{ "botclip",        U_CONT_BOTCLIP,             U_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		{ "fog",            U_CONT_FOG,                 U_CONT_SOLID,               0,                          0,                          C_FOG,                      C_SOLID },
		{ "sky",            0,                          0,                          U_SURF_SKY,                 0,                          C_SKY,                      0 },

		{ "slick",          0,                          0,                          U_SURF_SLICK,               0,                          0,                          0 },

		{ "noimpact",       0,                          0,                          U_SURF_NOIMPACT,            0,                          0,                          0 },
		{ "nomarks",        0,                          0,                          U_SURF_NOMARKS,             0,                          C_NOMARKS,                  0 },
		{ "ladder",         0,                          0,                          U_SURF_LADDER,              0,                          0,                          0 },
		{ "nodamage",       0,                          0,                          U_SURF_NODAMAGE,            0,                          0,                          0 },
		{ "metalsteps",     0,                          0,                          U_SURF_METALSTEPS,          0,                          0,                          0 },
		{ "flesh",          0,                          0,                          U_SURF_FLESH,               0,                          0,                          0 },
		{ "nosteps",        0,                          0,                          U_SURF_NOSTEPS,             0,                          0,                          0 },
		{ "nodlight",       0,                          0,                          U_SURF_NODLIGHT,            0,                          0,                          0 },
		{ "dust",           0,                          0,                          U_SURF_DUST,                0,                          0,                          0 },


		/* materials */
		{ "*mat_none",      0,                          0,                          U_MAT_NONE,                 U_MAT_MASK,                 0,                          0 },
		{ "*mat_tin",       0,                          0,                          U_MAT_TIN,                  U_MAT_MASK,                 0,                          0 },
		{ "*mat_aluminum",  0,                          0,                          U_MAT_ALUMINUM,             U_MAT_MASK,                 0,                          0 },
		{ "*mat_iron",      0,                          0,                          U_MAT_IRON,                 U_MAT_MASK,                 0,                          0 },
		{ "*mat_titanium",  0,                          0,                          U_MAT_TITANIUM,             U_MAT_MASK,                 0,                          0 },
		{ "*mat_steel",     0,                          0,                          U_MAT_STEEL,                U_MAT_MASK,                 0,                          0 },
		{ "*mat_brass",     0,                          0,                          U_MAT_BRASS,                U_MAT_MASK,                 0,                          0 },
		{ "*mat_copper",    0,                          0,                          U_MAT_COPPER,               U_MAT_MASK,                 0,                          0 },
		{ "*mat_cement",    0,                          0,                          U_MAT_CEMENT,               U_MAT_MASK,                 0,                          0 },
		{ "*mat_rock",      0,                          0,                          U_MAT_ROCK,                 U_MAT_MASK,                 0,                          0 },
		{ "*mat_gravel",    0,                          0,                          U_MAT_GRAVEL,               U_MAT_MASK,                 0,                          0 },
		{ "*mat_pavement",  0,                          0,                          U_MAT_PAVEMENT,             U_MAT_MASK,                 0,                          0 },
		{ "*mat_brick",     0,                          0,                          U_MAT_BRICK,                U_MAT_MASK,                 0,                          0 },
		{ "*mat_clay",      0,                          0,                          U_MAT_CLAY,                 U_MAT_MASK,                 0,                          0 },
		{ "*mat_grass",     0,                          0,                          U_MAT_GRASS,                U_MAT_MASK,                 0,                          0 },
		{ "*mat_dirt",      0,                          0,                          U_MAT_DIRT,                 U_MAT_MASK,                 0,                          0 },
		{ "*mat_mud",       0,                          0,                          U_MAT_MUD,                  U_MAT_MASK,                 0,                          0 },
		{ "*mat_snow",      0,                          0,                          U_MAT_SNOW,                 U_MAT_MASK,                 0,                          0 },
		{ "*mat_ice",       0,                          0,                          U_MAT_ICE,                  U_MAT_MASK,                 0,                          0 },
		{ "*mat_sand",      0,                          0,                          U_MAT_SAND,                 U_MAT_MASK,                 0,                          0 },
		{ "*mat_ceramic",   0,                          0,                          U_MAT_CERAMICTILE,          U_MAT_MASK,                 0,                          0 },
		{ "*mat_ceramictile",   0,                      0,                          U_MAT_CERAMICTILE,          U_MAT_MASK,                 0,                          0 },
		{ "*mat_linoleum",  0,                          0,                          U_MAT_LINOLEUM,             U_MAT_MASK,                 0,                          0 },
		{ "*mat_rug",       0,                          0,                          U_MAT_RUG,                  U_MAT_MASK,                 0,                          0 },
		{ "*mat_plaster",   0,                          0,                          U_MAT_PLASTER,              U_MAT_MASK,                 0,                          0 },
		{ "*mat_plastic",   0,                          0,                          U_MAT_PLASTIC,              U_MAT_MASK,                 0,                          0 },
		{ "*mat_cardboard", 0,                          0,                          U_MAT_CARDBOARD,            U_MAT_MASK,                 0,                          0 },
		{ "*mat_hardwood",  0,                          0,                          U_MAT_HARDWOOD,             U_MAT_MASK,                 0,                          0 },
		{ "*mat_softwood",  0,                          0,                          U_MAT_SOFTWOOD,             U_MAT_MASK,                 0,                          0 },
		{ "*mat_plank",     0,                          0,                          U_MAT_PLANK,                U_MAT_MASK,                 0,                          0 },
		{ "*mat_glass",     0,                          0,                          U_MAT_GLASS,                U_MAT_MASK,                 0,                          0 },
		{ "*mat_water",     0,                          0,                          U_MAT_WATER,                U_MAT_MASK,                 0,                          0 },
		{ "*mat_stucco",    0,                          0,                          U_MAT_STUCCO,               U_MAT_MASK,                 0,                          0 },


		/* null */
		{ NULL, 0, 0, 0, 0, 0, 0 }
	}
}



/* end marker */
#endif
