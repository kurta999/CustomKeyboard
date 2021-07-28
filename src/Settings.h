#pragma once

#include "utils/CSingleton.h"
#include "CustomMacro.h"

class MacroContainer;
class Settings : public CSingleton < Settings >
{
    friend class CSingleton < Settings >;

public:
    Settings() = default;
    ~Settings() = default;

    void Init();
    void LoadFile();
    void SaveFile();
    bool minimize_on_exit;
    bool minimize_on_startup;
    uint8_t default_page;

private:
    void WriteDefaultIniFile();
    void ParseMacroKeys(size_t id, const std::string& key_code, std::string& str, std::unique_ptr<MacroContainer>& c);
    std::string macro_section;
};