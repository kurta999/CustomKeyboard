This is a personal project for myself to improve my coding experience. It's open source, beucase why not, maybe somebody will benefit from it one day.

This program supports:
- 1. Connecting a second keyboard and binding macros to it's keys
- 2. Generate offsets of C structures
- 3. Saving screenshot to .png from clipboard
- 4. Backup foldres to another place(s) with a keypress.
- 5. Backend for sensors with SQLite database for measurements & graphs generation to .html file

1. Currently requires an external Nucleo L495ZG board with UART-TTL to USB adapter for tranmitting keypresses to PC. The nucleo is just a simple USB Host, which receives key-presses from the connected keyboard and transmits it to PC via UART. It could be done with hooking in windows, but this solution always crashed the debugger in MSVC, so I went with the more expensive way. Macros can be added in settings.ini file:
Possibilities:
KEY_TYPE[text] = Press & release given keys in sequence to type a text
KEY_SEQ[CTRL+C] = Press all given keys after each other and release it when each was pressed - ideal for key shortcats
DELAY[time in ms] = Waits for given milliseconds
DELAY[min ms - max ms] = Waits randomly between min ms and max ms

Examples:

Opening "About" dialog in visual studio with Num 9:
NUM_9 = MOUSE_MOVE[656,13] DELAY[10] MOUSE_CLICK[LEFT] DELAY[10] MOUSE_MOVE[732,253] DELAY[100] MOUSE_CLICK[LEFT]

Adding new file to Visual Studio with Num 3:
NUM_3 = KEY_SEQ[LCTRL+RSHIFT+A] DELAY[5000] KEY_SEQ[TAB] KEY_TYPE[Src/Test] KEY_SEQ[LSHIFT+TAB] KEY_TYPE[text here] DELAY[2000] KEY_SEQ[ESC]

Per-application macros also supported. Means different macros in Notepad++ & Visual Studio.

2. You paste the structure to input dialog and click on prarse. The application automatically calculates offsets for it's members. It's useful if you work with communication and had to calculate the offset of members in bytes manually.

3. Press screenshot save key (F12 by default, on second keyboard) and screenshot from the clipboard will be saved to Screenshots folder as .png file. For more info open settings.ini

4. Press backup key (F10 by default, on second keyboard) and every folder will be backed up from backup list. For more info open settings.ini

5. Backend means simple TCP server with boost asio where sensor conencts and sends measurements. Each measurement is inserted to a local SQLite database and a graph generated with last 30, last day & last week average data.

Libraries:
fmt
lodepng
sqlite3

Required external depencencies:
Boost 1.76.0
wxWidgets 3.1.5
