# How to Run *DeepMind Lab* as a Human

*DeepMind Lab* can be run from the command line via the stand-alone `:game`
target.



## Running with Bazel



Build and run *DeepMind Lab* using Bazel.
Specify the level name (without `.lua`) with the `-l` or `--level_script`
flag. For example, to run `game_scripts/levels/lt_chasm.lua`,

```shell
lab$ bazel run :game -- -l lt_chasm
```

Once the binary is built, you can also run it directly without Bazel:
```shell
lab$ bazel-bin/game -l lt_chasm
```
(Running through `bazel` ensures that the binary is rebuilt if necessary.)


Specify setting overrides with the `-s` or `--level_setting` flag:

```shell
lab$ bazel run :game -- -l lt_chasm \
   -s categoryCount=8 -s mazeHeight=23 -s mazeWidth=23 -s objectCount=15
```

