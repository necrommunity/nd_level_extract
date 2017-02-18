#include <iostream>
#include <map>
#include <iomanip>

#include "tinyxml.h"

#include "handle_memory.h"

#define ATTR_HARDCODED (1 << 16) // Use given value, don't read from memory
#define ATTR_READBYTE (1 << 17) // Read 1 byte from memory
#define ATTR_READUSTR (1 << 18) // [Currently does not work] Read Unicode string, using additional offset (length of string is at 0x4, string begins at 0x8)
#define ATTR_NOITEM (1 << 19) // Set to "no_item"
#define ATTR_BOOL (1 << 20) // Set to 1 if greater than 1

class objType
{
public:
	std::string name_singular;
	std::string name_plural;
	std::map<std::string, unsigned int> attributes;
	std::vector<std::map<std::string, unsigned int>> objList;
	std::vector<unsigned int> ids; // Compare with [[obj+0x0]+0x0]

	objType() {}

	objType(std::string name_singular, std::string name_plural, std::map<std::string, unsigned int> attributes, std::vector<unsigned int> ids)
	{
		this->name_singular = name_singular;
		this->name_plural = name_plural;
		this->attributes = attributes;
		this->ids = ids;
	}
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

	std::vector<objType> objType_list;
	std::map<std::string, unsigned int> attributes; // Temporary

	attributes = {
		{"cracked", ATTR_READBYTE | 0x60},
		{"torch", ATTR_BOOL | 0x64},
		{"type", 0x58},
		{"x", 0x14},
		{"y", 0x18},
		{"zone", 0x5C} // TODO: detect!
	};
	objType objType_tiles = objType("tile", "tiles", attributes, {0x1B3890});
	objType_list.push_back(objType_tiles);

