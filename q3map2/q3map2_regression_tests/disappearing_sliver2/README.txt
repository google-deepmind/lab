DESCRIPTION OF PROBLEM:
=======================

The example map, maps/disappearing_sliver2.map, contains an example of this
bug.  The triangle sliver surface in the middle of the room is not rendered
in the final BSP.

To trigger the bug, compile the map; you don't need -vis or -light.  Only
-bsp (the first q3map2 stage) is necessary to trigger the bug.  The only
entities in the map are a light and a info_player_deathmatch, so the map will
compile for any Q3 mod.


SOLUTION TO PROBLEM:
====================

It was discovered that BaseWindingForPlane() in polylib.c did some sloppy
mathematics with significant loss of precision.  Those problems have been
addressed in commits to revisions 371 and 377.


POSSIBLE SIDE EFFECTS:
======================

Great care was taken to preserve the exact behavior of the original
BaseWindingForPlane() function except for the loss of precision.  Therefore
no negative side effects should be seen.  In fact performance may be
increased.


IN-DEPTH DISCUSSION:
====================

Turns out that the problem is very similar to the original disappearing_sliver
regression test.  You should read that README.txt to familiarize yourself
with the situation.

The thing we need to look at is side 0 of brush 0, if you applied
winding_logging.patch from disappearing_sliver regression test:

  In ParseRawBrush() for brush 0
      Side 0:
          (6784.000000 16241.000000 -1722.000000)
          (6144.000000 16083.000000 -1443.000000)
          (6144.000000 16122.000000 -1424.000000)

That is the exact plane defninition of our problem sliver, and in fact those
are also the correct points for the actual vertices of the triangle.

Now the results of the winding for this surface after all the clipping takes
place:

  (6784.12500000 16241.02343750 -1722.06250000)
  (6144.00000000 16082.99218750 -1443.00781250)
  (6144.00000000 16122.00000000 -1424.00390625)

As you can see, 6784.12500000 is more than epsilon distance (0.1) away from
the correct point.  This is a big problem.

After we apply the fix committed in revision 371, the result after clipping
is this:

  (6784.06250000 16241.01171875 -1722.03515625)
  (6144.00000000 16082.99609375 -1443.00781250)
  (6144.00000000 16122.00000000 -1424.00585938)

As you can see, all points but one have an increase in accuracy.  This is
still not accurate enough in my opinion, but is a step in the right direction.

After the fix committed in revision 377, which is a further attempt to address
BaseWindingForPlane(), we get the following accuracy:

  (6784.00000000 16241.00000000 -1722.00000000)
  (6144.00000000 16083.00000000 -1443.00000000)
  (6144.00000000 16122.00000000 -1424.00000000)

It's just a fluke for this particular case, but obviouly revision 377 looks
favorably upon this regression test, because there is zero percent error.


MORE NOTES:
===========

I attempted to improve upon revision 371 by streamlining the code in
BaseWindingForPlane() some more.  Those attempts were committed as revision
375.  After revision 375:

  (6784.09375000 16241.01757812 -1722.04687500)
  (6144.00000000 16082.99414062 -1443.00390625)
  (6144.00000000 16122.00000000 -1424.00097656)

Revision 375 has since been reverted (undone) because of the loss in
accuracy.  Revision 377 is a fix for those failed attempts.
