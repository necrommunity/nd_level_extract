#include <windows.h>
#include <psapi.h>
#include <vector>

HANDLE openProcess(std::string window_name)
{
	HWND window = FindWindow(NULL, window_name.c_str());

  	if (window == 0)
	{
  		throw std::runtime_error("Window not found.");
  	}
	else
	{
		DWORD processId;
		GetWindowThreadProcessId(window, &processId);
		HANDLE handle_process = OpenProcess(PROCESS_ALL_ACCESS, false, processId);

		if (!handle_process)
		{
			throw std::runtime_error("Process not found.");
		}
		else
		{
			return handle_process;
		}
	}
  	return 0;
}

void closeProcess(HANDLE handle_process)
{
	CloseHandle(handle_process);
}

int getBaseAddress(HANDLE handle_process, std::string module_name)
{
	HMODULE hMods[1024];
	DWORD cbNeeded;
	TCHAR path_module[MAX_PATH];

	if (EnumProcessModules(handle_process, hMods, 1024, &cbNeeded))
	{
		for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			if (GetModuleFileNameEx(handle_process, hMods[i], path_module, sizeof(path_module) / sizeof(TCHAR)))
			{
				std::string path_module_string = path_module;

				if (path_module_string.find(module_name) != std::string::npos)
				{
					return (int)hMods[i];
				}
			}
		}
	}

	return -1;
}

int readMemoryInt(HANDLE handle_process, int address, unsigned int data_size)
{
	int value = 0;

	if (!ReadProcessMemory(handle_process, (LPVOID)address, &value, data_size, NULL))
	{
		throw std::runtime_error("Could not read from memory.");
	}

	return value;
}

float readMemoryFloat(HANDLE handle_process, int address)
{
	int temp = readMemoryInt(handle_process, address, 4);
	return *(float *)&temp;
}

std::wstring readMemoryUnicodeString(HANDLE handle_process, int address, unsigned int data_size, bool zeroTerminate)
{
	std::wstring value = L"";
	wchar_t temp;

	for (unsigned int i = 0; i < data_size; i++)
	{
		if (!ReadProcessMemory(handle_process, (LPVOID)address, &temp, 2, NULL))
		{
			throw std::runtime_error("Could not read from memory.");
		}

		if (zeroTerminate && temp == '\x00\x00')
		{
			break;
		}

		value += temp;
		address += 2;
	}

	return value;
}
