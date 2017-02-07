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

	int sprite_x = address_base;

	std::vector<int> pointers = { 0x3FC2BC, 0x10, 0x10, 0x10, 0x18, 0x14 };

	for (unsigned int i = 0; i < pointers.size(); i++)
	{
		sprite_x = readMemoryInt(handle_process, sprite_x + pointers[i]);
	}

	std::cout << sprite_x;

	closeProcess(handle_process);

	std::getchar();

	return 0;
}