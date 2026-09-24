#include "process.h"
#include <iostream>

namespace
{
    // This stores the process ID being searched and the best matching top-level window found so far.
    struct WindowSearchData
    {
        DWORD processId = 0;
        HWND window = nullptr;
        long long largestClientArea = 0;
    };

    // EnumWindows calls this once for each top-level desktop window.
    BOOL CALLBACK FindMainWindowCallback(HWND hwnd, LPARAM lParam)
    {
        // This recovers the search state passed by FindMainWindow().
        WindowSearchData* search = reinterpret_cast<WindowSearchData*>(lParam);

        // Invalid search state should stop enumeration because continuing cannot produce a result.
        if (search == nullptr)
            return FALSE;

        // This obtains the process ID that owns the current top-level window.
        DWORD windowProcessId = 0;
        GetWindowThreadProcessId(hwnd, &windowProcessId);

        // Windows owned by other processes cannot be Ravenfield's game window.
        if (windowProcessId != search->processId)
            return TRUE;

        // Invisible or owned popup windows are skipped in favor of the main visible game surface.
        if (!IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER) != nullptr)
            return TRUE;

        // This reads the candidate's client rectangle so tiny helper windows can be ignored.
        RECT clientRect{};
        if (!GetClientRect(hwnd, &clientRect))
            return TRUE;

        // This calculates the visible client area for the current candidate.
        const long long width = static_cast<long long>(clientRect.right - clientRect.left);
        const long long height = static_cast<long long>(clientRect.bottom - clientRect.top);
        const long long area = width * height;

        // The largest visible client area is the most reliable Ravenfield main-window candidate.
        if (width > 0 && height > 0 && area > search->largestClientArea)
        {
            search->largestClientArea = area;
            search->window = hwnd;
        }

        // Returning TRUE tells EnumWindows to keep looking for a potentially larger matching window.
        return TRUE;
    }
}

DWORD GetProcessId(const wchar_t* proc)
{
	DWORD procID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);
		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, proc))
				{
					procID = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procID;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				std::wcout << L"Found module: " << modEntry.szModule << L" at 0x" << std::hex << (uintptr_t)modEntry.modBaseAddr << std::dec << std::endl;
				
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					std::wcout << L"MATCHED: " << modEntry.szModule << L" at 0x" << std::hex << modBaseAddr << std::dec << std::endl;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
		CloseHandle(hSnap);
	}
	return modBaseAddr;
}

uintptr_t findDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

HWND FindMainWindow(DWORD procId)
{
    // This initializes a search that considers only windows owned by Ravenfield's process ID.
    WindowSearchData search{};
    search.processId = procId;

    // EnumWindows walks every top-level desktop window and lets the callback select the best match.
    EnumWindows(FindMainWindowCallback, reinterpret_cast<LPARAM>(&search));

    // This returns the largest visible Ravenfield-owned client window, or nullptr if none is ready yet.
    return search.window;
}
