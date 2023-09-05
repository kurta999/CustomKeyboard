#pragma once

#include <vector>
#include <filesystem>

#include "ICmdHelper.hpp"

using CommandStorage = std::vector<std::vector<std::vector<CommandTypes>>>;

class ICommandLoader
{
public:
    virtual ~ICommandLoader() { }

    virtual bool Load(const std::filesystem::path& path, CommandStorage& e, CommandPageNames& names, CommandPageIcons& icons) = 0;
    virtual bool Save(const std::filesystem::path& path, CommandStorage& e, CommandPageNames& names, CommandPageIcons& icons) const = 0;
};

class ICmdExecutor
{
public:
    virtual ~ICmdExecutor() { }

    virtual void Init() = 0;
    virtual void SetMediator(ICmdHelper* mediator) = 0;
    virtual void AddCommand(uint8_t page, uint8_t col, Command cmd) = 0;
    virtual void AddSeparator(uint8_t page, uint8_t col, Separator sep) = 0;
    virtual void AddCol(uint8_t page, uint8_t dest_index) = 0;
    virtual void DeleteCol(uint8_t page, uint8_t dest_index) = 0;
    virtual void AddPage(uint8_t page, uint8_t dest_index) = 0;
    virtual void CopyPage(uint8_t page, uint8_t dest_index) = 0;
    virtual void DeletePage(uint8_t page) = 0;
    virtual bool ReloadCommandsFromFile(const char* path) = 0;
    virtual bool Save(const char* path) = 0;
    virtual bool SaveToTempAndReload() = 0;
    virtual uint8_t GetColumns() const = 0;
    virtual CommandStorage& GetCommands() = 0;
    virtual CommandPageNames& GetPageNames() = 0;
    virtual CommandPageIcons& GetPageIcons() = 0;
};
