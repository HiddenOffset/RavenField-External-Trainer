# Ravenfield Trainer EA39 (updated 9/24/26)

## what’s ravenfield?
ravenfield is a single-player/team-based shooter with bots.

## what this project is
a small external trainer for the ea39 build of ravenfield. it finds key player stats and game objects in memory, then reads/writes them to provide features such as infinite health/ammo, no spread/overheat, faster movement, and a basic enemy esp.

the trainer is currently on **version 1.3** and includes a new **bot dot esp** that projects enemy world positions onto the screen and draws a red dot over each living enemy.

## features & hotkeys
- numpad 1: toggle health lock
- numpad 2: toggle ammo
- numpad 3: toggle ammo reserve
- numpad 4: bump y-axis position by +0.125
- numpad 5: toggle no gun spread
- numpad 6: toggle no overheat
- numpad 7: toggle ignore player
- numpad 8: toggle disable weapon bobbing
- numpad 9: cycle speed multiplier (1x → 2x → 3x → 1x)
- f1: toggle bot dot esp
- insert: exit trainer

## bot dot esp
the esp reads the game's actor list and displays a red dot for each living enemy bot.

it currently:
- filters out the local player
- filters out teammates
- filters out dead actors
- uses each enemy actor's cached world position
- uses the active camera world-to-local matrix for world-to-screen conversion
- dynamically adjusts to the current weapon zoom while ads
- dynamically follows the ravenfield window size and position

the current esp is intentionally simple and only draws dots. future improvements may include more accurate head positioning and additional visual options.

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
