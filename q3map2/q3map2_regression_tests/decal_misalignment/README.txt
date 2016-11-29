DESCRIPTION OF PROBLEM:
=======================

The info_null in the map for the decal is not 100% below the center of the
decal itself, because to be totally below it would have to lie on half-units.
So, the info_null lies almost directly below the center of the decal.  In
this particular case, all kinds of bad things happen to the decal.  For one,
during compiling we get warnings like this:

  Bad texture matrix! (B) (50.512253, -49.515625) != (50.484375, -49.515625)
  Bad texture matrix! (C) (48.723190, -49.522587) != (48.695312, -49.515625)
  Bad texture matrix! (B) (48.723186, -49.522587) != (48.695312, -49.515625)

If you look at where the decal (it's just a blue translucent tile texture)
meets the far wall, it's clearly not aligned correctly.  The tile on the decal
and the tile on the wall should align perfectly, and it's quite a bit off.
