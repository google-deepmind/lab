textures/map/lab_games/lg_logo
{
    qer_editorimage textures/map/lab_games/lg_logo.tga
    nopicmip
    {
        map textures/map/lab_games/lg_logo.tga
        rgbgen identity
        tcMod scroll 0.6 0
    }
}

textures/map/lab_games/lg_sky
{
    qer_editorimage textures/map/lab_games/lg_sky_01_sky_up_dull.tga
    surfaceparm noimpact
    surfaceparm nomarks
    q3map_surfacelight 500
    {
        map $whiteimage
        rgbgen const ( 0.59 0.88 1.00 )
    }
}

textures/map/arena_guide
{
    surfaceparm nodamage
    q3map_surfacelight 20000
    q3map_lightsubdivide 4
    {
        map $whiteimage
        rgbGen const ( 1.0 1.0 1.0 )
    }
}

textures/model/shield_energy
{
    {
        map textures/model/shield_energy_d.tga
        blendfunc add
    }
}

textures/model/overshield
{
    {
        map textures/model/overshield_d.tga
        blendfunc add
        rgbgen const ( 0.5 0.5 0.5 )
    }
}

textures/model/teleport_beam
{
    qer_editorimage textures/model/pickup_platform_glow.tga
    cull none
    nopicmip
    {
        map textures/model/pickup_platform_glow.tga
        blendfunc blend
        tcMod scroll 0.0 1.5
    }
}

textures/model/pickup_platform_d
{
    qer_editorimage textures/model/pickup_platform_d.tga
    nopicmip
    {
        map textures/model/pickup_platform_d.tga
    }
}

textures/model/containment_platform
{
    qer_editorimage textures/model/pickup_platform_d.tga
    nopicmip
    {
        map textures/model/pickup_platform_d.tga
    }
}

textures/map/lab_games/stadium_d
{
    qer_editorimage textures/map/lab_games/stadium_d.tga
    surfaceparm nolightmap
    {
        map textures/map/lab_games/stadium_d.tga
    }
    {
        map textures/map/lab_games/stadium_logo.tga
        blendfunc gl_one gl_one
        tcMod scroll -1.5 0.0
    }
}

textures/map/lab_games/blank
{
    surfaceparm nomarks
    surfaceparm noimpact
    surfaceparm nolightmap
    {
        map textures/map/black_d.tga
        blendfunc blend
        alphagen const 0.0
    }
}

textures/model/flags
{
    cull none
    nopicmip
    {
        map textures/model/beacon_d.tga
        blendfunc blend
        alphaFunc GE128
        depthwrite
    }
    {
        map $whiteimage
        blendfunc gl_zero gl_src_color
        rgbgen diffuseLighting
    }
    {
        map textures/model/beacon_light_d.tga
        blendfunc add
    }

}

