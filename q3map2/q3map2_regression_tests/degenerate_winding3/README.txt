DESCRIPTION OF PROBLEM:
=======================

The sample map contains a wedge brush.  The tip (the sharp edge) of the wedge
is chopped off by the YZ plane (side 5, or the last side, of the brush).
The height of the wedge where it is chopped is about 0.9.  This makes it
barely smaller than DEGENERATE_EPSILON.  So the face resulting from the chop
is probably degenerate, and that winding will be removed.  I'm now wondering
what happens to the rest of the brush.  0.9 rounded to the nearest 1/8 unit
is 1/8, so the top face of the brush should get a slight raise, making the
"hole" even bigger.  The sides will have degenerate edges near the chop, so
they will become triangles, creating open slivers in the sides.

Although this behavior is a tad nasty, it is expected based on the way the
code is written.  I want to make sure nothing really nasty happens.

I consider this regression test to be broken if there is a "hole" in the brush,
and I consider this test to be very broken if something more drastic happens.
