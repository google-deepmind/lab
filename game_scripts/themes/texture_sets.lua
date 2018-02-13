--[[ Copyright (C) 2018 Google Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
]]

local decals = require 'themes.decals'

local texture_sets = {}

texture_sets.MISHMASH = {
    floor = {
        {tex = 'map/lab_games/lg_style_01_floor_orange'},
        {tex = 'map/lab_games/lg_style_01_floor_orange_bright'},
        {tex = 'map/lab_games/lg_style_01_floor_blue'},
        {tex = 'map/lab_games/lg_style_01_floor_blue_bright'},
        {tex = 'map/lab_games/lg_style_02_floor_blue'},
        {tex = 'map/lab_games/lg_style_02_floor_blue_bright'},
        {tex = 'map/lab_games/lg_style_02_floor_green'},
        {tex = 'map/lab_games/lg_style_02_floor_green_bright'},
        {tex = 'map/lab_games/lg_style_03_floor_green'},
        {tex = 'map/lab_games/lg_style_03_floor_green_bright'},
        {tex = 'map/lab_games/lg_style_03_floor_blue'},
        {tex = 'map/lab_games/lg_style_03_floor_blue_bright'},
        {tex = 'map/lab_games/lg_style_04_floor_blue'},
        {tex = 'map/lab_games/lg_style_04_floor_blue_bright'},
        {tex = 'map/lab_games/lg_style_04_floor_orange'},
        {tex = 'map/lab_games/lg_style_04_floor_orange_bright'},
        {tex = 'map/lab_games/lg_style_05_floor_blue'},
        {tex = 'map/lab_games/lg_style_05_floor_blue_bright'},
        {tex = 'map/lab_games/lg_style_05_floor_orange'},
        {tex = 'map/lab_games/lg_style_05_floor_orange_bright'},
    },
    ceiling = {{tex = 'map/lab_games/fake_sky'}},
    wall = {
        {tex = 'map/lab_games/lg_style_01_wall_green'},
        {tex = 'map/lab_games/lg_style_01_wall_green_bright'},
        {tex = 'map/lab_games/lg_style_01_wall_red'},
        {tex = 'map/lab_games/lg_style_01_wall_red_bright'},
        {tex = 'map/lab_games/lg_style_02_wall_yellow'},
        {tex = 'map/lab_games/lg_style_02_wall_yellow_bright'},
        {tex = 'map/lab_games/lg_style_02_wall_blue'},
        {tex = 'map/lab_games/lg_style_02_wall_blue_bright'},
        {tex = 'map/lab_games/lg_style_03_wall_orange'},
        {tex = 'map/lab_games/lg_style_03_wall_orange_bright'},
        {tex = 'map/lab_games/lg_style_03_wall_gray'},
        {tex = 'map/lab_games/lg_style_03_wall_gray_bright'},
        {tex = 'map/lab_games/lg_style_04_wall_green'},
        {tex = 'map/lab_games/lg_style_04_wall_green_bright'},
        {tex = 'map/lab_games/lg_style_04_wall_red'},
        {tex = 'map/lab_games/lg_style_04_wall_red_bright'},
        {tex = 'map/lab_games/lg_style_05_wall_red'},
        {tex = 'map/lab_games/lg_style_05_wall_red_bright'},
        {tex = 'map/lab_games/lg_style_05_wall_yellow'},
        {tex = 'map/lab_games/lg_style_05_wall_yellow_bright'}
    },
    wallDecals = decals.decals,
}

texture_sets.TRON = {
    floor = {
        {tex = 'map/lab_games/lg_style_01_floor_orange'},
        {tex = 'map/lab_games/lg_style_01_floor_orange_bright'},
        {tex = 'map/lab_games/lg_style_01_floor_blue'},
        {tex = 'map/lab_games/lg_style_01_floor_blue_bright'},
    },
    ceiling = {{tex = 'map/lab_games/fake_sky'}},
    wall = {
        {tex = 'map/lab_games/lg_style_01_wall_green'},
        {tex = 'map/lab_games/lg_style_01_wall_green_bright'},
        {tex = 'map/lab_games/lg_style_01_wall_red'},
        {tex = 'map/lab_games/lg_style_01_wall_red_bright'},
    },
    wallDecals = decals.decals,
}

