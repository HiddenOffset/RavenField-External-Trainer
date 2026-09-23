//main.cpp defines the entry point to access RavenField.exe

#include <iostream>
#include <vector>
#include <Windows.h>
#include "process.h"
#include "addresses.h"

void PrintWelcome()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Red text for "Raven"
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << "Raven";
    
    // White text for "field Trainer EA32"
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "field Trainer EA32, by take1" << std::endl;
    std::cout << std::endl;
    
    Sleep(1000); // 1 seconds
    system("cls"); // Clear console
}

void PrintMenu(bool healthEnabled, bool ammoEnabled, bool ammoReserveEnabled,
               bool gunSpreadEnabled, bool overHeatEnabled, bool ignorePlayerEnabled,
               bool weaponBobbingEnabled, float speedMultiValue)
{
    system("cls"); // Clear console before printing menu
    std::cout << "============== version 1.2  ===========" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "[NUMPAD 1] Health: " << (healthEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 2] Ammo: " << (ammoEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 3] Ammo Reserve: " << (ammoReserveEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 4] Raise Y-Axis by +0.125" << std::endl;
    std::cout << "[NUMPAD 5] No Gun Spread: " << (gunSpreadEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 6] No OverHeat: " << (overHeatEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 7] Ignore Player: " << (ignorePlayerEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 8] Disable Weapon Bobbing: " << (weaponBobbingEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 9] Speed Multiplier: " << speedMultiValue << "x" << std::endl;
    std::cout << "[INSERT]   Exit" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
}

uintptr_t ResolveAddress(HANDLE hProcess, uintptr_t moduleBase, const Address& addr)
{
    uintptr_t dynamicPtrBaseAddr = moduleBase + addr.baseOffset;
    return findDMAAddy(hProcess, dynamicPtrBaseAddr, addr.offsets);
}

