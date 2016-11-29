DESCRIPTION OF PROBLEM:
=======================

This example map demonstrates what a relatively large value for plane distance
epsilon can do as far as destroying draw surfaces.  The plane distance
epsilon was 0.01 at the time of this writing.  This means that two planes
with the same normal and a distance 0.00999 apart are considered to be the
same plane.

The recommended value of plane distance epsilon is more along the lines of
0.001; however, we can't currently change this epsilon to that value due to
lack of resolution in 32 bit floating point integers in the 2^16 number range.
The smallest epsilon in 32 bit float land that can be added to 2^16 that
results in a value greater than 2^16 is approximately 0.007, which is already
practically the current default plane distance epsilon (0.01).

Brush 0 in the example map is a 6-sided brush with red top face.  The red top
face is defined such in the .map file:

  ( -127 256 513 ) ( -127 384 513 ) ( 1 384 512 )

During the -bsp stage,

  In ParseRawBrush() for brush 0
      Side 4:
          (-127.000000 256.000000 513.000000)
          (-127.000000 384.000000 513.000000)
          (1.000000 384.000000 512.000000)
          normal: (0.0078122616 0.0000000000 0.9999694824)
          dist: 511.9921875000

Now brush 1, the 4-sided brush with the red top face, has the following
defined as the red top face:

  ( -128 0 513 ) ( -128 128 513 ) ( 0 128 512 )

It's almost the same plane, only off by a distance of about 0.01.

During compiling:

  In ParseRawBrush() for brush 1
      Side 1:
          (-128.000000 0.000000 513.000000)
          (-128.000000 128.000000 513.000000)
          (0.000000 128.000000 512.000000)
          normal: (0.0078122616 0.0000000000 0.9999694824)
          dist: 511.9921875000

Note that the normal and dist are identical to the plane above, even though
the two are different.  This leads to multiplying errors later on, when this
side is intersected with the bottom green face.  In particular, the blue face
disappears from the map.  The blue face:

  In CreateBrushWindings() for brush 1
      Handling side 2 on the brush
          Before clipping we have:
              (-262144.00000000 0.00000000 262144.00000000)
              (262144.00000000 0.00000000 262144.00000000)
              (262144.00000000 -0.00000000 -262144.00000000)
              (-262144.00000000 0.00000000 -262144.00000000)
          After clipping w/ side 0 we have:
              (-262144.00000000 0.00000000 262144.00000000)
              (262144.00000000 0.00000000 262144.00000000)
              (262144.00000000 0.00000000 -19977.00073172)
              (-262144.00000000 0.00000000 20983.00073758)
          After clipping w/ side 1 we have:
              (262144.00000000 0.00000000 -1535.99218726)
              (262144.00000000 0.00000000 -19977.00073172)
              (-128.11106773 0.00000000 513.00868046)
          After clipping w/ side 3 we have:
              (0.00000000 0.00000000 503.00000293)
              (-128.11106773 0.00000000 513.00868046)
              (-0.00000000 0.00000000 512.00781274)

We see the error is greater than 0.1 in the X value -128.11106773.  This
causes the draw surface to be discarded in processing later on.  We must
not allow such a great error.  An error greater than or equal to 0.1 is
considered to be very bad.

This disappearing blue triangle can be fixed simply by deleting brush 0,
the 6-sided brush that causes the bad plane alias.  I would suggest deleting
this brush by editing the .map file manually (and then renumbering the brushes
manually).

After deleting this brush:

  In ParseRawBrush() for brush 0
      Side 1:
          (-128.000000 0.000000 513.000000)
          (-128.000000 128.000000 513.000000)
          (0.000000 128.000000 512.000000)
          normal: (0.0078122616 0.0000000000 0.9999694824)
          dist: 511.9843750000

  In CreateBrushWindings() for brush 0
      Handling side 1 on the brush
          Before clipping we have:
              (262132.00000000 262136.00000000 -1535.90625143)
              (262132.00000000 -262136.00000000 -1535.90625143)
              (-262124.00048828 -262136.00000000 2559.84375238)
              (-262124.00048828 262136.00000000 2559.84375238)
          After clipping w/ side 0 we have:
              (262132.00000000 262136.00000000 -1535.90625143)
              (262132.00000000 -262136.00000000 -1535.90625143)
              (-127.99993289 -262136.00000000 512.99999805)
              (-127.99993289 262136.00000000 512.99999805)
          After clipping w/ side 2 we have:
              (262132.00000000 262136.00000000 -1535.90625143)
              (262132.00000000 -0.00000000 -1535.90625143)
              (-127.99993289 -0.00000000 512.99999805)
              (-127.99993289 262136.00000000 512.99999805)
          After clipping w/ side 3 we have:
              (0.00000000 0.00000000 511.99999857)
              (-127.99993289 -0.00000000 512.99999805)
              (-127.99993289 127.99993289 512.99999805)

Note that we're way more accurate now, and we're nowhere near to approaching
the 0.1 error.  All it took was the deletion of a seemintly unrelated brush.


SOLUTION TO PROBLEM:
====================

The suggested fix is to increase the resolution of plane distance from 32 bit
float to 64 bit float for the relevant brush processing operations.  After that
is done, decrease the default plane distance epsilon from 0.01 to 0.001.  Note
that even though plane distance epsilon can be specified on the command-line,
it will do no good (even if set to 0) if the plane distance is close to
2^16.
