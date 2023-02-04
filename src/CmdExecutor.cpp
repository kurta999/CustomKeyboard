#include "pch.hpp"

static constexpr const char* COMMAND_FILE_PATH = "Cmds.xml";

void Command::Execute()
{
    HandleHarcdodedCommand();
    std::string cmd_to_execute = HandleParameters();

#ifdef _WIN32
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESHOWWINDOW;  // Requires STARTF_USESHOWWINDOW in dwFlags.
    si.wShowWindow = SW_SHOW;  // Prevents cmd window from flashing.

    PROCESS_INFORMATION pi = { 0 };
    BOOL fSuccess = CreateProcessA(NULL, (LPSTR)std::format("C:\\windows\\system32\\cmd.exe /c {}", cmd_to_execute).c_str(), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
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
    utils::exec(cmd_to_execute.c_str());
#endif
}

void Command::HandleHarcdodedCommand()
{
    if(m_name == "Set date")
    {
#ifdef _WIN32
        const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        const auto now_truncated_to_sec = std::chrono::floor<std::chrono::seconds>(now);
        m_cmd = std::format("adb shell \"date -s {:%Y-%m-%d} && date -s {:%H:%M:%OS}\"", now_truncated_to_sec, now_truncated_to_sec);
#endif
    }
}

std::string Command::HandleParameters()
{
    constexpr size_t param_len = std::char_traits<char>::length("[({PARAM:");
    size_t param_count = 0;
    size_t pos = 1;

    size_t first_end = m_cmd.find("})]", pos + 1);
    if(first_end != std::string::npos)
    {
        while(pos < m_cmd.length() - 1)
        {
            size_t first_pos = m_cmd.substr(0, first_end).find("[({PARAM:", pos - 1);

            if(first_pos != std::string::npos && first_end != std::string::npos)
            {
                if(param_count > 0)
                {
                    LOG(LogLevel::Warning, "Only 1 parameter is supported!");
                    break;
                }

                if(param_count == 0)
                {
                    std::string ret = utils::extract_string(m_cmd, first_pos, first_end, param_len);
                    DBG("ret: %s", ret.c_str());

                    std::string new_cmd{ m_cmd };
                    new_cmd.erase(first_pos, (first_end + 3) - first_pos);
                    new_cmd.insert(first_pos, m_param.empty() ? ret : m_param);
                    return new_cmd;
                }

                pos = first_end;
                ++param_count;
            }
        }
    }
    return m_cmd;
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

bool XmlCommandLoader::Load(const std::filesystem::path& path, CommandStorage& storage, CommandPageNames& names)
{
    if(!std::filesystem::exists(COMMAND_FILE_PATH))
    {
        CmdExecutor::WriteDefaultCommandsFile();
        LOG(LogLevel::Normal, "Default {} is missing, creating one", COMMAND_FILE_PATH);
    }

    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml(path.generic_string(), pt);

        storage.clear();
        m_Pages = pt.get_child("Commands").get_child("Pages").get_value<uint8_t>();
        if(m_Mediator)
            m_Mediator->OnPreReload(m_Pages);
        for(uint8_t p = 1; p <= m_Pages; p++)
        {
            auto pages_child = pt.get_child("Commands").get_child_optional(std::format("Page_{}", p));
            if(!pages_child.has_value())
            {
                LOG(LogLevel::Warning, "No child found with 'Page_{}', loading has been aborted!", p);
                break;
            }

            std::string page_name = pages_child->get<std::string>("<xmlattr>.name");
            names.push_back(std::move(page_name));

            m_Cols = pages_child->get_child("Columns").get_value<uint8_t>();
            if(m_Mediator)
                m_Mediator->OnPreReloadColumns(p, m_Cols);

            std::vector<std::vector<CommandTypes>> temp_cmds_per_page;
            for(uint8_t i = 1; i <= m_Cols; i++)
            {
                auto col_child = pages_child->get_child_optional(std::format("Col_{}", i));
                if(!col_child.has_value())
                {
                    LOG(LogLevel::Normal, "No child found with 'Col_{}' within 'Page_{}', loading of this page has been aborted!", i, p);
                    break;
                }

                std::vector<CommandTypes> temp_cmds;
                for(const boost::property_tree::ptree::value_type& v : pages_child->get_child(std::format("Col_{}", i)))
                {
                    if(v.first == "Cmd")
                    {
                        std::string cmd;
                        boost::optional<std::string> name;
                        boost::optional<std::string> param;
                        boost::optional<std::string> color;
                        boost::optional<std::string> bg_color;
                        boost::optional<std::string> is_bold;
                        boost::optional<std::string> scale;

                        auto is_name_present = v.second.get_child_optional("Execute");
                        if(is_name_present.has_value())
                        {
                            cmd = v.second.get_child("Execute").get_value<std::string>();

                            utils::xml::ReadChildIfexists<std::string>(v, "Name", name);
                            utils::xml::ReadChildIfexists<std::string>(v, "Param", param);
                            utils::xml::ReadChildIfexists<std::string>(v, "Color", color);
                            utils::xml::ReadChildIfexists<std::string>(v, "BackgroundColor", bg_color);
                            utils::xml::ReadChildIfexists<std::string>(v, "Bold", is_bold);
                            utils::xml::ReadChildIfexists<std::string>(v, "Scale", scale);
                        }
                        else
                        {
                            cmd = v.second.get_value<std::string>();

                            name = v.second.get_optional<std::string>("<xmlattr>.name");
                            param = v.second.get_optional<std::string>("<xmlattr>.param");
                            color = v.second.get_optional<std::string>("<xmlattr>.color");
                            bg_color = v.second.get_optional<std::string>("<xmlattr>.bg_color");
                            is_bold = v.second.get_optional<std::string>("<xmlattr>.bold");
                            scale = v.second.get_optional<std::string>("<xmlattr>.scale");
                        }

                        if(cmd.empty())
                        {
                            if(!name)
                                name = "Unknown";
                            LOG(LogLevel::Warning, "Empty cmd for command: {}", *name);
                        }

                        std::shared_ptr<Command> command = std::make_shared<Command>(
                            name.has_value() ? *name : "",
                            cmd,
                            param.has_value() ? *param : "",
                            color.has_value() ? utils::ColorStringToInt(*color) : 0,
                            bg_color.has_value() ? utils::ColorStringToInt(*bg_color) : 0xFFFFFF,
                            is_bold.has_value() ? utils::stob(*is_bold) : false,
                            scale.has_value() ? boost::lexical_cast<float>(*scale) : 1.0);

                        if(command)
                        {
                            temp_cmds.push_back(command);

                            DBG("loading command page: %d, col: %d\n", p, i);
                            if(m_Mediator)
                                m_Mediator->OnCommandLoaded(p, i, command);
                        }
                    }
                    else if(v.first == "Separator")
                    {
                        int width = v.second.get_value<int>();
                        temp_cmds.push_back(Separator(width));

                        if(m_Mediator)
                            m_Mediator->OnCommandLoaded(p, i, Separator(width));
                    }
                }
                temp_cmds_per_page.push_back(std::move(temp_cmds));
            }
            storage.push_back(std::move(temp_cmds_per_page));

            if(m_Mediator)
                m_Mediator->OnPostReload(p, m_Cols, names);
        }
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

bool XmlCommandLoader::Save(const std::filesystem::path& path, CommandStorage& storage, CommandPageNames& names)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    auto& root_node = pt.add_child("Commands", boost::property_tree::ptree{});
    root_node.put("Pages", std::to_string(storage.size()));

    uint8_t page_cnt = 1;
    for(auto& page : storage)
    {
        uint8_t cnt = 1;

        auto& page_node = root_node.add_child(std::format("Page_{}", page_cnt), boost::property_tree::ptree{});
        page_node.put("<xmlattr>.name", names[page_cnt - 1]);
        page_node.put("Columns", std::to_string(page.size()));
        for(auto& col : page)
        {
            auto& col_node = page_node.add_child(std::format("Col_{}", cnt), boost::property_tree::ptree{});
            for(auto& i : col)
            {
                std::visit([this, &col_node](auto& c)
                    {
                        using T = std::decay_t<decltype(c)>;
                        if constexpr(std::is_same_v<T, std::shared_ptr<Command>>)
                        {
                            auto& cmd_node = col_node.add_child("Cmd", boost::property_tree::ptree{});
                            if(c->GetName() != c->GetCmd() && !c->GetName().empty())
                                cmd_node.add("Name", c->GetName());

                            cmd_node.add("Execute", c->GetCmd());
                            if(!c->GetParam().empty())
                                cmd_node.add("Param", c->GetParam());
                            cmd_node.add("Color", utils::ColorIntToString(c->GetColor()));
                            cmd_node.add("BackgroundColor", utils::ColorIntToString(c->GetBackgroundColor()));
                            cmd_node.add("Bold", c->IsBold());
                            cmd_node.add("Scale", c->GetScale());
                        }
                        else if constexpr(std::is_same_v<T, Separator>)
                        {
                            auto& separator_node = col_node.add("Separator", c.width);
                        }
                        else
                            static_assert(always_false_v<T>, "XmlCommandLoader::Save Bad visitor!");
                    }, i);
            }
            cnt++;
        }
        page_cnt++;
    }

    try
    {
#ifdef DEBUG
        boost::property_tree::write_xml("Cmds2.xml", pt, std::locale(),
            boost::property_tree::xml_writer_make_settings<boost::property_tree::ptree::key_type>('\t', 1));
#else
        boost::property_tree::write_xml(path.generic_string(), pt, std::locale(),
            boost::property_tree::xml_writer_make_settings<boost::property_tree::ptree::key_type>('\t', 1));
#endif
    }
    catch(...)
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

void CmdExecutor::AddCommand(uint8_t page, uint8_t col, Command cmd)
{
    std::shared_ptr<Command> cmd_ptr = std::make_shared<Command>(cmd);
    if(AddItem(page, col, std::move(cmd_ptr)))
    {
        if(m_CmdMediator)
            m_CmdMediator->OnCommandLoaded(page, col, m_Commands[page - 1][col - 1].back());
    }
}

bool CmdExecutor::AddItem(uint8_t page, uint8_t col, std::shared_ptr<Command>&& cmd)
{
    if(col <= m_Commands.size())
    {
        m_Commands[page - 1][col - 1].push_back(cmd);
        return true;
    }
    return false;
}

bool CmdExecutor::ReloadCommandsFromFile()
{
    XmlCommandLoader loader(m_CmdMediator);
    return loader.Load(COMMAND_FILE_PATH, m_Commands, m_CommandPageNames);
}

bool CmdExecutor::Save()
{
    XmlCommandLoader loader(m_CmdMediator);
    return loader.Save(COMMAND_FILE_PATH, m_Commands, m_CommandPageNames);
}

uint8_t CmdExecutor::GetColumns()
{
    return m_Cols;
}

CommandStorage& CmdExecutor::GetCommands()
{
    return m_Commands;
}

CommandPageNames& CmdExecutor::GetPageNames()
{
    return m_CommandPageNames;
}

void CmdExecutor::WriteDefaultCommandsFile()
{
    std::string file_content = "<Commands>\
  <Pages>2</Pages>\
  <Page_1 name = \"Board\">\
    <Columns>4</Columns>\
    <Col_1>\
	    <Cmd>\
        <Name>Directory</Name>\
        <Execute>cd C:\\ & dir & ping 127.0.0.1 -n [({PARAM:3})] > nul</Execute>\
        <Color>0xFF0000</Color>\
        <BackgroundColor>green</BackgroundColor>\
        <Bold>true</Bold>\
        <Scale>2.0</Scale>\
      </Cmd>\
	  <Cmd>\
        <Name>Set date</Name>\
        <Execute>cd C:\\ & dir & ping 127.0.0.1 -n 3 > nul</Execute>\
        <Color>0xFF0000</Color>\
        <BackgroundColor>green</BackgroundColor>\
        <Bold>true</Bold>\
        <Scale>2.0</Scale>\
      </Cmd>\
	    <Cmd>cd ..</Cmd>\
      <Separator>4</Separator>\
      <Cmd>cd2 ..</Cmd>\
    </Col_1>  \
    <Col_2>\
	    <Cmd>dir C:</Cmd>\
	    <Cmd>cd ../..</Cmd>\
    </Col_2>\
  </Page_1>\
  <Page_2 name = \"Liunx VM\">\
    <Columns>2</Columns>\
    <Col_1>\
      <Cmd>\
        <Name>Print directory</Name>\
        <Execute>cd C:\\ & dir & ping 127.0.0.1 -n 3 > nul</Execute>\
        <Color>0xFF0000</Color>\
        <BackgroundColor>green</BackgroundColor>\
        <Bold>true</Bold>\
        <Scale>2.0</Scale>\
      </Cmd>\
    </Col_1>>\
    </Page_2>\
</Commands>";
    std::ofstream out(COMMAND_FILE_PATH, std::ofstream::binary);
    out << file_content;
}