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
    std::cout << "field Trainer EA32" << std::endl;
    std::cout << std::endl;
    
    Sleep(2000); // 2 seconds
    system("cls"); // Clear console
}

void PrintMenu(bool healthEnabled, bool ammoEnabled, bool ammoReserveEnabled)
{
    system("cls"); // Clear console before printing menu
    std::cout << "========== RAVENFIELD TRAINER ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "[NUMPAD 1] Health: " << (healthEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 2] Ammo: " << (ammoEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 3] Ammo Reserve: " << (ammoReserveEnabled ? "ON" : "OFF") << std::endl;
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
        return 1;
    }

    system("cls"); // Clear console before next window

    // Get Module Base Address
    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"UnityPlayer.dll");
    if (moduleBase == 0) {
        std::cout << "Failed to get module base address." << std::endl;
        return 1;
    }

    // Get Process Handle
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
    if (hProcess == NULL) {
        std::cout << "Failed to open process." << std::endl;
        return 1;
    }

    // Resolve all addresses
    uintptr_t healthAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::HEALTH);
    uintptr_t ammoAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::AMMO);
    uintptr_t ammoReserveAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::AMMO_RESERVE);

    if (healthAddr == 0 || ammoAddr == 0 || ammoReserveAddr == 0) {
        std::cout << "Failed to resolve one or more addresses." << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    system("cls"); // Clear console before main menu

    // Stored values for toggling
    float storedHealth = 0;
    int storedAmmo = 0;
    int storedAmmoReserve = 0;

    // Feature toggles
    bool healthEnabled = false;
    bool ammoEnabled = false;
    bool ammoReserveEnabled = false;
    bool lastHealthState = false;
    bool lastAmmoState = false;
    bool lastAmmoReserveState = false;

    PrintMenu(healthEnabled, ammoEnabled, ammoReserveEnabled); // Initial menu draw

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
                // Read current value before changing
                ReadProcessMemory(hProcess, (BYTE*)healthAddr, &storedHealth, sizeof(storedHealth), 0);
                float healthValue = 9999.0f;
                WriteProcessMemory(hProcess, (BYTE*)healthAddr, &healthValue, sizeof(healthValue), 0);
            } else {
                // Write back stored value
                WriteProcessMemory(hProcess, (BYTE*)healthAddr, &storedHealth, sizeof(storedHealth), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 2 to toggle ammo (0x62)
        if (GetAsyncKeyState(0x62) & 0x8000) {
            ammoEnabled = !ammoEnabled;
            
            if (ammoEnabled) {
                // Read current value before changing
                ReadProcessMemory(hProcess, (BYTE*)ammoAddr, &storedAmmo, sizeof(storedAmmo), 0);
                int ammoValue = 9999;
                WriteProcessMemory(hProcess, (BYTE*)ammoAddr, &ammoValue, sizeof(ammoValue), 0);
            } else {
                // Write back stored value
                WriteProcessMemory(hProcess, (BYTE*)ammoAddr, &storedAmmo, sizeof(storedAmmo), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 3 to toggle ammo reserve (0x63)
        if (GetAsyncKeyState(0x63) & 0x8000) {
            ammoReserveEnabled = !ammoReserveEnabled;
            
            if (ammoReserveEnabled) {
                // Read current value before changing
                ReadProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &storedAmmoReserve, sizeof(storedAmmoReserve), 0);
                int ammoReserveValue = 9999;
                WriteProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &ammoReserveValue, sizeof(ammoReserveValue), 0);
            } else {
                // Write back stored value
                WriteProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &storedAmmoReserve, sizeof(storedAmmoReserve), 0);
            }
            Sleep(200); // Debounce
        }

        // Only redraw menu if state changed
        if (healthEnabled != lastHealthState || ammoEnabled != lastAmmoState || 
            ammoReserveEnabled != lastAmmoReserveState) {
            PrintMenu(healthEnabled, ammoEnabled, ammoReserveEnabled);
            lastHealthState = healthEnabled;
            lastAmmoState = ammoEnabled;
            lastAmmoReserveState = ammoReserveEnabled;
        }

        Sleep(5);
    }

    CloseHandle(hProcess);
    system("cls");
    std::cout << "Exiting Ravenfield Trainer..." << std::endl;
    return 0;
}