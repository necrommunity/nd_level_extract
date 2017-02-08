#include <iostream>
#include <conio.h>

#include "handle_memory.h"

int main()
{
	HANDLE handle_process;
	std::vector<unsigned int> pointers; // temporary vector of pointers
	std::vector<unsigned int> tiles; // list of pointers to tile objects
	std::vector<unsigned int> entities; // list of pointers to entity objects

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

	// Get list of tiles & entities (entities get removed later)

	int tile_curr = address_base;

	pointers = { 0x3FC5C4, 0x10, 0x10, 0x10 };

	for (unsigned int i = 0; i < pointers.size(); i++)
	{
		tile_curr = readMemoryInt(handle_process, tile_curr + pointers[i]);
	}

	while (true)
	{
		int temp = readMemoryInt(handle_process, tile_curr + 0x18);
		if (temp)
		{
			tiles.push_back(temp);
			tile_curr = readMemoryInt(handle_process, tile_curr + 0x10);
		}
		else
		{
			break;
		}
	}
	
	// Get list of entities & remove entities from "tiles" vector

	int entity_curr = address_base;

	pointers = { 0x3FC2BC, 0x10, 0x10, 0x10 };

	for (unsigned int i = 0; i < pointers.size(); i++)
	{
		entity_curr = readMemoryInt(handle_process, entity_curr + pointers[i]);
	}

	while (true)
	{
		int temp = readMemoryInt(handle_process, entity_curr + 0x18);
		if (temp)
		{
			entities.push_back(temp);
			entity_curr = readMemoryInt(handle_process, entity_curr + 0x10);

			for (unsigned int i = 0; i < tiles.size(); i++) // Remove from tile list if it matches the entity
			{
				if (tiles[i] == entities.back())
				{
					tiles.erase(tiles.begin() + i);
					break;
				}
			}
		}

		else
		{
			break;
		}
	}

	std::cout << "Tiles:\n";

	for (unsigned int i = 0; i < tiles.size(); i++)
	{
		std::cout << tiles[i] << "\n";
	}

	std::cout << "Entities:\n";

	for (unsigned int i = 0; i < entities.size(); i++)
	{
		std::cout << entities[i] << "\n";
	}

	closeProcess(handle_process);

	std::getchar();

	return 0;
}