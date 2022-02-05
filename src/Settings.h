#pragma once

#include "utils/CSingleton.h"
#include "CustomMacro.h"

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

    // !\brief Default start page for application
    uint8_t default_page = 1;

    // !\brief Remember application window size when resized
    bool remember_window_size = false;

    // !\brief Main frame size
    wxSize window_size = wxSize(WINDOW_SIZE_X, WINDOW_SIZE_Y);

private:
    void ParseMacroKeys(size_t id, const std::string& key_code, std::string& str, std::unique_ptr<MacroAppProfile>& c);
    std::string macro_section;
};