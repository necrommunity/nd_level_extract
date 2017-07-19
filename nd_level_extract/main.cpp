#include <map>
#include <iomanip>
#include <iostream>
#include <pugixml.hpp>

#include "handle_memory.h"

#define ATTR_HARDCODED (1 << 16) // Use given value, don't read from memory
#define ATTR_READBYTE (1 << 17) // Read 1 byte from memory
#define ATTR_READUSTR (1 << 18) // [Currently does not work] Read Unicode string, using additional offset (length of string is at 0x4, string begins at 0x8)
#define ATTR_NOITEM (1 << 19) // Set to "no_item"
#define ATTR_BOOL (1 << 20) // Set to 1 if greater than 1
#define ATTR_READFLOAT (1 << 21) // Read value as float (temporarily: hardcode to 0)

class obj
{
public:
	unsigned int pointer;
	std::map<std::string, std::string> attributes;
};

class objType
{
public:
	std::string name_singular;
	std::string name_plural;
	std::map<std::string, unsigned int> attributes;
	std::vector<obj> objList;
	std::vector<unsigned int> offsets_firstObj;
};

int main()
{
	HANDLE handle_process;

	// Open handle for the game

	try
	{
		handle_process = openProcess("Crypt of the NecroDancer");
	}
	catch (std::exception e)
	{
		std::cout << "Could not find window.";
		return 1;
	}

	unsigned int address_base = getBaseAddress(handle_process, "NecroDancer.exe");

	// Create all instances of objType

	std::vector<objType> objType_list;

	objType objType_tiles;
	objType_tiles.name_singular = "tile";
	objType_tiles.name_plural = "tiles";
	objType_tiles.attributes = {
		{"cracked", ATTR_READBYTE | 0x60},
		{"torch", ATTR_BOOL | 0x64},
		{"type", 0x58},
		{"x", 0x14},
		{"y", 0x18},
		{"zone", 0x5C}
	};
	objType_tiles.offsets_firstObj = {0x42DBEC, 0x10, 0x10, 0x10};
	// Don't add to objType_list yet, special case

	objType objType_traps;
	objType_traps.name_singular = "trap";
	objType_traps.name_plural = "traps";
	objType_traps.attributes = {
		{"subtype", ATTR_READBYTE | 0x110}, // Buggy
		{"type", 0xF4},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType_traps.offsets_firstObj = {0x42D97C, 0x10, 0x10};
	objType_list.push_back(objType_traps);

	objType objType_enemies;
	objType_enemies.name_singular = "enemy";
	objType_enemies.name_plural = "enemies";
	objType_enemies.attributes = {
		{"beatDelay", 0x114},
		{"lord", ATTR_READBYTE | 0x118},
		{"type", 0x110},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType_enemies.offsets_firstObj = {0x42D9E0, 0x10, 0x10};
	objType_list.push_back(objType_enemies);

	objType objType_items;
	objType_items.name_singular = "item";
	objType_items.name_plural = "items";
	objType_items.attributes = {
		{"bloodCost", ATTR_READFLOAT | 0x148},
		{"saleCost", 0x100},
		{"singleChoice", ATTR_READBYTE | 0xF8},
		{"type", ATTR_READUSTR | 0xF4},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType_items.offsets_firstObj = {0x42D978, 0x10, 0x10};
	objType_list.push_back(objType_items);

	objType objType_chests;
	objType_chests.name_singular = "chest";
	objType_chests.name_plural = "chests";
	objType_chests.attributes = {
		{"color", 0xF4},
		{"contents", ATTR_READUSTR | 0xF8},
		{"hidden", 0x9C},
		{"saleCost", 0x114}, // Buggy
		{"singleChoice", ATTR_READBYTE | 0xFC},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType_chests.offsets_firstObj = {0x42D938, 0x10, 0x10};
	objType_list.push_back(objType_chests);

	objType objType_crates;
	objType_crates.name_singular = "crate";
	objType_crates.name_plural = "crates";
	objType_crates.attributes = {
		{"contents", ATTR_READUSTR | 0x238},
		{"type", 0x234},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType_crates.offsets_firstObj = {0x42D6AC, 0x10, 0x10};
	objType_list.push_back(objType_crates);

	objType objType_shrines;
	objType_shrines.name_singular = "shrine";
	objType_shrines.name_plural = "shrines";
	objType_shrines.attributes = {
		{"type", 0xF4},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType_shrines.offsets_firstObj = {0x42D6F8, 0x10, 0x10};
	objType_list.push_back(objType_shrines);

	// Get all objects of each type (except tiles)

	for (unsigned int i = 0; i < objType_list.size(); i++)
	{
		objType objType = objType_list[i];

		int temp = address_base;

		for each (unsigned int offset in objType.offsets_firstObj)
		{
			temp = readMemoryInt(handle_process, temp + offset);
		}

		while (true)
		{
			int temp2 = readMemoryInt(handle_process, temp + 0x18);

			if (!temp2)
			{
				break;
			}

			obj obj;
			obj.pointer = temp2;
			objType_list[i].objList.push_back(obj);

			temp = readMemoryInt(handle_process, temp + 0x10);
		}
	}

	// Get all tile objects

	int temp = address_base;

	for each (unsigned int offset in objType_tiles.offsets_firstObj)
	{
		temp = readMemoryInt(handle_process, temp + offset);
	}

	while (true)
	{
		int temp2 = readMemoryInt(handle_process, temp + 0x18);
		temp = readMemoryInt(handle_process, temp + 0x10);

		if (!temp2)
		{
			break;
		}

		for each (objType objType in objType_list)
		{
			for each (obj obj in objType.objList)
			{
				if (obj.pointer == temp2)
				{
					continue;
				}
			}
		}

		obj obj;
		obj.pointer = temp2;
		objType_tiles.objList.push_back(obj);
	}

	objType_list.insert(objType_list.begin(), objType_tiles);

	// Get attributes of each object

	for (unsigned int i = 0; i < objType_list.size(); i++)
	{
		for (unsigned int j = 0; j < objType_list[i].objList.size(); j++)
		{
			obj obj = objType_list[i].objList[j];

			for (auto const& p : objType_list[i].attributes)
			{
				if (p.second & ATTR_HARDCODED)
				{
					obj.attributes.insert(std::make_pair(p.first, std::to_string(LOWORD(p.second))));
				}
				else if (p.second & ATTR_READBYTE)
				{
					obj.attributes.insert(std::make_pair(p.first, std::to_string((byte)readMemoryInt(handle_process, obj.pointer + LOWORD(p.second), 1))));
				}
				else if (p.second & ATTR_NOITEM)
				{
					obj.attributes.insert(std::make_pair(p.first, std::to_string(p.second)));
				}
				else if (p.second & ATTR_BOOL)
				{
					obj.attributes.insert(std::make_pair(p.first, std::to_string(readMemoryInt(handle_process, obj.pointer + LOWORD(p.second)) > 1)));
				}
				else
				{
					obj.attributes.insert(std::make_pair(p.first, std::to_string(readMemoryInt(handle_process, obj.pointer + LOWORD(p.second)))));
				}
			}

			objType_list[i].objList[j] = obj;
		}
	}

	// Close handle for the game

	closeProcess(handle_process);

	// Log results

	for (unsigned int i = 0; i < objType_list.size(); i++)
	{
		std::cout << "Found " << objType_list[i].objList.size() << " " << objType_list[i].name_plural.c_str() << "\n";
	}

	// Generate XML document

	pugi::xml_document doc;

	pugi::xml_node node_dungeon = doc.append_child("dungeon");
	node_dungeon.append_attribute("character") = -1;
	node_dungeon.append_attribute("name") = "LEVEL";
	node_dungeon.append_attribute("numLevels") = 1;

	pugi::xml_node node_level = node_dungeon.append_child("level");
	node_level.append_attribute("bossNum") = -1;
	node_level.append_attribute("music") = 0;
	node_level.append_attribute("num") = 1;

	for (unsigned int i = 0; i < objType_list.size(); i++)
	{
		objType objType = objType_list[i];

		pugi::xml_node node_level_objects = node_level.append_child(objType.name_plural.c_str());

		for (unsigned int j = 0; j < objType.objList.size(); j++)
		{
			pugi::xml_node node_level_object = node_level_objects.append_child(objType.name_singular.c_str());

			for (auto const& p : objType.objList[j].attributes)
			{
				node_level_object.append_attribute(p.first.c_str()) = p.second.c_str();
			}
		}
	}

	// Save XML document

	doc.save_file("LEVEL.xml");
	std::cout << "XML file generated";

	return 0;
}