This is a personal project for myself to improve my daily computer usage, particularly with programming and testing. I've implemented things what I really needed to be more productive and accomplish things faster. It's open source, because why not, maybe somebody will benefit from it one day.

If you want to use/resuse some part(s) of the project and you have a question, feel free to open a Pull Request.

## Features
1. **CustomMacro** - Connect a second keyboard and binding macros to it's keys - full GUI support for macro editing with macro recorder
2. **Backend for Corsair's G Keys** - Bind macros to G keys as those were on second keyboard, without even installing iCUE
3. **CAN-USB Transreceiver** - Send and Receive CAN Frames to/from CAN bus via computer trough USB - see more info below
4. **Command excutor** - Bind specific commands (cmds) to GUI buttons, which is then excuted on GUI button click - see more info below
5. **StructParser** - Generate offsets of C structures
6. **ScreenshotSaver** - Saving screenshot to .png from clipboard
7. **DirectoryBackup** - Backup folders to another place(s) from tray menu
8. **CPU Power Saver** - Saves power by reducing CPU frequency after X idle time to Y percent. For example I can save 10-15W hourly by reducing my overclocked i7 10700K to 800 - 1200Mhz while my PC is in idle. If the median CPU usage gets above max configured percent due to some background tasks, the frequency will be restored and won't be limited again until it falls below configured minimum median load again.
9. **LinkCreator** - Easy symlink & hardlink creator within Windows' file explorer
10. **Sensors** - TCP Backend for sensors with SQLite database for measurements & HTTP Web server for reading measurement's graphs. By default, graphs can be accessed at: http://localhost:2005/graphs
11. **wxWidget's GUI editor** - A very basic GUI editor with move & resize support *(1)
12. **Filesystem browser** - List files and directories recursively by size starting from specific path - useful for finding bloat on storage
13. **AntiLock** - Bypass idle timeout for Windows to avoid lock screen by pressing SCROLL LOCK & moving mouse in given interval
14. **AntiNumLock** - Doesn't allow to disable NumLock, re-enables it immendiately when it's disabled.
15. **TerminalHotkey** - Hotkey for terminal, like in Linux. Can be launched from Windows Explorer with current path as starting directory and from Desktop
16. **File explorer opener** - Open file explorer by sending a specific TCP packet to this application
17. **MTA -> SA-MP MapConverter** - Convert maps from MTA-SA Race and MTA:SA DM to SA-MP with every item which is supported by SA-MP; objects, actors, vehicles with components, checkpoints and pickups.

*(1) This is an outdated part of this project, it doesn't support sizers. It's not recommended to create GUI without sizers, only use it if you know what you are doing.

## Explanation
1. **CustomMacro** - Currently requires an external Nucleo L495ZG board with UART-TTL to USB adapter for transmitting keypresses to PC. The nucleo is just a simple USB Host, which receives key-presses from the connected keyboard and transmits it to PC via UART. It could be done with hooking in windows, but this solution always crashed the debugger in MSVC, so I went with the more expensive way. Macros can be added in configuration page or in settings.ini file directly. Firmware for Nucle board is available here: https://github.com/kurta999/UsbHost

2. **Backend for Corsair's G Keys** - Corsair pretty well fucked up it's iCUE, sometimes memory usage can grow to 500MB. That's enormeous for an application which runs in background and executes macro for binded keys. It's possible to use CustomMacro feature to bind everything to Corsair's G keys (beside second keyboard, of coruse). This is a simple HID API which receives keypresses for G keys. Supports K95 RGB (18 G keys, older one) and K95 RGB Platinum.

3. **CAN-USB Transreceiver** - Requires NUCLEO-G474RE board with UART-TTL to USB adapter & Waveshare SN65HVD230 3.3v CAN Transreceiver or something else which converts TTL signals to real CAN signal. Supports standard, extended, ISO-TP (ISO 15765-2) CAN frames, logging and searching between them. Bits and bytes for CAN frame also can be binded to be able to manipulate them easyer with GUI. Firmware for nucleo board is available here: https://github.com/kurta999/CANUSB

4. **Command excutor** - Binding commands to GUI buttons possible in Cmds.xml or in the "CMD Executor" panel. That command will be executed on Windows with CreateProcess if you click on it's button, see the image below. This is very usful feature if you work often with command line, you don't have to copy paste every comand or type it's alias. Currently one variable parameter can be added to each command, you can change that before execution by clicking with MIDDLE mouse to the command button instead of the LEFT. 

