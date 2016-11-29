railDic
{
    cull disable
    deformVertexes autosprite
    {
        clampmap textures/model/gen_impact.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 1.3
    }
    {
        clampmap textures/model/gen_impact.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 1.3
    }
}

bulletExplosion
{
    cull disable
    {
        map textures/model/gen_impact.tga
        blendfunc gl_one gl_add
        rgbgen const ( 0.00 0.58 1.00 ) //blue
    }
}

lightningExplosion //modified for DeepMind Lab
{
    cull disable
    deformVertexes wave 9 sin 0 1 0 9
    {
        map textures/model/gen_impact.tga
        rgbgen const ( 0.00 0.58 1.00 ) //blue
        blendfunc add
    }
}

bfgExplocsion
{
    cull disable
    {
        map models/weaphits/bfgscroll.tga
        blendfunc add
        tcMod scroll -1.4 0
    }
    {
        map models/weaphits/bfgscroll.tga
        blendfunc add
        tcMod scroll -0.6 0
    }
}

bfgExplosion
{
    {
        clampmap textures/oa/bfgfiar.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 1.6
        tcMod rotate 77
        tcMod stretch sin 0.3 0.7 0 0.6
    }
    {
        clampmap textures/oa/bfgfiar.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 1.6
        tcMod rotate -17
        tcMod stretch sin 0 1.3 0 0.8
    }
    {
        clampmap textures/oa/bfgfiar.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 1.6
        tcMod rotate -77
        tcMod stretch sawtooth 0 1.3 0 0.8
    }
}

plasmaExplosion
{
    cull disable
    {
        map textures/model/gen_impact.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 ) //blue
    }
}

railExplosion
{
    cull disable
    {
        clampmap textures/model/disc_impact_d.tga
        blendfunc add
        rgbgen wave sin 1.0  1.0 0.0 0.8
    }
}

OLDplasmaExplosion
{
    cull disable
    {
        map models/weaphits/plasscroll2.tga
        blendfunc add
        tcMod scroll -1.2 7
    }
}

bloodExplosion
{
    {
        map textures/model/shield_fx.tga
        blendfunc add
        tcMod rotate 70
        rgbgen const ( 0.00 0.58 1.00 )
        rgbGen wave sin 0 1 0 0.5
    }
    {
        map textures/model/shield_fx.tga
        blendfunc add
        tcMod rotate -30
        rgbgen const ( 0.00 0.58 1.00 )
        rgbGen wave sin 0 1 0 0.5
    }
}



rocketExplosion
{
    {
        map textures/model/gen_impact.tga
        blendfunc add
        rgbgen const ( 0.00 0.58 1.00 )
    }
}

lasmaExplosion
{
    cull disable
    {
        clampmap textures/oa/fiar.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 4
        tcMod rotate 300
        tcMod stretch sin 0.3 0.4 0 0.4
    }
    {
        clampmap textures/oa/fiar2.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 4
        tcMod rotate 122
        tcMod stretch sin 0.8 -0.7 0 0.1
    }
}

ailExplosion
{
    cull disable
    {
        clampmap textures/oa/fiar.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 1
        tcMod rotate 12
        tcMod stretch sin 0.3 0.4 0 0.4
    }
    {
        clampmap textures/oa/fiar2.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 1
        tcMod rotate 15
        tcMod stretch sin 0.8 -0.4 0 0.1
    }
    {
        clampmap textures/oa/fiar.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 1
        tcMod rotate -12
    }
    {
        clampmap textures/oa/fiar2.tga
        blendfunc add
        rgbGen wave inversesawtooth 0 1 0 1
        tcMod rotate -78
    }
}
