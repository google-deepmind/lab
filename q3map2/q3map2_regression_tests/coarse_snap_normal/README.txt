DESCRIPTION OF PROBLEM:
=======================

Because of the coarse nature of SnapNormal(), planes that are 0.25 degrees
away from being axial are "snapped" to be axial.  The "normal epsilon"
is a very small value by default, and cannot go much smaller (without
running into limits of floating point numbers).  The problem with
SnapNormal() is that we compare the components of the normal that are near
1, instead of comaring the components that are near 0.  This leads to a very
coarse and inaccurate SnapNormal().

If you open the example map in Radiant, you can see that the red brick should
touch the middle checkered brick at the edge.  However, once this map is
compiled, the edges are a significant distance apart.  This is due to the
coarse and inaccurate nature of SnapNormal().  Likewise, the green brick should
be flush with the center tiled brick, but it's not.
