#pragma once
#include <windows.h>
#include <cstdint>

namespace mem
{
    // This safely reads one typed value from Ravenfield and confirms the entire value was copied.
    template <typename T>
    bool Read(HANDLE hProcess, uintptr_t address, T& value)
    {
        // Null/very-low addresses are never valid Ravenfield object fields and should be rejected immediately.
        if (hProcess == nullptr || address < 0x10000)
            return false;

        // ReadProcessMemory reports the number of bytes actually copied through this output parameter.
        SIZE_T bytesRead = 0;

        // A read succeeds only when Windows copied exactly sizeof(T) bytes into the destination object.
        return ReadProcessMemory(hProcess,
                                 reinterpret_cast<LPCVOID>(address),
                                 &value,
                                 sizeof(T),
                                 &bytesRead) != FALSE &&
               bytesRead == sizeof(T);
    }

    void PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess);
    void NopEx(BYTE* dst, unsigned int size, HANDLE hProcess);
}
