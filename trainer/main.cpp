//main.cpp defines the entry point to access RavenField.exe

#include <iostream>
#include <vector>
#include <Windows.h>
#include "process.h"

int main()
{
    //Get Proccess ID of RavenField
    DWORD procId = GetProcessId("RavenField.exe");

    //Get Module Base Address
    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"RavenField.exe");

    std::cout << "test << std::endl";
    //Get A Handle to RavenField

    //Resolving the BaseAddress and going through the Pointer Chain

    //Resolving the Ammo pointer chain

    //Assign ammo to variable

    //Change ammo value

    //Output to console the changed ammo value

    return 0;
}

