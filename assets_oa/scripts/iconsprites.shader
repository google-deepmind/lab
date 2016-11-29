textures/REGION
{
    surfaceparm nolightmap
}

lagometer
{
    nopicmip
    {
        map gfx/2d/lag.tga
    }
}

disconnected
{
    nopicmip
    {
        map gfx/2d/net.tga
    }
}

white
{
    {
        map *white
        blendfunc  GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

levelShotDetail
{
    nopicmip
    {
        map textures/map/black_d.tga
        blendFunc add
        rgbgen identity
    }
}

icons/teleporter
{
    nopicmip
    {
        map icons/teleporter.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}
icons/medkit
{
    nopicmip
    {
        map icons/medkit.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/envirosuit
{
    nopicmip
    {
        map icons/envirosuit.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}
icons/quad
{
    nopicmip
    {
        map icons/quad.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}
icons/haste
{
    nopicmip
    {
        map icons/haste
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}
icons/invis
{
    nopicmip
    {
        map icons/invis
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}
icons/regen
{
    nopicmip
    {
        map icons/regen
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/flight
{
    nopicmip
    {
        map icons/flight.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/invulnerability
{
    nopicmip
    {
        map icons/invulnerability
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/kamikaze
{
    nopicmip
    {
        map icons/kamikaze
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

medal_impressive
{
    nopicmip
    {
        clampmap textures/map/black_d.tga
        blendFunc add
    }
}

medal_frags
{
    nopicmip
    {
        clampmap textures/map/black_d.tga
        blendFunc add
    }
}

medal_excellent
{
    nopicmip
    {
        clampmap textures/map/black_d.tga
        blendFunc add
    }
}

medal_gauntlet
{
    nopicmip
    {
        clampmap textures/map/black_d.tga
        blendFunc add
    }
}

medal_assist
{
    nopicmip
    {
        clampmap textures/map/black_d.tga
        blendFunc add
    }
}

medal_defend
{
    nopicmip
    {
        clampmap textures/map/black_d.tga
        blendFunc add
    }
}

medal_capture
{
    nopicmip
    {
        clampmap textures/map/black_d.tga
        blendFunc add
    }
}


icons/iconw_gauntlet
{
    nopicmip
    {
        map textures/icons/iconw_gauntlet.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}
icons/iconw_machinegun
{
    nopicmip
    {
        map textures/icons/iconw_machinegun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}
icons/iconw_rocket
{
    nopicmip
    {
        map textures/icons/iconw_rocket.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_shotgun
{
    nopicmip
    {
        map icons/iconw_shotgun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen identitylighting
    }
}

icons/iconw_grenade
{
    nopicmip
    {
         map icons/iconw_grenade.tga
         blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_lightning
{
    nopicmip
    {
        map textures/icons/iconw_lightning.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_plasma
{
    nopicmip
    {
        map icons/iconw_plasma.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_railgun
{
    nopicmip
    {
        map textures/icons/iconw_railgun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_bfg
{
    nopicmip
    {
         map icons/iconw_bfg.tga
         blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_grapple
{
    nopicmip
    {
        map icons/iconw_grapple.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_machinegun
{
    nopicmip
    {
        map icons/icona_machinegun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}
icons/icona_rocket
{
    nopicmip
    {
        map icons/icona_rocket.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_shotgun
{
    nopicmip
    {
        map icons/icona_shotgun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen identitylighting
    }
}

icons/icona_grenade
{
    nopicmip
    {
        map icons/icona_grenade.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_lightning
{
    nopicmip
    {
        map icons/icona_lightning.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_plasma
{
    nopicmip
    {
        map icons/icona_plasma.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_railgun
{
    nopicmip
    {
        map icons/icona_railgun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_bfg
{
    nopicmip
    {
        map icons/icona_bfg.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}


icons/iconr_shard
{
    nopicmip
    {
        map icons/iconr_shard.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconr_yellow
{
    nopicmip
    {
        map icons/iconr_yellow.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconr_red
{
    nopicmip
    {
        map icons/iconr_red.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}


icons/iconh_green
{
    nopicmip
    {
        map icons/iconh_green.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconh_yellow
{
    nopicmip
    {
        map icons/iconh_yellow.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconh_red
{
    nopicmip
    {
        map icons/iconh_red.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }

}

icons/iconh_mega
{
    nopicmip
    {
        map icons/iconh_mega.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconf_red
{
    nopicmip
    {
        map icons/iconf_red.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconf_blu
{
    nopicmip
    {
        map icons/iconf_blu.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

gfx/2d/menuinfo
{
    nopicmip
    {
        map gfx/2d/menuinfo.tga
    }
}

gfx/2d/menuinfo2
{
    nopicmip
    {
        map gfx/2d/menuinfo2.tga
    }
}

gfx/2d/quit
{
    nopicmip
    nomipmaps
    {
        map gfx/2d/quit.tga
    }
}

gfx/2d/cursor
{
    nopicmip
    nomipmaps
    {
        map gfx/2d/cursor.tga
    }
}

sprites/balloon3
{
    {
        map sprites/balloon4.tga
        blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

viewBloodBlend
{
    sort  nearest
    {
        map gfx/damage/blood_screen.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identityLighting
        alphaGen vertex
    }
}

waterBubble
{
    sort  underwater
    cull none
    entityMergable
    {
        map sprites/bubble.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen    vertex
        alphaGen  vertex
    }
}

Grareflaader
{
    cull none
    {
        map gfx/misc/flare.tga
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

boens
{
    cull none
    {
        map gfx/misc/sun.tga
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

bloodMark
{
    nopicmip
    polygonOffset
    {
        clampmap gfx/damage/blood_stain.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identityLighting
        alphaGen vertex
    }
}

bloodTrail
{
    nopicmip
    entityMergable
    {
        clampmap gfx/damage/blood_spurt.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen    vertex
        alphaGen  vertex
    }
}

scoreboardName
{
    nopicmip
    nomipmaps
    {
        clampmap menu/tab/name.tga
        blendfunc blend
    }
}

scoreboardScore
{
    nopicmip
    nomipmaps
    {
        clampmap menu/tab/score.tga
        blendfunc blend
    }
}

scoreboardTime
{
    nopicmip
    nomipmaps
    {
        clampmap menu/tab/time.tga
        blendfunc blend
    }
}

scoreboardPing
{
    nopicmip
    nomipmaps
    {
        clampmap menu/tab/ping.tga
        blendfunc blend
    }
}

gfx/2d/crosshair
{
    nopicmip
    {
        map gfx/2d/crosshair.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/2d/crosshairb
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/2d/crosshairc
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/2d/crosshaird
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/2d/crosshaire
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/2d/crosshairf
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/2d/crosshairg
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/2d/crosshairh
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/2d/crosshairi
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }

}
gfx/2d/crosshairj
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}
gfx/2d/crosshairk
{
    nopicmip
    {
        map gfx/2d/crosshaire.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

gfx/2d/bigchars
{
    nopicmip
    nomipmaps
    {
        map gfx/2d/bigchars.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/select
{
    nopicmip
    {
        map gfx/2d/select.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen identity
        rgbgen vertex
    }
}

gfx/2d/assault1d
{
    nopicmip
    {
        map gfx/2d/assault1d.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

gfx/2d/armor1h
{
    nopicmip
    {
        map gfx/2d/armor1h.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

gfx/2d/health
{
    nopicmip
    {
        map gfx/2d/health.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

gfx/2d/blank
{
    nopicmip
    {
        map gfx/2d/blank.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

gfx/2d/numbers/zero_32b
{
    nopicmip
    {
        map gfx/2d/numbers/zero_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/one_32b
{
    nopicmip
    {
        map gfx/2d/numbers/one_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/two_32b
{
    nopicmip
    {
        map gfx/2d/numbers/two_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/three_32b
{
    nopicmip
    {
        map gfx/2d/numbers/three_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/four_32b
{
    nopicmip
    {
        map gfx/2d/numbers/four_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/five_32b
{
    nopicmip
    {
        map gfx/2d/numbers/five_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/six_32b
{
    nopicmip
    {
        map gfx/2d/numbers/six_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/seven_32b
{
    nopicmip
    {
        map gfx/2d/numbers/seven_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/eight_32b
{
    nopicmip
    {
        map gfx/2d/numbers/eight_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/nine_32b
{
    nopicmip
    {
        map gfx/2d/numbers/nine_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

gfx/2d/numbers/minus_32b
{
    nopicmip
    {
        map gfx/2d/numbers/minus_32b.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbgen vertex
    }
}

icons/iconw_chaingun
{
    nopicmip
    {
        map icons/iconw_chaingun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_chaingun_cl1
{
    nopicmip
    {
        map icons/iconw_chaingun_cl1.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_chaingun_cl2
{
    nopicmip
    {
        map icons/iconw_chaingun_cl2.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_grapple
{
    nopicmip
    {
        map icons/iconw_grapple.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_nailgun
{
    nopicmip
    {
        map icons/iconw_nailgun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_proxlauncher
{
    nopicmip
    {
        map icons/iconw_proxlauncher.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_chaingun
{
    nopicmip
    {
        map icons/icona_chaingun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_proxlauncher
{
    nopicmip
    {
        map icons/icona_proxlauncher.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_nailgun
{
    nopicmip
    {
        map icons/icona_nailgun.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/guard
{
    nopicmip
    {
        map icons/guard.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/doubler
{
    nopicmip
    {
        map icons/doubler.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/ammo_regen
{
    nopicmip
    {
        map icons/ammo_regen.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/scout
{
    nopicmip
    {
        map icons/scout.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}


icons/icona_red
{
    nopicmip
    {
        map icons/icona_red.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_blue
{
    nopicmip
    {
        map icons/icona_blue.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_white
{
    nopicmip
    {
        map icons/icona_white.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconb_red
{
    nopicmip
    {
        map icons/iconb_red.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconb_blue
{
    nopicmip
    {
        map icons/iconb_blue.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconb_white
{
    nopicmip
    {
        map icons/iconb_white.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_red
{
    nopicmip
    {
        map icons/icona_red.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_blue
{
    nopicmip
    {
        map icons/icona_blue.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconf_blu
{
    nopicmip
    {
        map icons/iconf_blu.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconf_red
{
    nopicmip
    {
        map icons/iconf_red.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

menu/art/skill1
{
    nopicmip
    {
        clampmap menu/art/skill1
        blendFunc blend
    }
}

menu/art/skill2
{
    nopicmip
    {
        clampmap menu/art/skill2
        blendFunc blend
    }
}

menu/art/skill3
{
    nopicmip
    {
        clampmap menu/art/skill3
        blendFunc blend
    }
}

menu/art/skill4
{
    nopicmip
    {
        clampmap menu/art/skill4
        blendFunc blend
    }
}

menu/art/skill5 {
    nopicmip
    {
        clampmap menu/art/skill5
        blendFunc blend
    }
}
