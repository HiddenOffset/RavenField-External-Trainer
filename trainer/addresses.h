#pragma once

#include <vector>
#include <cstdint>

struct Address
{
    const char* name;
    uintptr_t baseOffset;
    std::vector<unsigned int> offsets;
    const char* type;
};

namespace GameAddresses
{
    // ActorManager pointer chain used by the dot ESP.
    const Address ACTOR_MANAGER = {
        "Actor Manager",
        0x01A6D6A0,
        { 0x120, 0x48, 0x60, 0x178, 0xA80 },
        "pointer"
    };

    // Player Object + 0x128.
    const Address HEALTH = {
        "Health",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x128 },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x14C.
    const Address AMMO = {
        "Ammo",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x14C },
        "int"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x1F8.
    const Address AMMO_RESERVE = {
        "Ammo Reserve",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x1F8 },
        "int"
    };

    // Separate chain that remains independent of the Player Object chain.
    const Address Y_AXIS = {
        "Y-Axis",
        0x019EA110,
        { 0x10, 0x60, 0x0, 0x60, 0x0, 0x1FC },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x1A0.
    const Address GUN_SPREAD = {
        "Gun Spread",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x1A0 },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x1B4.
    const Address NO_OVERHEAT = {
        "No OverHeat",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x1B4 },
        "float"
    };

    // Player Object + 0x2A3.
    const Address IGNORE_PLAYER = {
        "Ignore Player",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x2A3 },
        "bool"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x138.
    const Address WALK_BOB_MULTI = {
        "Walk Bob Multi",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x138 },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x13C.
    const Address SPRINT_BOB_MULTI = {
        "Sprint Bob Multi",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x13C },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x140.
    const Address PRONE_BOB_MULTI = {
        "Prone Bob Multi",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x140 },
        "float"
    };

    // Player Object + 0x360.
    const Address SPEED_MULTI = {
        "Speed Multi",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x360 },
        "float"
    };

    // Player cached X coordinate.
    const Address POSITION_X = {
        "Position X",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x2EC },
        "float"
    };

    // Player cached Y coordinate.
    const Address POSITION_Y = {
        "Position Y",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x2F0 },
        "float"
    };

    // Player cached Z coordinate.
    const Address POSITION_Z = {
        "Position Z",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x2F4 },
        "float"
    };
}

// These offsets are relative to already-resolved game objects.
namespace GameOffsets
{
    // ActorManager + 0x100 points to the managed List<Actor>.
    inline constexpr uintptr_t ACTOR_MANAGER_ACTORS = 0x100;

    // ActorManager + 0x110 points to the local player's Actor.
    inline constexpr uintptr_t ACTOR_MANAGER_PLAYER = 0x110;

    // List<T> + 0x10 points to its managed backing array.
    inline constexpr uintptr_t LIST_ITEMS = 0x10;

    // List<T> + 0x18 stores the number of valid entries.
    inline constexpr uintptr_t LIST_SIZE = 0x18;

    // Managed reference arrays begin their first element at +0x20.
    inline constexpr uintptr_t MANAGED_ARRAY_FIRST_ELEMENT = 0x20;

    // Actor + 0x18 stores the Actor's team.
    inline constexpr uintptr_t ACTOR_TEAM = 0x18;

    // Actor + 0x20 points to the Actor's controller.
    inline constexpr uintptr_t ACTOR_CONTROLLER = 0x20;

    // Actor + 0x128 stores the Actor's health.
    inline constexpr uintptr_t ACTOR_HEALTH = 0x128;

    // Actor + 0x2EC stores cachedPosition as a Vector3.
    inline constexpr uintptr_t ACTOR_CACHED_POSITION = 0x2EC;

    // Controller + 0x40 points to fpParent.
    inline constexpr uintptr_t CONTROLLER_FP_PARENT = 0x40;

    // Controller + 0x120 stores the active camera WorldToLocal Matrix4x4 inline.
    inline constexpr uintptr_t CONTROLLER_WORLD_TO_LOCAL = 0x120;

    // fpParent + 0x0F4 stores the current ADS interpolation ratio.
    inline constexpr uintptr_t FP_PARENT_FOV_RATIO = 0x0F4;

    // fpParent + 0x104 stores the normal hip-fire FOV.
    inline constexpr uintptr_t FP_PARENT_NORMAL_FOV = 0x104;

    // fpParent + 0x108 stores the current weapon's fully zoomed ADS FOV.
    inline constexpr uintptr_t FP_PARENT_ZOOM_FOV = 0x108;
}