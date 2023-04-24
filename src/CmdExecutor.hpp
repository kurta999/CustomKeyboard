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
    Command(const std::string& name, const std::string& cmd, const std::string& param, uint32_t color, uint32_t bg_color, bool is_bold, float scale) :
        m_name(name), m_cmd(cmd), m_param(param), m_color(color), m_bg_color(bg_color), m_is_bold(is_bold), m_scale(scale)
    {

    }

    void Execute();

    const std::string& GetName() { return m_name; }
    Command& SetName(const std::string& name) { m_name = name; return *this; }    

    const std::string& GetCmd() { return m_cmd; }
    Command& SetCmd(const std::string& cmd) { m_cmd = cmd; return *this; }

    const std::string& GetParam() { return m_param; }
    Command& SetParam(const std::string& param) { m_param = param; return *this; }

    uint32_t GetColor() { return m_color; }
    Command& SetColor(uint32_t color) { m_color = color; return *this; }

    uint32_t GetBackgroundColor() { return m_bg_color; }
    Command& SetBackgroundColor(uint32_t bg_color) { m_bg_color = bg_color; return *this; }

    bool IsBold() { return m_is_bold; }
    Command& SetBold(bool is_bold) { m_is_bold = is_bold; return *this; }

    float GetScale() { return m_scale; }
    Command& SetScale(float scale) { m_scale = scale; return *this; }

private:
    
    // !\ brief Handles hardcoded commands like set current time - ugly, but no time for better solution
    void HandleHarcdodedCommand();

    // !\ brief Handles parameter replacing
    std::string HandleParameters();

    std::string m_name;
    std::string m_cmd;
    std::string m_param;
    uint32_t m_color;
    uint32_t m_bg_color;
    bool m_is_bold;
    float m_scale;
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

using CommandStorage = std::vector<std::vector<std::vector<CommandTypes>>>;

class ICommandLoader
{
public:
    virtual ~ICommandLoader() { }

    virtual bool Load(const std::filesystem::path& path, CommandStorage& e, CommandPageNames& names) = 0;
    virtual bool Save(const std::filesystem::path& path, CommandStorage& e, CommandPageNames& names) = 0;
};

class XmlCommandLoader : public ICommandLoader
{
public:
    XmlCommandLoader(ICmdHelper* mediator);
    virtual ~XmlCommandLoader();

    bool Load(const std::filesystem::path& path, CommandStorage& e, CommandPageNames& names) override;
    bool Save(const std::filesystem::path& path, CommandStorage& e, CommandPageNames& names) override;

private:
    uint8_t m_Cols = 0;
    uint8_t m_Pages = 0;
    ICmdHelper* m_Mediator = nullptr;
};

class ICmdExecutor
{
public:
    virtual ~ICmdExecutor() { }

    virtual void Init() = 0;
    virtual void SetMediator(ICmdHelper* mediator) = 0;
    virtual void AddCommand(uint8_t page, uint8_t col, Command cmd) = 0;
    virtual bool ReloadCommandsFromFile() = 0;
    virtual bool Save() = 0;
    virtual uint8_t GetColumns() = 0;
    virtual CommandStorage& GetCommands() = 0;
    virtual CommandPageNames& GetPageNames() = 0;
};

class CmdExecutor : public ICmdExecutor
{
public:
    CmdExecutor() = default;
    virtual ~CmdExecutor();

    void Init() override;
    void SetMediator(ICmdHelper* mediator) override;
    void AddCommand(uint8_t page, uint8_t col, Command cmd) override;
    bool ReloadCommandsFromFile() override;
    bool Save() override;
    uint8_t GetColumns() override;
    CommandStorage& GetCommands() override;
    CommandPageNames& GetPageNames() override;

    static void WriteDefaultCommandsFile();
private:
    bool AddItem(uint8_t page, uint8_t col, std::shared_ptr<Command>&& cmd);

    uint8_t m_Cols = 2;
    CommandStorage m_Commands;
    CommandPageNames m_CommandPageNames;
    ICmdHelper* m_CmdMediator = nullptr;
};