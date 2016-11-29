DESCRIPTION OF PROBLEM:
=======================

The 4-sided brush in the middle of the room (brush 0) has a duplicate plane.
The last side (side 4) is a duplicate of the third side (side 2) with the
vertexes re-arranged.

I wanted to make sure that ChopWindingInPlaceAccu() is doing the right thing
by setting a winding to NULL when all its points lie on the plane that the
winding is being chopped with.  This seems to be the case.  My concern was
that both windings for that duplicate plane will be NULL'ed out.

Seems that some other code might be fixing duplicate planes.  That's OK.
