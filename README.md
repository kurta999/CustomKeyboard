This is a personal project for myself to improve my coding experience. It's open source, because why not, maybe somebody will benefit from it one day.

If you want to use/resuse some part(s) of the project and you have a question, feel free to open a Pull Request.

## Features
1. **CustomMacro** - Connect a second keyboard and binding macros to it's keys - full GUI support for macro editing (WIP recording mode)
2. **CAN-USB** - Send CAN Frames to CAN bus from computer - see more info below
3. **StructParser** - Generate offsets of C structures
4. **ScreenshotSaver** - Saving screenshot to .png from clipboard
5. **DirectoryBackup** - Backup folders to another place(s) from tray menu
6. **LinkCreator** - Easy symlink & hardlink creator within Windows' file explorer
7. **Sensors** - Backend for sensors with SQLite database for measurements & graphs generation to .html file
8. **wxWidget's GUI editor** - A very basic GUI editor with move & resize support
9. **AntiLock** - Bypass idle timeout for Windows to avoid lock screen by pressing SCROLL LOCK & moving mouse in given interval
10. **AntiNumLock** - Doesn't allow to disable NumLock, re-enables it immendiately when it's disabled.
11. **TerminalHotkey** - Hotkey for terminal, like in Linux. Can be started from Windows Explorer and from Desktop

## Explanation
1. **CustomMacro** - Currently requires an external Nucleo L495ZG board with UART-TTL to USB adapter for transmitting keypresses to PC. The nucleo is just a simple USB Host, which receives key-presses from the connected keyboard and transmits it to PC via UART. It could be done with hooking in windows, but this solution always crashed the debugger in MSVC, so I went with the more expensive way. Macros can be added in configuration page or in settings.ini file directly. Firmware for Nucle board is available here: https://github.com/kurta999/UsbHost

2. **CAN-USB** - Requires NUCLEO-G474RE board with UART-TTL to USB adapter & Waveshare SN65HVD230 3.3v CAN Transreceiver. Firmware for nucleo board is available here: https://github.com/kurta999/CANUSB

3. **StructParser** - Paste the structure to input dialog and click on prarse. The application automatically calculates offsets for it's members. It's useful if you work with communication and had to calculate the offset of members in bytes manually. Supports embedded structures, struct alignment (fixed alignment supported, pragma pack implementation isn't finished yet), preprocessor definitions & unions. 

4. **ScreenshotSaver** - Press screenshot save key (F12 by default, on second keyboard) and screenshot from the clipboard will be saved to (app path)/Screenshots folder by default as .png file. Can be changed in configuration.

5. **DirectoryBackup** - Setup backups in configuration or in settings.ini manually and those will appear in tray menu of this application. Click on them, and backing up files will start. Supports ignore list & SHA-256 checksum for backed up files for more secure backups. 

6. **LinkCreator** - Select desired files from File Explorer and click the mark key (key down on second keyboard by default) and files (directories too) will be marked for link creation. Go to target directory where you want to place the links for marked files, press symlink key (KEY UP) by default for symlink or hardlink key (KEY RIGHT) by default for hardlinks. That's all, symlink or hardlink is created from marked files in target directory.

7. **Sensors** - Backend means simple TCP server with boost asio where sensor conencts and sends measurements. Each measurement is inserted to a local SQLite database and a graph generated with last 30 measurements, last day & last week average data by default, this can be changed in settings.ini. Database is updated in every 10 minutes, but you can update manually by clicking on "Generate graphs" in front panel.

8. **wxWidget's GUI editor** - Oversimplified GUI editor which sometimes can be useful beside [wxFormBuilder](https://github.com/wxFormBuilder/wxFormBuilder "wxFormBuilder's Homepage"). GUI items are movable & resizable - directly without sizers. C++ code generation also implemented, current workspace can be saved & loaded to/from XML file.

## Libraries
- [fmt](https://fmt.dev/latest/index.html "fmt's Homepage")
- [lodepng](https://lodev.org/lodepng/ "lodepng's Homepage")
- [sqlite3](https://www.sqlite.org/index.html "sqlite3's Homepage")
- [enumser](http://www.naughter.com/enumser.html "enumser's Homepage")
- [sha256](https://github.com/B-Con/crypto-algorithms "sha256's Homepage")
- [AsyncSerial](https://github.com/fedetft/serial-port "AsyncSerial's Homepage")
- [Chart.js](https://www.chartjs.org/ "Charts.js' Homepage")

Required external depencencies:
- [Boost 1.79.0](https://www.boost.org/ "Boost's Homepage")
- [wxWidgets 3.1.7](https://www.wxwidgets.org/ "wxWidgets' Homepage")

## Building

**Windows**
1. Get the latest version of Visual Studio 2022, boost & wxWidgets. My default directories are; 
- boost: C:\Program Files\boost\boost_1_79_0
- wxWidgets: C:\wxWidgets-3.1.7

Feel free to change, but don't forget to change them too in Visual Studio's project file.

2. Open CustomKeyboard.sln and build the desired build.

Available build configurations:
- x86 - Debug, Release, Static Release
- x64 - Debug, Release, Static Release

If you want to go with CMake, you can use this command as a starting point:
cmake .. -DCMAKE_PREFIX_PATH="C:\GIT_Local\CustomKeyboard\fmt-8.1.1\build;C:\Program Files\boost\boost_1_79_0\stage\lib\cmake" -DwxWidgets_ROOT_DIR=C:\wxWidgets-3.1.7 -DBoost_INCLUDE_DIR="C:\Program Files\boost\boost_1_79_0" -DBoost_LIBRARY_DIR="C:\Program Files\boost\boost_1_79_0\stage\lib" -DFMT_LIB_DIR=C:\GIT_Local\CustomKeyboard\fmt-8.1.1 -G "Visual Studio 17 2022"

**Linux**

1. Install boost 1.79.0, wxWidgets 3.17.0, fmt 8.1.1
2. Execute these commands in project root directory:
mkdir build
cd build
cmake ..
make  (or "make -j$(nproc)" for faster build)

## Screenshots
**Main Page**

![Alt text](/github_screens/main_page.png?raw=true "Main page")

**Sensors**

![Alt text](/github_screens/sensors_js_graph.png?raw=true "Temperature graph for last week")

**CAN-USB Transreceiver**

![Alt text](/github_screens/can_usb.png?raw=true "CAN-USB GUI")

**Structure parser**

![Alt text](/github_screens/struct_parser.png?raw=true "C Structure parser (offset generation)")

**Configuration**

![Alt text](/github_screens/config_main_page.png?raw=true "Configuration")

**Macro editor**

![Alt text](/github_screens/macro_editor_1.png?raw=true "Macro editor sample 1")
![Alt text](/github_screens/macro_editor_2.png?raw=true "Macro editor sample 2")
![Alt text](/github_screens/macro_add.png?raw=true "Macro editor add macro")

**Backup**

![Alt text](/github_screens/backup_config.png?raw=true "Backup page")
![Alt text](/github_screens/backup_progress.png?raw=true "Backup in progress")

**GUI Editor**

![Alt text](/github_screens/gui_editor_1.png?raw=true "GUI Editor 1")
![Alt text](/github_screens/gui_editor_2.png?raw=true "GUI Editor 2")

**Log**

![Alt text](/github_screens/log.png?raw=true "Log")