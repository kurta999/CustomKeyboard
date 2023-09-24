#pragma once

#include "utils/CSingleton.hpp"
#include <string>
#include <thread>
#include <memory>
#include <variant>

#include "ICmdHelper.hpp"
#include "ICmdExecutor.hpp"
#include "IBasicGuiCustomization.hpp"

static constexpr const char* COMMAND_FILE_PATH = "Cmds.xml";

class ICmdExecutorItem
{

};

class Command : public BasicGuiTextCustomization
{
public:
    Command(const std::string& name, const std::string& cmd, const std::string& icon, bool hide_console, uint32_t color, uint32_t bg_color, bool is_bold, const std::string& font_face, float scale, wxSize min_size = wxDefaultSize, bool use_sizer = false, bool add_to_prev_sizer = false) :
        m_name(name), m_cmd(cmd), m_icon(icon), m_hideConsole(hide_console), BasicGuiTextCustomization(color, bg_color, is_bold, scale, font_face),
        m_minSize(min_size), m_useSizer(use_sizer), m_addToPrevSizer(add_to_prev_sizer)
    {
        LoadParametersFromString();
    }

    Command(const Command& rhs) :
        m_name(rhs.m_name), m_cmd(rhs.m_cmd), m_icon(rhs.m_icon), m_hideConsole(rhs.m_hideConsole), BasicGuiTextCustomization(rhs.m_color, rhs.m_bg_color, rhs.m_is_bold, rhs.m_scale, rhs.m_font_face),
        m_useSizer(rhs.m_useSizer), m_addToPrevSizer(rhs.m_addToPrevSizer)
    {
        LoadParametersFromString();
    }

    void Execute();
    void SaveParametersToString();

    const std::string& GetName() const { return m_name; }
    Command& SetName(const std::string& name) { m_name = name; return *this; }    

    const std::string& GetCmd() const { return m_cmd; }
    Command& SetCmd(const std::string& cmd) { m_cmd = cmd; return *this; }

    const std::string& GetIcon() const { return m_icon; }
    Command& SetIcon(const std::string& icon) { m_icon = icon; return *this; }

    bool IsConsoleHidden() const { return m_hideConsole; }
    Command& SetConsoleHidden(bool is_console_hidden) { m_hideConsole = is_console_hidden; return *this; }

    uint32_t GetColor() const { return m_color; }
    Command& SetColor(uint32_t color) { m_color = color; return *this; }

    uint32_t GetBackgroundColor() const { return m_bg_color; }
    Command& SetBackgroundColor(uint32_t bg_color) { m_bg_color = bg_color; return *this; }

    bool IsBold() const { return m_is_bold; }
    Command& SetBold(bool is_bold) { m_is_bold = is_bold; return *this; }

    const std::string& GetFontFace() const { return m_font_face; }
    Command& SetFontFace(const std::string& font_face) { m_font_face = font_face; return *this; }

    float GetScale() const { return m_scale; }
    Command& SetScale(float scale) { m_scale = scale; return *this; }

    wxSize GetMinSize() const { return m_minSize; }
    Command& SetMinSize(wxSize size) { m_minSize = size; return *this; }

    bool IsUsingSizer() const { return m_useSizer; }
    Command& SetUseSizer(bool is_usesizer) { m_useSizer = is_usesizer; return *this; }

    bool IsAddToPrevSizer() const { return m_addToPrevSizer; }
    Command& SetAddToPrevSizer(bool is_prevsizer) { m_addToPrevSizer = is_prevsizer; return *this; }

    std::vector<std::string> m_params;

private:
    
    // !\ brief Handles hardcoded commands like set current time - ugly, but no time for better solution
    void HandleHarcdodedCommand();

    // !\ brief Handles parameter replacing
    std::string HandleParameters();

    void LoadParametersFromString();

    std::string m_name;
    std::string m_cmd;
    std::string m_icon;
    bool m_hideConsole{ false };
    wxSize m_minSize{ wxDefaultSize };
    bool m_useSizer{ false };
    bool m_addToPrevSizer{ false };
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

class XmlCommandLoader : public ICommandLoader
{
public:
    XmlCommandLoader(ICmdHelper* mediator);
    virtual ~XmlCommandLoader() = default;

    bool Load(const std::filesystem::path& path, CommandStorage& e, CommandPageNames& names, CommandPageIcons& icons) override;
    bool Save(const std::filesystem::path& path, CommandStorage& e, CommandPageNames& names, CommandPageIcons& icons) const override;

private:
    uint8_t m_Cols = 0;
    uint8_t m_Pages = 0;
    ICmdHelper* m_Mediator = nullptr;
};

class CmdExecutor : public ICmdExecutor
{
public:
    CmdExecutor() = default;
    virtual ~CmdExecutor() = default;

    void Init() override;
    void SetMediator(ICmdHelper* mediator) override;
    void AddCommand(uint8_t page, uint8_t col, Command cmd) override;
    void RotateCommand(uint8_t page, uint8_t col, Command& cmd, uint8_t direction) override;
    void AddSeparator(uint8_t page, uint8_t col, Separator sep) override;
    void AddCol(uint8_t page, uint8_t dest_index) override;
    void DeleteCol(uint8_t page, uint8_t dest_index) override;
    void AddPage(uint8_t page, uint8_t dest_index) override;
    void CopyPage(uint8_t page, uint8_t dest_index) override;
    void DeletePage(uint8_t page) override;
    bool ReloadCommandsFromFile(const char* path = COMMAND_FILE_PATH) override;
    bool Save(const char* path = COMMAND_FILE_PATH) override;
    bool SaveToTempAndReload() override;
    uint8_t GetColumns() const override;
    CommandStorage& GetCommands() override;
    CommandPageNames& GetPageNames() override;
    CommandPageIcons& GetPageIcons() override;

    void ExecuteByName(const std::string& page_name, const std::string& cmd_name);

    static void WriteDefaultCommandsFile();
private:
    bool AddItem(uint8_t page, uint8_t col, std::shared_ptr<Command>&& cmd);

    uint8_t m_Cols = 2;
    CommandStorage m_Commands;
    CommandPageNames m_CommandPageNames;
    CommandPageIcons m_CommandPageIcons;
    ICmdHelper* m_CmdMediator = nullptr;
};