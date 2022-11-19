#include "pch.hpp"

static constexpr const char* COMMAND_FILE_PATH = "Cmds.xml";

void Command::Execute()
{
    HandleHarcdodedCommand();
#ifdef _WIN32
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESHOWWINDOW;  // Requires STARTF_USESHOWWINDOW in dwFlags.
    si.wShowWindow = SW_SHOW;  // Prevents cmd window from flashing.

    PROCESS_INFORMATION pi = { 0 };
    BOOL fSuccess = CreateProcessA(NULL, (LPSTR)std::format("C:\\windows\\system32\\cmd.exe /c {}", GetCmd()).c_str(), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
    if(fSuccess)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        LOG(LogLevel::Error, "CreateProcess failed with error code: {}", GetLastError());
    }
#else
    utils::exec(c->cmd.c_str());
#endif
}

void Command::HandleHarcdodedCommand()
{
    if(m_name == "Set date")
    {
        const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        const auto now_truncated_to_sec = std::chrono::floor<std::chrono::seconds>(now);
        m_cmd = std::format("adb shell \"date -s {:%Y-%m-%d}\" && date -s \"{:%H:%M:%OS}\"", now_truncated_to_sec, now_truncated_to_sec);
    }
}

CmdExecutor::~CmdExecutor()
{

}

XmlCommandLoader::XmlCommandLoader(ICmdHelper* mediator)
{
    m_Mediator = mediator;
}

XmlCommandLoader::~XmlCommandLoader()
{

}

bool XmlCommandLoader::Load(const std::filesystem::path& path, CommandStorage& storage)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml(path.generic_string(), pt);

        storage.clear();
        m_Cols = pt.get_child("Commands").get_child("Columns").get_value<uint8_t>();
        if(m_Mediator)
            m_Mediator->OnPreReload(m_Cols);
        for(uint8_t i = 1; i <= m_Cols; i++)
        {
            auto col_child = pt.get_child("Commands").get_child_optional(std::format("Col_{}", i));
            if(!col_child.has_value())
            {
                LOG(LogLevel::Normal, "No child found with 'Col_{}', loading has been aborted!", i);
                break;
            }
            
            std::vector<CommandTypes> temp_cmds;
            for(const boost::property_tree::ptree::value_type& v : pt.get_child("Commands").get_child(std::format("Col_{}", i)))
            {
                if(v.first == "Cmd")
                {
                    std::string cmd;
                    boost::optional<std::string> name;
                    boost::optional<std::string> color;
                    boost::optional<std::string> bg_color;
                    boost::optional<std::string> is_bold;
                    boost::optional<std::string> scale;

                    auto is_name_present = v.second.get_child_optional("Name");
                    if(is_name_present.has_value())
                    {
                        cmd = v.second.get_child("Execute").get_value<std::string>();

                        utils::xml::ReadChildIfexists<std::string>(v, "Name", name);
                        utils::xml::ReadChildIfexists<std::string>(v, "Color", color);
                        utils::xml::ReadChildIfexists<std::string>(v, "BackgroundColor", bg_color);
                        utils::xml::ReadChildIfexists<std::string>(v, "Bold", is_bold);
                        utils::xml::ReadChildIfexists<std::string>(v, "Scale", scale);
                    }
                    else
                    {
                        cmd = v.second.get_value<std::string>();

                        name = v.second.get_optional<std::string>("<xmlattr>.name");
                        color = v.second.get_optional<std::string>("<xmlattr>.color");
                        bg_color = v.second.get_optional<std::string>("<xmlattr>.bg_color");
                        is_bold = v.second.get_optional<std::string>("<xmlattr>.bold");
                        scale = v.second.get_optional<std::string>("<xmlattr>.scale");
                    }

                    std::shared_ptr<Command> command = std::make_shared<Command>(
                        name.has_value() ? *name : "",
                        cmd,
                        color.has_value() ? utils::ColorStringToInt(*color) : 0,
                        bg_color.has_value() ? utils::ColorStringToInt(*bg_color) : 0xFFFFFF,
                        is_bold.has_value() ? utils::stob(*is_bold) : false,
                        scale.has_value() ? boost::lexical_cast<float>(*scale) : 1.0);

                    if(command)
                    {
                        temp_cmds.push_back(command);

                        if(m_Mediator)
                            m_Mediator->OnCommandLoaded(i, command);
                    }
                }
                else if(v.first == "Separator")
                {
                    int width = v.second.get_value<int>();
                    temp_cmds.push_back(Separator(width));

                    if(m_Mediator)
                        m_Mediator->OnCommandLoaded(i, Separator(width));
                }
            }
            storage.push_back(std::move(temp_cmds));
        }
        if(m_Mediator)
            m_Mediator->OnPostReload(m_Cols);
    }
    catch(boost::property_tree::xml_parser_error& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}, {}", e.filename(), e.what());
        ret = false;
    }
    catch(std::exception& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}", e.what());
        ret = false;
    }
    return ret;
}

