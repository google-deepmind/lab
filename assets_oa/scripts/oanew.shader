lowShotgunFlash
{
    {
        map textures/map/black_d.tga
        blendfunc blend
        alphagen const 0.0
    }
}

lowMachgunFlash
{
    {
        map textures/map/black_d.tga
        blendfunc blend
        alphagen const 0.0
    }
}

lowPlasmaaFlash
{
    {
        map textures/map/black_d.tga
        blendfunc blend
        alphagen const 0.0
    }
}

lowRocketlFlash
{
    {
        map textures/map/black_d.tga
        blendfunc blend
        alphagen const 0.0
    }
}

lowGrenadeFlash
{
    {
        map textures/map/black_d.tga
        blendfunc blend
        alphagen const 0.0
    }
}

lowRailgunFlash
{
    {
        map textures/map/black_d.tga
        blendfunc blend
        alphagen const 0.0
    }
}

lapscrn
{
  tessSize 64
  {
    map models/mapobjects/laptop/lapscrn.tga
  }
  {
    map models/mapobjects/laptop/invert.tga
    blendfunc gl_one_minus_dst_color gl_one_minus_src_color
    tcGen environment
  }
}

lowLightnnFlash
{
  //surfaceparm nodraw
  deformVertexes autosprite
  {
    //map models/weapons2/lightning/f_lightning.tga
    map textures/model/gen_impact.tga
    rgbgen const ( 0.00 0.58 1.00 ) //blue
    blendfunc add
    tcMod rotate 675756
  }
}

LightnnFlash
{
  {
      map textures/map/black_d.tga
      blendfunc blend
      alphagen const 0.0
  }
}

// sawtooth stretch 0 values result in ugly clamping
textures/effects/jumpcirc
{
  q3map_lightimage textures/effects/jumpcirc.tga
  q3map_surfacelight 466
  {
    map $whiteimage
    blendfunc add
    rgbgen const ( 1.0 0.0 0.0 )
  }
  //{
  //  clampmap textures/effects/jumpcirc.tga
  //  tcMod stretch sawtooth 0.5 1 0 1
  //  tcMod rotate 75
  //}
  //{
  //  clampmap textures/effects/jumpcirc.tga
  //  blendfunc add
  //  tcMod stretch sawtooth 0.5 1 0 1
  //  tcMod rotate -120
  //}
}

// textures/effects/jumpcircblue is the same as textures/ctf/blue_telep
// sawtooth stretch 0 values result in ugly clamping
textures/effects/jumpcircblue
{
  q3map_lightimage textures/effects/jumpcircblue.tga
  q3map_surfacelight 466
  {
    clampmap textures/effects/jumpcircblue.tga
    tcMod stretch sawtooth 0.5 1 0 1
    tcMod rotate 75
  }
  {
    clampmap textures/effects/jumpcircblue.tga
    blendfunc add
    tcMod stretch sawtooth 0.5 1 0 1
    tcMod rotate -120
  }
}

// textures/effects/jumpcircblue is the same as textures/ctf/blue_telep
// sawtooth stretch 0 values result in ugly clamping
textures/effects/jumpcircred
{
  q3map_lightimage textures/effects/jumpcircred.tga
  q3map_surfacelight 466
  {
    clampmap textures/effects/jumpcircred.tga
    tcMod stretch sawtooth 0.5 1 0 1
    tcMod rotate 75
  }
  {
    clampmap textures/effects/jumpcircred.tga
    blendfunc add
    tcMod stretch sawtooth 0.5 1 0 1
    tcMod rotate -120
  }
}

textures/ctf_unified/floor_decal_blue
{
  cull disable
  {
    map textures/symbols/blueteam.tga
    blendfunc add
  }
}

textures/ctf_unified/floor_decal_red
{
  cull disable
  {
    map textures/symbols/redteam.tga
    blendfunc add
  }
}

textures/ctf_unified/monologo_flash_blue
{
  cull disable
  {
    map textures/symbols/blueteam.tga
  }
}

