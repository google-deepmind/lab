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
        rgbgen const ( 1.00 0.00 0.00 )
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

grenadeExplosion
{
    {
        map textures/model/gen_impact.tga
        blendfunc add
        rgbgen const ( 0.00 1.00 0.58 )
    }
}
