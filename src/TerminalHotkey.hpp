#pragma once

#include "utils/CSingleton.hpp"
#include <string>
#include <chrono>

typedef enum : uint8_t
{
    WINDOWS_TERMINAL,
    COMMAND_LINE,
    POWER_SHELL,
    BASH_TERMINAl,
} TerminalType;

class TerminalHotkey : public CSingleton < TerminalHotkey >
{
    friend class CSingleton < TerminalHotkey >;

public:
    TerminalHotkey() = default;

    // !\brief Process function
    void Process();

    // !\brief Is enabled?
    bool is_enabled = true;

    // !\brief Type of terminal which one to open
    TerminalType type{TerminalType::WINDOWS_TERMINAL};

    void SetKey(const std::string& key_str);

    std::string GetKey();

    void UpdateHotkeyRegistration();

private:
    // !\brief VK key code for trigger key
    int vkey;

    // !\brief Open terminal with given path
    // !\param path [in] Path where to open the terminal
    void OpenTerminal(std::wstring& path);
};