DESCRIPTION OF PROBLEM:
=======================

The example map, maps/snap_plane.map, contains an example of some bugs.
The green brush with the red face is transformed into something totally
unexpected.

To trigger the bug, compile the map; you don't need -vis or -light.  Only
-bsp (the first q3map2 stage) is necessary to trigger the bug.  The only
entities in the map are a light and a info_player_deathmatch, so the map will
compile for any Q3 mod.


SOLUTION TO PROBLEM:
====================

I actually came up with this regression test after reading the code to
SnapPlane() and SnapNormal().  I decided to make this map as a test of
whether or not I understand how the code is broken.  Sure enough, the map
is broken in the same way that I would expect it to be broken.

The problem is that the red face of the green brush in the center of the room
is very close to being axially aligned, but not quite.  This plane will
therefore be "snapped" to become axial.  This is not the problem.  The
problem is that after the snap, the plane will be shifted a great deal,
and the corner of the green brush will no longer come even close to meeting
the corner of the other cube brush next to it (the two corners of the two
brushes will drift apart a great deal).

If you study the legacy SnapPlane() code, you will see that planes are
defined as a unit normal and distance from origin.  Well, because
of the position of this brush on the side of the world extents (far from
the original), the snap will cause drastic effects on the points that the
plane was supposed to approximate.

Another problem with SnapPlane() is that SnapNormal() snaps too liberally.
Once SnapNormal() is changed to only snap normals that are REALLY close
to axial (and not anything within 0.25 degrees), this bug will no longer
appear, but the SnapPlane() problem will still be present.  So, this
regression test should be fixed BEFORE SnapNormal() is fixed, otherwise
create another regression test with a red face that is much much closer to
being axially aligned.  We want to make sure that SnapPlane() is fixed when
things actually do get snapped.

The code to address these issues is currently being worked on.
