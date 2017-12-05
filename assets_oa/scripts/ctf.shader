sprites/friend
{
  nomipmaps
  {
    map sprites/friend1.tga
    blendfunc blend
  }
}

sprites/foe
{
  nomipmaps
  {
    map sprites/foe2.tga
    blendfunc blend
  }
}

models/flags/b_flag
{
  cull none
  nopicmip
  {
    map models/flags/b_flag.tga
  }
}

models/flags/pole
{
  {
    map textures/base_wall/chrome_env.tga
    rgbGen lightingDiffuse
    tcMod scale 0.5 0.5
    tcGen environment
  }
  {
    map models/flags/pole.tga
    blendfunc filter
    rgbGen identity
  }
}

models/flags/r_flag
{
  cull none
  nopicmip
  {
    map models/flags/r_flag.tga
  }
}