texture_sets.MINESWEEPER = {
    floor = {
        {tex = 'map/lab_games/lg_style_04_floor_blue'},
        {tex = 'map/lab_games/lg_style_04_floor_blue_bright'},
        {tex = 'map/lab_games/lg_style_04_floor_orange'},
        {tex = 'map/lab_games/lg_style_04_floor_orange_bright'},
    },
    ceiling = {{tex = 'map/lab_games/fake_sky'}},
    wall = {
        {tex = 'map/lab_games/lg_style_04_wall_green'},
        {tex = 'map/lab_games/lg_style_04_wall_green_bright'},
        {tex = 'map/lab_games/lg_style_04_wall_red'},
        {tex = 'map/lab_games/lg_style_04_wall_red_bright'},
    },
    wallDecals = decals.decals,
    floorModels = {
        {mod = 'models/fut_obj_barbell_01.md3'},
        {mod = 'models/fut_obj_cylinder_01.md3'},
    },
}


texture_sets.TETRIS = {
    floor = {
        {tex = 'map/lab_games/lg_style_02_floor_blue'},
        {tex = 'map/lab_games/lg_style_02_floor_blue_bright'},
        {tex = 'map/lab_games/lg_style_02_floor_green'},
        {tex = 'map/lab_games/lg_style_02_floor_green_bright'},
    },
    ceiling = {{tex = 'map/lab_games/fake_sky'}},
    wall = {
        {tex = 'map/lab_games/lg_style_02_wall_yellow'},
        {tex = 'map/lab_games/lg_style_02_wall_yellow_bright'},
        {tex = 'map/lab_games/lg_style_02_wall_blue'},
        {tex = 'map/lab_games/lg_style_02_wall_blue_bright'},
    },
    wallDecals = decals.decals,
}

texture_sets.GO = {
    floor = {
        {tex = 'map/lab_games/lg_style_03_floor_green'},
        {tex = 'map/lab_games/lg_style_03_floor_green_bright'},
        {tex = 'map/lab_games/lg_style_03_floor_blue'},
        {tex = 'map/lab_games/lg_style_03_floor_blue_bright'},
    },
    ceiling = {{tex = 'map/lab_games/fake_sky'}},
    wall = {
        {tex = 'map/lab_games/lg_style_03_wall_orange'},
        {tex = 'map/lab_games/lg_style_03_wall_orange_bright'},
        {tex = 'map/lab_games/lg_style_03_wall_gray'},
        {tex = 'map/lab_games/lg_style_03_wall_gray_bright'},
    },
    wallDecals = decals.decals,
    floorModels = {
        {mod = 'models/fut_obj_barbell_01.md3'},
        {mod = 'models/fut_obj_coil_01.md3'},
        {mod = 'models/fut_obj_cone_01.md3'},
        {mod = 'models/fut_obj_crossbar_01.md3'},
        {mod = 'models/fut_obj_cube_01.md3'},
        {mod = 'models/fut_obj_cylinder_01.md3'},
        {mod = 'models/fut_obj_doubleprism_01.md3'},
        {mod = 'models/fut_obj_glowball_01.md3'}
    }
}

texture_sets.PACMAN = {
    floor = {
        {tex = 'map/lab_games/lg_style_05_floor_blue'},
        {tex = 'map/lab_games/lg_style_05_floor_blue_bright'},
        {tex = 'map/lab_games/lg_style_05_floor_orange'},
        {tex = 'map/lab_games/lg_style_05_floor_orange_bright'},
    },
    ceiling = {{tex = 'map/lab_games/fake_sky'}},
    wall = {
        {tex = 'map/lab_games/lg_style_05_wall_red'},
        {tex = 'map/lab_games/lg_style_05_wall_red_bright'},
        {tex = 'map/lab_games/lg_style_05_wall_yellow'},
        {tex = 'map/lab_games/lg_style_05_wall_yellow_bright'},
    },
    wallDecals = decals.decals,
    floorModels = {
        {mod = 'models/fut_obj_toroid_01.md3'},
        {mod = 'models/fut_obj_cylinder_01.md3'},
        {mod = 'models/fut_obj_crossbar_01.md3'},
        {mod = 'models/fut_obj_cube_01.md3'},
    },
}

