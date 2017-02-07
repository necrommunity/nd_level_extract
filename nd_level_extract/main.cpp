#include <iostream>
#include <conio.h>

#include "handle_memory.h"

int main()
{
	HANDLE handle_process;

	try
	{
		handle_process = openProcess("Crypt of the NecroDancer");
	}
	catch (std::exception e)
	{
		std::cout << "Could not find window.";
		return 0;
	}

	unsigned int address_base = getBaseAddress(handle_process, "NecroDancer.exe");

	int sprite = address_base;

	std::vector<int> pointers = { 0x3FC2BC, 0x10, 0x10, 0x10 }; // 2nd pointer to 0x48 = traps, 3rd to 0x38 = miniboss

	for (unsigned int i = 0; i < pointers.size(); i++)
	{
		sprite = readMemoryInt(handle_process, sprite + pointers[i]);
	}

	while (true)
	{
		try
		{
			std::cout << readMemoryInt(handle_process, readMemoryInt(handle_process, sprite + 0x18) + 0x14) << "\n";
		}
		catch (std::runtime_error)
		{
			break;
		}

		sprite = readMemoryInt(handle_process, sprite + 0x10);
	}

	closeProcess(handle_process);

	std::getchar();

	return 0;
}