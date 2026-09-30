# Ravenfield Trainer EA39 (updated 9/29/26)

## what’s ravenfield?
ravenfield is a single-player/team-based shooter with bots.

## what this project is
a small external trainer for the ea39 build of ravenfield. it finds key player stats, weapon data, actors, and camera information in memory, then reads/writes them to provide gameplay features such as infinite health/ammo, no recoil, no overheat, faster movement, enemy esp, and basic aim assist.

the trainer is currently on **version 1.3**.

the project also includes a **bot dot esp** that projects enemy world positions onto the screen and draws a red dot over each living enemy, as well as a basic **hold-to-aim assist** that uses the same screen-space enemy data.

## features & hotkeys
- numpad 1: toggle health lock
- numpad 2: toggle ammo
- numpad 3: toggle ammo reserve
- numpad 4: bump y-axis position by +0.125
- numpad 5: toggle no recoil
- numpad 6: toggle no overheat
- numpad 7: toggle ignore player
- numpad 8: toggle disable weapon bobbing
- numpad 9: cycle speed multiplier (1x → 2x → 3x → 1x)
- f1: toggle bot dot esp
- left alt: hold aim assist
- insert: exit trainer

## no recoil
the no recoil feature combines the previous no gun spread functionality with additional weapon recoil configuration values.

while enabled, it disables values related to:
- kickback
- random kick
- weapon spread
- follow-up spread
- snap
- rattle
- prone recoil/spread multipliers

the trainer stores the original recoil configuration separately for each weapon/configuration encountered while the feature is enabled.

this allows the player to switch between weapons without losing their individual original recoil values.

when no recoil is disabled, each modified weapon is restored to its original configuration.

the trainer also restores modified recoil values when the trainer exits.

## bot dot esp
the esp reads the game's actor list and displays a red dot for each living enemy bot.

it currently:
- filters out the local player
- filters out teammates
- filters out dead actors
- uses each enemy actor's cached world position
- applies a temporary vertical offset to approximate head position
- uses the active camera world-to-local matrix for world-to-screen conversion
- dynamically adjusts projection to the current weapon zoom while ads
- dynamically follows the ravenfield window size and position
- provides screen-space enemy positions for the aim assist system

the current esp is intentionally simple and only draws dots.

future improvements may include using the actual animated head transform instead of a fixed vertical offset and adding additional visual options.

## aim assist
the trainer includes an experimental screen-space aim assist.

while **left alt** is held, the trainer:
- reads visible enemy positions using the existing esp system
- projects enemies into screen space
- filters enemies outside the aim-assist targeting radius
- selects the enemy closest to the center of the screen
- moves the mouse toward the selected target

the aim assist currently uses the same approximate head position as the bot dot esp.

future improvements may include reading the actual enemy head transform for more accurate targeting.

## technical overview
the trainer currently uses:
- external process memory reading and writing
- multi-level pointer chains
- unity/mono runtime structures
- managed actor lists
- weapon configuration objects
- camera world-to-local matrices
- dynamic field-of-view interpolation
- 3d world-to-screen projection
- win32 transparent overlay rendering
- windows `SendInput` for screen-space aim movement

## requirements
- visual studio 2026 (or compatible msvc toolset)
- windows
- ravenfield ea39 running
- target module: `unityplayer.dll`
- suggested build: **release | x64**

## build & run
1. open the solution in visual studio.
2. set the configuration to **release | x64**.
3. build the project.
4. launch ravenfield ea39.
5. run the trainer **as administrator**.
6. use the hotkeys in-game.
7. press **f1** to enable or disable the bot dot esp.
8. hold **left alt** while an enemy is near the crosshair to use aim assist.

## current limitations
- esp and aim assist currently use a fixed vertical offset rather than the enemy's exact head transform.
- some workshop/community-made maps may not behave the same as standard ravenfield maps.
- the trainer is designed specifically around the ravenfield **ea39** build and may require updated offsets for other versions.

## planned improvements
- actual enemy head/bone position for esp and aim assist
- improved esp visuals
- configurable hotkeys
- improved menu ui/ux
- additional trainer features
- improved stability and address validation
