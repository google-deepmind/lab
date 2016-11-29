invisItem
{
  cull disable
  {
    map textures/effects/tinfx.tga
    blendfunc add
    rgbGen wave sin 0.3 0.4 0 0.5
    tcGen environment
  }
}

quadDamage
{
  {
    map textures/oafx/quadshell.tga
    tcMod scroll 0.5 0.5
    tcGen environment
  }
  {
    map textures/sfx/specular.tga
    blendfunc add
    tcGen environment
  }
}

teleportEffect
{
  //deformVertexes autosprite
  {
    //clampmap models/powerups/instant/quadlite.tga
    map textures/model/spawn_d.tga
    blendfunc add
    rgbGen wave inversesawtooth 0 1 0 2
    tcMod scroll 0 1
    //tcMod rotate 85
    //tcMod stretch sawtooth 0 1.5 0 2
  }
}

quadAura
{
  deformVertexes autosprite
  {
    clampmap models/powerups/instant/quadlite.tga
    blendfunc add
    tcMod rotate 6
    tcMod stretch sin 1 0.1 0 1
  }
  {
    clampmap models/powerups/instant/quadlite2.tga
    blendfunc add
    tcMod rotate -8
    tcMod stretch sin 1 -0.1 0 1
  }
}

//medkit
models/powerups/holdable/medkit_base
{
  {
    map textures/base_wall/chrome_env.jpg
    tcGen environment
    rgbGen lightingdiffuse
    tcmod scale .5 .5
  }
}

models/powerups/holdable/medkit_plunger
{
  {
    map textures/effects/envmapdimb.jpg
    tcgen environment
    blendfunc add
  }
}

models/powerups/holdable/medkit_slime
{
  {
    map textures/liquids/slime9.jpg
    tcmod scroll 1 1
    blendfunc add
  }
}

//personal teleporter
models/powerups/holdable/teleport_center
{
  {
    map textures/base_wall/chrome_env.jpg
    tcgen environment
    rgbGen lightingdiffuse
    tcmod scale .5 .5
  }
}

models/powerups/holdable/teleport_outer
{
  {
    map textures/liquids/lavafloor.jpg
    tcmod scroll .5 .5
    blendfunc add
  }
}

//enviro suit
models/powerups/instant/ebase
{
  {
    map textures/effects/envmapgold.jpg
    tcgen environment
    rgbGen vertex
  }
}

models/powerups/instant/ering
{
  {
    map textures/effects/envmapgold.jpg
    tcgen environment
    rgbgen vertex
  }
}
//regeneration
rcad
{
  {
    map textures/effects/regenmap2.jpg
    tcgen environment
    rgbgen vertex
  }
}

rring
{
  {
    map textures/effects/regenmap2.jpg
    tcgen environment
    rgbgen vertex
  }
}

//haste
models/powerups/instant/h_center
{
  {
    map textures/effects/hastenv.jpg
    tcgen environment
    rgbgen vertex
  }
}

models/powerups/instant/h_ring
{
  {
    map textures/effects/hastenv.jpg
    tcgen environment
    rgbgen vertex
  }
}

//flight
models/powerups/instant/flight_body
{
  {
    map textures/effects/flightenv.jpg
    tcgen environment
    rgbgen vertex
  }
}

models/powerups/instant/flight_ring
{
  {
    map textures/effects/flightenv.jpg
    tcgen environment
    rgbgen vertex
  }
}

models/powerups/armor/shard
{
  {
    map models/powerups/armor/shard_env.jpg
    tcgen environment
  }
}

models/powerups/armor/shard_env
{
  {
    map models/powerups/armor/shard_env.jpg
    blendfunc add
    rgbgen wave sin .2 .5 0 .5
    tcgen environment
  }
}

//gibs ;-p
models/gibs/heart
{
  deformvertexes wave 20 sin 1.4 .3 0 1.2
  {
    map models/gibs/heart.tga
    rgbGen lightingDiffuse
  }
}

models/gibs/veins
{
  {
    map models/gibs/veins.tga
    alphafunc GE128
    depthwrite
    rgbGen lightingDiffuse
  }
}

models/gibs/splurt
{
  {
    map models/gibs/splurt.tga
    alphafunc GE128
    depthwrite
    tcmod scroll 0 3
    rgbGen lightingDiffuse
  }
}

models/gibs/f_veins
{
  {
    map models/gibs/f_veins.tga
    alphafunc GE128
    depthwrite
    rgbGen lightingDiffuse
  }
}
