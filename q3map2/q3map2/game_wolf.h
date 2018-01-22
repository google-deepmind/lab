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
#ifndef GAME_WOLF_H
#define GAME_WOLF_H



/* -------------------------------------------------------------------------------

   content and surface flags

   ------------------------------------------------------------------------------- */

/* game flags */
#define W_CONT_SOLID                1           /* an eye is never valid in a solid */
#define W_CONT_LAVA                 8
#define W_CONT_SLIME                16
#define W_CONT_WATER                32
#define W_CONT_FOG                  64

#define W_CONT_MISSILECLIP          0x80        /* wolf ranged missile blocking */
#define W_CONT_ITEM                 0x100       /* wolf item contents */
#define W_CONT_AI_NOSIGHT           0x1000      /* wolf ai sight blocking */
#define W_CONT_CLIPSHOT             0x2000      /* wolf shot clip */
#define W_CONT_AREAPORTAL           0x8000

#define W_CONT_PLAYERCLIP           0x10000
#define W_CONT_MONSTERCLIP          0x20000
#define W_CONT_TELEPORTER           0x40000
#define W_CONT_JUMPPAD              0x80000
#define W_CONT_CLUSTERPORTAL        0x100000
#define W_CONT_DONOTENTER           0x200000
#define W_CONT_DONOTENTER_LARGE     0x400000    /* wolf dne */

#define W_CONT_ORIGIN               0x1000000   /* removed before bsping an entity */

#define W_CONT_BODY                 0x2000000   /* should never be on a brush, only in game */
#define W_CONT_CORPSE               0x4000000
#define W_CONT_DETAIL               0x8000000   /* brushes not used for the bsp */
#define W_CONT_STRUCTURAL           0x10000000  /* brushes used for the bsp */
#define W_CONT_TRANSLUCENT          0x20000000  /* don't consume surface fragments inside */
#define W_CONT_TRIGGER              0x40000000
#define W_CONT_NODROP               0x80000000  /* don't leave bodies or items (death fog, lava) */

#define W_SURF_NODAMAGE             0x1         /* never give falling damage */
#define W_SURF_SLICK                0x2         /* effects game physics */
#define W_SURF_SKY                  0x4         /* lighting from environment map */
#define W_SURF_LADDER               0x8
#define W_SURF_NOIMPACT             0x10        /* don't make missile explosions */
#define W_SURF_NOMARKS              0x20        /* don't leave missile marks */
#define W_SURF_CERAMIC              0x40        /* wolf ceramic material */
#define W_SURF_NODRAW               0x80        /* don't generate a drawsurface at all */
#define W_SURF_HINT                 0x100       /* make a primary bsp splitter */
#define W_SURF_SKIP                 0x200       /* completely ignore, allowing non-closed brushes */
#define W_SURF_NOLIGHTMAP           0x400       /* surface doesn't need a lightmap */
#define W_SURF_POINTLIGHT           0x800       /* generate lighting info at vertexes */
#define W_SURF_METAL                0x1000      /* wolf metal material */
#define W_SURF_NOSTEPS              0x2000      /* no footstep sounds */
#define W_SURF_NONSOLID             0x4000      /* don't collide against curves with this set */
#define W_SURF_LIGHTFILTER          0x8000      /* act as a light filter during q3map -light */
#define W_SURF_ALPHASHADOW          0x10000     /* do per-pixel light shadow casting in q3map */
#define W_SURF_NODLIGHT             0x20000     /* don't dlight even if solid (solid lava, skies) */
#define W_SURF_WOOD                 0x40000     /* wolf wood material */
#define W_SURF_GRASS                0x80000     /* wolf grass material */
#define W_SURF_GRAVEL               0x100000    /* wolf gravel material */
#define W_SURF_GLASS                0x200000    /* wolf glass material */
#define W_SURF_SNOW                 0x400000    /* wolf snow material */
#define W_SURF_ROOF                 0x800000    /* wolf roof material */
#define W_SURF_RUBBLE               0x1000000   /* wolf rubble material */
#define W_SURF_CARPET               0x2000000   /* wolf carpet material */

