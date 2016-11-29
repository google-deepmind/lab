//GADGET DECAL EFFECTS

gfx/damage/bullet_mrk // rapid
{
    polygonOffset
    {
        map textures/model/circuit_decal.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 ) //blue
    }
}

gfx/damage/burn_med_mrk //orb
{
    polygonOffset
    {
        map textures/model/circuit_decal.tga
        blendFunc add
        rgbgen const ( 0.00 0.58 1.00 ) //blue
    }
}

gfx/damage/hole_lg_mrk //beam
{
    polygonOffset
    {
        map textures/model/circuit_decal.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 ) //blue
    }
}

//OTHER

gfx/misc/tracer
{
    cull disable
    surfaceparm nodraw
    {
        map $whiteimage
        blendfunc filter
    }
}

//BLOB SHADOW
markShadow
{
    polygonoffset
    {
        map gfx/damage/shadow.tga
        blendfunc gl_zero gl_one_minus_src_color
        rgbGen Vertex
    }
}

waterBubble
{
    sort underwater
    cull disable
    {
        map sprites/bubble.tga
        blendfunc blend
        rgbGen Vertex
        alphaGen Vertex
    }
}
