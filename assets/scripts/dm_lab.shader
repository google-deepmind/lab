textures/model/mothership_d
{
   qer_editorimage textures/model/mothership_d.tga
   //surfaceparm noimpact
   //surfaceparm nolightmap
   //surfaceparm trans
   {
      map textures/model/mothership_d.tga
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/map/facing_test
{
   qer_editorimage textures/map/mothership_trail.tga
   surfaceparm noimpact
   surfaceparm nolightmap
   //surfaceparm trans
   nomipmaps
   //cull back
   DeformVertexes autosprite
   {
      clampmap textures/map/mothership_trail.tga
      blendfunc add
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/map/decal_test
{
   qer_editorimage textures/map/facing_test.tga
   surfaceparm noimpact
   surfaceparm nolightmap
   //surfaceparm trans
   //DeformVertexes autosprite
   {
      map textures/map/facing_test.tga
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/map/fur_test
{
   qer_editorimage textures/map/facing_test.tga
   surfaceparm noimpact
   surfaceparm nolightmap
   q3map_fur 4 0.1 0.5
   {
      map textures/map/facing_test.tga
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/map/spec_test
{
   qer_editorimage textures/map/spec_test.tga
   {
      map $lightmap
      rgbGen identity
   }
   {
      map textures/model/fern_leaf_d.tga
      rgbGen identity
      blendFunc GL_DST_COLOR GL_SRC_ALPHA
   }
}

textures/map/additive_test
{
   qer_editorimage textures/map/mothership_trail.tga
   //surfaceparm noimpact
   //surfaceparm nolightmap
   //surfaceparm trans
   //DeformVertexes autosprite
   {
      clampmap textures/map/mothership_trail.tga
      blendfunc add
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/map/mothership_trail
{
   qer_editorimage textures/map/mothership_trail.tga
   surfaceparm noimpact
   surfaceparm nolightmap
   //surfaceparm trans
   nomipmaps
   //cull back
   DeformVertexes autosprite
   {
      clampmap textures/map/mothership_trail.tga
      blendfunc add
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/map/mothership_trail2
{
   qer_editorimage textures/map/mothership_trail.tga
   surfaceparm noimpact
   surfaceparm nolightmap
   //surfaceparm trans
   nomipmaps
   //cull back
   DeformVertexes autosprite2
   {
      clampmap textures/map/mothership_trail.tga
      blendfunc add
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/map/mothership_trail_flat
{
   qer_editorimage textures/map/mothership_trail.tga
   surfaceparm noimpact
   surfaceparm nolightmap
   //surfaceparm trans
   nomipmaps
   //cull back
   //DeformVertexes autosprite2
   {
      clampmap textures/map/mothership_trail.tga
      blendfunc add
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/model/grass_d2
{
   qer_editorimage textures/model/grass_d
   surfaceparm noimpact
   surfaceparm nolightmap
   surfaceparm trans
   surfaceparm alphashadow
   //nomipmaps
   cull none
   deformVertexes wave 5.5 sin 0.0 0.8 0.1 2.0
   {
      clampmap textures/model/grass_d.tga
      blendfunc blend
      depthfunc depthwrite
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/model/vine_d
{
   qer_editorimage textures/model/vine_d.tga
   surfaceparm noimpact
   surfaceparm nolightmap
   surfaceparm trans
   surfaceparm alphashadow
   //nomipmaps
   cull none
   deformVertexes wave 5.5 sin 0.0 0.8 0.1 2.0
   {
      clampmap textures/model/vine_d.tga
      blendfunc blend
      depthfunc depthwrite
      //rgbGen wave sin .5 .5 0 1.5
   }
}

textures/map/space_glass
{
   qer_editorimage textures/model/glass
   //surfaceparm noimpact
   surfaceparm nolightmap
   surfaceparm trans
   nomipmaps
   {
      map textures/map/fut_glass_env.tga
      blendfunc add
      tcgen environment
   }
   {
      clampmap textures/map/glass_d.tga
      blendfunc blend
      depthfunc depthwrite
   }
}

textures/model/ss_piping_lowerwall_01_d_BOB
{
   qer_editorimage textures/model/ss_piping_lowerwall_01_d.tga
   //surfaceparm noimpact
   surfaceparm nolightmap
   surfaceparm trans
   nomipmaps
   {
      map textures/map/fut_glass_env.tga
      blendfunc add
      tcgen environment
   }
   {
      clampmap textures/model/ss_piping_lowerwall_01_d.tga
      blendfunc gl_one gl_src_alpha //take alpha and multiply framebuffer by it
   }
}

textures/map/fire
{
   qer_editorimage textures/map/fire3.tga
   q3map_lightimage textures/map/fire3.tga
   surfaceparm trans
   surfaceparm nomarks
   surfaceparm nolightmap
   cull none
   nomipmaps
   q3map_surfacelight 1800
   DeformVertexes autosprite
   {
      animMap 8 textures/map/fire1.tga textures/map/fire2.tga textures/map/fire3.tga textures/map/fire4.tga textures/map/fire5.tga textures/map/fire6.tga textures/map/fire7.tga
      //map textures/map/fire1.tga
      //blendFunc GL_ONE GL_ONE
      blendfunc add
      rgbGen wave inverseSawtooth 0 1 0 10
   }
   {
      animMap 6 textures/map/fire3.tga textures/map/fire4.tga textures/map/fire5.tga textures/map/fire6.tga textures/map/fire7.tga textures/map/fire1.tga textures/map/fire2.tga
      //map textures/map/fire1.tga
      //blendFunc GL_ONE GL_ONE
      blendfunc add
      rgbGen wave inverseSawtooth 0 1 0.25 8
   }
      {
      animMap 2 textures/map/fire5.tga textures/map/fire6.tga textures/map/fire7.tga textures/map/fire1.tga textures/map/fire2.tga textures/map/fire3.tga textures/map/fire4.tga
      //map textures/map/fire1.tga
      //blendFunc GL_ONE GL_ONE
      blendfunc add
      rgbGen wave inverseSawtooth 0 1 0.5 12
   }
   {
      animMap 3 textures/map/fire7.tga textures/map/fire1.tga textures/map/fire2.tga textures/map/fire3.tga textures/map/fire4.tga textures/map/fire5.tga textures/map/fire6.tga
      //map textures/map/fire1.tga
      //blendFunc GL_ONE GL_ONE
      blendfunc add
      rgbGen wave inverseSawtooth 0 1 0.75 6
   }
}

textures/map/nodrop
{
   surfaceparm trans
   surfaceparm nomarks
   surfaceparm noimpact
   surfaceparm nolightmap
   surfaceparm nodrop
   surfaceparm nodraw
   surfaceparm nonsolid
}

textures/map/lensflare_01
{
   qer_editorimage textures/map/lensflare_01.tga
   surfaceparm trans
   surfaceparm nomarks
   surfaceparm nolightmap
   nomipmaps
   DeformVertexes autosprite
   {
      map textures/map/lensflare_01.tga

      blendfunc add
      //rgbGen wave inverseSawtooth 0 1 0 10
      rgbGen wave sin .5 .5 0 1.5
   }

}

textures/map/lensflare_02
{
   qer_editorimage textures/map/lensflare_02.tga
   surfaceparm trans
   surfaceparm nomarks
   surfaceparm nolightmap
   nomipmaps
   DeformVertexes autosprite
   {
      map textures/map/lensflare_02.tga

      blendfunc add
      rgbGen wave inverseSawtooth 0 1 0 1
   }

}

textures/map/lensflare_01_red
{
   qer_editorimage textures/map/lensflare_01_red.tga
   surfaceparm trans
   surfaceparm nomarks
   surfaceparm nolightmap
   nomipmaps
   DeformVertexes autosprite
   {
      map textures/map/lensflare_01_red.tga

      blendfunc add
      //rgbGen wave inverseSawtooth 0 1 0 10
      rgbGen wave square .5 1 0 0.6
   }

}

textures/map/lensflare_01_blue
{
   qer_editorimage textures/map/lensflare_01_blue.tga
   surfaceparm trans
   surfaceparm nomarks
   surfaceparm nolightmap
   nomipmaps
   DeformVertexes autosprite
   {
      map textures/map/lensflare_01_blue.tga

      blendfunc add
      //rgbGen wave inverseSawtooth 0 1 0 10
      rgbGen wave square .5 1 0 0.6
   }

}

textures/map/lensflare_01_green
{
   qer_editorimage textures/map/lensflare_01_green.tga
   surfaceparm trans
   surfaceparm nomarks
   surfaceparm nolightmap
   nomipmaps
   DeformVertexes autosprite
   {
      map textures/map/lensflare_01_green.tga

      blendfunc add
      //rgbGen wave inverseSawtooth 0 1 0 10
      rgbGen wave square .5 .5 0 1.5
   }

}

textures/map/lensflare_01_yellow
{
   qer_editorimage textures/map/lensflare_01_yellow.tga
   surfaceparm trans
   surfaceparm nomarks
   surfaceparm nolightmap
   nomipmaps
   DeformVertexes autosprite
   {
      map textures/map/lensflare_01_yellow.tga

      blendfunc add
      //rgbGen wave inverseSawtooth 0 1 0 10
      rgbGen wave triangle .5 .5 0 1.5
   }

}

textures/model/structural/ss_floor_plate_01_d
{
   qer_editorimage textures/model/structural/ss_floor_plate_01_d
   //surfaceparm noimpact
   surfaceparm nolightmap
   surfaceparm trans
   //surfaceparm alphashadow
   nomipmaps
   {
      clampmap textures/model/structural/ss_floor_plate_01_d
      blendfunc blend
      depthfunc depthwrite
   }
}

textures/map/decal_test_mult_d
{
   qer_editorimage textures/map/decal_splat_d.tga
   surfaceparm nodamage
   surfaceparm nolightmap
   surfaceparm nonsolid
   surfaceparm nomarks
   //surfaceparm trans
   //polygonoffset
   //q3map_backsplash 0 0   // Avoid point source lighting on face
   //q3map_surfacelight 400
   //q3map_lightsubdivide 64   // Finer subdevision
   {
      clampmap textures/map/decal_splat_d.tga
      blendfunc filter
      //rgbGen Vertex
      //tcMod rotate 45
   }
}

textures/map/decal_fan_anim
{
   qer_editorimage textures/map/fan_d.tga
   surfaceparm nodamage
   surfaceparm nolightmap
   surfaceparm nonsolid
   surfaceparm nomarks
   //surfaceparm trans
   //polygonoffset
   //q3map_backsplash 0 0   // Avoid point source lighting on face
   //q3map_surfacelight 400
   //q3map_lightsubdivide 64   // Finer subdivision
   nomipmaps
   {
      clampmap textures/map/fan_blades_d.tga
      blendfunc add
      tcMod rotate -45
   }
   {
      clampmap textures/map/fan_guard_d.tga
      blendfunc gl_dst_color gl_one
   //   //tcMod rotate 45
   }
   {
      map textures/map/fan_volumetric_d.tga
      blendfunc gl_dst_color gl_one
      //tcMod turb 0.1 0.02 0 0.5
   }
}

textures/map/light_shaft
{
   qer_editorimage textures/map/light_shaft.tga
   surfaceparm nodamage
   surfaceparm nolightmap
   surfaceparm nonsolid
   surfaceparm nomarks
   //surfaceparm trans
   //polygonoffset
   //q3map_backsplash 0 0   // Avoid point source lighting on face
   //q3map_surfacelight 400
   //q3map_lightsubdivide 64   // Finer subdivision
   nomipmaps
   {
      map textures/map/light_shaft.tga
      blendfunc add
      //tcMod turb 0.1 0.02 0 0.5
      tcmod scroll 0.6 -0.1
   }

}

textures/map/computer_screen_d
{
   qer_editorimage textures/map/computer_screen_d.tga
   surfaceparm nodamage
   surfaceparm nolightmap
   surfaceparm nonsolid
   surfaceparm nomarks
   //surfaceparm trans
   //polygonoffset
   //q3map_backsplash 0 0   // Avoid point source lighting on face
   //q3map_surfacelight 400
   //q3map_lightsubdivide 64   // Finer subdivision
   nomipmaps
   {
      map textures/map/computer_text_d.tga
      //blendfunc add
      //tcMod turb 0.1 0.02 0 0.5
      tcmod scroll 0.0 0.2
   }
   {
      map textures/map/computer_screen_d.tga
      blendfunc filter
   }

}

textures/map/beam_d
{
   qer_editorimage textures/map/beam_d.tga
   surfaceparm nodamage
   surfaceparm nolightmap
   surfaceparm nonsolid
   surfaceparm nomarks
   //surfaceparm trans
   //polygonoffset
   //q3map_backsplash 0 0   // Avoid point source lighting on face
   //q3map_surfacelight 400
   //q3map_lightsubdivide 64   // Finer subdivision
   deformVertexes autosprite2
   nomipmaps
   {
      map textures/map/beam_d.tga
      blendfunc add
      tcMod turb 0.5 0.01 0 3
      tcmod scroll 0.0 1.2
   }
   {
      map textures/map/beam_d.tga
      blendfunc add
      tcMod turb 0.7 0.02 0 5
      //tcmod scroll 0.0 -1.0
   }
}

textures/map/kingofthehill
{
    surfaceparm nodamage
    surfaceparm nomarks
    nomipmaps
    {
        map textures/map/grey_clang.jpg
        rgbGen identity
    }
    {
        map $whiteimage
        rgbGen const ( 0 1.0 0 )
        blendfunc filter
    }
}

textures/map/red_team_goal
{
    surfaceparm nodamage
    surfaceparm nomarks
    nomipmaps
    {
        map textures/map/grey_clang.jpg
        rgbGen identity
    }
    {
        map $whiteimage
        rgbGen const ( 1.0 0 0 )
        blendfunc filter
    }
}

textures/map/blue_team_goal
{
    surfaceparm nodamage
    surfaceparm nomarks
    nomipmaps
    {
        map textures/map/grey_clang.jpg
        rgbGen identity
    }
    {
        map $whiteimage
        rgbGen const ( 0 0 1.0 )
        blendfunc filter
    }
}

textures/map/invis
{
     surfaceparm trans
     surfaceparm noimpact
     surfaceparm nomarks
     {
         map $whiteimage
         blendfunc blend
         alphaGen const 0.0
     }
}

textures/model/fern_test
{
    surfaceparm nomarks
    deformVertexes move 0.2 0.2 0.2 sin 0.25 1.5 0 2
    {
       map $whiteimage //replace with model texture
       rgbGen const ( 0.1 0.1 0.1 ) //replace numbers with randoms (0-0.2)
    }


}



textures/map/red_glow
{
    surfaceparm nodamage
    surfaceparm nomarks
    nomipmaps
    q3map_lightRGB 1 0 0
    q3map_surfacelight 1500
    q3map_lightsubdivide 32
    {
        //map $whiteimage
        //rgbGen const ( 0 1 0 )
        map textures/map/red_glow.tga
    }
}

textures/map/blue_glow_pulse
{
    surfaceparm nodamage
    surfaceparm nomarks
    nomipmaps
    q3map_lightRGB 0 0 1
    q3map_surfacelight 1500
    q3map_lightsubdivide 32
    q3map_lightStyle 2
    //q3map_colorGen wave 6.0 sin 0.0 2.0 0.1 1.1
    {
        map $whiteimage
        rgbGen const ( 0 0 1 )
    }
}

textures/map/green_glow_pulse
{
    surfaceparm nodamage
    surfaceparm nomarks
    nomipmaps
    q3map_lightRGB 0 1 0
    q3map_surfacelight 1500
    q3map_lightsubdivide 32
    q3map_lightStyle 2
    //q3map_colorGen wave 6.0 sin 0.0 2.0 0.1 1.1
    {
        map $whiteimage
        rgbGen const ( 0 1 0 )
    }
}

textures/map/fut_utility_panel_hightess
{
    nomipmaps
    tessSize 32
    {
        map textures/map/fut_utility_panel_01_d.tga
        rgbGen vertex
    }

}

textures/map/alpha_000
{
   qer_editorimage textures/map/black_d.tga
   q3map_alphaMod volume
   q3map_alphaMod set 0.0
   surfaceparm nodraw
   surfaceparm nonsolid
   surfaceparm trans
   qer_trans 0.7
}

textures/map/alpha_001
{
   qer_editorimage textures/map/white_d.tga
   q3map_alphaMod volume
   q3map_alphaMod set 1.0
   surfaceparm nodraw
   surfaceparm nonsolid
   surfaceparm trans
   qer_trans 0.7
}

textures/map/terrain_a
{
   qer_editorimage textures/map/beach_sand_d.tga
   q3map_tcGen ivector ( 128 0 0 ) ( 0 128 0 )
   {
      map textures/map/hedge_grass_d.tga
      rgbGen identity
   }
   {
      map textures/map/beach_sand_d.tga
      blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
      alphaGen vertex
      rgbGen identity
   }
   {
      map $lightmap
      blendFunc GL_DST_COLOR GL_ZERO
      rgbGen identity
   }
}

textures/map/terrain_bc
{
  //Used for radiosity lighting
  //q3map_lightImage textures/map/hedge_grass_d.tga

  q3map_nonplanar
  q3map_shadeAngle 180
  q3map_tcGen ivector ( 128 0 0 ) ( 0 128 0 )
  q3map_tcMod rotate 33
  q3map_lightmapAxis z

  // this means dot product squared, for faster falloff between vertical and horizontal planes
  q3map_alphaMod dotproduct2 ( 0 0 0.95 )

  //surfaceparm nonsolid
  //surfaceparm pointlight

  {
    map textures/map/hedge_grass_d.tga
    rgbGen identity
  }
  {
    map textures/map/beach_sand_d.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    rgbGen identity
  }
        {
                map $lightmap
                blendFunc GL_DST_COLOR GL_ZERO
                rgbGen identity
        }
}

textures/map/terrain_b
{
   //q3map_nonplanar
   q3map_shadeangle 10
   {
     map textures/map/beach_sand_d.tga
     //rgbGen vertex
   }
}

textures/model/bark_jungle_d
{
   q3map_shadeAngle 180
   {
   map textures/model/bark_jungle_d.tga
   blendfunc add
   rgbGen vertex
   }
}

textures/map/terrain_d
{
   qer_editorimage textures/map/soil_damp_01_d.tga
   q3map_tcGen ivector ( 128 0 0 ) ( 0 128 0 )
   q3map_shadeangle 180
   q3map_forcesunlight
   {
      map textures/map/soil_damp_01_d.tga
      rgbGen identity
   }
   {
      map textures/map/hedge_grass_d.tga
      blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
      alphaGen vertex
      rgbGen identity
   }
   {
      map textures/map/clouds_d.tga
      blendfunc filter
      tcmod scroll 0.6 -0.1
   }
   {
      map $lightmap
      blendFunc GL_DST_COLOR GL_ZERO
      rgbGen identity
   }
}

textures/map/terrain_e
{
   qer_editorimage textures/map/hedge_grass_d.tga
   q3map_tcGen ivector ( 128 0 0 ) ( 0 128 0 )
   q3map_shadeangle 180
   q3map_forcesunlight
   {
      map textures/map/hedge_grass_d.tga
      rgbGen identity
   }
   {
      map textures/map/jungle_floor_roots_d.tga
      blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
      alphaGen vertex
      rgbGen identity
   }
   {
      map textures/map/clouds_d.tga
      blendfunc filter
      tcmod scroll 0.6 -0.1
   }
   {
      map $lightmap
      blendFunc GL_DST_COLOR GL_ZERO
      rgbGen identity
   }
}

textures/map/ocean
{
       qer_editorimage textures/map/fog.tga
       surfaceparm trans
       surfaceparm nonsolid
       surfaceparm fog
       surfaceparm nolightmap
       fogparms ( .2 .65 .73 ) 384
       sort 2
       {
                map textures/map/water_env.tga
                blendfunc add
                tcgen environment
       }
}

textures/map/river
{
       qer_editorimage textures/map/fog.tga
       surfaceparm trans
       surfaceparm nonsolid
       surfaceparm fog
       surfaceparm nolightmap
       surfaceparm water
       fogparms ( .01 .1 .24 ) 256
       sort 7
       {
                map textures/map/river_d.tga
                blendfunc blend
                rgbGen identity
                //tcMod scroll 0.06 0.1
                //tcMod turb 0.5 0.01 0 3
       }
       {
                map textures/map/water_env.tga
                blendfunc add
                tcgen environment
       }
       {
                map $lightmap
                blendFunc GL_DST_COLOR GL_ZERO
                rgbGen identity
       }
}

textures/map/rivertop
{
       qer_editorimage textures/map/river_d.tga
       surfaceparm trans
       surfaceparm nonsolid
       sort 8
       {
                map textures/map/river_d.tga
                blendfunc blend
                rgbGen vertex
                //alphaGen const 0.5
                tcMod scroll 0.4 0.02
                tcMod turb 0.025 0.025 0 0.05
       }
       {
                map textures/map/river_d.tga
                blendfunc blend
                tcMod scroll 0.3 0.01
                tcMod turb 0.07 0.07 0 0.05
       }
       {
                map textures/map/ref_test.tga
                blendfunc add
                tcgen environment
       }
       {
                map $lightmap
                blendFunc GL_DST_COLOR GL_ZERO
                rgbGen identity
       }
}

textures/map/lake
{
       qer_editorimage textures/map/river_d.tga
       surfaceparm trans
       surfaceparm nonsolid
       surfaceparm nolightmap
       cull front
       //cull back
       nomipmaps
       //sort 8
       {
                map textures/map/river_d.tga
                blendfunc blend
                //rgbGen vertex
                //alphaGen const 0.5
                tcMod scale 0.8 0.7
                tcMod turb 1.0 0.03 0.0 0.12
                tcMod scroll -0.02 -0.01
       }
       {
                map textures/map/river_d.tga
                blendfunc blend
                tcMod scale 1 1.2
                tcMod turb 1.0 -0.04 0.5 0.14
                tcMod scroll 0.02 0.01
       }
       {
                map textures/map/river_d.tga
                blendfunc blend
                tcMod scale 1.1 1.2
                tcMod turb 1.0 0.025 1.0 0.08
                tcMod scroll -0.01 0.01
       }
       {
                map textures/map/river_d.tga
                blendfunc blend
                tcMod scale 1.5 1.7
                tcMod turb 1.0 0.01 1.5 0.045
                tcMod scroll -0.01 0.01
       }
       {
                map textures/map/sky_ref_d.tga
                blendfunc add
                tcgen environment
       }
       {
                map $lightmap
                blendFunc GL_DST_COLOR GL_ZERO
                rgbGen identity
       }
}

textures/map/ocean2
{
       qer_editorimage textures/map/beach_sand_d.tga
       q3map_tcGen ivector ( 128 0 0 ) ( 0 128 0 )
       surfaceparm trans
       surfaceparm nonsolid
       surfaceparm fog
       surfaceparm nolightmap
       fogparms ( .2 .65 .73 ) 256
       {
                map textures/map/beach_sand_d.tga
                rgbGen identity
       }
       {
                map textures/map/glass_d.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
                alphaGen vertex
                rgbGen identity
       }
       {
                map $lightmap
                blendFunc GL_DST_COLOR GL_ZERO
                rgbGen identity
       }
}

textures/map/caustics
{
     surfaceparm nonsolid
     surfaceparm nolightmap
     {
          map textures/map/caustics_d.tga
          blendfunc add
          tcMod scale 0.7 0.7
          //tcMod scroll 0.06 0.1
          tcMod stretch sin 0.3 0.02 0 0.1
          tcMod turb 0 0.04 0 0.12
     }
     {
          map textures/map/caustics_d.tga
          blendfunc add
          tcMod scale 0.5 0.5
          //tcMod scroll 0.1 0.06
          tcMod stretch sin 0.2 0.04 0 0.2
          tcMod turb 0 0.06 0.5 0.1
     }
}

textures/map/fog_blue
{
    qer_editorimage textures/map/fog.tga
    surfaceparm trans
    surfaceparm nonsolid
    surfaceparm fog
    surfaceparm nolightmap
    fogparms ( .0 .12 .5 ) 1024
}

textures/map/fog_sky
{
    qer_editorimage textures/map/fog.tga
    surfaceparm trans
    surfaceparm nonsolid
    surfaceparm fog
    surfaceparm nolightmap
    //fogparms ( .44 .67 0.82 ) 4096
    fogparms ( .65 .8 0.89 ) 16384
}

textures/model/palm_tree_d
{
     qer_editorimage textures/model/palm_tree_d.tga
     //surfaceparm trans
     cull none
     sort 6
     //deformVertexes wave 6.0 sin 0.0 2.0 0.1 1.1
     {
          map textures/model/palm_tree_d.tga
          blendfunc blend
     }
     //{
          //map $whiteimage
          //rgbGen const ( 0.2 0.2 0.2)
          //blendfunc add
     //}
}



textures/model/fir_leaves_d
{
     qer_editorimage textures/model/fir_leaves_d.tga
     cull none
     deformVertexes wave 6.0 sin 0.0 2.0 0.1 1.1
     {
          map textures/model/fir_leaves_d.tga
          //rgbGen identity
          rgbGen vertex
     }
     {
          map $lightmap
          blendFunc GL_DST_COLOR GL_ZERO
          rgbGen identity
     }
}

textures/model/jungle_tree_canopy_d
{
     qer_editorimage textures/model/jungle_tree_canopy_d.tga
     cull none
     //deformVertexes wave 6.0 sin 0.0 2.0 0.1 1.1
     {
          map textures/model/jungle_tree_canopy_d.tga
          rgbGen identity
          //rgbGen vertex
     }
     {
          map $lightmap
          blendFunc GL_DST_COLOR GL_ZERO
          rgbGen identity
     }
}

textures/map/fog_jungle
{
    qer_editorimage textures/map/fog.tga
    surfaceparm trans
    surfaceparm nonsolid
    surfaceparm fog
    surfaceparm nolightmap
    fogparms ( .59 .59 .3 ) 8192
}

textures/model/jungle_foliage_02_d
{
     qer_editorimage textures/model/jungle_foliage_02_d
     cull none
     //deformVertexes wave 6.0 sin 0.0 2.0 0.1 1.1
     {
          map textures/model/jungle_foliage_02_d
          //rgbGen identity
          rgbGen vertex
     }
     {
          map $lightmap
          blendFunc GL_DST_COLOR GL_ZERO
          rgbGen identity
     }
}

textures/map/terrain_forest2
{
   qer_editorimage textures/map/soil_damp_01_d.tga
   q3map_tcGen ivector ( 128 0 0 ) ( 0 128 0 )
   q3map_shadeangle 180
   q3map_forcesunlight
   {
      map textures/map/soil_damp_01_d.tga
      rgbGen identity
   }
   {
      map textures/map/hedge_grass_d.tga
      blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
      alphaGen vertex
      rgbGen identity
   }
   {
      map textures/map/clouds_d.tga
      blendfunc filter
      tcmod scroll 0.6 -0.1
   }
   {
      map $lightmap
      blendFunc GL_DST_COLOR GL_ZERO
      rgbGen identity
   }
}

textures/map/lr2_barrier_d
{
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm noimpact
        surfaceparm nomarks
        cull front
        {
                map textures/map/lr2_barrier_d.tga
                blendfunc add
        }
}

textures/map/terrain_forest
{
        qer_editorimage textures/map/hedge_grass_d.tga
  q3map_shadeAngle 60
  q3map_tcGen ivector ( 1024 0 0 ) ( 0 1024 0 )
  q3map_lightmapAxis z
        q3map_lightmapmergable
        nomipmaps
        q3map_nonPlanar
        //q3map_lightmapSampleSize 256
        q3map_alphaMod dotproduct2 ( 0.0 0.0 0.95 )
  {
    map textures/map/soil_damp_01_d.tga
    rgbGen identity
  }
        {
    map textures/map/hedge_grass_d.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    rgbGen identity
                alphaGen vertex
  }
        {
                map textures/map/light_shadow.tga
                blendFunc filter
                tcmod scale 2 2
                //tcGen ivector ( 512 0 0 ) ( 0 512 0 )
                //rgbGen identity
        }
        {
                map textures/map/scrolling_clouds_shadow.tga
                blendFunc filter
                //tcmod scale 0.07 0.07
                //tcGen ivector ( 512 0 0 ) ( 0 512 0 )
                tcmod scroll 0.05 0.0
                //rgbGen identity
        }
        {
                map $lightmap
                blendFunc GL_DST_COLOR GL_ZERO
                rgbGen identity
        }
}

textures/map/terrain_desert2
{
        q3map_lightmapmergable
        q3map_nonPlanar
        q3map_lightmapSampleSize 256
        qer_editorimage textures/map/lr2_desert_ground_d.tga
  {
    map textures/map/lr2_desert_ground_d.tga
    rgbGen vertex
  }
}



textures/map/cheap_sky_d
{
        q3map_sun 0.6 0.55 0.4 280 235 80
        q3map_surfacelight 10
        //tessSize 4096
        //surfaceparms nolightmap
  {
    map textures/map/cheap_sky_d.tga
    rgbGen identity
    //rgbGen vertex
  }
}

//textures/map/terrain_havok
textures/map/terrain_desert3
{
    qer_editorimage textures/map/lr2_desert_ground_d.tga
    q3map_surfaceModel models/cactus.md3 8 0.5 0.8 1.2 -10 10 0
    {
    map textures/map/lr2_desert_ground_d.tga
    }
}

textures/map/terrain3
{
  q3map_terrain
  qer_editorimage textures/map/soil_damp_01_d.tga
  surfaceparm nodraw
  surfaceparm nomarks
  surfaceparm nolightmap
}

textures/map/terrain4
{
  qer_editorimage textures/map/soil_damp_01_d.tga
  surfaceparm dust
  surfaceparm nodraw
  surfaceparm nomarks
  surfaceparm nolightmap
  {
    map textures/map/soil_damp_01_d.tga
  }
}

textures/map/fur_grass
{
  q3map_fur 4 8 0.5
  {
  map textures/map/soil_damp_01_d.tga
  }
}



//Terrain generic bits

textures/map/alpha_scale
{
   qer_editorimage textures/map/black_d.tga
   q3map_alphaMod volume
   //q3map_alphaMod set 0.0
   q3map_alphaMod scale 50
   surfaceparm nodraw
   surfaceparm nonsolid
   surfaceparm trans
   qer_trans 0.5
}


//misc tests ********************************************************************************



textures/map/sky_clouds
{
     qer_editorimage textures/map/scrolling_clouds_d.tga
     surfaceparm trans
     //q3map_tessSize 16
     nopicmip
     surfaceparm nomarks
     surfaceparm noclip
     surfaceparm nolightmap
     {
          map textures/map/scrolling_clouds_d.tga
          blendfunc add
          //rgbGen identity
          //alphaFunc GE128
          tcmod scroll 0.02 0.0
     }
}

textures/map/test
{
   noPicMip
   surfaceparm nolightmap
   surfaceparm nomarks
   surfaceparm nonsolid
   surfaceparm trans
   qer_alphaFunc gequal 0.5
   polygonOffset
   //surfaceparm detail
   {
      map textures/map/test.tga
      alphaFunc GE128
      depthWrite
   }
   {
      map $lightmap
      blendFunc filter
      depthFunc equal
   }
}

textures/map/terrain_test
{
        qer_editorimage textures/map/desert_sandground_01_d.tga
        q3map_globalTexture
        q3map_terrain //try this soon
  //q3map_lightmapfilterradius 2 0 //4 0 is good //this slows down the build alot
        q3map_shadeAngle 30
  q3map_tcGen ivector ( 256 0 0 ) ( 0 256 0 )
        nomipmaps
        q3map_nonPlanar //new version of lightmapmergable but it works
        q3map_lightmapSampleSize 4 // number of sample per unit (4 is good) desert_biome_256 takes 300s+ with 32 samples
        q3map_alphaMod dotproduct2 ( 0.0 0.0 0.95 ) // for stark land
  {
    map textures/map/desert_rockface_01_d.tga
    //map textures/map/desert_sandground_01_d.tga
    //map textures/map/blend_test_texture1_d.tga
    rgbGen vertex
                //rgbGen identity
  }
  {
    map textures/map/desert_sandground_01_d.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    //rgbGen vertex
    alphaGen vertex
  }
        {
                map textures/map/scrolling_clouds_shadow.tga
                blendFunc filter
                //tcmod scale 0.07 0.07
                tcGen vector ( 256 0 0 ) ( 0 256 0 )
                tcmod scroll 0.075 0.0
                //rgbGen identity
        }
        {
                map $lightmap
                blendFunc GL_DST_COLOR GL_ZERO
                rgbGen identity
        }

}
textures/map/terrain_desert_old
{
        qer_editorimage textures/map/desert_sandground_01_d.tga
  q3map_shadeAngle 90
  q3map_tcGen ivector ( 256 0 0 ) ( 0 256 0 )
  q3map_lightmapAxis z
        q3map_lightmapmergable
        nomipmaps
        q3map_nonPlanar
        q3map_lightmapSampleSize 16
        //q3map_lightmapSampleSize 1
        q3map_alphaMod dotproduct2 ( 0.0 0.0 0.95 ) // for flat land
  {
    map textures/map/desert_rockface_01_d.tga //blend_test_texture1_d
    rgbGen vertex
  }
  {
    map textures/map/desert_sandground_01_d.tga //blend_test_texture2_d
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    alphaFunc GE128
    rgbGen vertex
    //alphaGen vertex
  }
        {
                map textures/map/med_shadow.tga
                blendFunc filter
                tcGen vector ( 0.0025 0 0 ) ( 0 0.0025 0 )
                //rgbGen identity
        }
        {
        //        map textures/map/med_shadow.tga
                map textures/map/scrolling_clouds_shadow.tga
                blendFunc filter
                //tcmod scale 0.07 0.07
                tcGen vector ( 1 0 0 ) ( 0 1 0 )
                //tcmod scroll 0.075 0.0
                //rgbGen identity
        }
        {
                map $lightmap
                blendFunc GL_DST_COLOR GL_ZERO
                rgbGen identity
        }
}




//Desert Biome  ******************************************************************************

textures/map/fog_haze
{
    qer_editorimage textures/map/fog.tga
    surfaceparm trans
    surfaceparm nonsolid
    surfaceparm fog
    surfaceparm nolightmap
    fogparms ( .56 .63 .71 ) 65536
}

textures/map/fog_grey
{
    qer_editorimage textures/map/fog.tga
    surfaceparm trans
    surfaceparm nonsolid
    surfaceparm fog
    surfaceparm nolightmap
    fogparms ( .3 .3 .3 ) 1000
}

textures/map/fog_deepwater
{
    qer_editorimage textures/map/fog.tga
    surfaceparm trans
    surfaceparm nonsolid
    surfaceparm water
    surfaceparm fog
    surfaceparm nolightmap
    fogparms ( .05 .14 .27 ) 768
}

textures/map/col
{
    qer_trans 0.5
    surfaceparm trans
    surfaceparm nodraw
    surfaceparm nomarks
    //surfaceparm noimpact
    surfaceparm nolightmap
}

textures/map/terrain_desert
{
        qer_editorimage textures/map/desert_sandground_01_d.tga
        q3map_globalTexture
        q3map_terrain //try this soon
        //q3map_lightmapfilterradius 2 0 //4 0 is good //this slows down the build alot
        q3map_shadeAngle 25
        q3map_tcGen ivector ( 256 0 0 ) ( 0 256 0 )
        //nomipmaps
        q3map_nonPlanar //new version of lightmapmergable but it works
        q3map_lightmapSampleSize 16 // number of sample per unit (4 is good) desert_biome_256 takes 300s+ with 32 samples
        q3map_alphaMod dotproduct2 ( 0.0 0.0 0.95 ) // for stark land
  {
    map textures/map/desert_rockface_01_d.tga
    //map textures/map/desert_sandground_01_d.tga
    //map textures/map/blend_test_texture1_d.tga
    rgbGen vertex
    //rgbGen identity
  }
  {
    map textures/map/desert_sandground_01_d.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    //rgbGen vertex
    alphaGen vertex
  }
        {
                map $lightmap
                blendFunc GL_DST_COLOR GL_ZERO
                rgbGen identity
        }
}

textures/map/desert_sandground_01_d
{
  qer_editorimage textures/map/desert_sandground_01_d.tga
  q3map_shadeAngle 60
  q3map_globalTexture
  q3map_terrain
  q3map_tcGen ivector ( 256 0 0 ) ( 0 256 0 )
  nopicmip
  q3map_nonPlanar
  q3map_lightmapSampleSize 1
  {
    map textures/map/desert_sandground_01_d.tga
    rgbGen identity
  }
  {
    map $lightmap
    blendFunc GL_DST_COLOR GL_ZERO
    rgbGen identity
  }
}

textures/model/bush_desert_01_d
{
     qer_editorimage textures/model/bush_desert_01_d.tga
     //surfaceparm trans
     cull none
     surfaceparm alphashadow
     deformVertexes wave 1.0 triangle 0.0 1.0 0.0 1.8
     q3map_vertexScale 2.5
     {
          map textures/model/bush_desert_01_d.tga
          blendfunc blend
          alphaFunc GE128
          //alphaFunc GT0
          depthWrite
          rgbGen vertex
          //rgbGen identity
     }

}

textures/model/bush_desert_02_d
{
     qer_editorimage textures/model/bush_desert_02_d.tga
     surfaceparm trans
     cull none
     nomipmaps
     surfaceparm alphashadow
     deformVertexes wave 1.0 triangle 0.0 1.0 0.0 1.8
     q3map_vertexScale 2.5
     {
          map textures/model/bush_desert_02_d.tga
          blendfunc blend
       alphaFunc GE128
          //alphaFunc GT0
          depthWrite
          rgbGen vertex
          //rgbGen identity
     }

}

textures/model/bush_desert_03_d
{
     qer_editorimage textures/model/bush_desert_03_d.tga
     surfaceparm trans
     cull none
     surfaceparm alphashadow
     //deformVertexes wave 1.0 triangle 0.0 1.0 0.0 1.8
     q3map_vertexScale 1.5
     {
          map textures/model/bush_desert_03_d.tga
          blendfunc blend
          alphaFunc GE128
          depthWrite
          rgbGen vertex
     }

}

textures/model/cactus_d2
{
     //q3map_LightMapSampleOffset 1.2
     q3map_lightmapSampleSize 1024
     qer_editorimage textures/model/cactus_d.tga
     {
          map $lightmap
          //blendFunc GL_DST_COLOR GL_ZERO
          rgbGen identity
          rgbgen lightingDiffuse
     }
     //{
          //map textures/model/cactus_d.tga
          //blendfunc filter
          //rgbgen identity
          //rgbGen const ( 1 1 1 )
     //}
}

textures/model/rock_desert_d
{
     nomipmaps
     qer_editorimage textures/model/rock_desert_d.tga
     {
          map textures/model/rock_desert_d.tga
          rgbgen vertex
     }
}


textures/models/players/crash_death
{
     qer_editorimage textures/model/bush_desert_03_d.tga
     surfaceparm trans
     cull none
     surfaceparm alphashadow
     //deformVertexes wave 1.0 triangle 0.0 1.0 0.0 1.8
     q3map_vertexScale 1.5
     {
          map textures/model/bush_desert_03_d.tga
          blendfunc blend
          alphaFunc GE128
          depthWrite
          rgbGen vertex
     }

}

textures/model/mushroom_desert_white
{
     qer_editorimage textures/model/mushroom_desert_white_d.tga
     {
          map textures/model/mushroom_desert_white_d.tga
          rgbgen vertex
     }
}

textures/model/mushroom_desert_yellow
{
     qer_editorimage textures/model/mushroom_desert_yellow_d.tga
     {
          map textures/model/mushroom_desert_yellow_d.tga
          rgbgen vertex
     }
}

textures/model/mushroom_desert_brown
{
     qer_editorimage textures/model/mushroom_desert_brown_d.tga
     {
          map textures/model/mushroom_desert_brown_d.tga
          rgbgen vertex
     }
}

//***************************************WEAPON STUFF
textures/model/rocketball_d
{
    {
        map textures/model/rocketball_d.tga
        //rgbGen lightingDiffuse
        blendfunc add
    }
}


textures/model/scotspine
{
  cull none
  {
    map textures/model/scotspine.tga
    blendfunc blend
    rgbgen identity
  }

}

textures/map/color_key_door
{
    qer_editorimage textures/map/color_key_door.tga
    {
        map textures/map/color_key_door.tga
        blendfunc blend
    }
}


textures/map/fut_door_d
{
    qer_editorimage textures/map/fut_door_d.tga
    {
        map textures/map/fut_door_d.tga
        rgbgen vertex
    }
}
