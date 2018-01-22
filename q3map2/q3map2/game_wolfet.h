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

   Support for Wolfenstein: Enemy Territory by ydnar@splashdamage.com

   ------------------------------------------------------------------------------- */



/* marker */
#ifndef GAME_WOLFET_H
#define GAME_WOLFET_H



/* -------------------------------------------------------------------------------

   content and surface flags

   ------------------------------------------------------------------------------- */

/* this file must be included *after* game_wolf.h because it shares defines! */

#define W_SURF_SPLASH               0x00000040  /* enemy territory water splash surface */
#define W_SURF_LANDMINE             0x80000000  /* enemy territory 'landminable' surface */



/* -------------------------------------------------------------------------------

   game_t struct

   ------------------------------------------------------------------------------- */

{
	"et",               /* -game x */
	"etmain",           /* default base game data dir */
	".etwolf",          /* unix home sub-dir */
	"et",               /* magic path word */
	"scripts",          /* shader directory */
	1024,               /* max lightmapped surface verts */
	1024,               /* max surface verts */
	6144,               /* max surface indexes */
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

		/* wolf et landmine-able surface */
		{ "landmine",       0,                          0,                          W_SURF_LANDMINE,            0,                          0,                          0 },

		/* materials */
		{ "metal",          0,                          0,                          W_SURF_METAL,               0,                          0,                          0 },
		{ "metalsteps",     0,                          0,                          W_SURF_METAL,               0,                          0,                          0 },
		{ "glass",          0,                          0,                          W_SURF_GLASS,               0,                          0,                          0 },
		{ "splash",         0,                          0,                          W_SURF_SPLASH,              0,                          0,                          0 },
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