5. **StructParser** - Paste the structure to input dialog and click on prarse. The application automatically calculates offsets for it's members. It's useful if you work with communication and had to calculate the offset of members in bytes manually. Supports embedded structures, struct alignment (fixed alignment supported, pragma pack implementation isn't finished yet), preprocessor definitions & unions. 

6. **ScreenshotSaver** - Press the screenshot save key (F12 by default, on second keyboard) and the screenshot from the clipboard will be saved to (app path)/Screenshots folder by default as .png file. Can be changed in configuration.

7. **DirectoryBackup** - Setup backups in configuration or in settings.ini manually and those will appear in tray menu of this application. Click on them, and backing up files will start. Supports ignore list & SHA-256 checksum for backed up files for more secure backups. 

8. **LinkCreator** - Select desired files from File Explorer and click the mark key (key down on second keyboard by default) and files (directories too) will be marked for link creation. Go to target directory where you want to place the links for marked files, press symlink key (KEY UP) by default for symlink or hardlink key (KEY RIGHT) by default for hardlinks. That's all, symlink or hardlink is created from marked files in target directory.

9. **Sensors** - Backend means simple TCP server with boost asio where sensor conencts and sends measurements. Each measurement is inserted to a local SQLite database and a graph generated with last 30 measurements, last day & last week average data by default, this can be changed in settings.ini. Database is updated in every 10 minutes, but you can update manually by clicking on "Generate graphs" in front panel.

10. **wxWidget's GUI editor** - Oversimplified GUI editor which sometimes can be useful beside [wxFormBuilder](https://github.com/wxFormBuilder/wxFormBuilder "wxFormBuilder's Homepage"). GUI items are movable & resizable - directly without sizers. C++ code generation also implemented, current workspace can be saved & loaded to/from XML file.

11. **File explorer opener** - This function can be useful if you work with VirtualBox or WSL and using samba for accessing specific parts on guest OS filesystem. First you have to map the network drive in Windows, default drive character is Z: (can be changed in settings.ini - "SharedDriveLetter"), also don't forget to enable TCP_Backend in settings.ini. Here is an example command for opening the file explorer on Windows: "echo expw$(pwd) | netcat <ip address of host os> <TCP_Backend port from settings.ini>". For the best experience, I recommend creating an alias for this command.

12. **MTA -> SA-MP MapConverter** - If you worry that some webpages will stole your maps during conversion, you can safely use this tool. Even if you don't trust me, you can check the source code and rebuild this project yourself.

## Libraries
- [lodepng](https://lodev.org/lodepng/ "lodepng's Homepage")
- [sqlite3](https://www.sqlite.org/index.html "sqlite3's Homepage")
- [enumser](http://www.naughter.com/enumser.html "enumser's Homepage")
- [sha256](https://github.com/B-Con/crypto-algorithms "sha256's Homepage")
- [AsyncSerial](https://github.com/fedetft/serial-port "AsyncSerial's Homepage")
- [Chart.js](https://www.chartjs.org/ "Charts.js' Homepage")
- [bitfield](https://github.com/openxc/bitfield-c "bitfield's Homepage")
- [isotp](https://github.com/lishen2/isotp-c "iso-tp's Homepage")

Required external depencencies:
- [Boost 1.81.0](https://www.boost.org/ "Boost's Homepage")
- [wxWidgets 3.2.0](https://www.wxwidgets.org/ "wxWidgets' Homepage")
- [HIDAPI](https://github.com/libusb/hidapi "HIDAPI's Homepage")

## Building

**Windows**
1. Get the latest version of Visual Studio 2022, boost & wxWidgets. My default directories are; 
- boost: C:\Program Files\boost\boost_1_81_0
- wxWidgets: C:\wxWidgets-3.2.0
- HIDAPI: C:\hidapi

Feel free to change, but don't forget to change them too in Visual Studio's project file.

2. Open CustomKeyboard.sln and build the desired build.

Available build configurations:
- x86 - Debug, Release, Static Release
- x64 - Debug, Release, Static Release

CMake support for windows is available, hovewer I'm not using it so it's abandoned. Up to date version is only for Linux, use .sln files when using Windows! Example command for CMake on Windows:
cmake .. -DCMAKE_PREFIX_PATH="C:\GIT_Local\CustomKeyboard\fmt-8.1.1\build;C:\Program Files\boost\boost_1_81_0\stage\lib\cmake" -DwxWidgets_ROOT_DIR=C:\wxWidgets-3.2.0 -DBoost_INCLUDE_DIR="C:\Program Files\boost\boost_1_81_0" -DBoost_LIBRARY_DIR="C:\Program Files\boost\boost_1_81_0\stage\lib" -DFMT_LIB_DIR=C:\GIT_Local\CustomKeyboard\fmt-8.1.1 -G "Visual Studio 17 2022"

**Linux**

The project is using C++20 features, so I recommend using the newest compilers. I'm using clang15 because it's faster for me than GCC.

1. Install boost 1.81.0, wxWidgets 3.2.0, fmt 8.0.0, hidapi
2. Execute these commands in project root directory:

If you do not want to use HIDAPI, you can disable it by setting USE_HIDAPI CMake option to false. (cmake .. -DUSE_HIDAPI=false or do it within CMake GUI)

With Make:
mkdir build
cd build
cmake ..
make  (or "make -j$(nproc)" for faster build)

With ninja:
mkdir build
cd build
cmake -GNinja ..
ninja

## Screenshots
**Main Page**

![Alt text](/github_screens/main_page.png?raw=true "Main page")

**Sensors**

![Alt text](/github_screens/sensors_js_graph.png?raw=true "Temperature graph for last week")

**CAN-USB Transreceiver**

![Alt text](/github_screens/can_usb.png?raw=true "CAN-USB GUI")

**Structure parser**

![Alt text](/github_screens/struct_parser.png?raw=true "C Structure parser (offset generation)")

**Command executor**

![Alt text](/github_screens/cmd_executor.png?raw=true "Execute command by clicking it's button")

**Filesystem browser**

![Alt text](/github_screens/file_browser.png?raw=true "Filesystem browser")

**Configuration**

![Alt text](/github_screens/config_main_page.png?raw=true "Configuration")

**MTA -> SA-MP Map Converter**

![Alt text](/github_screens/map_converter.png?raw=true "MTA -> SA-MP Map Converter")

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

### Linux
**Main Page**

![Alt text](/github_screens/main_page_linux.png?raw=true "Main page in linux build under Ubuntu")

**MTA -> SA-MP Map Converter**

![Alt text](/github_screens/map_converter_linux.png?raw=true "MTA -> SA-MP Map Converter in linux build under Ubuntu")