textures/ctf_unified/monologo_flash_red
{
  cull disable
  {
    map textures/symbols/redteam.tga
  }
}

textures/base_wall/glass01
{
  surfaceparm trans
  cull disable
  {
    map textures/effects/tinfx.tga
    blendfunc add
    rgbGen identity
    tcGen environment
  }
  {
    map textures/detail/d_met2.tga
    blendfunc gl_dst_color gl_src_color
    tcMod scale 4 4
    detail
  }
  {
    map $lightmap
    blendfunc filter
    rgbGen identity
    tcGen lightmap
  }
}

redArmor
{
  {
    map models/powerups/armor/redarmor.tga
    rgbGen lightingDiffuse
  }
  {
    map gfx/fx/spec/spots.tga
    blendfunc gl_src_alpha gl_one
    rgbGen lightingDiffuse
    tcGen environment
    alphaGen lightingSpecular
  }
  {
    map gfx/fx/detail/d_met2.tga
    blendfunc gl_dst_color gl_src_color
    tcMod scale 4 4
    detail
  }
}

yellowArmor
{
  {
    map models/powerups/armor/yellowarmor.tga
    rgbGen lightingDiffuse
  }
  {
    map gfx/fx/spec/spots.tga
    blendfunc gl_src_alpha gl_one
    rgbGen lightingDiffuse
    tcGen environment
    alphaGen lightingSpecular
  }
  {
    map gfx/fx/detail/d_met2.tga
    blendfunc gl_dst_color gl_src_color
    tcMod scale 4 4
    detail
  }
}

textures/base_floor/metfloor1
{
  surfaceparm metalsteps
  {
    map textures/base_floor/metfloor1.tga
    rgbGen const ( 1 1 1 )
  }
  {
    clampmap textures/base_wall/chrome_env2.tga
    blendfunc add
    rgbGen const ( 0.188235 0.188235 0.188235 )
    tcGen environment
  }
  {
    map textures/base_floor/metfloor1.tga
    blendfunc filter
    rgbGen const ( 0.737255 0.737255 0.737255 )
  }
  {
    map $lightmap
    blendfunc filter
    tcGen lightmap
  }
}

textures/base_trim/tinfx
{
  {
    map textures/base_trim/tinfx.tga
    tcGen environment
  }
  {
    map $lightmap
    blendfunc filter
    tcGen lightmap
  }
}

console
{
    {
        map textures/sfx/logo256.tga
    }
}

grassobj
{
  cull disable
  {
    map models/mapobjects/out/grass.tga
    rgbGen lightingDiffuse
    alphaFunc GE128
  }
}

ameatygib
{
  {
    map models/gibs/genericgibmeat.tga
    rgbGen lightingDiffuse
  }
  {
    map gfx/fx/detail/d_sand.tga
    blendfunc gl_dst_color gl_src_color
    rgbGen lightingDiffuse
    tcMod scale 8 8
    alphaGen lightingSpecular
    detail
  }
  {
    map models/gibs/gibmeatspec.tga
    blendfunc gl_src_alpha gl_one
    rgbGen lightingDiffuse
    alphaGen lightingSpecular
    detail
  }
}

oalogo
{
  {
    map textures/oa/water.tga
    tcMod scroll 0.01 0.01
  }
  {
    map textures/oa/water.tga
    blendfunc add
    tcMod scroll 0.04 0.01
  }
  {
    map textures/oa/water.tga
    blendfunc add
    tcMod scroll -0.01 -0.02
  }
  {
    map textures/oa/flamelet.tga
    tcMod scroll -0.2 0.4
  }
  {
    map textures/oa/flamelet.tga
    blendfunc add
    tcMod scroll 0.2 0.7
  }
  {
    map textures/oa/flamelet.tga
    blendfunc add
    tcMod scroll 0 0.6
  }
}

