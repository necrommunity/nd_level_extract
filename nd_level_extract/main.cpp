#include <iostream>
#include <map>
#include <conio.h>

#include "tinyxml.h"

#include "handle_memory.h"

#define ATTR_HARDCODED 0x80000000

class objType
{
public:
	std::string name_singular;
	std::string name_plural;
	std::vector<unsigned int> pointers;
	std::map<std::string, unsigned int> attributes;
	std::vector<std::map<std::string, unsigned int>> attributes_list;

	objType(std::string name_singular, std::string name_plural, std::vector<unsigned int> pointers, std::map<std::string, unsigned int> attributes, std::vector<std::map<std::string, unsigned int>> attributes_list)
	{
		this->name_singular = name_singular;
		this->name_plural = name_plural;
		this->pointers = pointers;
		this->attributes = attributes;
		this->attributes_list = attributes_list;
	}
};

int main()
{
	HANDLE handle_process;
	std::map<std::string, int> map; // temporary map of attributes
	std::vector<unsigned int> nonTileObjects;

	// Open handle for the game

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

	std::vector<objType> vec_objType;

	std::vector<unsigned int> pointers;
	std::map<std::string, unsigned int> attributes;
	std::vector<std::map<std::string, unsigned int>> tiles, traps, enemies, items, chests, crates, shrines;

	// Define object types except for tiles

	pointers = {0x3FC5C4, 0x10, 0x10, 0x10};
	attributes = {
		{"cracked", ATTR_HARDCODED | 0}, // TODO: detect!
		{"torch", ATTR_HARDCODED | 0}, // TODO: detect!
		{"type", 0x58},
		{"x", 0x14},
		{"y", 0x18},
		{"zone", ATTR_HARDCODED | 0} // TODO: detect!
	};
	objType objType_tiles = objType("tile", "tiles", pointers, attributes, tiles);
	// Don't add to vec_objType, special case

	pointers = {0x3FC5C4, 0x164, 0x10, 0x10};
	attributes = {
		{"subtype", 0x10C},
		{"type", 0xF0},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_traps = objType("trap", "traps", pointers, attributes, traps);
	vec_objType.push_back(objType_traps);

	pointers = {0x3FC5C4, 0x184, 0x10, 0x10};
	attributes = {
		{"beatDelay", ATTR_HARDCODED | 0}, // 0 is correct for most enemies
		{"lord", 0x100},
		{"type", 0x10C},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_enemies = objType("enemy", "enemies", pointers, attributes, enemies);
	vec_objType.push_back(objType_enemies);

	pointers = {0x3FC5C4, 0x1A4, 0x10, 0x10};
	attributes = {
		{"bloodCost", ATTR_HARDCODED | 0}, // float at offset 0x13C
		{"saleCost", ATTR_HARDCODED | 0}, // float at offset 0x140
		{"singleChoice", ATTR_HARDCODED | 0}, // TODO: detect
		{"type", ATTR_HARDCODED | 0}, // TODO: detect
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_items = objType("item", "items", pointers, attributes, items);
	vec_objType.push_back(objType_items);

	pointers = {0x3FC5C4, 0x1C4, 0x10, 0x10};
	attributes = {
		{"color", 0xF0},
		{"contents", ATTR_HARDCODED | 0}, // TODO: detect
		{"hidden", ATTR_HARDCODED | 0}, // TODO: detect
		{"saleCost", ATTR_HARDCODED | 0}, // TODO: detect
		{"singleChoice", ATTR_HARDCODED | 0}, // TODO: detect
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_chests = objType("chest", "chests", pointers, attributes, chests);
	vec_objType.push_back(objType_chests);

	pointers = {0x3FC5C4, 0x1E4, 0x10, 0x10};
	attributes = {
		{"contents", ATTR_HARDCODED | 0}, // TODO: detect
		{"type", 0x22C},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_crates = objType("crate", "crates", pointers, attributes, crates);
	vec_objType.push_back(objType_crates);

	pointers = {0x3FC5C4, 0x204, 0x10, 0x10};
	attributes = {
		{"type", 0xF0},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_shrines = objType("shrine", "shrines", pointers, attributes, shrines);
	vec_objType.push_back(objType_shrines);

	// Get all objects for each objType

	for (unsigned int i = 0; i < vec_objType.size(); i++)
	{
		objType objType = vec_objType[i];

		int obj_curr = address_base;

		for (unsigned int j = 0; j < objType.pointers.size(); j++)
		{
			obj_curr = readMemoryInt(handle_process, obj_curr + objType.pointers[j]);
		}

		while (true)
		{
			int temp = readMemoryInt(handle_process, obj_curr + 0x18);
			if (temp)
			{
				std::map<std::string, unsigned int> obj_attributes;

				for (auto const& p : objType.attributes)
				{
					if (p.second & ATTR_HARDCODED)
					{
						obj_attributes.insert(std::make_pair(p.first, p.second ^ ATTR_HARDCODED));
					}
					else
					{
						obj_attributes.insert(std::make_pair(p.first, readMemoryInt(handle_process, temp + p.second)));
					}
				}

				vec_objType[i].attributes_list.push_back(obj_attributes);
				nonTileObjects.push_back(temp);
				obj_curr = readMemoryInt(handle_process, obj_curr + 0x10);
			}

			else
			{
				break;
			}
		}
	}

	// Get all tiles

	int tile_curr = address_base;

	for (unsigned int i = 0; i < objType_tiles.pointers.size(); i++)
	{
		tile_curr = readMemoryInt(handle_process, tile_curr + objType_tiles.pointers[i]);
	}

	while (true)
	{
		int temp = readMemoryInt(handle_process, tile_curr + 0x18);
		if (temp)
		{
			bool isTile = true;

			for (unsigned int i = 0; i < nonTileObjects.size(); i++)
			{
				if (temp == nonTileObjects[i])
				{
					isTile = false;
					break;
				}
			}

			if (isTile)
			{
				std::map<std::string, unsigned int> attributes_tile;

				for (auto const& p : objType_tiles.attributes)
				{
					if (p.second & ATTR_HARDCODED)
					{
						attributes_tile.insert(std::make_pair(p.first, p.second ^ ATTR_HARDCODED));
					}
					else
					{
						attributes_tile.insert(std::make_pair(p.first, readMemoryInt(handle_process, temp + p.second)));
					}
				}

				objType_tiles.attributes_list.push_back(attributes_tile);
			}
			
			tile_curr = readMemoryInt(handle_process, tile_curr + 0x10);
		}
		else
		{
			vec_objType.insert(vec_objType.begin(), objType_tiles);
			break;
		}
	}

	// Close handle for the game

	closeProcess(handle_process);

	// Create XML file

	TiXmlDocument xml;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");

	TiXmlElement *el_dungeon = new TiXmlElement("dungeon");
	el_dungeon->SetAttribute("character", -1);
	el_dungeon->SetAttribute("name", "LEVEL");
	el_dungeon->SetAttribute("numLevels", 1);

	TiXmlElement *el_level = new TiXmlElement("level");
	el_level->SetAttribute("bossNum", -1);
	el_level->SetAttribute("music", 0);
	el_level->SetAttribute("num", 1);

	for (unsigned int i = 0; i < vec_objType.size(); i++)
	{
		objType objType = vec_objType[i];

		TiXmlElement* el_level_objects = new TiXmlElement(objType.name_plural.c_str());

		for (unsigned int j = 0; j < objType.attributes_list.size(); j++)
		{
			TiXmlElement *el_level_object = new TiXmlElement(objType.name_singular.c_str());

			for (auto const& p : objType.attributes_list[j])
			{
				el_level_object->SetAttribute(p.first.c_str(), p.second);
			}

			el_level_objects->LinkEndChild(el_level_object);
		}

		el_level->LinkEndChild(el_level_objects);
	}

	el_dungeon->LinkEndChild(el_level);

	xml.LinkEndChild(decl);
	xml.LinkEndChild(el_dungeon);
	xml.SaveFile("LEVEL.xml");

	return 0;
}