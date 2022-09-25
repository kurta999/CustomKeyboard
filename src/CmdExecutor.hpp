#pragma once

#include "utils/CSingleton.hpp"
#include <string>
#include <thread>
#include <memory>
#include <variant>

#include "ICmdHelper.hpp"

class Command
{
public:
    Command(std::string name_, std::string cmd_, uint32_t color_, uint32_t bg_color_, bool is_bold_, float scale_) :
        name(name_), cmd(cmd_), color(color_), bg_color(bg_color_), is_bold(is_bold_), scale(scale_)
    {

    }

    std::string name;
    std::string cmd;
    uint32_t color;
    uint32_t bg_color;
    bool is_bold;
    float scale;
};

class Separator
{
public:
    Separator(int width_) :
        width(width_)
    {

    }

    int width;
};

using CommandStorage = std::vector<std::vector<CommandTypes>>;

class ICommandLoader
{
public:
    virtual bool Load(const std::filesystem::path& path, CommandStorage& e) = 0;
    virtual bool Save(const std::filesystem::path& path, CommandStorage& e) = 0;
};

class XmlCommandLoader : public ICommandLoader
{
public:
    XmlCommandLoader(ICmdHelper* mediator);
    virtual ~XmlCommandLoader();

    bool Load(const std::filesystem::path& path, CommandStorage& e) override;
    bool Save(const std::filesystem::path& path, CommandStorage& e) override;

private:
    uint8_t m_Cols = 0;
    ICmdHelper* m_Mediator = nullptr;
};

class ICmdExecutor
{
public:
    virtual void Init() = 0;
    virtual void SetMediator(ICmdHelper* mediator) = 0;
    virtual bool ReloadCommandsFromFile() = 0;
    virtual uint8_t GetColumns() = 0;
};

class CmdExecutor : public ICmdExecutor
{
public:
    CmdExecutor() = default;
    virtual ~CmdExecutor();

    void Init() override;
    void SetMediator(ICmdHelper* mediator) override;
    bool ReloadCommandsFromFile() override;
    uint8_t GetColumns() override;

private:
    uint8_t m_Cols = 2;
    CommandStorage m_Commands;
    ICmdHelper* m_CmdMediator = nullptr;
};