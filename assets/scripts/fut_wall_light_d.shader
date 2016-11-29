textures/model/fut_wall_light_d
{
     surfaceparm nodraw
     surfaceparm noimpact
     //surfaceparm nonsolid

     q3map_lightimage textures/map/white_d.tga
     // this TGA is the source for the color of the blended light

     qer_editorimage textures/map/white_d.tga
     //base TGA (used because the shader is used with several
     // different light values

     q3map_surfacelight 500
     //emitted light value of 10,000

     //{
        //map $lightmap
        //source texture is affected by the lightmap
        //rgbGen identity
        //this command handles the overbright bits created by "sunlight"
        //in the game
     //}

     //{
        //map textures/map/white_d.tga
       // blendFunc filter
        //rgbGen identity
     //}

     //{
        //map textures/map/white_d.tga
        //blendFunc add
     //}
}
