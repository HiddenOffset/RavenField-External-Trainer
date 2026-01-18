//main.cpp defines the entry point to access RavenField.exe

#include <iostream>
#include <vector>
#include <Windows.h>
#include "process.h"

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

void PrintMenu(bool healthEnabled)
{
    system("cls"); // Clear console before printing menu
    std::cout << "========== RAVENFIELD TRAINER ==========" << std::endl;
    std::cout << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "[INSERT] Health: " << (healthEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "[ESC]    Exit" << std::endl;
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
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

    // Resolve health address
    uintptr_t dynamicPtrBaseAddr = moduleBase + 0x1A6D6A0;
    std::vector<unsigned int> healthOffsets = { 0x128, 0x38, 0x60, 0x110, 0x128 };
    uintptr_t healthAddr = findDMAAddy(hProcess, dynamicPtrBaseAddr, healthOffsets);

    if (healthAddr == 0) {
        std::cout << "Failed to resolve health address." << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    system("cls"); // Clear console before main menu

    // Step 3: Main feature menu
    float originalHealth = 0;
    ReadProcessMemory(hProcess, (BYTE*)healthAddr, &originalHealth, sizeof(originalHealth), 0);

    float modifiedHealth = 9999.0f;
    bool healthEnabled = false;
    bool lastHealthState = false;

    PrintMenu(healthEnabled); // Initial menu draw

    // Main loop
    while (true) {
        // ESC key to exit (0x1B)
        if (GetAsyncKeyState(0x1B) & 0x8000) {
            break;
        }

        // INSERT key to toggle health (0x2D)
        if (GetAsyncKeyState(0x2D) & 0x8000) {
            healthEnabled = !healthEnabled;
            float valueToWrite = healthEnabled ? modifiedHealth : originalHealth;
            WriteProcessMemory(hProcess, (BYTE*)healthAddr, &valueToWrite, sizeof(valueToWrite), 0);
            Sleep(200); // Debounce
        }

        // Only redraw menu if state changed
        if (healthEnabled != lastHealthState) {
            PrintMenu(healthEnabled);
            lastHealthState = healthEnabled;
        }

        Sleep(5);
    }

    CloseHandle(hProcess);
    system("cls");
    std::cout << "Exiting Ravenfield Trainer..." << std::endl;
    return 0;
}