int main()
{
    // Step 1: Welcome screen
    PrintWelcome();

    // Step 2: Get Process ID
    DWORD procId = GetProcessId(L"ravenfield.exe");
    if (procId == 0) {
        std::cout << "Failed to get process ID. Make sure RavenField.exe is running." << std::endl;
        Sleep(2000);
        return 1;
    }

    system("cls"); // Clear console before next window

    // Get Module Base Address
    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"UnityPlayer.dll");
    if (moduleBase == 0) {
        std::cout << "Failed to get module base address." << std::endl;
        Sleep(2000);
        return 1;
    }

    // Get Process Handle
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
    if (hProcess == NULL) {
        std::cout << "Failed to open process." << std::endl;
        Sleep(2000);
        return 1;
    }

    // Resolve all addresses
    uintptr_t healthAddr           = ResolveAddress(hProcess, moduleBase, GameAddresses::HEALTH);
    uintptr_t ammoAddr             = ResolveAddress(hProcess, moduleBase, GameAddresses::AMMO);
    uintptr_t ammoReserveAddr      = ResolveAddress(hProcess, moduleBase, GameAddresses::AMMO_RESERVE);
    uintptr_t yAxisAddr            = ResolveAddress(hProcess, moduleBase, GameAddresses::Y_AXIS);
    uintptr_t gunSpreadAddr        = ResolveAddress(hProcess, moduleBase, GameAddresses::GUN_SPREAD);
    uintptr_t overHeatAddr         = ResolveAddress(hProcess, moduleBase, GameAddresses::NO_OVERHEAT);
    uintptr_t ignorePlayerAddr     = ResolveAddress(hProcess, moduleBase, GameAddresses::IGNORE_PLAYER);
    uintptr_t walkBobMultiAddr     = ResolveAddress(hProcess, moduleBase, GameAddresses::WALK_BOB_MULTI);
    uintptr_t sprintBobMultiAddr   = ResolveAddress(hProcess, moduleBase, GameAddresses::SPRINT_BOB_MULTI);
    uintptr_t proneBobMultiAddr    = ResolveAddress(hProcess, moduleBase, GameAddresses::PRONE_BOB_MULTI);
    uintptr_t speedMultiAddr       = ResolveAddress(hProcess, moduleBase, GameAddresses::SPEED_MULTI);

    if (healthAddr == 0 || ammoAddr == 0 || ammoReserveAddr == 0 || yAxisAddr == 0 ||
        gunSpreadAddr == 0 || overHeatAddr == 0 || ignorePlayerAddr == 0 ||
        walkBobMultiAddr == 0 || sprintBobMultiAddr == 0 || proneBobMultiAddr == 0 || speedMultiAddr == 0) {
        std::cout << "Failed to resolve one or more addresses." << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    system("cls"); // Clear console before main menu

    // Stored values for toggling
    float storedHealth = 0;
    int   storedAmmo = 0;
    int   storedAmmoReserve = 0;
    float storedGunSpread = 0.0f;
    float storedOverHeat = 0.0f;
    bool  storedIgnorePlayer = false;

    // Feature toggles
    bool healthEnabled = false;
    bool ammoEnabled = false;
    bool ammoReserveEnabled = false;
    bool gunSpreadEnabled = false;
    bool overHeatEnabled = false;
    bool ignorePlayerEnabled = false;
    bool weaponBobbingEnabled = false; // Starts off (bobbing is normal)

    // State tracking for menu redraw
    bool lastHealthState = false;
    bool lastAmmoState = false;
    bool lastAmmoReserveState = false;
    bool lastGunSpreadState = false;
    bool lastOverHeatState = false;
    bool lastIgnorePlayerState = false;
    bool lastWeaponBobbingState = false;
    float lastSpeedMultiValue = 1.0f;

    // Y-axis one-shot press state
    bool lastYAxisPress = false;
    // Speed multiplier cycling
    bool lastSpeedPress = false;
    float speedMultiValue = 1.0f;

    PrintMenu(healthEnabled, ammoEnabled, ammoReserveEnabled,
              gunSpreadEnabled, overHeatEnabled, ignorePlayerEnabled,
              weaponBobbingEnabled, speedMultiValue); // Initial menu draw

    // Main loop
    while (true) {
        // INSERT key to exit (0x2D)
        if (GetAsyncKeyState(0x2D) & 0x8000) {
            break;
        }

        // NUMPAD 1 to toggle health (0x61)
        if (GetAsyncKeyState(0x61) & 0x8000) {
            healthEnabled = !healthEnabled;
            
            if (healthEnabled) {
                ReadProcessMemory(hProcess, (BYTE*)healthAddr, &storedHealth, sizeof(storedHealth), 0);
                float healthValue = 9999.0f;
                WriteProcessMemory(hProcess, (BYTE*)healthAddr, &healthValue, sizeof(healthValue), 0);
            } else {
                WriteProcessMemory(hProcess, (BYTE*)healthAddr, &storedHealth, sizeof(storedHealth), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 2 to toggle ammo (0x62)
        if (GetAsyncKeyState(0x62) & 0x8000) {
            ammoEnabled = !ammoEnabled;
            
            if (ammoEnabled) {
                ReadProcessMemory(hProcess, (BYTE*)ammoAddr, &storedAmmo, sizeof(storedAmmo), 0);
                int ammoValue = 9999;
                WriteProcessMemory(hProcess, (BYTE*)ammoAddr, &ammoValue, sizeof(ammoValue), 0);
            } else {
                WriteProcessMemory(hProcess, (BYTE*)ammoAddr, &storedAmmo, sizeof(storedAmmo), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 3 to toggle ammo reserve (0x63)
        if (GetAsyncKeyState(0x63) & 0x8000) {
            ammoReserveEnabled = !ammoReserveEnabled;
            
            if (ammoReserveEnabled) {
                ReadProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &storedAmmoReserve, sizeof(storedAmmoReserve), 0);
                int ammoReserveValue = 9999;
                WriteProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &ammoReserveValue, sizeof(ammoReserveValue), 0);
            } else {
                WriteProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &storedAmmoReserve, sizeof(storedAmmoReserve), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 4 to raise Y-axis by +0.125 (0x64) — one-shot per press
        bool yPress = (GetAsyncKeyState(0x64) & 0x8000) != 0;
        if (yPress && !lastYAxisPress) {
            uintptr_t newYAxisAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::Y_AXIS);
            if (newYAxisAddr != 0 && newYAxisAddr != yAxisAddr) {
                yAxisAddr = newYAxisAddr;
            }

            float currentY = 0.0f;
            if (ReadProcessMemory(hProcess, (BYTE*)yAxisAddr, &currentY, sizeof(currentY), 0)) {
                currentY += 0.125f;
                WriteProcessMemory(hProcess, (BYTE*)yAxisAddr, &currentY, sizeof(currentY), 0);
            }
        }
        lastYAxisPress = yPress;

        // NUMPAD 5 to toggle no gun spread (0x65) — continuous -1.0f
        if (GetAsyncKeyState(0x65) & 0x8000) {
            gunSpreadEnabled = !gunSpreadEnabled;

            if (gunSpreadEnabled) {
                ReadProcessMemory(hProcess, (BYTE*)gunSpreadAddr, &storedGunSpread, sizeof(storedGunSpread), 0);
                float spreadValue = -1.0f;
                WriteProcessMemory(hProcess, (BYTE*)gunSpreadAddr, &spreadValue, sizeof(spreadValue), 0);
            } else {
                WriteProcessMemory(hProcess, (BYTE*)gunSpreadAddr, &storedGunSpread, sizeof(storedGunSpread), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 6 to toggle no overheat (0x66) — continuous 0.0f
        if (GetAsyncKeyState(0x66) & 0x8000) {
            overHeatEnabled = !overHeatEnabled;

            if (overHeatEnabled) {
                ReadProcessMemory(hProcess, (BYTE*)overHeatAddr, &storedOverHeat, sizeof(storedOverHeat), 0);
                float overHeatValue = 0.0f;
                WriteProcessMemory(hProcess, (BYTE*)overHeatAddr, &overHeatValue, sizeof(overHeatValue), 0);
            } else {
                WriteProcessMemory(hProcess, (BYTE*)overHeatAddr, &storedOverHeat, sizeof(storedOverHeat), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 7 to toggle ignore player (0x67) — bool 0/1
        if (GetAsyncKeyState(0x67) & 0x8000) {
            ignorePlayerEnabled = !ignorePlayerEnabled;

            ReadProcessMemory(hProcess, (BYTE*)ignorePlayerAddr, &storedIgnorePlayer, sizeof(storedIgnorePlayer), 0);
            bool value = ignorePlayerEnabled ? true : storedIgnorePlayer;
            WriteProcessMemory(hProcess, (BYTE*)ignorePlayerAddr, &value, sizeof(value), 0);
            Sleep(200); // Debounce
        }

        // NUMPAD 8 to toggle weapon bobbing multipliers (0x68)
        if (GetAsyncKeyState(0x68) & 0x8000) {
            weaponBobbingEnabled = !weaponBobbingEnabled;

            if (weaponBobbingEnabled) {
                float bobValue = 0.0f; // disable bobbing
                WriteProcessMemory(hProcess, (BYTE*)sprintBobMultiAddr, &bobValue, sizeof(bobValue), 0);
                WriteProcessMemory(hProcess, (BYTE*)proneBobMultiAddr, &bobValue, sizeof(bobValue), 0);
                WriteProcessMemory(hProcess, (BYTE*)walkBobMultiAddr, &bobValue, sizeof(bobValue), 0);
            } else {
                float bobValue = 1.0f; // Enable (restore to 1.0)
                WriteProcessMemory(hProcess, (BYTE*)walkBobMultiAddr, &bobValue, sizeof(bobValue), 0);
                WriteProcessMemory(hProcess, (BYTE*)sprintBobMultiAddr, &bobValue, sizeof(bobValue), 0);
                WriteProcessMemory(hProcess, (BYTE*)proneBobMultiAddr, &bobValue, sizeof(bobValue), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 9 to cycle speed multiplier (0x69): 1 -> 2 -> 3 -> 1
        bool speedPress = (GetAsyncKeyState(0x69) & 0x8000) != 0;
        if (speedPress && !lastSpeedPress) {
            uintptr_t newSpeedAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::SPEED_MULTI);
            if (newSpeedAddr != 0 && newSpeedAddr != speedMultiAddr) {
                speedMultiAddr = newSpeedAddr;
            }

            speedMultiValue += 1.0f;
            if (speedMultiValue > 3.0f) {
                speedMultiValue = 1.0f;
            }
            WriteProcessMemory(hProcess, (BYTE*)speedMultiAddr, &speedMultiValue, sizeof(speedMultiValue), 0);
        }
        lastSpeedPress = speedPress;

        // Re-resolve dynamic addresses and enforce values
        if (healthEnabled) {
            uintptr_t newHealthAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::HEALTH);
            if (newHealthAddr != 0 && newHealthAddr != healthAddr) healthAddr = newHealthAddr;
            float healthValue = 9999.0f;
            WriteProcessMemory(hProcess, (BYTE*)healthAddr, &healthValue, sizeof(healthValue), 0);
        }

        if (ammoEnabled) {
            uintptr_t newAmmoAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::AMMO);
            if (newAmmoAddr != 0 && newAmmoAddr != ammoAddr) ammoAddr = newAmmoAddr;
            int ammoValue = 9999;
            WriteProcessMemory(hProcess, (BYTE*)ammoAddr, &ammoValue, sizeof(ammoValue), 0);
        }

        if (ammoReserveEnabled) {
            uintptr_t newAmmoReserveAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::AMMO_RESERVE);
            if (newAmmoReserveAddr != 0 && newAmmoReserveAddr != ammoReserveAddr) ammoReserveAddr = newAmmoReserveAddr;
            int ammoReserveValue = 9999;
            WriteProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &ammoReserveValue, sizeof(ammoReserveValue), 0);
        }

        if (gunSpreadEnabled) {
            uintptr_t newGunSpreadAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::GUN_SPREAD);
            if (newGunSpreadAddr != 0 && newGunSpreadAddr != gunSpreadAddr) gunSpreadAddr = newGunSpreadAddr;
            float spreadValue = -1.0f;
            WriteProcessMemory(hProcess, (BYTE*)gunSpreadAddr, &spreadValue, sizeof(spreadValue), 0);
        }

        if (overHeatEnabled) {
            uintptr_t newOverHeatAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::NO_OVERHEAT);
            if (newOverHeatAddr != 0 && newOverHeatAddr != overHeatAddr) overHeatAddr = newOverHeatAddr;
            float overHeatValue = 0.0f;
            WriteProcessMemory(hProcess, (BYTE*)overHeatAddr, &overHeatValue, sizeof(overHeatValue), 0);
        }

        if (ignorePlayerEnabled) {
            uintptr_t newIgnoreAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::IGNORE_PLAYER);
            if (newIgnoreAddr != 0 && newIgnoreAddr != ignorePlayerAddr) ignorePlayerAddr = newIgnoreAddr;
            bool value = true;
            WriteProcessMemory(hProcess, (BYTE*)ignorePlayerAddr, &value, sizeof(value), 0);
        }

        if (weaponBobbingEnabled) {
            uintptr_t newWalkAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::WALK_BOB_MULTI);
            if (newWalkAddr != 0 && newWalkAddr != walkBobMultiAddr) walkBobMultiAddr = newWalkAddr;
            
            uintptr_t newSprintAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::SPRINT_BOB_MULTI);
            if (newSprintAddr != 0 && newSprintAddr != sprintBobMultiAddr) sprintBobMultiAddr = newSprintAddr;
            
            uintptr_t newProneAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::PRONE_BOB_MULTI);
            if (newProneAddr != 0 && newProneAddr != proneBobMultiAddr) proneBobMultiAddr = newProneAddr;

            float bobValue = 0.0f;
            WriteProcessMemory(hProcess, (BYTE*)walkBobMultiAddr, &bobValue, sizeof(bobValue), 0);
            WriteProcessMemory(hProcess, (BYTE*)sprintBobMultiAddr, &bobValue, sizeof(bobValue), 0);
            WriteProcessMemory(hProcess, (BYTE*)proneBobMultiAddr, &bobValue, sizeof(bobValue), 0);
        }

        // Always enforce selected speed multiplier value
        uintptr_t newSpeedAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::SPEED_MULTI);
        if (newSpeedAddr != 0 && newSpeedAddr != speedMultiAddr) speedMultiAddr = newSpeedAddr;
        WriteProcessMemory(hProcess, (BYTE*)speedMultiAddr, &speedMultiValue, sizeof(speedMultiValue), 0);

        // Only redraw menu if state changed
        if (healthEnabled != lastHealthState || ammoEnabled != lastAmmoState || 
            ammoReserveEnabled != lastAmmoReserveState || gunSpreadEnabled != lastGunSpreadState ||
            overHeatEnabled != lastOverHeatState || ignorePlayerEnabled != lastIgnorePlayerState ||
            weaponBobbingEnabled != lastWeaponBobbingState || speedMultiValue != lastSpeedMultiValue) {
            PrintMenu(healthEnabled, ammoEnabled, ammoReserveEnabled,
                      gunSpreadEnabled, overHeatEnabled, ignorePlayerEnabled,
                      weaponBobbingEnabled, speedMultiValue);
            lastHealthState = healthEnabled;
            lastAmmoState = ammoEnabled;
            lastAmmoReserveState = ammoReserveEnabled;
            lastGunSpreadState = gunSpreadEnabled;
            lastOverHeatState = overHeatEnabled;
            lastIgnorePlayerState = ignorePlayerEnabled;
            lastWeaponBobbingState = weaponBobbingEnabled;
            lastSpeedMultiValue = speedMultiValue;
        }

        Sleep(5);
    }

    CloseHandle(hProcess);
    system("cls");
    std::cout << "Exiting Ravenfield Trainer..." << std::endl;
    return 0;
}

//new features in next update:
/*
* - improved speed hack precision
* - better UI/UX for menu
* - additional toggles as needed
* - configurable hotkeys
* - enhanced stability and performance
* - thorough testing and validation
*/