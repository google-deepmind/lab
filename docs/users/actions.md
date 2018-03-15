# DM Lab Actions

## Discrete Actions

All of these input values must be integers.

| Action Name                        | Range         | Description             |
| :--------------------------------- | :------------ | :---------------------- |
| `LOOK_LEFT_RIGHT_PIXELS_PER_FRAME` | \[-512, 512\] | Look left/right angular |
:                                    :               : velocity.               :
| `LOOK_DOWN_UP_PIXELS_PER_FRAME`    | \[-512, 512\] | Look down/up angular    |
:                                    :               : velocity.               :
| `STRAFE_LEFT_RIGHT`                | \[-1, 1\]     | Strafe left/right.      |
| `MOVE_BACK_FORWARD`                | \[-1, 1\]     | Move back/forward       |
| `FIRE`                             | \[0, 1\]      | Fire button held.       |
| `JUMP`                             | \[0, 1\]      | Jump button held.       |
| `CROUCH`                           | \[0, 1\]      | Crouch button held.     |

Look left/right and up/down values are in the units 'pixels' and are the number
of pixels the mouse would move running at 640x480 at 60 frames per second. (With
default mouse speed etc.) (A value of about 57 will rotate the player 360
degrees in one second of game time.)

## Additional Discrete Actions

The following additional actions can be enabled via flags in settings:

Action Name     | Flag           | Range     | Description
:-------------- | -------------: | :-------- | :--------------------------------
`SELECT_GADGET` | `gadgetSelect` | \[0, 10\] | If value > 0 selects gadget value
`SWITCH_GADGET` | `gadgetSwitch` | \[-1, 1\] | Prev/Next Gadget
