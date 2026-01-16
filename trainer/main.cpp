//main.cpp defines the entry point to access RavenField.exe

#include <iostream>
#include <vector>
#include <Windows.h>
#include "process.h"

int main()
{
    //Get Proccess ID of RavenField
    DWORD procId = GetProcessId(L"ravenfield.exe");
    if (procId == 0) {
        std::cout << "Failed to get process ID" << std::endl;
        return 1;
    }

    //Get Module Base Address of UnityPlayer.dll
    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"ravenfield.exe");
    if (moduleBase == 0) {
        std::cout << "Failed to get module base" << std::endl;
        return 1;
    }
    std::cout << "Module Base: 0x" << std::hex << moduleBase << std::dec << std::endl;

    //Get A Handle to RavenField
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
    if (hProcess == NULL) {
        std::cout << "Failed to open process" << std::endl;
        return 1;
    }

    //Resolving the BaseAddress and going through the Pointer Chain
    uintptr_t dynamicPtrBaseAddr = moduleBase + 0x1A6D6A0;
    std::cout << "Dynamic Ptr Base: 0x" << std::hex << dynamicPtrBaseAddr << std::dec << std::endl;

    //Resolving the health pointer chain
    std::vector<unsigned int> healthOffsets = { 0x128, 0x38, 0x60, 0x110, 0x128 };
    uintptr_t healthAddr = findDMAAddy(hProcess, dynamicPtrBaseAddr, healthOffsets);
    
    std::cout << "Health Address: 0x" << std::hex << healthAddr << std::dec << std::endl;

    //Change health value
    float value = 9999;
    BOOL result = WriteProcessMemory(hProcess, (BYTE*)healthAddr, &value, sizeof(value), 0);
    
    if (result) {
        std::cout << "Successfully wrote health!" << std::endl;
    } else {
        std::cout << "WriteProcessMemory failed. Error: " << GetLastError() << std::endl;
    }

    CloseHandle(hProcess);
    return 0;
}

