This is a personal project for myself to improve my coding experience. It's open source, because why not, maybe somebody will benefit from it one day.

## Features
1. Connecting a second keyboard and binding macros to it's keys - full GUI support for macro editing (WIP recording mode)
2. Generate offsets of C structures
3. Saving screenshot to .png from clipboard
4. Backup folders to another place(s) with a keypress or from tray menu
5. Easy symlink & hardlink creator within Windows' file explorer
6. Backend for sensors with SQLite database for measurements & graphs generation to .html file

## Explanation
1. Currently requires an external Nucleo L495ZG board with UART-TTL to USB adapter for tranmitting keypresses to PC. The nucleo is just a simple USB Host, which receives key-presses from the connected keyboard and transmits it to PC via UART. It could be done with hooking in windows, but this solution always crashed the debugger in MSVC, so I went with the more expensive way. Macros can be added in configuration page or in settings.ini file directly.

2. Paste the structure to input dialog and click on prarse. The application automatically calculates offsets for it's members. It's useful if you work with communication and had to calculate the offset of members in bytes manually. Supports embedded structures, struct alignment (pragma pack) and preprocessor definitions. Unions are not supported.

3. Press screenshot save key (F12 by default, on second keyboard) and screenshot from the clipboard will be saved to Screenshots folder as .png file. For more info open settings.ini

4. Press backup key (F10 by default, on second keyboard) and every folder will be backed up from backup list. For more info open settings.ini

5. Select desired files from File Explorer and click the mark key (key down on second keyboard by default) and files (directories too) will be marked for link creation. Go to target directory where you want to place the links for marked files, press symlink key (KEY UP) by default for symlink or hardlink key (KEY RIGHT) by default for hardlinks. That's all, symlink or hardlink is created from marked files in target directory.

6. Backend means simple TCP server with boost asio where sensor conencts and sends measurements. Each measurement is inserted to a local SQLite database and a graph generated with last 30 measurements and last day & last week average data by default, this can ben changed in settings.ini.

## Libraries
- fmt
- lodepng
- sqlite3
- enumser

Required external depencencies:
- Boost 1.77.0
- wxWidgets 3.1.5

## Building

1. Get the latest version of Visual Studio 2019, boost & wxWidgets. My default directories are; 
- boost: C:\Program Files\boost\boost_1_77_0
- wxWidgets: C:\wxWidgets-3.1.5

Feel free to change, but don't forget to change them too in visual studio project file.

2. Open CustomKeyboard.sln and build the desired build.

Available build configurations:
- x86 - Debug, Release, Static Release
- x64 - Debug, Release, Static Release

## Screenshots

**Main Page**

![Alt text](/github_screens/main_page.png?raw=true "Main page")

**Sensors**

![Alt text](/github_screens/sensors_js_graph.png?raw=true "Temperature graph for last week")

**Structure parser**

![Alt text](/github_screens/struct_parser.png?raw=true "C Structure parser (offset generation)")

**Configuration**

![Alt text](/github_screens/config_main_page.png?raw=true "Main page")

**Macro editor**

![Alt text](/github_screens/macro_editor_1.png?raw=true "Macro editor sample 1")
![Alt text](/github_screens/macro_editor_2.png?raw=true "Macro editor sample 2")
![Alt text](/github_screens/macro_add.png?raw=true "Macro editor add macro")

**Backup**

![Alt text](/github_screens/backup_config.png?raw=true "Backup page")

**Log**

![Alt text](/github_screens/log.png?raw=true "Log")