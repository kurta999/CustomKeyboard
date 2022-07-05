#pragma once

#include "utils/CSingleton.hpp"
#include <string>
#include <chrono>

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

    // !\brief Last execution timepoint (used for avoid debouncing)
    std::chrono::steady_clock::time_point last_execution;

private:
    // !\brief Open terminal with given path
    // !\param path [in] Path where to open the terminal
    void OpenTerminal(std::wstring& path);
};