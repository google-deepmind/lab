textures/model/fut_teleport_d
{
      qer_editorimage textures/model/fut_teleport_d.tga
      surfaceparm metalsteps
      {
          map textures/map/water_env.tga
          blendfunc add
          tcGen environment
      }
      {
          map textures/model/fut_teleport_d.tga
          blendfunc GL_ZERO GL_SRC_ALPHA
      }

      {
          map textures/model/fut_teleport_d.tga
      }
      {
          map $lightmap
          blendfunc filter
          tcGen lightmap
      }
      {
          map textures/model/fut_teleport_e.tga
          rgbGen wave sin 0.3 0.3 0.0 0.5
          blendfunc add
      }
}


