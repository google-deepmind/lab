models/players/crash/redskin
{
    cull disable
    nopicmip
    {
        map models/players/crash/redskin.tga
        blendfunc blend
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
        blendfunc add
    }
}

models/players/crash/blueskin
{
    cull disable
    nopicmip
    {
        map models/players/crash/blueskin.tga
        blendfunc blend
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
        blendfunc add
    }
}

models/players/crash/skin1
{
    cull disable
    nopicmip
    {
        map models/players/crash/skin1.tga
        blendfunc blend
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
        blendfunc add
    }
}

models/players/crash_color/skin_base //crash color shader
{
    qer_editorimage models/players/crash_color/skin_base.tga
    nomipmaps
    {
        map models/players/crash_color/skin_base.tga
        blendfunc blend
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
        blendfunc add
    }
}
