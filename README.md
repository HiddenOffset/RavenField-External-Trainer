# Ravenfield Trainer EA39 (updated 9/21/26)

## What’s Ravenfield?
Ravenfield is a single-player/team-based shooter with bots. 

## What this project is
A small external trainer for the EA39 build of Ravenfield. It finds key player stats in memory, then reads/writes them to give tweaks (infinite health/ammo, no spread/overheat, faster movement, etc.). This project has been updated to version 1.2 to remain compatible with the latest game update.

## Features & hotkeys
- Numpad 1: Toggle health lock 
- Numpad 2: Toggle ammo 
- Numpad 3: Toggle ammo reserve 
- Numpad 4: Bump Y-axis position by +0.125
- Numpad 5: Toggle no gun spread 
- Numpad 6: Toggle no overheat 
- Numpad 7: Toggle ignore player 
- Numpad 8: Toggle disable weapon bobbing 
- Numpad 9: Cycle speed multiplier (1x → 2x → 3x → 1x)
- Insert: Exit trainer

## Requirements
- Visual Studio 2026 (or compatible MSVC toolset)
- Windows (trainer opens the game process)
- Ravenfield EA39 running (target module: `UnityPlayer.dll`)
- Suggested build: x64 Release (matches the game’s 64-bit EA39 build)

## Build & run
1. Open the solution in Visual Studio.
2. Set configuration to **Release | x64**.
3. Build the project.
4. Launch Ravenfield EA39.
5. Run the trainer **as Administrator** (needed for process memory access).
6. Use the numpad hotkeys in-game.
