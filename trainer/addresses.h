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
}