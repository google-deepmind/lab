DESCRIPTION OF PROBLEM:
=======================

The sample map contains a wedge brush.  The tip (the sharp edge) of the wedge
is chopped off by 2 planes, leaving very narrow windings.  Each of these 2
narrow windings is less than 0.1 units tall.  However, the wedge has height
exactly equal to 1/8 unit at the point where it is chopped.  Therefore,
the two narrow sides caused by the chops are expected to be degenerate and the
top face of the wedge is expected to be unaffected.  This should leave a
"hole" in the narrow part of the wedge.

The hole isn't desirable but it's expected based on the logic in the code.
Still, if there is a hole in the brush, I consider this regression test to
be broken.
