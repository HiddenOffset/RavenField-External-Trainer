#pragma once
#include <vector>
#include <cstdint>

struct Address {
    const char* name;
    uintptr_t baseOffset;
    std::vector<unsigned int> offsets;
    const char* type;
};

namespace GameAddresses {

    // ActorManager pointer chain used by the read-only dot ESP.
    const Address ACTOR_MANAGER = {
        "Actor Manager",
        0x01A6D6A0,
        { 0x120, 0x48, 0x60, 0x178, 0xA80 },
        "pointer"
    };

    // Player Object + 0x128
    const Address HEALTH = {
        "Health",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x128 },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x14C
    const Address AMMO = {
        "Ammo",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x14C },
        "int"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x1F8
    const Address AMMO_RESERVE = {
        "Ammo Reserve",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x1F8 },
        "int"
    };

    // Separate chain - still working after the update
    const Address Y_AXIS = {
        "Y-Axis",
        0x019EA110,
        { 0x10, 0x60, 0x0, 0x60, 0x0, 0x1FC },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x1A0
    const Address GUN_SPREAD = {
        "Gun Spread",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x1A0 },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x1B4
    const Address NO_OVERHEAT = {
        "No OverHeat",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x1B4 },
        "float"
    };

    // Player Object + 0x2A3
    const Address IGNORE_PLAYER = {
        "Ignore Player",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x2A3 },
        "bool"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x138
    const Address WALK_BOB_MULTI = {
        "Walk Bob Multi",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x138 },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x13C
    const Address SPRINT_BOB_MULTI = {
        "Sprint Bob Multi",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x13C },
        "float"
    };

    // Player Object + 0xA0 -> Weapon Object + 0x140
    const Address PRONE_BOB_MULTI = {
        "Prone Bob Multi",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0xA0, 0x140 },
        "float"
    };

    // Player Object + 0x360
    const Address SPEED_MULTI = {
        "Speed Multi",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x360 },
        "float"
    };


    // Plyaer xyz position 
    const Address POSITION_X = {
    "Position X",
    0x01A6D6A0,
    { 0x128, 0x48, 0x60, 0x178, 0x20, 0x2EC },
    "float"
    };

    const Address POSITION_Y = {
        "Position Y",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x2F0 },
        "float"
    };

    const Address POSITION_Z = {
        "Position Z",
        0x01A6D6A0,
        { 0x128, 0x48, 0x60, 0x178, 0x20, 0x2F4 },
        "float"
    };
}

// These object-relative offsets describe the Mono structures used by the dot ESP.
namespace GameOffsets {
    inline constexpr uintptr_t ACTOR_MANAGER_ACTORS = 0x100;
    inline constexpr uintptr_t ACTOR_MANAGER_PLAYER = 0x110;

    inline constexpr uintptr_t LIST_ITEMS = 0x10;
    inline constexpr uintptr_t LIST_SIZE = 0x18;
    inline constexpr uintptr_t MANAGED_ARRAY_FIRST_ELEMENT = 0x20;

    inline constexpr uintptr_t ACTOR_TEAM = 0x18;
    inline constexpr uintptr_t ACTOR_CONTROLLER = 0x20;
    inline constexpr uintptr_t ACTOR_HEALTH = 0x128;
    inline constexpr uintptr_t ACTOR_CACHED_POSITION = 0x2EC;

    inline constexpr uintptr_t CONTROLLER_FP_PARENT = 0x40;
    inline constexpr uintptr_t CONTROLLER_WORLD_TO_LOCAL = 0x120;

    // This is the 0.0-to-1.0 interpolation amount used while transitioning into ADS.
    inline constexpr uintptr_t FP_PARENT_FOV_RATIO = 0x0F4;

    // This is the normal hip-fire vertical field of view.
    inline constexpr uintptr_t FP_PARENT_NORMAL_FOV = 0x104;

    // This is the current weapon's fully zoomed ADS field of view.
    inline constexpr uintptr_t FP_PARENT_ZOOM_FOV = 0x108;
}
