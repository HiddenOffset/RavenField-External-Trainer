#include <vector>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcessId(const wchar_t* proc);

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);

uintptr_t findDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);