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
    const Address HEALTH = {
        "Health",
        0x1A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0x128 },
        "float"
    };

    const Address AMMO = {
        "Ammo",
        0x01A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0xA0, 0x14C },
        "int"
    };

    const Address AMMO_RESERVE = {
        "Ammo Reserve",
        0x01A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0xA0, 0x1F8 },
        "int"
    };

    const Address Y_AXIS = {
        "Y-Axis",
        0x019EA110,
        { 0x10, 0x60, 0x0, 0x60, 0x0, 0x1FC },
        "float"
    };

    const Address GUN_SPREAD = {
        "Gun Spread",
        0x01A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0xA0, 0x1A0 },
        "float"
    };

    const Address NO_OVERHEAT = {
        "No OverHeat",
        0x01A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0xA0, 0x1B4 },
        "float"
    };

    const Address IGNORE_PLAYER = {
        "Ignore Player",
        0x1A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0x2A3 },
        "bool"
    };

    const Address WALK_BOB_MULTI = {
        "Walk Bob Multi",
        0x1A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0xA0, 0x138 },
        "float"
    };

    const Address SPRINT_BOB_MULTI = {
        "Sprint Bob Multi",
        0x1A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0xA0, 0x13C },
        "float"
    };

    const Address PRONE_BOB_MULTI = {
        "Prone Bob Multi",
        0x1A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0xA0, 0x140 },
        "float"
    };

    const Address SPEED_MULTI = {
        "Speed Multi",
        0x01A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0x360 },
        "float"
    };
}