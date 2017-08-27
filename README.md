# nd_level_extract

A tool to generate level editor compatible levels for Crypt of the NecroDancer v2.57 (DLC)

## Usage

Simply run the .exe with the game launched and while playing a dungeon

Command line arguments:
```
  -o [--output] <arg>
Output destination. 0 = file, 1 = stdout. Default: 0.

  -f [--filepath] <arg>
Filepath for file output. Default: "LEVEL.xml".

  -c [--character] <arg>
Dungeon character ID. Default: -1 (any).

  -n [--dungeonname] <arg>
Dungeon name. Default: "LEVEL".

  -m [--music] <arg>
Level music. Default: 0 (1-1).
```
