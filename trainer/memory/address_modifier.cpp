#include "address_modifier.h"
#include "../process.h"
#include <iostream>

void ModifyAddress(HANDLE hProcess, uintptr_t moduleBase, const Address& addr, const void* value, size_t valueSize)
{
    uintptr_t dynamicPtrBaseAddr = moduleBase + addr.baseOffset;
    uintptr_t finalAddr = findDMAAddy(hProcess, dynamicPtrBaseAddr, addr.offsets);
    
    if (finalAddr == 0) {
        std::cout << "Failed to resolve address: " << addr.name << std::endl;
        return;
    }

    BOOL result = WriteProcessMemory(hProcess, (BYTE*)finalAddr, (void*)value, valueSize, 0);
    
    if (result) {
        std::cout << "Successfully modified " << addr.name << std::endl;
    } else {
        std::cout << "Failed to modify " << addr.name << ". Error: " << GetLastError() << std::endl;
    }
}

void NOPInstruction(HANDLE hProcess, uintptr_t instructionAddr, size_t instructionSize)
{
    // Change memory protection to allow writing
    DWORD oldProtect;
    if (!VirtualProtectEx(hProcess, (LPVOID)instructionAddr, instructionSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        std::cout << "Failed to change memory protection. Error: " << GetLastError() << std::endl;
        return;
    }

    // Create array of NOP bytes (0x90 is NOP in x86/x64)
    BYTE* nopBytes = new BYTE[instructionSize];
    for (size_t i = 0; i < instructionSize; ++i) {
        nopBytes[i] = 0x90;
    }

    // Write NOP bytes to replace the instruction
    BOOL result = WriteProcessMemory(hProcess, (BYTE*)instructionAddr, nopBytes, instructionSize, 0);

    if (result) {
        std::cout << "Successfully NOPed instruction at 0x" << std::hex << instructionAddr << std::dec << std::endl;
    } else {
        std::cout << "Failed to NOP instruction. Error: " << GetLastError() << std::endl;
    }

    // Restore original memory protection
    VirtualProtectEx(hProcess, (LPVOID)instructionAddr, instructionSize, oldProtect, &oldProtect);

    delete[] nopBytes;
}