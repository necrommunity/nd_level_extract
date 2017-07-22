#pragma once

#include <vector>
#include <windows.h>

HANDLE openProcess(std::string window_name);
void closeProcess(HANDLE handle_process);
int getBaseAddress(HANDLE handle_process, std::string module_name);
int readMemoryInt(HANDLE handle_process, int address, unsigned int data_size = 4);
float readMemoryFloat(HANDLE handle_process, int address);
std::wstring readMemoryUnicodeString(HANDLE handle_process, int address, unsigned int data_size = UINT_MAX, bool zeroTerminate = false);