	attributes = {
		{"subtype", ATTR_READBYTE | 0x10C},
		{"type", 0xF0},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_traps = objType("trap", "traps", attributes, {0x1C1E00});
	objType_list.push_back(objType_traps);

	attributes = {
		{"beatDelay", ATTR_HARDCODED | 0}, // 0 is correct for most enemies
		{"lord", ATTR_HARDCODED | 0}, // TODO: detect
		{"type", 0x10C},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_enemies = objType("enemy", "enemies", attributes, {0x1E39A0, 0x1AD5B0, 0x218D30, 0xF3AC0});
	objType_list.push_back(objType_enemies);

	attributes = {
		{"bloodCost", ATTR_HARDCODED | 0}, // Float at offset 0x13C
		{"saleCost", ATTR_HARDCODED | 0}, // Float at offset 0x140
		{"singleChoice", ATTR_HARDCODED | 0}, // TODO: detect
		{"type", ATTR_NOITEM}, // TODO: detect
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_items = objType("item", "items", attributes, {0x180C40, 0x170890});
	objType_list.push_back(objType_items);

	attributes = {
		{"color", 0xF0},
		{"contents", ATTR_NOITEM}, // TODO: detect
		{"hidden", ATTR_HARDCODED | 0}, // TODO: detect
		{"saleCost", ATTR_HARDCODED | 0}, // TODO: detect
		{"singleChoice", ATTR_HARDCODED | 0}, // TODO: detect
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_chests = objType("chest", "chests", attributes, {0x1C6C80, 0x1833F0});
	objType_list.push_back(objType_chests);

	attributes = {
		{"contents", ATTR_NOITEM}, // TODO: detect (unicode string 0x230)
		{"type", 0x22C},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_crates = objType("crate", "crates", attributes, {0x1A7F00});
	objType_list.push_back(objType_crates);

	attributes = {
		{"type", 0xF0},
		{"x", 0x14},
		{"y", 0x18}
	};
	objType objType_shrines = objType("shrine", "shrines", attributes, {0x167220});
	objType_list.push_back(objType_shrines);

	// Get all objects

	std::vector<unsigned int> obj_pointer_offsets = {0x3FF6CC, 0x10, 0x10, 0x10};
	int obj = address_base;

	for (unsigned int i = 0; i < obj_pointer_offsets.size(); i++)
	{
		obj = readMemoryInt(handle_process, obj + obj_pointer_offsets[i]);
	}

	while (true)
	{
		int temp = readMemoryInt(handle_process, obj + 0x18);

		if (!temp)
		{
			break;
		}

		int obj_id = readMemoryInt(handle_process, readMemoryInt(handle_process, temp + 0x0) + 0x0) - address_base;

		bool found = false;
		unsigned int i = 0;

		while(i < objType_list.size())
		{
			for (unsigned int j = 0; j < objType_list[i].ids.size(); j++)
			{
				if (obj_id == objType_list[i].ids[j])
				{
					found = true;
					break;
				}
			}

			if (found)
			{
				break;
			}

			i++;
		}

		if (!found)
		{
			std::cout << "Error: object ID 0x" << std::hex << obj_id - address_base << std::dec << " is undefined. Detected object coordinates: " << readMemoryInt(handle_process, temp + 0x14) << ", " << readMemoryInt(handle_process, temp + 0x18) << "\n";
		}
		else
		{
			std::map<std::string, unsigned int> obj_attributes;

			for (auto const& p : objType_list[i].attributes)
			{
				if (p.second & ATTR_HARDCODED)
				{
					obj_attributes.insert(std::make_pair(p.first, LOWORD(p.second)));
				}
				else if (p.second & ATTR_READBYTE)
				{
					obj_attributes.insert(std::make_pair(p.first, (byte)readMemoryInt(handle_process, temp + LOWORD(p.second), 1)));
				}
				else if (p.second & ATTR_NOITEM)
				{
					obj_attributes.insert(std::make_pair(p.first, p.second));
				}
				else if (p.second & ATTR_BOOL)
				{
					obj_attributes.insert(std::make_pair(p.first, readMemoryInt(handle_process, temp + LOWORD(p.second)) > 1));
				}
				else
				{
					obj_attributes.insert(std::make_pair(p.first, readMemoryInt(handle_process, temp + LOWORD(p.second))));
				}
			}

			objType_list[i].objList.push_back(obj_attributes);
		}
		obj = readMemoryInt(handle_process, obj + 0x10);
	}

	// Close handle for the game

	closeProcess(handle_process);

	// Log results

	for (unsigned int i = 0; i < objType_list.size(); i++)
	{
		std::cout << "Found " << objType_list[i].objList.size() << " " << objType_list[i].name_plural.c_str() << "\n";
	}

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

	for (unsigned int i = 0; i < objType_list.size(); i++)
	{
		objType objType = objType_list[i];

		TiXmlElement* el_level_objects = new TiXmlElement(objType.name_plural.c_str());

		for (unsigned int j = 0; j < objType.objList.size(); j++)
		{
			TiXmlElement *el_level_object = new TiXmlElement(objType.name_singular.c_str());

			for (auto const& p : objType.objList[j])
			{
				/*if (p.second & ATTR_READUSTR)
				{
					int text_len = readMemoryInt(handle_process, LOWORD(p.second) + 0x4);
					std::wstring text = readMemoryUnicodeString(handle_process, LOWORD(p.second) + 0x8, text_len);
					el_level_object->SetAttribute(p.first.c_str(), (char*)text.c_str()); // lazy conversion
				}*/
				if (p.second == ATTR_NOITEM) // TODO: make this actually better
				{
					el_level_object->SetAttribute(p.first.c_str(), "no_item");
				}
				else
				{
					el_level_object->SetAttribute(p.first.c_str(), p.second);
				}
			}

			el_level_objects->LinkEndChild(el_level_object);
		}

		el_level->LinkEndChild(el_level_objects);
	}

	el_dungeon->LinkEndChild(el_level);

	xml.LinkEndChild(decl);
	xml.LinkEndChild(el_dungeon);
	xml.SaveFile("LEVEL.xml");

	// Log results

	std::cout << "XML file generated";

	return 0;
}