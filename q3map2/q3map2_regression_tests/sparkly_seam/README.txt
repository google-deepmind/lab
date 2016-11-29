DESCRIPTION OF PROBLEM:
=======================

The example map, maps/sparkly_seam.map, contains two triangular brushes near
the side of the room.  The seam between these two brushes "sparkles" even
though the endpoints of the edges are exactly the same.

To trigger the bug, compile the map; you don't need -vis or -light.  Only
-bsp (the first q3map2 stage) is necessary to trigger the bug.  The only
entities in the map are a light and a info_player_deathmatch, so the map will
compile for any Q3 mod.


SOLUTION TO PROBLEM:
====================

None yet.  The problem is likely caused by sloppy math operations (significant
loss of precision).  This bug pops in and out of existence with every other
commit at the moment.  The problem is likely caused by the operations in the
brush winding computation (where the planes are intersected with each other).
I have not gotten around to addressing that code yet.
