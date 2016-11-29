DESCRIPTION OF PROBLEM:
=======================

In the disappearing_triangle regression test I outlined a potential problem
in the tjunction.c code, having to do with 2 points being too close together
on a winding, and that winding becoming invalid (drawsurf disappearing).

This example map is the same as disappearing_triangle.map, except that the
floor is raised by 1 unit.  Therefore the floor intersects the sliver triangle.
The resulting chopped sliver has a very short edge that is "degenerate" and
causes that draw surf to go away due to problems in the tjunction.c code.

This is speculation at this point.  I will provide more info after the bug has
been fixed.