//REALLY COOL MENU BACKGROUNDY
menubacknologo_blueish
{
  {
    map gfx/fx/detail/d_sand.tga
    rgbGen const ( 0.627451 0.666667 0.796079 )
    tcMod scroll 0.1 0.1
  }
  {
    map gfx/fx/detail/d_sand.tga
    blendfunc gl_dst_color gl_src_color
    rgbGen const ( 0.247059 0.803922 0.721569 )
    tcMod scroll -0.04 0.1
    tcMod scale -1.1 0.8
  }
  {
    map gfx/fx/detail/d_ice.tga
    blendfunc gl_dst_color gl_src_color
    tcMod scale 2 2
  }
  {
    map $whiteimage
    blendfunc filter
    rgbGen const ( 0.121569 0.12549 0.152941 )
    tcMod scale 0.5 1
  }
}

//REALLY COOL MENU BACKGROUNDY
menuback_blueish
{
  {
    map gfx/fx/detail/d_sand.tga
    rgbGen const ( 0.627451 0.666667 0.796079 )
    tcMod scroll 0.1 0.1
  }
  {
    map gfx/fx/detail/d_sand.tga
    blendfunc gl_dst_color gl_src_color
    rgbGen const ( 0.247059 0.803922 0.721569 )
    tcMod scroll -0.04 0.1
    tcMod scale -1.1 0.8
  }
  {
    map gfx/fx/detail/d_ice.tga
    blendfunc gl_dst_color gl_src_color
    tcMod scale 2 2
  }
  {
    map textures/sfx/logo256.tga
    blendfunc filter
    tcMod scale 0.5 1
  }
}

//REALLY COOL MENU BACKGROUNDY
menuback
{
  {
    map textures/map/black_d
    blendfunc add
  }
}

//REALLY COOL MENU BACKGROUNDY
menubacknologo
{
  {
    map $whiteimage
  }
}

boomzor
{
  deformVertexes autosprite
  {
    map textures/model/rocketball_explo_d.tga
    blendfunc add
  }
}

smokePuff
{
  cull disable
  nopicmip
  {
      clampmap textures/model/orb_trail_d.tga
    blendfunc add
    //rgbgen const ( 0.00 0.58 1.00 )
    rgbgen wave sin 1.0  1.0 0.0 0.35
  }
}

smokePuffRagePro
{
  cull disable
  {
    map gfx/misc/smokepuffragepro.tga
    blendfunc blend
    alphaGen Vertex
  }
}

hasteSmokePuff
{
  cull disable
  {
    map gfx/misc/hastesmoke.tga
    blendfunc blend
    tcMod rotate 64
    alphaGen Vertex
  }
}

shotgunSmokePuff
{
  cull disable
  {
    clampmap gfx/misc/smokepuff3.tga
    blendfunc blend
    tcMod rotate -45
    alphaGen entity
  }
}

sprites/plasma1
{
  {
    clampmap sprites/plasmaa.tga
    blendfunc gl_src_alpha gl_one
    tcMod rotate -145
  }
  {
    clampmap sprites/plasmaa.tga
    blendfunc gl_src_alpha gl_one
    tcMod rotate 177
  }
}

bfgshot
{
  cull disable
  deformVertexes autosprite
  {
    clampmap textures/oafx/bfgfx.tga
    blendfunc add
    tcMod rotate -700
  }
  {
    clampmap textures/oafx/bfgfx2.tga
    blendfunc add
    tcMod rotate 54
  }
  {
    clampmap textures/oafx/bfgfx2.tga
    blendfunc add
    tcMod rotate -14
  }
}

bfgsho
{
  cull disable
  deformVertexes autosprite
  {
    clampmap textures/oafx/bfgfx3.tga
    blendfunc add
    tcMod rotate 64
  }
}

// STUPID explosion thing introduced in q3's 1.30 patch
explode11
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode12
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode13
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode14
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode15
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode16
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode17
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode18
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode19
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode110
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode111
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode112
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode113
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode114
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode115
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode116
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode117
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode118
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode119
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode120
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode121
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode122
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode123
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}

explode124
{
  {
    map textures/map/black_d.tga
    blendfunc add
  }
}