textures/model/lg_barrier_01_blue
{
    qer_editorimage textures/model/lg_barrier_01_blue_d.tga
    nopicmip
    {
        map textures/model/lg_barrier_01_blue_d.tga
        rgbgen diffuseLighting
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/model/lg_barrier_01_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_01_4tile
{
    qer_editorimage textures/map/lab_games/lg_style_01_4tile_d.tga
    nopicmip
    {
        map textures/map/lab_games/lg_style_01_4tile_d.tga
        rgbgen diffuseLighting
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/floor_corner_90_d
{
    qer_editorimage textures/map/floor_corner_90_d.tga
    nopicmip
    {
        map textures/map/floor_corner_90_d.tga
        rgbgen diffuseLighting
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/floor_edge
{
    qer_editorimage textures/map/floor_edge_d.tga
    nopicmip
    {
        map textures/map/floor_edge_d.tga
        rgbgen diffuseLighting
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// FLOORS

textures/map/lab_games/lg_style_01_floor_orange
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_orange_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_orange_d.tga
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_floor_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_floor_orange_bright
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_orange_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_orange_bright_d.tga
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_floor_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_floor_grey
{
    qer_editorimage textures/map/lab_games/lg_floor_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_floor_d.tga
        rgbgen const ( 0.40 0.40 0.40 )
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_01_floor_red
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_red_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_red_d.tga
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_floor_light_m
        blendfunc add
        rgbgen const ( 0.50 0.50 0.50 )
    }
}

textures/map/lab_games/lg_style_01_floor_red_bright
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_red_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_red_bright_d.tga
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_floor_light_m
        blendfunc add
        rgbgen const ( 0.50 0.50 0.50 )
    }
}

textures/map/lab_games/lg_style_01_floor_blue
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_blue_d.tga
        rgbgen const ( 0.00 0.58 1.00 )
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_floor_light_m
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_floor_blue_bright
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_blue_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_blue_bright_d.tga
        rgbgen const ( 0.00 0.58 1.00 )
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_floor_light_m
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_floor_red_team
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_red_team_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_red_team_d.tga
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_floor_light_m
        blendfunc add
        rgbgen const ( 0.50 0.50 0.50 )
    }
}

textures/map/lab_games/lg_style_01_floor_blue_team
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_blue_team_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_blue_team_d.tga
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_floor_light_m
        blendfunc add
        rgbgen const ( 0.50 0.50 0.50 )
    }
}

textures/map/lab_games/lg_style_02_floor_blue
{
    qer_editorimage textures/map/lab_games/lg_style_02_floor_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_02_floor_blue_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_02_floor_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_02_floor_blue_bright
{
    qer_editorimage textures/map/lab_games/lg_style_02_floor_blue_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_02_floor_blue_bright_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_02_floor_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_02_floor_blue
{
    qer_editorimage textures/map/lab_games/lg_style_02_floor_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_02_floor_blue_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_02_floor_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_02_floor_blue_bright
{
    qer_editorimage textures/map/lab_games/lg_style_02_floor_blue_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_02_floor_blue_bright_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_02_floor_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_02_floor_green
{
    qer_editorimage textures/map/lab_games/lg_style_02_floor_green_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_02_floor_green_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_02_floor_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_02_floor_green_bright
{
    qer_editorimage textures/map/lab_games/lg_style_02_floor_green_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_02_floor_green_bright_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_02_floor_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_03_floor_green
{
    qer_editorimage textures/map/lab_games/lg_style_03_floor_green_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_03_floor_green_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light1_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light2_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.3
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light3_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_03_floor_green_bright
{
    qer_editorimage textures/map/lab_games/lg_style_03_floor_green_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_03_floor_green_bright_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light1_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light2_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.3
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light3_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_03_floor_blue
{
    qer_editorimage textures/map/lab_games/lg_style_03_floor_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_03_floor_blue_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light1_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light2_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.3
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light3_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_03_floor_blue_bright
{
    qer_editorimage textures/map/lab_games/lg_style_03_floor_blue_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_03_floor_blue_bright_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light1_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light2_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.3
    }
    {
        map textures/map/lab_games/lg_style_03_floor_light3_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.15
    }
}

textures/map/lab_games/lg_style_04_floor_blue
{
    qer_editorimage textures/map/lab_games/lg_style_04_floor_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_04_floor_blue_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_04_floor_blue_bright
{
    qer_editorimage textures/map/lab_games/lg_style_04_floor_blue_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_04_floor_blue_bright_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_04_floor_orange
{
    qer_editorimage textures/map/lab_games/lg_style_04_floor_orange_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_04_floor_orange_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_04_floor_orange_bright
{
    qer_editorimage textures/map/lab_games/lg_style_04_floor_orange_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_04_floor_orange_bright_d.tga
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_05_floor_blue
{
    qer_editorimage textures/map/lab_games/lg_style_05_floor_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_05_floor_blue_d.tga
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_05_floor_light1_m.tga
        blendfunc add
        rgbGen wave sin 0.0 1.0 0.25 0.5
    }
    {
        map textures/map/lab_games/lg_style_05_floor_light2_m.tga
        blendfunc add
        rgbGen wave sin 0.0 1.0 0.75 0.5
    }
}

textures/map/lab_games/lg_style_05_floor_blue_bright
{
    qer_editorimage textures/map/lab_games/lg_style_05_floor_blue_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_05_floor_blue_bright_d.tga
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_05_floor_light1_m.tga
        blendfunc add
        rgbGen wave sin 0.0 1.0 0.5 0.5
    }
    {
        map textures/map/lab_games/lg_style_05_floor_light2_m.tga
        blendfunc add
        rgbGen wave sin 0.0 1.0 0.0 0.5
    }
}

textures/map/lab_games/lg_style_05_floor_orange
{
    qer_editorimage textures/map/lab_games/lg_style_05_floor_orange_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_05_floor_orange_d.tga
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_05_floor_light1_m.tga
        blendfunc add
        rgbGen wave sin 0.0 0.5 0.5 0.5
    }
    {
        map textures/map/lab_games/lg_style_05_floor_light2_m.tga
        blendfunc add
        rgbGen wave sin 0.0 0.5 0.0 0.5
    }
}

textures/map/lab_games/lg_style_05_floor_orange_bright
{
    qer_editorimage textures/map/lab_games/lg_style_05_floor_orange_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_05_floor_orange_bright_d.tga
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_05_floor_light1_m.tga
        blendfunc add
        rgbGen wave sin 0.0 0.5 0.5 0.5
    }
    {
        map textures/map/lab_games/lg_style_05_floor_light2_m.tga
        blendfunc add
        rgbGen wave sin 0.0 0.5 0.0 0.5
    }
}

// WALLS

textures/map/lab_games/lg_style_01_door
{
    qer_editorimage textures/map/lab_games/lg_style_01_door_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_style_01_door_d.tga
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}


textures/map/lab_games/lg_style_01_wall_green
{
    qer_editorimage textures/map/lab_games/lg_style_01_wall_green_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        rgbgen const ( 0.00 0.15 0.25 )
        tcMod scroll 0.50 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.29 0.50 )
        tcMod scroll 0.35 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
        tcMod scroll 0.80 0.00
    }
    {
        map textures/map/lab_games/lg_style_01_wall_green_d.tga
        blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.05 0.05 0.05 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_wall_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_wall_green_bright
{
    qer_editorimage textures/map/lab_games/lg_style_01_wall_green_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        rgbgen const ( 0.00 0.15 0.25 )
        tcMod scroll 0.50 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.29 0.50 )
        tcMod scroll 0.35 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
        tcMod scroll 0.80 0.00
    }
    {
        map textures/map/lab_games/lg_style_01_wall_green_bright_d.tga
        blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.05 0.05 0.05 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_wall_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_wall_blue
{
    qer_editorimage textures/map/lab_games/lg_style_01_wall_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        rgbgen const ( 0.00 0.15 0.25 )
        tcMod scroll 0.50 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.29 0.50 )
        tcMod scroll 0.35 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
        tcMod scroll 0.80 0.00
    }
    {
        map textures/map/lab_games/lg_style_01_wall_blue_d.tga
        blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.05 0.05 0.05 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_wall_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_wall_red
{
    qer_editorimage textures/map/lab_games/lg_style_01_wall_red_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        rgbgen const ( 0.00 0.15 0.25 )
        tcMod scroll 0.50 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.29 0.50 )
        tcMod scroll 0.35 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
        tcMod scroll 0.80 0.00
    }
    {
        map textures/map/lab_games/lg_style_01_wall_red_d.tga
        blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.05 0.05 0.05 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_wall_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_wall_red_bright
{
    qer_editorimage textures/map/lab_games/lg_style_01_wall_red_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        rgbgen const ( 0.00 0.15 0.25 )
        tcMod scroll 0.50 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.29 0.50 )
        tcMod scroll 0.35 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
        tcMod scroll 0.80 0.00
    }
    {
        map textures/map/lab_games/lg_style_01_wall_red_bright_d.tga
        blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.05 0.05 0.05 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_wall_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_wall_purple
{
    qer_editorimage textures/map/lab_games/lg_style_01_wall_purple_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        rgbgen const ( 0.00 0.15 0.25 )
        tcMod scroll 0.50 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.29 0.50 )
        tcMod scroll 0.35 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
        tcMod scroll 0.80 0.00
    }
    {
        map textures/map/lab_games/lg_style_01_wall_purple_d.tga
        blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbgen const ( 0.82 0.79 0.69 )
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.05 0.05 0.05 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_wall_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}


textures/map/lab_games/lg_style_01_wall_yellow
{
    qer_editorimage textures/map/lab_games/lg_style_01_wall_yellow_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        rgbgen const ( 0.00 0.15 0.25 )
        tcMod scroll 0.50 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.29 0.50 )
        tcMod scroll 0.35 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
        tcMod scroll 0.80 0.00
    }
    {
        map textures/map/lab_games/lg_style_01_wall_yellow_d.tga
        blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.05 0.05 0.05 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_wall_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_wall_cerise
{
    qer_editorimage textures/map/lab_games/lg_style_01_wall_cerise_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        rgbgen const ( 0.00 0.15 0.25 )
        tcMod scroll 0.50 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.29 0.50 )
        tcMod scroll 0.35 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
        tcMod scroll 0.80 0.00
    }
    {
        map textures/map/lab_games/lg_style_01_wall_cerise_d.tga
        blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.05 0.05 0.05 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_wall_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_01_wall_cyan
{
    qer_editorimage textures/map/lab_games/lg_style_01_wall_cyan_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        rgbgen const ( 0.00 0.15 0.25 )
        tcMod scroll 0.50 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.29 0.50 )
        tcMod scroll 0.35 0.00
    }
    {
        map textures/map/lab_games/lg_grad_bar_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
        tcMod scroll 0.80 0.00
    }
    {
        map textures/map/lab_games/lg_style_01_wall_cyan_d.tga
        blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.05 0.05 0.05 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_01_wall_light_m.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

textures/map/lab_games/lg_style_02_wall_yellow
{
    qer_editorimage textures/map/lab_games/lg_style_02_wall_yellow_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/grad_bar_hor_g.tga
        tcMod scroll 0.00 -0.30
    }
    {
        map textures/map/lab_games/lg_style_02_wall_yellow_d.tga
        blendfunc blend
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_02_wall_yellow_bright
{
    qer_editorimage textures/map/lab_games/lg_style_02_wall_yellow_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/grad_bar_hor_g.tga
        tcMod scroll 0.00 -0.30
    }
    {
        map textures/map/lab_games/lg_style_02_wall_yellow_bright_d.tga
        blendfunc blend
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_02_wall_blue
{
    qer_editorimage textures/map/lab_games/lg_style_02_wall_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/grad_bar_hor_g.tga
        tcMod scroll 0.00 -0.30
    }
    {
        map textures/map/lab_games/lg_style_02_wall_blue_d.tga
        blendfunc blend
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_02_wall_blue_bright
{
    qer_editorimage textures/map/lab_games/lg_style_02_wall_blue_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/grad_bar_hor_g.tga
        tcMod scroll 0.00 -0.30
    }
    {
        map textures/map/lab_games/lg_style_02_wall_blue_bright_d.tga
        blendfunc blend
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_02_wall_orange
{
    qer_editorimage textures/map/lab_games/lg_style_02_wall_orange_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/grad_bar_hor_g.tga
        tcMod scroll 0.00 -0.30
    }
    {
        map textures/map/lab_games/lg_style_02_wall_orange_d.tga
        blendfunc blend
        rgbgen identity
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_03_wall_orange
{
    qer_editorimage textures/map/lab_games/lg_style_03_wall_orange_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_03_wall_orange_d.tga
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_03_wall_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
}

textures/map/lab_games/lg_style_03_wall_orange_bright
{
    qer_editorimage textures/map/lab_games/lg_style_03_wall_orange_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_03_wall_orange_bright_d.tga
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_03_wall_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
}

textures/map/lab_games/lg_style_03_wall_gray
{
    qer_editorimage textures/map/lab_games/lg_style_03_wall_gray_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_03_wall_gray_d.tga
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_03_wall_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
}

textures/map/lab_games/lg_style_03_wall_gray_bright
{
    qer_editorimage textures/map/lab_games/lg_style_03_wall_gray_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_03_wall_gray_bright_d.tga
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_style_03_wall_light_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
}


textures/map/lab_games/lg_style_04_wall_green
{
    qer_editorimage textures/map/lab_games/lg_style_04_wall_green_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_circuit_r.tga
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map textures/map/lab_games/lg_style_04_wall_green_d.tga
        blendfunc gl_src_color gl_one_minus_src_alpha
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_04_wall_green_bright
{
    qer_editorimage textures/map/lab_games/lg_style_04_wall_green_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_circuit_r.tga
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map textures/map/lab_games/lg_style_04_wall_green_bright_d.tga
        blendfunc gl_src_color gl_one_minus_src_alpha
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_04_wall_red
{
    qer_editorimage textures/map/lab_games/lg_style_04_wall_red_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_circuit_r.tga
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map textures/map/lab_games/lg_style_04_wall_red_d.tga
        blendfunc gl_src_color gl_one_minus_src_alpha
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_04_wall_red_bright
{
    qer_editorimage textures/map/lab_games/lg_style_04_wall_red_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_circuit_r.tga
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map textures/map/lab_games/lg_style_04_wall_red_bright_d.tga
        blendfunc gl_src_color gl_one_minus_src_alpha
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_05_wall_red
{
    qer_editorimage textures/map/lab_games/lg_style_05_wall_red_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_circuit_r.tga
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map textures/map/lab_games/lg_style_05_wall_red_d.tga
        blendfunc gl_src_color gl_one_minus_src_alpha
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_05_wall_red_bright
{
    qer_editorimage textures/map/lab_games/lg_style_05_wall_red_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_circuit_r.tga
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map textures/map/lab_games/lg_style_05_wall_red_bright_d.tga
        blendfunc gl_src_color gl_one_minus_src_alpha
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_05_wall_yellow
{
    qer_editorimage textures/map/lab_games/lg_style_05_wall_yellow_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_circuit_r.tga
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map textures/map/lab_games/lg_style_05_wall_yellow_d.tga
        blendfunc gl_src_color gl_one_minus_src_alpha
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lab_games/lg_style_05_wall_yellow_bright
{
    qer_editorimage textures/map/lab_games/lg_style_05_wall_yellow_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_circuit_r.tga
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map textures/map/lab_games/lg_style_05_wall_yellow_bright_d.tga
        blendfunc gl_src_color gl_one_minus_src_alpha
        rgbgen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// CEILING

textures/map/lab_games/lg_style_01_ceiling_grey
{
    qer_editorimage textures/map/lab_games/lg_style_01_ceiling_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    q3map_surfacelight 500
    {
        map textures/map/lab_games/lg_style_01_ceiling_d.tga
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }

}

// PROCEDURAL MAZE

textures/map/lg_floor_purple_d
{
    qer_editorimage textures/map/lab_games/lg_floor_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_floor_d.tga
        rgbgen const ( 0.76 0.54 0.97 )
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lg_floor_red_d
{
    qer_editorimage textures/map/lab_games/lg_floor_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_floor_d.tga
        rgbgen const ( 0.94 0.37 0.28 )
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lg_floor_orange_d
{
    qer_editorimage textures/map/lab_games/lg_floor_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_floor_d.tga
        rgbgen const ( 0.94 0.53 0.00 )
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lg_floor_green_d
{
    qer_editorimage textures/map/lab_games/lg_floor_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_floor_d.tga
        rgbgen const ( 0.00 0.70 0.44 )
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/map/lg_floor_blue_d
{
    qer_editorimage textures/map/lab_games/lg_floor_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_floor_d.tga
        rgbgen const ( 0.00 0.38 0.77 )
    }
    {
        map textures/map/lab_games/lg_circuit_r.tga
        blendfunc add
        rgbgen const ( 0.06 0.06 0.06 )
        tcgen environment
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// WALLS

textures/map/lg_wall_green_d
{
    qer_editorimage textures/map/lab_games/lg_wall_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_wall_d.tga
        rgbgen const ( 0.00 0.70 0.44 )
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_wall_glow_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
}

textures/map/lg_wall_purp_d
{
    qer_editorimage textures/map/lab_games/lg_wall_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_wall_d.tga
        rgbgen const ( 0.76 0.54 0.97 )
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_wall_glow_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
}

textures/map/lg_wall_red_d
{
    qer_editorimage textures/map/lab_games/lg_wall_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_wall_d.tga
        rgbgen const ( 0.94 0.37 0.28 )
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_wall_glow_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
}

textures/map/lg_wall_orange_d
{
    qer_editorimage textures/map/lab_games/lg_wall_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_wall_d.tga
        rgbgen const ( 0.94 0.53 0.01 )
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_wall_glow_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
}

textures/map/lg_wall_blue_d
{
    qer_editorimage textures/map/lab_games/lg_wall_d.tga
    nopicmip
    q3map_lightmapSampleSize 32
    {
        map textures/map/lab_games/lg_wall_d.tga
        rgbgen const ( 0.00 0.69 0.97 )
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/map/lab_games/lg_wall_glow_m.tga
        blendfunc add
        rgbGen wave sin 1.0 1.0 0.0 0.5
    }
}

// EFFECTS

teleportEffect
{
    {
        map textures/model/spawn_d.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 2
        tcMod scroll 0 1
    }
}

textures/map/poltergeist
{
    qer_editorimage textures/map/glass_d.tga
    surfaceparm nodraw
    surfaceparm trans
    surfaceparm playerclip
    surfaceparm nolightmap
    {
        map textures/map/ghost.tga
        blendfunc blend
    }
}

textures/map/ghost
{
    qer_editorimage textures/map/glass_d.tga
    surfaceparm nodraw
    surfaceparm trans
    surfaceparm nonsolid
    {
        map textures/map/ghost.tga
        blendfunc blend
    }
    {
        map textures/map/fut_glass_env.tga
        blendfunc add
        tcGen environment
    }
}

// SPECIFIC OBJECTS

textures/model/mothership_d
{
    qer_editorimage textures/model/mothership_d.tga
    {
        map textures/model/mothership_d.tga
    }
}

// SKIES

textures/map/lab_games/fake_sky
{
    qer_editorimage textures/map/fake_sky.tga
    surfaceparm noimpact
    surfaceparm nomarks
    q3map_surfacelight 2000
    {
        map $whiteimage
        rgbgen const ( 0.59 0.88 1.00 )
    }
}

// FULL SKYBOX

textures/map/lab_games/sky/lg_sky_01
{
    nomipmaps
    qer_editorimage textures/map/lab_games/sky/lg_sky_01_up.tga
    surfaceparm noimpact
    surfaceparm nolightmap
    q3map_globaltexture
    q3map_lightsubdivide 256
    q3map_surfacelight 600
    surfaceparm sky
    q3map_sun   1 1 1 32   90 90
    skyparms textures/map/lab_games/sky/lg_sky_01 - -
}

textures/map/lab_games/sky/lg_sky_01_up
{
    nomipmaps
    {
        map textures/map/lab_games/sky/lg_sky_01_up.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_01_dn
{
    nomipmaps
    {
        map textures/map/lab_games/sky/lg_sky_01_dn.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_01_lf
{
    nomipmaps
    {
    map textures/map/lab_games/sky/lg_sky_01_lf.tga
    rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_01_rt
{
    nomipmaps
    {
        map textures/map/lab_games/sky/lg_sky_01_rt.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_01_ft
{
    nomipmaps
    {
        map textures/map/lab_games/sky/lg_sky_01_ft.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_01_bk
{
    nomipmaps
    {
        map textures/map/lab_games/sky/lg_sky_01_bk.tga
        rgbGen identity
    }
}

// SKY DAY

textures/map/lab_games/sky/lg_sky_02
{
    nomipmaps
    qer_editorimage textures/map/lab_games/sky/lg_sky_02_up.tga
    surfaceparm noimpact
    surfaceparm nolightmap
    q3map_globaltexture
    q3map_lightsubdivide 256
    q3map_surfacelight 600
    surfaceparm sky
    q3map_sun   1 1 1 32   90 90
    skyparms textures/map/lab_games/sky/lg_sky_02 - -
}

textures/map/lab_games/sky/lg_sky_02_up
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_02_up.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_02_dn
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_02_dn.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_02_lf
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_02_lf.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_02_rt
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_02_rt.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_02_ft
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_02_ft.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_02_bk
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_02_bk.tga
        rgbGen identity
    }
}


textures/map/lab_games/sky/lg_sky_03
{
    nomipmaps
    qer_editorimage textures/map/lab_games/sky/lg_sky_03_up.tga
    surfaceparm noimpact
    surfaceparm nolightmap
    q3map_globaltexture
    q3map_lightsubdivide 256
    q3map_surfacelight 600
    surfaceparm sky
    q3map_sun   1 1 1 32   90 90
    skyparms textures/map/lab_games/sky/lg_sky_03 - -
}

textures/map/lab_games/sky/lg_sky_03_up
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_03_up.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_03_dn
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_03_dn.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_03_lf
{
    nomipmaps
    q3map_surfacelight 5000
    {
    map textures/map/lab_games/sky/lg_sky_03_lf.tga
    rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_03_rt
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_03_rt.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_03_ft
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_03_ft.tga
        rgbGen identity
    }
}

textures/map/lab_games/sky/lg_sky_03_bk
{
    nomipmaps
    q3map_surfacelight 5000
    {
        map textures/map/lab_games/sky/lg_sky_03_bk.tga
        rgbGen identity
    }
}

// SKYBOX OBJECTS

textures/map/lab_games/signal_pulse_red
{
    surfaceparm nolightmap
    q3map_surfacelight 500
    {
        map $whiteimage
        blendfunc add
        rgbgen const ( 0.50 0.00 0.00 )
    }
    {
        map textures/map/lab_games/lg_grad_bar2_g.tga
        blendfunc add
        rgbgen const ( 0.50 0.00 0.00 )
        tcMod scroll 0.0 0.5
    }
}

textures/map/lab_games/signal_pulse_blue
{
    surfaceparm nolightmap
    q3map_surfacelight 500
    {
        map $whiteimage
        blendfunc add
        rgbgen const ( 0.00 0.00 0.50 )
    }
    {
        map textures/map/lab_games/lg_grad_bar2_g.tga
        blendfunc add
        rgbgen const ( 0.00 0.00 0.50 )
        tcMod scroll 0.0 0.5
    }
}
