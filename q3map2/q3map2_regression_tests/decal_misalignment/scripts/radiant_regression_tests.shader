textures/radiant_regression_tests/tile_trans
{
    qer_trans 0.9
    q3map_bounceScale 0.0
    surfaceparm trans
    cull disable
    polygonOffset
    {
        map textures/radiant_regression_tests/tile_trans.tga
        blendFunc blend
        alphaGen const 1
    }
}

textures/radiant_regression_tests/glass
{
    qer_editorImage textures/radiant_regression_tests/qer_glass.tga
    qer_trans 0.6
    q3map_bounceScale 0.0
    surfaceparm nolightmap
    surfaceparm detail
    surfaceparm trans
    cull disable
    {
        map textures/radiant_regression_tests/glass.tga
        blendFunc add
        tcGen environment
    }
}
