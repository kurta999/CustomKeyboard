#include "pch.hpp"

static constexpr const char* COMMAND_FILE_PATH = "Cmds.xml";

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
    LOG(LogLevel::Normal, "Save function isn't implemented yet.");
    return false;
}

void CmdExecutor::Init()
{
    ReloadCommandsFromFile();
}

void CmdExecutor::SetMediator(ICmdHelper* mediator)
{
    m_CmdMediator = mediator;
}

bool CmdExecutor::ReloadCommandsFromFile()
{
    XmlCommandLoader loader(m_CmdMediator);
    return loader.Load(COMMAND_FILE_PATH, m_Commands);
}

uint8_t CmdExecutor::GetColumns()
{
    return m_Cols;
}