bool XmlCommandLoader::Save(const std::filesystem::path& path, CommandStorage& storage)
{
    bool ret = true;
    std::ofstream out("Cmds.xml", std::ofstream::binary);
    if(out.is_open())
    {
        out << "<Commands>\n";

        uint8_t cnt = 1;
        for(auto& col : storage)
        {
            out << std::format("\t<Col_{}>\n", cnt);
            for(auto& i : col)
            {
                std::visit([this, &out](auto& c)
                    {
                        using T = std::decay_t<decltype(c)>;
                        if constexpr(std::is_same_v<T, std::shared_ptr<Command>>)
                        {
                            out << "\t\t<Cmd>\n";
                            if(c->GetName() != c->GetCmd() && !c->GetName().empty())
                                out << std::format("\t\t\t<Name>{}</Name>\n", c->GetName());
                            out << std::format("\t\t\t<Execute>{}</Execute>\n", c->GetCmd());
                            out << std::format("\t\t\t<Color>0x{:X}</Color>\n", c->GetColor());
                            out << std::format("\t\t\t<BackgroundColor>0x{:X}</BackgroundColor>\n", c->GetBackgroundColor());
                            out << std::format("\t\t\t<Bold>{}</Bold>\n", c->IsBold());
                            out << std::format("\t\t\t<Scale>{}</Scale>\n", c->GetScale());
                            out << "\t\t</Cmd>\n";
                        }
                        else if constexpr(std::is_same_v<T, Separator>)
                        {
                            out << std::format("\t\t<Separator>{}</Separator>\n", c.width);
                        }
                        else
                            static_assert(always_false_v<T>, "XmlCommandLoader::Save Bad visitor!");
                    }, i);
            }
            out << std::format("\t</Col_{}>\n", cnt);
            cnt++;
        }
        out << "</Commands>\n";
    }
    else
    {
        ret = false;
    }
    return ret;
}

void CmdExecutor::Init()
{

}

void CmdExecutor::SetMediator(ICmdHelper* mediator)
{
    m_CmdMediator = mediator;
}

void CmdExecutor::AddCommand(uint8_t col, Command cmd)
{
    std::shared_ptr<Command> cmd_ptr = std::make_shared<Command>(cmd);
    if(AddItem(col, std::move(cmd_ptr)))
    {
        if(m_CmdMediator)
            m_CmdMediator->OnCommandLoaded(col, m_Commands[col - 1].back());
    }
}

bool CmdExecutor::AddItem(uint8_t col, std::shared_ptr<Command>&& cmd)
{
    if(col <= m_Commands.size())
    {
        m_Commands[col - 1].push_back(cmd);
        return true;
    }
    return false;
}

bool CmdExecutor::ReloadCommandsFromFile()
{
    XmlCommandLoader loader(m_CmdMediator);
    return loader.Load(COMMAND_FILE_PATH, m_Commands);
}

bool CmdExecutor::Save()
{
    XmlCommandLoader loader(m_CmdMediator);
    return loader.Save(COMMAND_FILE_PATH, m_Commands);
}

uint8_t CmdExecutor::GetColumns()
{
    return m_Cols;
}