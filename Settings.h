#pragma once

#include "utils/CSingleton.h"
#include "CustomMacro.h"

class MacroContainer;
class Settings : public CSingleton < Settings >
{
    friend class CSingleton < Settings >;

public:
    Settings() = default;
    void Init(void);
private:
    void ParseMacroKeys(size_t id, const char key_code, std::string& str, MacroContainer* c);
    std::string macro_section;
};