texture_sets.INVISIBLE_WALLS = {
    floor = {{tex = 'map/lab_games/lg_style_01_floor_orange'}},
    ceiling = {{tex = "map/lab_games/fake_sky"}},
    wall = {{tex = 'map/poltergeist'}},
}

texture_sets.CUSTOMIZABLE_FLOORS = {
    variations = {
        A = {floor = {{tex = 'map/lab_games/lg_style_01_floor_placeholder_A'}}},
        B = {floor = {{tex = 'map/lab_games/lg_style_01_floor_placeholder_B'}}},
        C = {floor = {{tex = 'map/lab_games/lg_style_01_floor_placeholder_C'}}},
        D = {floor = {{tex = 'map/lab_games/lg_style_01_floor_placeholder_D'}}},
        E = {floor = {{tex = 'map/lab_games/lg_style_01_floor_placeholder_E'}}},
        F = {floor = {{tex = 'map/lab_games/lg_style_01_floor_placeholder_F'}}},
    },
    floor = {{tex = 'map/lab_games/lg_style_01_floor_placeholder_0'}},
    ceiling = {{tex = 'map/lab_games/fake_sky'}},
    wall = {
        {tex = 'map/lab_games/lg_style_01_wall_green'},
        {tex = 'map/lab_games/lg_style_01_wall_green_bright'},
        {tex = 'map/lab_games/lg_style_01_wall_red'},
        {tex = 'map/lab_games/lg_style_01_wall_red_bright'},
        {tex = 'map/lab_games/lg_style_02_wall_yellow'},
        {tex = 'map/lab_games/lg_style_02_wall_yellow_bright'},
        {tex = 'map/lab_games/lg_style_02_wall_blue'},
        {tex = 'map/lab_games/lg_style_02_wall_blue_bright'},
        {tex = 'map/lab_games/lg_style_03_wall_orange'},
        {tex = 'map/lab_games/lg_style_03_wall_orange_bright'},
        {tex = 'map/lab_games/lg_style_03_wall_gray'},
        {tex = 'map/lab_games/lg_style_03_wall_gray_bright'},
        {tex = 'map/lab_games/lg_style_04_wall_green'},
        {tex = 'map/lab_games/lg_style_04_wall_green_bright'},
        {tex = 'map/lab_games/lg_style_04_wall_red'},
        {tex = 'map/lab_games/lg_style_04_wall_red_bright'},
        {tex = 'map/lab_games/lg_style_05_wall_red'},
        {tex = 'map/lab_games/lg_style_05_wall_red_bright'},
        {tex = 'map/lab_games/lg_style_05_wall_yellow'},
        {tex = 'map/lab_games/lg_style_05_wall_yellow_bright'}
    },
    wallDecals = decals.decals,
}

texture_sets.CAPTURE_THE_FLAG = {
    variations = {
        A = {
            floor = {{tex = 'map/lab_games/lg_style_01_floor_red_team_d'}},
            wall = {{tex = 'map/lab_games/lg_style_04_wall_red'}},
        },
        B = {
            floor = {{tex = 'map/lab_games/lg_style_01_floor_blue_team_d'}},
            wall = {{tex = 'map/lab_games/lg_style_02_wall_blue'}},
        },
        E = {
            floor = {{tex = 'map/lab_games/lg_style_05_floor_orange_bright'}},
            wall = {{tex = 'map/lab_games/lg_style_03_wall_gray'}},
        },
    },
    floor = {{tex = 'map/lab_games/lg_style_04_floor_orange'}},
    ceiling = {{tex = 'map/lab_games/fake_sky'}},
    wall = {{tex = 'map/lab_games/lg_style_02_wall_yellow'}},
    wallDecals = decals.decals,
}

return texture_sets
