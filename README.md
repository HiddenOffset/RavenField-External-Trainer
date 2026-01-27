# Ravenfield Trainer EA32

## What’s Ravenfield?
Ravenfield is a single-player/team-based shooter with bots. 

## What this project is
A small external trainer for the EA32 build of Ravenfield. It finds key player stats in memory, then reads/writes them to give tweaks (infinite health/ammo, no spread/overheat, faster movement, etc.).

## Features & hotkeys
- Numpad 1: Toggle health lock (9999)
- Numpad 2: Toggle ammo (9999)
- Numpad 3: Toggle ammo reserve (9999)
- Numpad 4: Bump Y-axis position by +0.125
- Numpad 5: Toggle no gun spread (forces -1.0)
- Numpad 6: Toggle no overheat (forces 0.0)
- Numpad 7: Toggle ignore player (bool on)
- Numpad 8: Toggle walk bobbing (off while enabled)
- Numpad 9: Cycle speed multiplier (1x → 2x → 3x → 1x)
- Insert: Exit trainer

## Requirements
- Visual Studio 2026 (or compatible MSVC toolset)
- Windows (trainer opens the game process)
- Ravenfield EA32 running (target module: `UnityPlayer.dll`)
- Suggested build: x86 Release (matches the game’s 32-bit EA32 build)

## Build & run
1. Open the solution in Visual Studio.
2. Set configuration to **Release | Win32** (x86).
3. Build the project.
4. Launch Ravenfield EA32.
5. Run the trainer **as Administrator** (needed for process memory access).
6. Use the numpad hotkeys in-game.

## How it works (simple version)
- Each stat lives at a dynamic address: `moduleBase (UnityPlayer.dll) + baseOffset + multi-level offsets`.
- The trainer goes through the pointer chain at runtime (see `addresses.h` for base offsets and offsets).
- Reading/writing is done with WinAPI (`ReadProcessMemory`, `WriteProcessMemory`).
- Hotkeys that writes so you can toggle or bump values on demand.