#define W_SURF_MONSTERSLICK         0x4000000   /* wolf npc slick surface */
#define W_SURF_MONSLICK_W           0x8000000   /* wolf slide bodies west */
#define W_SURF_MONSLICK_N           0x10000000  /* wolf slide bodies north */
#define W_SURF_MONSLICK_E           0x20000000  /* wolf slide bodies east */
#define W_SURF_MONSLICK_S           0x40000000  /* wolf slide bodies south */

/* ydnar flags */
#define W_SURF_VERTEXLIT            ( W_SURF_POINTLIGHT | W_SURF_NOLIGHTMAP )



/* -------------------------------------------------------------------------------

   game_t struct

   ------------------------------------------------------------------------------- */

{
	"wolf",             /* -game x */
	"main",             /* default base game data dir */
	".wolf",            /* unix home sub-dir */
	"wolf",             /* magic path word */
	"scripts",          /* shader directory */
	64,                 /* max lightmapped surface verts */
	999,                /* max surface verts */
	6000,               /* max surface indexes */
	qfalse,             /* flares */
	"flareshader",      /* default flare shader */
	qtrue,              /* wolf lighting model? */
	128,                /* lightmap width/height */
	1.0f,               /* lightmap gamma */
	1.0f,               /* lightmap compensate */
	"IBSP",             /* bsp file prefix */
	47,                 /* bsp file version */
	qfalse,             /* cod-style lump len/ofs order */
	LoadIBSPFile,       /* bsp load function */
	WriteIBSPFile,      /* bsp write function */

	{
		/* name				contentFlags				contentFlagsClear			surfaceFlags				surfaceFlagsClear			compileFlags				compileFlagsClear */

		/* default */
		{ "default",        W_CONT_SOLID,               -1,                         0,                          -1,                         C_SOLID,                    -1 },


		/* ydnar */
		{ "lightgrid",      0,                          0,                          0,                          0,                          C_LIGHTGRID,                0 },
		{ "antiportal",     0,                          0,                          0,                          0,                          C_ANTIPORTAL,               0 },
		{ "skip",           0,                          0,                          0,                          0,                          C_SKIP,                     0 },


		/* compiler */
		{ "origin",         W_CONT_ORIGIN,              W_CONT_SOLID,               0,                          0,                          C_ORIGIN | C_TRANSLUCENT,   C_SOLID },
		{ "areaportal",     W_CONT_AREAPORTAL,          W_CONT_SOLID,               0,                          0,                          C_AREAPORTAL | C_TRANSLUCENT,   C_SOLID },
		{ "trans",          W_CONT_TRANSLUCENT,         0,                          0,                          0,                          C_TRANSLUCENT,              0 },
		{ "detail",         W_CONT_DETAIL,              0,                          0,                          0,                          C_DETAIL,                   0 },
		{ "structural",     W_CONT_STRUCTURAL,          0,                          0,                          0,                          C_STRUCTURAL,               0 },
		{ "hint",           0,                          0,                          W_SURF_HINT,                0,                          C_HINT,                     0 },
		{ "nodraw",         0,                          0,                          W_SURF_NODRAW,              0,                          C_NODRAW,                   0 },

		{ "alphashadow",    0,                          0,                          W_SURF_ALPHASHADOW,         0,                          C_ALPHASHADOW | C_TRANSLUCENT,  0 },
		{ "lightfilter",    0,                          0,                          W_SURF_LIGHTFILTER,         0,                          C_LIGHTFILTER | C_TRANSLUCENT,  0 },
		{ "nolightmap",     0,                          0,                          W_SURF_VERTEXLIT,           0,                          C_VERTEXLIT,                0 },
		{ "pointlight",     0,                          0,                          W_SURF_VERTEXLIT,           0,                          C_VERTEXLIT,                0 },


		/* game */
		{ "nonsolid",       0,                          W_CONT_SOLID,               W_SURF_NONSOLID,            0,                          0,                          C_SOLID },

		{ "trigger",        W_CONT_TRIGGER,             W_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		{ "water",          W_CONT_WATER,               W_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },
		{ "slag",           W_CONT_SLIME,               W_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },
		{ "lava",           W_CONT_LAVA,                W_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },

		{ "playerclip",     W_CONT_PLAYERCLIP,          W_CONT_SOLID,               0,                          0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "monsterclip",    W_CONT_MONSTERCLIP,         W_CONT_SOLID,               0,                          0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "clipmissile",    W_CONT_MISSILECLIP,         W_CONT_SOLID,               0,                          0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "clipshot",       W_CONT_CLIPSHOT,            W_CONT_SOLID,               0,                          0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "nodrop",         W_CONT_NODROP,              W_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		{ "clusterportal",  W_CONT_CLUSTERPORTAL,       W_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },
		{ "donotenter",     W_CONT_DONOTENTER,          W_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },
		{ "nonotenterlarge",W_CONT_DONOTENTER_LARGE,    W_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },
		{ "donotenterlarge",W_CONT_DONOTENTER_LARGE,    W_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		{ "fog",            W_CONT_FOG,                 W_CONT_SOLID,               0,                          0,                          C_FOG,                      C_SOLID },
		{ "sky",            0,                          0,                          W_SURF_SKY,                 0,                          C_SKY,                      0 },

		{ "slick",          0,                          0,                          W_SURF_SLICK,               0,                          0,                          0 },

		{ "noimpact",       0,                          0,                          W_SURF_NOIMPACT,            0,                          0,                          0 },
		{ "nomarks",        0,                          0,                          W_SURF_NOMARKS,             0,                          C_NOMARKS,                  0 },
		{ "ladder",         0,                          0,                          W_SURF_LADDER,              0,                          0,                          0 },
		{ "nodamage",       0,                          0,                          W_SURF_NODAMAGE,            0,                          0,                          0 },
		{ "nosteps",        0,                          0,                          W_SURF_NOSTEPS,             0,                          0,                          0 },
		{ "nodlight",       0,                          0,                          W_SURF_NODLIGHT,            0,                          0,                          0 },


		/* materials */
		{ "metal",          0,                          0,                          W_SURF_METAL,               0,                          0,                          0 },
		{ "metalsteps",     0,                          0,                          W_SURF_METAL,               0,                          0,                          0 },
		{ "glass",          0,                          0,                          W_SURF_GLASS,               0,                          0,                          0 },
		{ "ceramic",        0,                          0,                          W_SURF_CERAMIC,             0,                          0,                          0 },
		{ "woodsteps",      0,                          0,                          W_SURF_WOOD,                0,                          0,                          0 },
		{ "grasssteps",     0,                          0,                          W_SURF_GRASS,               0,                          0,                          0 },
		{ "gravelsteps",    0,                          0,                          W_SURF_GRAVEL,              0,                          0,                          0 },
		{ "rubble",         0,                          0,                          W_SURF_RUBBLE,              0,                          0,                          0 },
		{ "carpetsteps",    0,                          0,                          W_SURF_CARPET,              0,                          0,                          0 },
		{ "snowsteps",      0,                          0,                          W_SURF_SNOW,                0,                          0,                          0 },
		{ "roofsteps",      0,                          0,                          W_SURF_ROOF,                0,                          0,                          0 },


		/* ai */
		{ "ai_nosight",     W_CONT_AI_NOSIGHT,          W_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },

		/* ydnar: experimental until bits are confirmed! */
		{ "ai_nopass",      W_CONT_DONOTENTER,          W_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },
		{ "ai_nopasslarge", W_CONT_DONOTENTER_LARGE,    W_CONT_SOLID,               0,                          0,                          C_TRANSLUCENT,              C_SOLID },


		/* sliding bodies */
		{ "monsterslick",   0,                          0,                          W_SURF_MONSTERSLICK,        0,                          C_TRANSLUCENT,              0 },
		{ "monsterslicknorth",  0,                      0,                          W_SURF_MONSLICK_N,          0,                          C_TRANSLUCENT,              0 },
		{ "monsterslickeast",   0,                      0,                          W_SURF_MONSLICK_E,          0,                          C_TRANSLUCENT,              0 },
		{ "monsterslicksouth",  0,                      0,                          W_SURF_MONSLICK_S,          0,                          C_TRANSLUCENT,              0 },
		{ "monsterslickwest",   0,                      0,                          W_SURF_MONSLICK_W,          0,                          C_TRANSLUCENT,              0 },


		/* null */
		{ NULL, 0, 0, 0, 0, 0, 0 }
	}
}



/* end marker */
#endif
