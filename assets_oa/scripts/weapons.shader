models/weapons2/machinegun/standard_d
{
    {
        map textures/model/standard_d.tga
        rgbgen identity
    }
}

textures/model/rapid_d
{
    {
        map textures/model/rapid_d.tga
        blendfunc add
        rgbgen identity
    }
}

textures/model/orb_d
{
    cull none
    {
        map textures/model/orb_d.tga
        blendfunc add
    }
}

models/weapons2/lightning/beam_d
{
    cull none
    {
        map textures/model/beam_d.tga
        blendfunc add
        rgbgen identity
    }
}

models/weapons2/railgun/disc_d
{
    {
        map textures/model/disc_d.tga
        rgbgen identity
    }
}

models/weapons2/gauntlet/impulse_d
{
    cull none
    {
        map textures/model/impulse_d.tga
        blendfunc add
        rgbgen identity
        tcMod scroll 0.0 -0.75
    }
}

rocketThrustf2
{
    surfaceparm nodraw
}
