// FLOORS FOR DYNAMIC COLOURING

textures/map/lab_floors/lg_style_06_floor_1
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

textures/map/lab_floors/lg_style_06_floor_2
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

textures/map/lab_floors/lg_style_06_floor_3
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_blue_d.tga
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

textures/map/lab_floors/lg_style_06_floor_4
{
    qer_editorimage textures/map/lab_games/lg_style_01_floor_blue_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_01_floor_blue_bright_d.tga
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

textures/map/lab_floors/lg_style_06_floor_5
{
    qer_editorimage textures/map/lab_games/lg_style_02_floor_blue_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_02_floor_blue_d.tga
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

textures/map/lab_floors/lg_style_06_floor_6
{
    qer_editorimage textures/map/lab_games/lg_style_02_floor_blue_bright_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_02_floor_blue_bright_d.tga
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

textures/map/lab_floors/lg_style_06_floor_7
{
    qer_editorimage textures/map/lab_games/lg_style_02_floor_green_d.tga
    nopicmip
    q3map_lightmapSampleSize 8
    {
        map textures/map/lab_games/lg_style_02_floor_green_d.tga
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
