# DMLab-30

DMLab-30 is a set of environments designed for DeepMind Lab. These environments
enable a researcher to develop agents for a large spectrum of interesting tasks
either individually or in a multi-task setting.

1.  [`rooms_collect_good_objects_{test,train}`](#collect-good-objects)
1.  [`rooms_exploit_deferred_effects_{test,train}`](#exploit-deferred-effects)
1.  [`rooms_select_nonmatching_object`](#select-non-matching-object)
1.  [`rooms_watermaze`](#watermaze)
1.  [`rooms_keys_doors_puzzle`](#keys-doors-puzzle)
1.  [`language_select_described_object`](#select-described-object)
1.  [`language_select_located_object`](#select-located-object)
1.  [`language_execute_random_task`](#execute-random-task)
1.  [`language_answer_quantitative_question`](#answer-quantitative-question)
1.  [`lasertag_one_opponent_small`](#one-opponent-small)
1.  [`lasertag_three_opponents_small`](#three-opponents-small)
1.  [`lasertag_one_opponent_large`](#one-opponent-large)
1.  [`lasertag_three_opponents_large`](#three-opponents-large)
1.  [`natlab_fixed_large_map`](#fixed-large-map)
1.  [`natlab_varying_map_regrowth`](#varying-map-regrowth)
1.  [`natlab_varying_map_randomized`](#varying-map-randomized)
1.  [`skymaze_irreversible_path_hard`](#irreversible-path-hard)
1.  [`skymaze_irreversible_path_varied`](#irreversible-path-varied)
1.  [`psychlab_arbitrary_visuomotor_mapping`](#arbitrary-visuomotor-mapping)
1.  [`psychlab_continuous_recognition`](#continuous-recognition)
1.  [`psychlab_sequential_comparison`](#sequential-comparison)
1.  [`psychlab_visual_search`](#visual-search)
1.  [`explore_object_locations_small`](#object-locations-small)
1.  [`explore_object_locations_large`](#object-locations-large)
1.  [`explore_obstructed_goals_small`](#obstructed-goals-small)
1.  [`explore_obstructed_goals_large`](#obstructed-goals-large)
1.  [`explore_goal_locations_small`](#goal-locations-small)
1.  [`explore_goal_locations_large`](#goal-locations-large)
1.  [`explore_object_rewards_few`](#object-rewards-few)
1.  [`explore_object_rewards_many`](#object-rewards-many)

## Rooms

### Collect Good Objects

<div align="center">
  <a href="https://www.youtube.com/watch?v=k0mk0CI7G0s" target="_blank">
    <img src="http://img.youtube.com/vi/k0mk0CI7G0s/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=3F8RJ7G3cPg" target="_blank">
    <img src="http://img.youtube.com/vi/3F8RJ7G3cPg/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

The agent must learn to collect good objects and avoid bad objects in two
environments. During training, only some combinations of objects/environments
are shown, hence the agent could assume the environment matters to the task due
to this correlational structure. However it does not and will be detrimental in
a transfer setting. We explicitly verify that by testing transfer performance on
a held-out objects/environment combination. For more details, please see:
[Higgins, Irina et al. "DARLA: Improving Zero-Shot Transfer in Reinforcement Learning" (2017)](https://arxiv.org/abs/1707.08475).

Test Regime: Test set consists of held-out combinations of objects/environments
never seen during training.

Observation Spec: RGBD

Level Name: `rooms_collect_good_objects_{test,train}`

### Exploit Deferred Effects

<div align="center">
  <a href="https://www.youtube.com/watch?v=HIkWgTAn7Rs" target="_blank">
    <img src="http://img.youtube.com/vi/HIkWgTAn7Rs/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=--BSrVGahGk" target="_blank">
    <img src="http://img.youtube.com/vi/--BSrVGahGk/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task requires the agent to make a conceptual leap from picking up a special
object to getting access to more rewards later on, even though this is never
shown in a single environment and is costly.  Expected to be hard for model-free
agents to learn, but should be simple when using some model-based/predictive
strategy.

Test Regime: Tested in a room configuration never seen during training, where
picking up a special object suddenly becomes useful.

Observation Spec: RGBD

Level Name: `rooms_exploit_deferred_effects_{test,train}`


### Select Non-matching Object

<div align="center">
  <a href="https://www.youtube.com/watch?v=mMD12_7gYGc" target="_blank">
    <img src="http://img.youtube.com/vi/mMD12_7gYGc/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=46hxLxkrLlk" target="_blank">
    <img src="http://img.youtube.com/vi/46hxLxkrLlk/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task requires the agent to choose and collect an object that is different
from the one it is shown. The agent is placed into a small room containing an
out-of-reach object and a teleport pad. Touching the pad awards the agent with
1 point, and teleports them to a second room. The second room contains two
objects, one of which matches the object in the previous room.

* Collect matching object: -10 points.
* Collect non-matching object:  +10 points.

Once either object is collected the agent is returned to the first room, with
the same initial object being shown.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `rooms_select_nonmatching_object`

### Watermaze

<div align="center">
  <a href="https://www.youtube.com/watch?v=XaI6SjFcmd0" target="_blank">
    <img src="http://img.youtube.com/vi/XaI6SjFcmd0/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=9XkGLVvY4yg" target="_blank">
    <img src="http://img.youtube.com/vi/9XkGLVvY4yg/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

The agent must find a hidden platform which, when found, generates a reward.
This is difficult to find the first time, but in subsequent trials the agent
should try to remember where it is and go straight back to this place. Tests
episodic memory and navigation ability.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `rooms_watermaze`


### Keys Doors Puzzle

<div align="center">
  <a href="https://www.youtube.com/watch?v=FYTeAUUiwpQ" target="_blank">
    <img src="http://img.youtube.com/vi/FYTeAUUiwpQ/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=kdM6rzP557s" target="_blank">
    <img src="http://img.youtube.com/vi/kdM6rzP557s/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

A procedural planning puzzle.
The agent must reach the goal object, located in a position that is blocked by a
series of coloured doors. Single use coloured keys can be used to open matching
doors and only one key can be held at a time. The objective is to figure out the
correct sequence in which the keys must be collected and the rooms traversed.
Visiting the rooms or collecting keys in the wrong order can make the goal
unreachable.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `rooms_keys_doors_puzzle`


## Language

For details on the addition of language instructions, see:
[Hermann, Karl Moritz, & Hill, Felix et al. "Grounded language learning in a simulated 3D world. (2017)"](https://arxiv.org/abs/1706.06551).

### Select Described Object

<div align="center">
  <a href="https://www.youtube.com/watch?v=JCJOgEcNgKY" target="_blank">
    <img src="http://img.youtube.com/vi/JCJOgEcNgKY/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=P2mp3zSHFOU" target="_blank">
    <img src="http://img.youtube.com/vi/P2mp3zSHFOU/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

The agent is placed into a small room containing two objects. An instruction is
used to describe one of the objects. The agent must successfully follow the
instruction and collect the goal object.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD and language

Level Name: `language_select_described_object`


### Select Located Object

<div align="center">
  <a href="https://www.youtube.com/watch?v=I_NmlAn_3gY" target="_blank">
    <img src="http://img.youtube.com/vi/I_NmlAn_3gY/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=Ko7f9hnX5es" target="_blank">
    <img src="http://img.youtube.com/vi/Ko7f9hnX5es/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

The agent is asked to collect a specified coloured object in a specified
coloured room. Example instruction: “Pick the red object in the blue room.”
There are four variants of the task, each of which have an equal chance of being
selected. Variants have a different amount of rooms (between 2-6). Variants with
more rooms have more distractors, making the task more challenging.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD and language

Level Name: `language_select_located_object`

### Execute Random Task

<div align="center">
  <a href="https://www.youtube.com/watch?v=6sIQoHeYaP8" target="_blank">
    <img src="http://img.youtube.com/vi/6sIQoHeYaP8/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=1sFeDd3jHHk" target="_blank">
    <img src="http://img.youtube.com/vi/1sFeDd3jHHk/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

The agent is given one of seven possible tasks, each with a different type of
language instruction. Example instruction: “Get the red hat from the blue room.”
The agent is rewarded for collecting the correct object, and penalised for
collecting the wrong object. When any object is collected, the level restarts
and a new task is selected.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD and language

Level Name: `language_execute_random_task`

### Answer Quantitative Question

<div align="center">
  <a href="https://www.youtube.com/watch?v=QMtSQI_r2ag" target="_blank">
    <img src="http://img.youtube.com/vi/QMtSQI_r2ag/0.jpg"
         alt="Human player test"
         width="" height="300" border="0" />
  </a>
  <a href="https://www.youtube.com/watch?v=kspqM0D2nu8" target="_blank">
    <img src="http://img.youtube.com/vi/kspqM0D2nu8/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="0" />
  </a>
</div>


The agent is given a yes or no question based on object colors and counts. The
agent selects a certain object to respond:

* White sphere = yes
* Black sphere = no
* Example questions:
* “Are all cars blue?”
* “Is any car blue?”
* “Is anything blue?”
* “Are most cars blue?”

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD and language

Level Name: `language_answer_quantitative_question`

## LaserTag

### One Opponent Small

<div align="center">
  <a href="https://www.youtube.com/watch?v=Sv32PoqL390" target="_blank">
    <img src="http://img.youtube.com/vi/Sv32PoqL390/0.jpg"
         alt="Human player test"
         width="" height="300" border="0" />
  </a>
  <a href="https://www.youtube.com/watch?v=23J1RlkHgQw" target="_blank">
    <img src="http://img.youtube.com/vi/23J1RlkHgQw/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="0" />
  </a>
</div>

This task requires the agent to play laser tag in a procedurally generated map
containing random gadgets and power-ups. The map is small and there is 1
opponent bot of difficulty level 4. The agent begins the episode with the
default Rapid Gadget and a limit of 100 tags. The agent’s Shield will begin at
125 and slowly drop to the max amount of 100. The gadgets, powerups and map
layout are random per episode and so the agent must adapt to each new
environment.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `lasertag_one_opponent_small`

### Three Opponents Small

<div align="center">
  <a href="https://www.youtube.com/watch?v=yj7_5zFP8pA" target="_blank">
    <img src="http://img.youtube.com/vi/yj7_5zFP8pA/0.jpg"
         alt="Human player test"
         width="" height="300" border="0" />
  </a>
  <a href="https://www.youtube.com/watch?v=SW6Zd21YZDg" target="_blank">
    <img src="http://img.youtube.com/vi/SW6Zd21YZDg/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="0" />
  </a>
</div>

This task requires the agent to play laser tag in a procedurally generated map
containing random gadgets and power-ups. The map is small and there are 3
opponent bots of difficulty level 4. The agent begins the episode with the
default Rapid Gadget and a limit of 100 tags. The agent’s Shield will begin at
125 and slowly drop to the max amount of 100. The gadgets, powerups and map
layout are random per episode and so the agent must adapt to each new
environment.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `lasertag_three_opponents_small`

### One Opponent Large

<div align="center">
  <a href="https://www.youtube.com/watch?v=iNNL8XXj-YI" target="_blank">
    <img src="http://img.youtube.com/vi/iNNL8XXj-YI/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=H6DBUNni2Ak" target="_blank">
    <img src="http://img.youtube.com/vi/H6DBUNni2Ak/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task requires the agent to play laser tag in a procedurally generated map
containing random gadgets and power-ups. The map is large and there is 1
opponent bot of difficulty level 4. The agent begins the episode with the
default Rapid Gadget and a limit of 100 tags. The agent’s Shield will begin at
125 and slowly drop to the max amount of 100. The gadgets, powerups and map
layout are random per episode and so the agent must adapt to each new
environment.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `lasertag_one_opponent_large`

### Three Opponents Large

<div align="center">
  <a href="https://www.youtube.com/watch?v=7uDO25eyBRY" target="_blank">
    <img src="http://img.youtube.com/vi/7uDO25eyBRY/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=CY-JYDz8qL4" target="_blank">
    <img src="http://img.youtube.com/vi/CY-JYDz8qL4/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task requires the agent to play laser tag in a procedurally generated map
containing random gadgets and power-ups. The map is large and there are 3
opponent bots of difficulty level 4. The agent begins the episode with the
default Rapid Gadget and a limit of 100 tags. The agent’s Shield will begin at
125 and slowly drop to the max amount of 100. The gadgets, powerups and map
layout are random per episode and so the agent must adapt to each new
environment.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `lasertag_three_opponents_large`

## NatLab

### Fixed Large Map

<div align="center">
  <a href="https://www.youtube.com/watch?v=urYc9vaWQ7A" target="_blank">
    <img src="http://img.youtube.com/vi/urYc9vaWQ7A/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=ucJEnnn5iC8" target="_blank">
    <img src="http://img.youtube.com/vi/ucJEnnn5iC8/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This is a long term memory variation of a mushroom foraging task. The agent must
collect mushrooms within a naturalistic terrain environment to maximise score.
The mushrooms do not regrow. The map is a fixed large-sized environment. The
time of day is randomised (day, dawn, night). Each episode the spawn location is
picked randomly from a set of potential spawn locations.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `natlab_fixed_large_map`

### Varying Map Regrowth

<div align="center">
  <a href="https://www.youtube.com/watch?v=pcIznPLhGpc" target="_blank">
    <img src="http://img.youtube.com/vi/pcIznPLhGpc/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=P0ctwiDvcN4" target="_blank">
    <img src="http://img.youtube.com/vi/P0ctwiDvcN4/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This is a short term memory variation of a mushroom foraging task. The agent
must collect mushrooms within a naturalistic terrain environment to maximise
score. The mushrooms regrow after around one minute in the same location
throughout the episode. The map is a randomized small-sized environment. The
topographical variation, and number, position, orientation and sizes of shrubs,
cacti and rocks are all randomized. The time of day is randomised (day, dawn,
night). The spawn location is randomised for each episode.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `natlab_varying_map_regrowth`

### Varying Map Randomized

<div align="center">
  <a href="https://www.youtube.com/watch?v=S58T_obXZYk" target="_blank">
    <img src="http://img.youtube.com/vi/S58T_obXZYk/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=TnH5v0859U8" target="_blank">
    <img src="http://img.youtube.com/vi/TnH5v0859U8/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This is a randomized variation of a mushroom foraging task. The agent must
collect mushrooms within a naturalistic terrain environment to maximise score.
The mushrooms do not regrow. The map is randomly generated and of intermediate
size. The topographical variation, and number, position, orientation and sizes
of shrubs, cacti and rocks are all randomised. Locations of mushrooms are
randomized. The time of day is randomized (day, dawn, night). The spawn location
is randomized for each episode.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `natlab_varying_map_randomized`

## SkyMaze

### Irreversible Path Hard

<div align="center">
  <a href="https://www.youtube.com/watch?v=cJLCbuHmsTw" target="_blank">
    <img src="http://img.youtube.com/vi/cJLCbuHmsTw/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=osuOo5wuR6I" target="_blank">
    <img src="http://img.youtube.com/vi/osuOo5wuR6I/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task requires agents to reach a goal located at a distance from the agent’s
starting position. The goal and target are connected by a sequence of platforms
placed at different heights. Jumping is disabled, so higher platforms are
unreachable and the agent won’t be able to backtrack to a higher platform. This
means that the agent is required to plan their route to ensure they do not
become stuck and fail the task.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `skymaze_irreversible_path_hard`

### Irreversible Path Varied

<div align="center">
  <a href="https://www.youtube.com/watch?v=3onoF_i0ioA" target="_blank">
    <img src="http://img.youtube.com/vi/3onoF_i0ioA/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=3yPCTOy2-yg" target="_blank">
    <img src="http://img.youtube.com/vi/3yPCTOy2-yg/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

A variation of the Irreversible Path Hard task. This version of the task will
select a map layout of random difficulty for the agent to solve. The jump action
is disabled (NOOP) for this task.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `skymaze_irreversible_path_varied`

## PsychLab


For details, see:
[Leibo, Joel Z. et al. "Psychlab: A Psychology Laboratory for Deep Reinforcement Learning Agents (2018)"](https://arxiv.org/abs/1801.08116).

### Arbitrary Visuomotor Mapping

In this task, the agent is shown consecutive images with which they must
remember associations with specific movement patterns (locations to point at).
The agent is rewarded if it can remember the action associated with a given
object. The images are drawn from a set of ~ 2500, and the specific associations
are randomly generated and different in each episode.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `psychlab_arbitrary_visuomotor_mapping`

### Continuous Recognition

This task tests familiarity memory. Consecutive images are shown, and the agent
must indicate whether or not they have seen the image before during that
episode. Looking at the left square indicates no, and right indicates yes. The
images (drawn from a set of ~2500) are shown in a different random order in
every episode.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `psychlab_continuous_recognition`

### Sequential Comparison

<div align="center">
  <a href="https://www.youtube.com/watch?v=liS7e1EdW9w" target="_blank">
    <img src="http://img.youtube.com/vi/liS7e1EdW9w/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=foDX24n7rpY" target="_blank">
    <img src="http://img.youtube.com/vi/foDX24n7rpY/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

Two consecutive patterns are shown to the agent. The agent must indicate whether
or not the two patterns are identical. The delay time between the study pattern
and the test pattern is variable.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `psychlab_sequential_comparison`

### Visual Search

<div align="center">
  <a href="https://www.youtube.com/watch?v=G35bJr4aLSo" target="_blank">
    <img src="http://img.youtube.com/vi/G35bJr4aLSo/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=eQntSh48zsE" target="_blank">
    <img src="http://img.youtube.com/vi/eQntSh48zsE/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

A collection of shapes are shown to the agent. The agent must identify whether
or not a specific shape is present in the collection. Each trial consists of the
agent searching for a pink ‘T’ shape. Two black squares at the bottom of the
screen are used for ‘yes’ and ‘no’ responses.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `psychlab_visual_search`

## Explore

### Object Locations Small

<div align="center">
  <a href="https://www.youtube.com/watch?v=P6ViELmBPbI" target="_blank">
    <img src="http://img.youtube.com/vi/P6ViELmBPbI/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=JdfewHpTJ7Q" target="_blank">
    <img src="http://img.youtube.com/vi/JdfewHpTJ7Q/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task requires agents to collect apples. Apples are placed in rooms within
the maze. The agent must collect as many apples as possible before the episode
ends to maximise their score. Upon collecting all of the apples, the level will
reset, repeating until the episode ends. Apple locations, level layout and theme
are randomized per episode. Agent spawn location is randomised per reset.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `explore_object_locations_small`

### Object Locations Large

<div align="center">
  <a href="https://www.youtube.com/watch?v=JLRHiNe1wMo" target="_blank">
    <img src="http://img.youtube.com/vi/JLRHiNe1wMo/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=bSzIDiWa2uY" target="_blank">
    <img src="http://img.youtube.com/vi/bSzIDiWa2uY/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task is the same as Object Locations Small, but with a larger map and
longer episode duration. Apple locations, level layout and theme are randomised
per episode. Agent spawn location is randomised per reset.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `explore_object_locations_large`

### Obstructed Goals Small

<div align="center">
  <a href="https://www.youtube.com/watch?v=kGehDxLDLAU" target="_blank">
    <img src="http://img.youtube.com/vi/kGehDxLDLAU/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=Py8kToAl87c" target="_blank">
    <img src="http://img.youtube.com/vi/Py8kToAl87c/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task is similar to Goal Locations Small - agents are required to find the
goal as fast as possible, but now with randomly opened and closed doors. After
the goal is found, the level restarts. Goal location, level layout and theme are
randomized per episode. Agent spawn location is randomised per reset. Door
states (open/closed) are randomly selected per reset, but a path to the goal
always exists.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `explore_obstructed_goals_small`

### Obstructed Goals Large

<div align="center">
  <a href="https://www.youtube.com/watch?v=6RtJ45V_3Ds" target="_blank">
    <img src="http://img.youtube.com/vi/6RtJ45V_3Ds/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=nGddvGLvkbk" target="_blank">
    <img src="http://img.youtube.com/vi/nGddvGLvkbk/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task is the same as Obstructed Goals Small, but with a larger map and
longer episode duration. Goal location, level layout and theme are randomised
per episode. Agent spawn location is randomised per reset. Door states
(open/closed) are randomly selected per reset, but a path to the goal always
exists.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `explore_obstructed_goals_large`

### Goal Locations Small

<div align="center">
  <a href="https://www.youtube.com/watch?v=_tUXM7ZfvU4" target="_blank">
    <img src="http://img.youtube.com/vi/_tUXM7ZfvU4/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=olSyE5pEClE" target="_blank">
    <img src="http://img.youtube.com/vi/olSyE5pEClE/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task requires agents to find the goal object as fast as possible. After the
goal object is found, the level restarts. Goal location, level layout and theme
are randomised per episode. Agent spawn location is randomised per reset.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `explore_goal_locations_small`

### Goal Locations Large

<div align="center">
  <a href="https://www.youtube.com/watch?v=Do-a5RreNTI" target="_blank">
    <img src="http://img.youtube.com/vi/Do-a5RreNTI/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=nZFl9Wza8RA" target="_blank">
    <img src="http://img.youtube.com/vi/nZFl9Wza8RA/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task is the same as Goal Locations Small, but with a larger map and longer
episode duration. Goal location, level layout and theme are randomised per
episode. Agent spawn location is randomised per reset.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `explore_goal_locations_large`

### Object Rewards Few

<div align="center">
  <a href="https://www.youtube.com/watch?v=CWsEIudVNVA" target="_blank">
    <img src="http://img.youtube.com/vi/CWsEIudVNVA/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=aML4OoHUZes" target="_blank">
    <img src="http://img.youtube.com/vi/aML4OoHUZes/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task requires agents to collect human-recognisable objects placed around a
room. Some objects are from a positive rewarding category, and some are
negative. After all positive category objects are collected, the level restarts.
Level theme, object categories and object reward per category are randomised per
episode. Agent spawn location, object locations and number of objects per
category are randomised per reset.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `explore_object_rewards_few`

### Object Rewards Many

<div align="center">
  <a href="https://www.youtube.com/watch?v=N7c-0Vg4he8" target="_blank">
    <img src="http://img.youtube.com/vi/N7c-0Vg4he8/0.jpg"
         alt="Human player test"
         width="" height="300" border="5" />
  </a>
  <a href="https://www.youtube.com/watch?v=uHisk5tA8i4" target="_blank">
    <img src="http://img.youtube.com/vi/uHisk5tA8i4/0.jpg"
         alt="Performance of IMPALA architecture"
         width="" height="300" border="5" />
  </a>
</div>

This task is a more difficult variant of Object Rewards Few, with an increased
number of goal objects and longer episode duration. Level theme, object
categories and object reward per category are randomised per episode. Agent
spawn location, object locations and number of objects per category are
randomised per reset.

Test Regime: Training and testing levels drawn from the same distribution.

Observation Spec: RGBD

Level Name: `explore_object_rewards_many`
