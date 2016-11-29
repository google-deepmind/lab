textures/map/ctp_tech_lava_d
{
    q3map_lightimage $whiteimage
    qer_editorimage textures/map/ctp_tech_lava_d.tga
    q3map_surfacelight 500
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/map/ctp_tech_lava_d.tga
        blendFunc filter
        rgbGen identity
    }
    {
        map textures/map/ctp_tech_lava_d.tga
        blendFunc add
    }
}
