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

void PrintMenu(bool healthEnabled, bool ammoEnabled, bool ammoReserveEnabled, bool yAxisEnabled)
{
    system("cls"); // Clear console before printing menu
    std::cout << "========== RAVENFIELD TRAINER ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "[NUMPAD 1] Health: " << (healthEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 2] Ammo: " << (ammoEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 3] Ammo Reserve: " << (ammoReserveEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[NUMPAD 4] Y-Axis: " << (yAxisEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[INSERT]   Exit" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
}

uintptr_t ResolveAddress(HANDLE hProcess, uintptr_t moduleBase, const Address& addr)
{
    uintptr_t dynamicPtrBaseAddr = moduleBase + addr.baseOffset;
    return findDMAAddy(hProcess, dynamicPtrBaseAddr, addr.offsets);
}

bool VerifyAddress(HANDLE hProcess, uintptr_t addr)
{
    if (addr == 0) return false;
    
    // Try reading a small value to verify address is valid
    int testValue = 0;
    BOOL result = ReadProcessMemory(hProcess, (BYTE*)addr, &testValue, sizeof(testValue), 0);
    return result != 0;
}

uintptr_t VerifyAndResolveAddress(HANDLE hProcess, uintptr_t moduleBase, uintptr_t& currentAddr, const Address& addr)
{
    // If current address is valid, return it
    if (VerifyAddress(hProcess, currentAddr)) {
        return currentAddr;
    }
    
    // Address became invalid, re-resolve it
    std::cout << "Re-resolving " << addr.name << "..." << std::endl;
    currentAddr = ResolveAddress(hProcess, moduleBase, addr);
    
    if (currentAddr == 0) {
        std::cout << "Failed to re-resolve " << addr.name << std::endl;
    } else {
        std::cout << "Successfully re-resolved " << addr.name << std::endl;
    }
    
    Sleep(500);
    return currentAddr;
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
    uintptr_t yAxisAddr = ResolveAddress(hProcess, moduleBase, GameAddresses::Y_AXIS);

    if (healthAddr == 0 || ammoAddr == 0 || ammoReserveAddr == 0 || yAxisAddr == 0) {
        std::cout << "Failed to resolve one or more addresses." << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    system("cls"); // Clear console before main menu

    // Read original values
    float originalHealth = 0;
    int originalAmmo = 0;
    int originalAmmoReserve = 0;
    float originalYAxis = 0;

    ReadProcessMemory(hProcess, (BYTE*)healthAddr, &originalHealth, sizeof(originalHealth), 0);
    ReadProcessMemory(hProcess, (BYTE*)ammoAddr, &originalAmmo, sizeof(originalAmmo), 0);
    ReadProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &originalAmmoReserve, sizeof(originalAmmoReserve), 0);
    ReadProcessMemory(hProcess, (BYTE*)yAxisAddr, &originalYAxis, sizeof(originalYAxis), 0);

    // Feature toggles
    bool healthEnabled = false;
    bool ammoEnabled = false;
    bool ammoReserveEnabled = false;
    bool yAxisEnabled = false;
    bool lastHealthState = false;
    bool lastAmmoState = false;
    bool lastAmmoReserveState = false;
    bool lastYAxisState = false;

    PrintMenu(healthEnabled, ammoEnabled, ammoReserveEnabled, yAxisEnabled); // Initial menu draw

    // Main loop
    while (true) {
        // INSERT key to exit (0x2D)
        if (GetAsyncKeyState(0x2D) & 0x8000) {
            break;
        }

        // NUMPAD 1 to toggle health (0x61)
        if (GetAsyncKeyState(0x61) & 0x8000) {
            healthAddr = VerifyAndResolveAddress(hProcess, moduleBase, healthAddr, GameAddresses::HEALTH);
            if (healthAddr != 0) {
                ReadProcessMemory(hProcess, (BYTE*)healthAddr, &originalHealth, sizeof(originalHealth), 0);
                healthEnabled = !healthEnabled;
                float healthValue = healthEnabled ? 9999.0f : originalHealth;
                WriteProcessMemory(hProcess, (BYTE*)healthAddr, &healthValue, sizeof(healthValue), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 2 to toggle ammo (0x62)
        if (GetAsyncKeyState(0x62) & 0x8000) {
            ammoAddr = VerifyAndResolveAddress(hProcess, moduleBase, ammoAddr, GameAddresses::AMMO);
            if (ammoAddr != 0) {
                ReadProcessMemory(hProcess, (BYTE*)ammoAddr, &originalAmmo, sizeof(originalAmmo), 0);
                ammoEnabled = !ammoEnabled;
                int ammoValue = ammoEnabled ? 9999 : originalAmmo;
                WriteProcessMemory(hProcess, (BYTE*)ammoAddr, &ammoValue, sizeof(ammoValue), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 3 to toggle ammo reserve (0x63)
        if (GetAsyncKeyState(0x63) & 0x8000) {
            ammoReserveAddr = VerifyAndResolveAddress(hProcess, moduleBase, ammoReserveAddr, GameAddresses::AMMO_RESERVE);
            if (ammoReserveAddr != 0) {
                ReadProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &originalAmmoReserve, sizeof(originalAmmoReserve), 0);
                ammoReserveEnabled = !ammoReserveEnabled;
                int ammoReserveValue = ammoReserveEnabled ? 9999 : originalAmmoReserve;
                WriteProcessMemory(hProcess, (BYTE*)ammoReserveAddr, &ammoReserveValue, sizeof(ammoReserveValue), 0);
            }
            Sleep(200); // Debounce
        }

        // NUMPAD 4 to toggle y-axis teleport (0x64)
        if (GetAsyncKeyState(0x64) & 0x8000) {
            yAxisAddr = VerifyAndResolveAddress(hProcess, moduleBase, yAxisAddr, GameAddresses::Y_AXIS);
            if (yAxisAddr != 0) {
                ReadProcessMemory(hProcess, (BYTE*)yAxisAddr, &originalYAxis, sizeof(originalYAxis), 0);
                yAxisEnabled = !yAxisEnabled;
                float yAxisValue = yAxisEnabled ? originalYAxis + 100.0f : originalYAxis;
                WriteProcessMemory(hProcess, (BYTE*)yAxisAddr, &yAxisValue, sizeof(yAxisValue), 0);
            }
            Sleep(200); // Debounce
        }

        // Only redraw menu if state changed
        if (healthEnabled != lastHealthState || ammoEnabled != lastAmmoState || 
            ammoReserveEnabled != lastAmmoReserveState || yAxisEnabled != lastYAxisState) {
            PrintMenu(healthEnabled, ammoEnabled, ammoReserveEnabled, yAxisEnabled);
            lastHealthState = healthEnabled;
            lastAmmoState = ammoEnabled;
            lastAmmoReserveState = ammoReserveEnabled;
            lastYAxisState = yAxisEnabled;
        }

        Sleep(5);
    }

    CloseHandle(hProcess);
    system("cls");
    std::cout << "Exiting Ravenfield Trainer..." << std::endl;
    return 0;
}

