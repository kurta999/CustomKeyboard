#pragma once

#include "utils/CSingleton.hpp"
#include "CustomMacro.hpp"

class UsedPages
{
public:
    uint16_t pages;
    union
    {
        uint8_t main : 1;
        uint8_t config : 1;
        uint8_t wxeditor : 1;
        uint8_t escaper : 1;
        uint8_t debug : 1;
        uint8_t struct_parser : 1;
        uint8_t file_browser : 1;
        uint8_t cmd_executor : 1;
        uint8_t can : 1;
        uint8_t modbus_master : 1;
        uint8_t log : 1;
    };
};

class MacroAppProfile;
class Settings : public CSingleton < Settings >
{
    friend class CSingleton < Settings >;

public:
    Settings() = default;
    ~Settings() = default;

    // !\brief Initialize settings
    void Init();

    // !\brief Load application settings from settings.ini file
    void LoadFile();

    // !\brief Save application settings to settings.ini file
    // !\param write_default_macros [in] If settings.ini file doesn't exists - write a few macro lines there as an example 
    void SaveFile(bool write_default_macros);

    // !\brief Minimize application on exit
    bool minimize_on_exit = false;

    // !\brief Start application as minimized
    bool minimize_on_startup = false;

    // !\brief Used pages
    UsedPages used_pages;

    // !\brief Default start page for application
    uint8_t default_page = 1;

    // !\brief Remember application window size when resized
    bool remember_window_size = false;

    // !\brief Main frame size
    wxSize window_size = wxSize(WINDOW_SIZE_X, WINDOW_SIZE_Y);

    // !\brief Num lock always on status
    bool always_on_numlock = false;

    // !\bried Shared drive mapped letter
    char shared_drive_letter = 'Z';
private:
    // !\brief Parse and insert macro keys from ini format to it's container 
    // !\param id [in] ID of given entry
    // !\param key_code [in] Reference to key code
    // !\param str [in] String to parse
    // !\param c [in] Reference to macro profile's unique pointer
    void ParseMacroKeys(size_t id, const std::string& key_code, std::string& str, std::unique_ptr<MacroAppProfile>& c);

    // !\brief Current macro section
    std::string macro_section;
};