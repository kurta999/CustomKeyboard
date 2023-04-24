#pragma once

#include <string>
#include <memory>
#include <variant>

class Command;
class Separator;

using CommandTypes = std::variant<std::shared_ptr<Command>, Separator>;
using CommandPageNames = std::vector<std::string>;

class ICmdHelper
{
public:
    virtual ~ICmdHelper() { }

    virtual void OnPreReload(uint8_t page) = 0;
    virtual void OnPreReloadColumns(uint8_t pages, uint8_t cols) = 0;
    virtual void OnCommandLoaded(uint8_t page, uint8_t col, CommandTypes cmd) = 0;
    virtual void OnPostReload(uint8_t page, uint8_t cols, CommandPageNames& names) = 0;

protected:
};