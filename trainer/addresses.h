#pragma once
#include <vector>
#include <cstdint>

struct Address {
    const char* name;  // Changed from const wchar_t*
    uintptr_t baseOffset;
    std::vector<unsigned int> offsets;
    const char* type; // "float", "int", "double", etc.
};

namespace GameAddresses {
    // UnityPlayer.dll base addresses
    const Address HEALTH = {
        "Health",  // Regular string
        0x1A6D6A0,
        { 0x128, 0x38, 0x60, 0x110, 0x128 },
        "float"
    };

    const Address HEALTH_WRITE = {
        "Health Write Instruction",
        0xAE4,
        {},
        "instruction"
    };

    // TODO: Add ammo and stamina addresses later
}