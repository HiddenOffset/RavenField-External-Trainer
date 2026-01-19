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
        { 0x128, 0x38, 0x60, 0x110, 0xA0, 0x130 },
        "int"
    };

    const Address AMMO_RESERVE = {
        "Ammo Reserve",
        0x01A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0xA0, 0x1DC },
        "int"
    };

    const Address Y_AXIS = {
        "Y-Axis",
        0x019EA110,
        { 0x10, 0x60, 0x0, 0x60, 0x0, 0x1FC },
        "float"
    };
}