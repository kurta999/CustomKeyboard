#pragma once

#include "utils/CSingleton.hpp"
#include <string>

class TerminalHotkey : public CSingleton < TerminalHotkey >
{
    friend class CSingleton < TerminalHotkey >;

public:
    TerminalHotkey() = default;

    // !\brief Process function
    void Process();

    // !\brief Is enabled?
    bool is_enabled;

    // !\brief wxKeyCode for trigger key
    wxKeyCode vkey;  
};