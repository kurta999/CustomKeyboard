#include "pch.hpp"

AlarmTrigger AlarmStringToTrigger(const std::string& in)
{
    AlarmTrigger ret = AlarmTrigger::Invalid;
    if (in == "macro")
        ret = AlarmTrigger::Macro;
    return ret;
}

const std::string AlarmTriggerToString(AlarmTrigger trigger)
{
    std::string ret = "Invalid";
    switch (trigger)
    {
        case AlarmTrigger::Macro:
        {
            ret = "macro";
            break;
        }
    }
    return ret;
}

bool XmlAlarmEntryLoader::Load(const std::filesystem::path& path, std::vector<std::unique_ptr<AlarmEntry>>& e)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml(path.generic_string(), pt);
        for (const boost::property_tree::ptree::value_type& v : pt.get_child("AlarmsXml")) /* loop over each Alarm */
        {
            std::string name = v.second.get_child("Name").get_value<std::string>();

            auto name_cnt = std::ranges::count(e, name, &AlarmEntry::name);
            if (name_cnt != 0)
            {
                LOG(LogLevel::Warning, "Alarm with name {} has been already added to the list, skipping this one", name_cnt);
                continue;
            }

            std::string trigger_str = v.second.get_child("Trigger").get_value<std::string>();
            std::string key = v.second.get_child("Key").get_value<std::string>();
            std::string execute = v.second.get_child("Execute").get_value<std::string>();
            bool show_dialog = v.second.get_child("ShowDialog").get_value<bool>();

            AlarmTrigger trigger = AlarmStringToTrigger(trigger_str);
            std::unique_ptr<AlarmEntry> local_entry = std::make_unique<AlarmEntry>(name, trigger, key, execute, show_dialog);


            auto& macros = CustomMacro::Get()->GetMacros();

            CustomMacro::Get()->ParseMacroKeys(0, key, execute, macros[0], MacroFlags::Alarm);
            e.push_back(std::move(local_entry));
        }
    }
    catch (const boost::property_tree::xml_parser_error& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}, {}", e.filename(), e.what());
        ret = false;
    }
    catch (const std::exception& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}", e.what());
        ret = false;
    }
    return ret;
}

bool XmlAlarmEntryLoader::Save(const std::filesystem::path& path, std::vector<std::unique_ptr<AlarmEntry>>& e) const
{
    bool ret = true;
    boost::property_tree::ptree pt;
    auto& root_node = pt.add_child("AlarmsXml", boost::property_tree::ptree{});
    for (auto& i : e)
    {
        /* TODO */
    }

    try
    {
        boost::property_tree::write_xml(path.generic_string(), pt, std::locale(),
            boost::property_tree::xml_writer_make_settings<boost::property_tree::ptree::key_type>('\t', 1));
    }
    catch (const std::exception& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}", e.what());
        ret = false;
    }
    return ret;
}

AlarmEntryHandler::AlarmEntryHandler(IAlarmEntryLoader& loader) :
    m_AlarmEntryLoader(loader)
{

}

AlarmEntryHandler::~AlarmEntryHandler()
{

}

void AlarmEntryHandler::Init()
{
    Load();
}

bool AlarmEntryHandler::Load()
{
    bool ret = LoadAlarms(default_alarms);
    return ret;
}

bool AlarmEntryHandler::LoadAlarms(std::filesystem::path& path)
{
    std::scoped_lock lock{ m };
    if (path.empty())
        path = default_alarms;

    entries.clear();
    bool ret = m_AlarmEntryLoader.Load(path, entries);
    if (ret)
    {
        LOG(LogLevel::Debug, "Alarms loaded, total: {}", entries.size());
    }
    return ret;
}

bool AlarmEntryHandler::SaveAlarms(std::filesystem::path& path)
{
    if (path.empty())
        path = default_alarms;
    bool ret = m_AlarmEntryLoader.Save(path, entries);
    return ret;
}

void AlarmEntryHandler::HandleKeypress(const std::string& key)
{
    auto it = std::find_if(entries.begin(), entries.end(), [&key](std::unique_ptr<AlarmEntry>& e) { return e->trigger_key == key; });
    if (it != entries.end())
    {

    }
}