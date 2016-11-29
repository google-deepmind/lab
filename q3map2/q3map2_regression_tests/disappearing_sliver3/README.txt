DESCRIPTION OF PROBLEM:
=======================

The example map, maps/disappearing_sliver3.map, contains an example of this
bug.  The triangle sliver surface in the middle of the room is not rendered
in the final BSP.

To trigger the bug, compile the map; you don't need -vis or -light.  Only
-bsp (the first q3map2 stage) is necessary to trigger the bug.  The only
entities in the map are a light and a info_player_deathmatch, so the map will
compile for any Q3 mod.


SOLUTION TO PROBLEM:
====================

More work has been done to BaseWindingForPlane() to make it more accurate.
This function is in polylib.c.  The changes to fix this regression test were
committed in revision 377; however, those changes are not "good enough".


IN-DEPTH DISCUSSION:
====================

This is the problem triangle:

  In ParseRawBrush() for brush 0
      Side 0:
          (6144.000000 16122.000000 -2048.000000)
          (6144.000000 16083.000000 -2048.000000)
          (6784.000000 16241.000000 -2048.000000)

Computed winding before fix:

  (6784.16406250 16241.04101562 -2048.00000000)
  (6144.00000000 16122.00976562 -2048.00000000)
  (6144.00000000 16083.00000000 -2048.00000000)

Obviously the 6784.16406250 is beyond epsilon error.

After revision 377:

  (6783.85937500 16240.96484375 -2048.00000000)
  (6144.00000000 16121.99218750 -2048.00000000)
  (6144.00000000 16083.00000000 -2048.00000000)

Even though this fixes the regression test, the error in 6783.85937500 is
still greater than epsilon (but fortunately in the opposite direction).  So
I don't consider this test case to be fixed quite yet.
