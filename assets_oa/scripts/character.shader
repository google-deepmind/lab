models/players/crash/redskin
{
    cull disable
    nopicmip
    {
        map models/players/crash/redskin.tga
        blendfunc gl_src_alpha gl_one_minus_src_alpha
        alphaFunc GE128
        depthwrite
    }
    {
        map models/players/crash/skin1_scroll.tga
        blendfunc add
        tcMod scroll -1.6 0
    }
    {
        map models/players/crash/thruster_glow.tga
        blendfunc gl_one gl_one
    }
}

models/players/crash/blueskin
{
    cull disable
    nopicmip
    {
        map models/players/crash/blueskin.tga
        blendfunc gl_src_alpha gl_one_minus_src_alpha
        alphaFunc GE128
        depthwrite
    }
    {
        map models/players/crash/skin1_scroll.tga
        blendfunc add
        tcMod scroll -1.6 0
    }
    {
        map models/players/crash/thruster_glow.tga
        blendfunc gl_one gl_one
    }
}

models/players/crash/skin1
{
    cull disable
    nopicmip
    {
        map models/players/crash/skin1.tga
        blendfunc gl_src_alpha gl_one_minus_src_alpha
        alphaFunc GE128
        depthwrite
    }
    {
        map models/players/crash/skin1_scroll.tga
        blendfunc gl_one gl_one
        tcMod scroll -1.6 0
    }
    {
        map models/players/crash/thruster_glow.tga
        blendfunc gl_one gl_one
    }
}

models/players/crash_color/skin_base //crash color shader
{
    qer_editorimage models/players/crash_color/skin_base.tga
    nomipmaps
    {
        map models/players/crash_color/skin_base.tga
        blendfunc gl_src_alpha gl_one_minus_src_alpha
        alphaFunc GE128
        depthwrite
    }

    {
        map models/players/crash_color/dm_character_skin_mask_a.tga
        blendfunc blend
        rgbGen lightingDiffuse
    }

    {
        map models/players/crash_color/dm_character_skin_mask_b.tga
        blendfunc blend
        rgbGen lightingDiffuse
    }

    {
        map models/players/crash_color/dm_character_skin_mask_c.tga
        blendfunc blend
        rgbGen lightingDiffuse
    }
    {
        map models/players/crash/skin1_scroll.tga
        blendfunc gl_one gl_one
        tcMod scroll -1.6 0
    }
    {
        map models/players/crash/thruster_glow.tga
        blendfunc gl_one gl_one
    }
}
