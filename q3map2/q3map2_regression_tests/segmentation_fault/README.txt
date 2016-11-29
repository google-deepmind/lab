DESCRIPTION OF PROBLEM:
=======================

The example map, maps/segmentation_fault.map, contains an example of this
bug.  q3map2 might segfault while compiling this map.  This sort of thing
might happen in certain intermediate versions of q3map2 while work is being
done on fixing the math accuracy.  The bug may not have happened in older
version of q3map2, before the math accuracy issues were addressed.

To trigger the bug, compile the map; you don't need -vis or -light.  Only
-bsp (the first q3map2 stage) is necessary to trigger the bug.  The only
entities in the map are a light and a info_player_deathmatch, so the map will
compile for any Q3 mod.

Here is a description of the problem brush (brush #0):

  side 0: -z face
  side 1: +z face
  side 2: -y face
  side 3: +x face
  side 4: +y face
  side 5: -x face
  side 6: problem side "accidentally showed up" :-)

Side 6 is actually a superfluous plane and will be NULL'ed out in the code.
If the code does not handle a NULL'ed out winding correctly, it will
segfault.
