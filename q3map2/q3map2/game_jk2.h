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
#ifndef GAME_JK2_H
#define GAME_JK2_H



/* -------------------------------------------------------------------------------

   content and surface flags

   ------------------------------------------------------------------------------- */

/* this file must be included *after* game_sof2.h because it shares defines! */



/* -------------------------------------------------------------------------------

   game_t struct

   ------------------------------------------------------------------------------- */

{
	"jk2",              /* -game x */
	"base",             /* default base game data dir */
	".jk2",             /* unix home sub-dir */
	"GameData",         /* magic path word */
	"shaders",          /* shader directory */
	64,                 /* max lightmapped surface verts */
	999,                /* max surface verts */
	6000,               /* max surface indexes */
	qtrue,              /* flares */
	"gfx/misc/flare",   /* default flare shader */
	qfalse,             /* wolf lighting model? */
	128,                /* lightmap width/height */
	1.0f,               /* lightmap gamma */
	1.0f,               /* lightmap compensate */
	"RBSP",             /* bsp file prefix */
	1,                  /* bsp file version */
	qfalse,             /* cod-style lump len/ofs order */
	LoadRBSPFile,       /* bsp load function */
	WriteRBSPFile,      /* bsp write function */

	{
		/* name				contentFlags				contentFlagsClear			surfaceFlags				surfaceFlagsClear			compileFlags				compileFlagsClear */

		/* default */
		{ "default",        S_CONT_SOLID | S_CONT_OPAQUE,   -1,                     0,                          -1,                         C_SOLID,                    -1 },


		/* ydnar */
		{ "lightgrid",      0,                          0,                          0,                          0,                          C_LIGHTGRID,                0 },
		{ "antiportal",     0,                          0,                          0,                          0,                          C_ANTIPORTAL,               0 },
		{ "skip",           0,                          0,                          0,                          0,                          C_SKIP,                     0 },


		/* compiler */
		{ "origin",         0,                          S_CONT_SOLID,               0,                          0,                          C_ORIGIN | C_TRANSLUCENT,   C_SOLID },
		{ "areaportal",     S_CONT_TRANSLUCENT,         S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_AREAPORTAL | C_TRANSLUCENT,   C_SOLID },
		{ "trans",          S_CONT_TRANSLUCENT,         0,                          0,                          0,                          C_TRANSLUCENT,              0 },
		{ "detail",         S_CONT_DETAIL,              0,                          0,                          0,                          C_DETAIL,                   0 },
		{ "structural",     0,                          0,                          0,                          0,                          C_STRUCTURAL,               0 },
		{ "hint",           0,                          0,                          0,                          0,                          C_HINT,                     0 },
		{ "nodraw",         0,                          0,                          S_SURF_NODRAW,              0,                          C_NODRAW,                   0 },

		{ "alphashadow",    0,                          0,                          0,                          0,                          C_ALPHASHADOW | C_TRANSLUCENT,  0 },
		{ "lightfilter",    0,                          0,                          0,                          0,                          C_LIGHTFILTER | C_TRANSLUCENT,  0 },
		{ "nolightmap",     0,                          0,                          0,                          0,                          C_VERTEXLIT,                0 },
		{ "pointlight",     0,                          0,                          0,                          0,                          C_VERTEXLIT,                0 },


		/* game */
		{ "nonsolid",       0,                          S_CONT_SOLID,               0,                          0,                          0,                          C_SOLID },
		{ "nonopaque",      0,                          S_CONT_OPAQUE,              0,                          0,                          C_TRANSLUCENT,              0 },        /* setting trans ok? */

		{ "trigger",        S_CONT_TRIGGER,             S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_TRANSLUCENT,              C_SOLID },

		{ "water",          S_CONT_WATER,               S_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },
		{ "slime",          S_CONT_SLIME,               S_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },
		{ "lava",           S_CONT_LAVA,                S_CONT_SOLID,               0,                          0,                          C_LIQUID | C_TRANSLUCENT,   C_SOLID },

		{ "shotclip",       S_CONT_SHOTCLIP,            S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },  /* setting trans/detail ok? */
		{ "playerclip",     S_CONT_PLAYERCLIP,          S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "monsterclip",    S_CONT_MONSTERCLIP,         S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "nodrop",         S_CONT_NODROP,              S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },

		{ "terrain",        S_CONT_TERRAIN,             S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "ladder",         S_CONT_LADDER,              S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "abseil",         S_CONT_ABSEIL,              S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },
		{ "outside",        S_CONT_OUTSIDE,             S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },

		{ "botclip",        S_CONT_BOTCLIP,             S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_DETAIL | C_TRANSLUCENT,   C_SOLID },

		{ "fog",            S_CONT_FOG,                 S_CONT_SOLID | S_CONT_OPAQUE,   0,                      0,                          C_FOG | C_DETAIL | C_TRANSLUCENT,   C_SOLID },  /* nonopaque? */
		{ "sky",            0,                          0,                          S_SURF_SKY,                 0,                          C_SKY,                      0 },

		{ "slick",          0,                          0,                          S_SURF_SLICK,               0,                          0,                          0 },

		{ "noimpact",       0,                          0,                          S_SURF_NOIMPACT,            0,                          0,                          0 },
		{ "nomarks",        0,                          0,                          S_SURF_NOMARKS,             0,                          C_NOMARKS,                  0 },
		{ "nodamage",       0,                          0,                          S_SURF_NODAMAGE,            0,                          0,                          0 },
		{ "metalsteps",     0,                          0,                          S_SURF_METALSTEPS,          0,                          0,                          0 },
		{ "nosteps",        0,                          0,                          S_SURF_NOSTEPS,             0,                          0,                          0 },
		{ "nodlight",       0,                          0,                          S_SURF_NODLIGHT,            0,                          0,                          0 },
		{ "nomiscents",     0,                          0,                          S_SURF_NOMISCENTS,          0,                          0,                          0 },
		{ "forcefield",     0,                          0,                          S_SURF_FORCEFIELD,          0,                          0,                          0 },


		/* materials */
		{ "*mat_none",      0,                          0,                          S_MAT_NONE,                 S_MAT_MASK,                 0,                          0 },
		{ "*mat_solidwood", 0,                          0,                          S_MAT_SOLIDWOOD,            S_MAT_MASK,                 0,                          0 },
		{ "*mat_hollowwood",    0,                      0,                          S_MAT_HOLLOWWOOD,           S_MAT_MASK,                 0,                          0 },
		{ "*mat_solidmetal",    0,                      0,                          S_MAT_SOLIDMETAL,           S_MAT_MASK,                 0,                          0 },
		{ "*mat_hollowmetal",   0,                      0,                          S_MAT_HOLLOWMETAL,          S_MAT_MASK,                 0,                          0 },
		{ "*mat_shortgrass",    0,                      0,                          S_MAT_SHORTGRASS,           S_MAT_MASK,                 0,                          0 },
		{ "*mat_longgrass",     0,                      0,                          S_MAT_LONGGRASS,            S_MAT_MASK,                 0,                          0 },
		{ "*mat_dirt",      0,                          0,                          S_MAT_DIRT,                 S_MAT_MASK,                 0,                          0 },
		{ "*mat_sand",      0,                          0,                          S_MAT_SAND,                 S_MAT_MASK,                 0,                          0 },
		{ "*mat_gravel",    0,                          0,                          S_MAT_GRAVEL,               S_MAT_MASK,                 0,                          0 },
		{ "*mat_glass",     0,                          0,                          S_MAT_GLASS,                S_MAT_MASK,                 0,                          0 },
		{ "*mat_concrete",  0,                          0,                          S_MAT_CONCRETE,             S_MAT_MASK,                 0,                          0 },
		{ "*mat_marble",    0,                          0,                          S_MAT_MARBLE,               S_MAT_MASK,                 0,                          0 },
		{ "*mat_water",     0,                          0,                          S_MAT_WATER,                S_MAT_MASK,                 0,                          0 },
		{ "*mat_snow",      0,                          0,                          S_MAT_SNOW,                 S_MAT_MASK,                 0,                          0 },
		{ "*mat_ice",       0,                          0,                          S_MAT_ICE,                  S_MAT_MASK,                 0,                          0 },
		{ "*mat_flesh",     0,                          0,                          S_MAT_FLESH,                S_MAT_MASK,                 0,                          0 },
		{ "*mat_mud",       0,                          0,                          S_MAT_MUD,                  S_MAT_MASK,                 0,                          0 },
		{ "*mat_bpglass",   0,                          0,                          S_MAT_BPGLASS,              S_MAT_MASK,                 0,                          0 },
		{ "*mat_dryleaves", 0,                          0,                          S_MAT_DRYLEAVES,            S_MAT_MASK,                 0,                          0 },
		{ "*mat_greenleaves",   0,                      0,                          S_MAT_GREENLEAVES,          S_MAT_MASK,                 0,                          0 },
		{ "*mat_fabric",    0,                          0,                          S_MAT_FABRIC,               S_MAT_MASK,                 0,                          0 },
		{ "*mat_canvas",    0,                          0,                          S_MAT_CANVAS,               S_MAT_MASK,                 0,                          0 },
		{ "*mat_rock",      0,                          0,                          S_MAT_ROCK,                 S_MAT_MASK,                 0,                          0 },
		{ "*mat_rubber",    0,                          0,                          S_MAT_RUBBER,               S_MAT_MASK,                 0,                          0 },
		{ "*mat_plastic",   0,                          0,                          S_MAT_PLASTIC,              S_MAT_MASK,                 0,                          0 },
		{ "*mat_tiles",     0,                          0,                          S_MAT_TILES,                S_MAT_MASK,                 0,                          0 },
		{ "*mat_carpet",    0,                          0,                          S_MAT_CARPET,               S_MAT_MASK,                 0,                          0 },
		{ "*mat_plaster",   0,                          0,                          S_MAT_PLASTER,              S_MAT_MASK,                 0,                          0 },
		{ "*mat_shatterglass",  0,                      0,                          S_MAT_SHATTERGLASS,         S_MAT_MASK,                 0,                          0 },
		{ "*mat_armor",     0,                          0,                          S_MAT_ARMOR,                S_MAT_MASK,                 0,                          0 },
		{ "*mat_computer",  0,                          0,                          S_MAT_COMPUTER,             S_MAT_MASK,                 0,                          0 },


		/* null */
		{ NULL, 0, 0, 0, 0, 0, 0 }
	}
}



/* end marker */
#endif
