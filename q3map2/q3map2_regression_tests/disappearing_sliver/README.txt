DESCRIPTION OF PROBLEM:
=======================

The example map, maps/disappearing_sliver.map, contains an example of this bug.
There are 6 walls in the map, and one tall thin triangular sliver brush in the
middle of the room (7 brushes total).  Only one face of the sliver in the
middle of the room is a draw surface.  The bug is that this sliver surface is
not rendered in the compiled BSP.  Note that the sliver brush was hand-crafted
to demonstrate the bug.  If you re-save the map, Radiant might adjust the
order in which the planes on the brush are defined, and this might, as a side
effect, get rid of the immediate bug.

To trigger the bug, compile the map; you don't need -vis or -light.  Only
-bsp (the first q3map2 stage) is necessary to trigger the bug.  The only
entities in the map are 2 lights and a single info_player_deathmatch, so the
map will compile for any Q3 mod.


SOLUTION TO PROBLEM:
====================

Several days were spent studying this problem in great detail.

The fix for this problem was to make the outcome of the VectorNormalize()
function libs/mathlib/mathlib.c more accurate.  The previous code in this
function looks something like this:

  vec_t length, ilength; // vec_t is a typedef for 32 bit float.

  /* Compute length */

  ilength = 1.0f/length;
  out[0] = in[0]*ilength;
  out[1] = in[1]*ilength;
  out[2] = in[2]*ilength;

As you can see, we are introducing a lot of extra error into our normalized
vector by multiplying by the reciprocal length instead of outright dividing
by the length.  The new fixed code looks like this:

  out[0] = in[0]/length;
  out[1] = in[1]/length;
  out[2] = in[2]/length;

And we get rid of the recpirocal length ilength altogether.  Even the
slightest math errors are magnified in successive calls to linear algebra
functions.

The change described above was commmitted to GtkRadiant trunk as revision 363.


POSSIBLE SIDE EFFECTS:
======================

The only negative side effect is that compilation of a map might take longer
due to an increased number of divide operations.  (I'm actually not sure if
that is indeed the case.)  Another side effect might be that if you're used
to a map being broken (missing triangles) or having "sparklies" between
brushes, those might be gone now.  :-)


IN-DEPTH DISCUSSION:
====================

VectorNormalize() is used very frequently in Radiant and tools code.  My goal
for this fix was to make the least amount of code change but still be able to
demonstrate a significant improvement in math accuracy (including a fix to
the test case).  At the same time don't risk that any other bugs pop up as a
side effect of this change.

Here is the sequence of calls (stack trace) that cause the example bug to
happen:

  main() in main.c -->
    BSPMain() in bsp.c -->
      LoadMapFile() in map.c -->
        ParseMapEntity() in map.c -->
          ParseBrush() in map.c -->
            FinishBrush() in map.c -->
              CreateBrushWindings() in brush.c -->
                ChopWindingInPlace() in polylib.c

What basically happens in this sequence of calls is that a brush is parsed
out of the map file, "infinite" planes are created for each brush face, and
then the planes are "intersected" to find the exact vertex topologies of each
brush face.  The vertex topology of the visible face of the sliver (in the
example map) gets computed with a significant amount of math error.  If we
did our math with infinite precision, the sliver face would have the following
vertices:

  (67 -1022 0)
  (88 -892 -768)
  (134 -1015 0)

In fact, if you open the map file (disappearing_sliver.map), you can actually
see these exact points embedded in the third plane defined on brush 0.

I managed to print out the actual computed vertices of the sliver face before
and after this bug fix.  Basically this is printed out after all the
ChopWindingInPlace() calls in the above stack trace:

  (66.984695 -1021.998657 0.000000)
  (87.989571 -891.969116 -768.174316)
  (133.998917 -1014.997314 0.000000)

(If you want to print this out for yourself, use winding_logging.patch.)

The same vertices after the bugfix have the following coordinates:

  (67.000229 -1021.998657 0.000000)
  (88.000175 -891.999146 -767.997437)
  (133.999146 -1014.998779 0.000000)

As you can see, the vertices after the fix are substantially more accurate,
and all it took was an improvement to VectorNormalize().

The problem before the fix was the Z coordinate of the second point, namely
-768.174316.  There is a lot of "snap to nearest 1/8 unit" and "epsilon 0.1"
code used throughout q3map2.  0.174 is greater than the 0.1 epsilon, and that
is the problem.

  main() in main.c -->
    BSPMain() in bsp.c -->
      ProcessModels() in bsp.c -->
        ProcessWorldModel() in bsp.c -->
          ClipSidesIntoTree() in surface.c -->
            ClipSideIntoTree_r() in surface.c -->
              ClipWindingEpsilon() in polylib.c

Now what ClipWindingEpsilon() does is, since -768.174316 reaches below the
plane z = -768 (and over the 0.1 epsilon), it clips the winding_t and creates
two points where there used to be only one.

  main() in main.c -->
    BSPMain() in bsp.c -->
      ProcessModels() in bsp.c -->
        ProcessWorldModel() in bsp.c
          FixTJunctions() in tjunction.c
            FixBrokenSurface() in tjunction.c

FixBrokenSurface() realizes that there are two points very close together
(in our case, since they were "snapped", the are coincident in fact).
Therefore it reports the surface to be broken.  The drawable surface is
deleted as a result.


RELATED BUGS:
=============

A lot of the math operations in the Radiant codebase cut corners like this
example demonstrates.  There is a lot more code like this that can be
improved upon.  In fact, it may make sense to use 64 bit floating points in
some important math operations (and then convert back to 32 bit for the return
values).  Plans are to look at similar code and improve it.

The following "issue" was discovered while doing research for this bug.
If FixBrokenSurface() sees two points very close together, it attempts to
partially fix the problem (the code is not complete) and then returns false,
which means that the surface is broken and should not be used.  So in fact
it attempts to fix the problem partially but none of the fixes are used.
It seems that FixBrokenSurface() should be fixed to completely fix the case
where there are two close points, and should report the surface as fixed.
This might be a destabilizing change however, so if this is indeed fixed, it
may make sense to activate the fix only if a certain flag is set.


MORE NOTES:
===========

As stated above, the accuracy after revision 363 is:

  (67.000229 -1021.998657 0.000000)
  (88.000175 -891.999146 -767.997437)
  (133.999146 -1014.998779 0.000000)

A further change was committed for a related problem in revision 377.  After
this change:

  (66.99955750 -1022.00262451 0.00000000)
  (87.99969482 -892.00170898 -768.00524902)
  (133.99958801 -1015.00195312 0.00000000)

The results look similar with respect to the amount of error present.
