#pragma once
#include <Windows.h>
#include "../addresses.h"

void ModifyAddress(HANDLE hProcess, uintptr_t moduleBase, const Address& addr, const void* value, size_t valueSize);
void NOPInstruction(HANDLE hProcess, uintptr_t instructionAddr, size_t instructionSize);