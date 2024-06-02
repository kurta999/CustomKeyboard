#include "pch.hpp"

AlarmTrigger AlarmStringToTrigger(const std::string& in)
{
    AlarmTrigger ret = AlarmTrigger::Invalid;
    if (in == "macro")
        ret = AlarmTrigger::Macro;
    else if (in == "gui")
        ret = AlarmTrigger::Gui;
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
        case AlarmTrigger::Gui:
        {
            ret = "gui";
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
    m_worker = std::make_unique<std::jthread>(std::bind_front(&AlarmEntryHandler::WorkerThread, this));
    if(m_worker)
        utils::SetThreadName(*m_worker, "AlarmEntryHandler");
}

AlarmEntryHandler::~AlarmEntryHandler()
{
    {
        std::unique_lock lock{ m };
        m_cv.notify_all();
    }

    m_worker.reset(nullptr);
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

void AlarmEntryHandler::SetupAlarm(uint8_t id)
{
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if (frame)
    {
        frame->alarm_panel->ShowAlarmDialog();

        frame->alarm_panel->On10MsTimer();
        frame->alarm_panel->WaitForAlarmSemaphore();

        std::string duration_str = frame->alarm_panel->GetAlarmTime();

        SetupAlarm(entries[id].get());
    }
}

void AlarmEntryHandler::SetupAlarm(AlarmEntry* entry)
{
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if (frame)
    {
        frame->alarm_panel->ShowAlarmDialog();

        frame->alarm_panel->On10MsTimer();
        frame->alarm_panel->WaitForAlarmSemaphore();

        std::string duration_str = frame->alarm_panel->GetAlarmTime();
 
        entry->is_armed = true;
        entry->duration = ParseDurationStringToSeconds(duration_str);

        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        std::unique_lock lock(frame->mtx);
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::AlarmSetup), entry->name, entry->duration });
    }
}

void AlarmEntryHandler::CancelAlarm(AlarmEntry* entry)
{
    entry->duration = 0s;
    entry->is_armed = false;
}

void AlarmEntryHandler::HandleKeypress(const std::string& key, bool force_timer_call)
{
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if(frame)
    {
        frame->alarm_panel->ShowAlarmDialog();

        if (force_timer_call)
            frame->alarm_panel->On10MsTimer();
        frame->alarm_panel->WaitForAlarmSemaphore();

        std::string duration_str = frame->alarm_panel->GetAlarmTime();

        auto it = std::find_if(entries.begin(), entries.end(), [&key](std::unique_ptr<AlarmEntry>& e) { return e->trigger_key == key; });
        if(it != entries.end())
        {
            (*it)->is_armed = true;
            (*it)->duration = ParseDurationStringToSeconds(duration_str);

            MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
            std::unique_lock lock(frame->mtx);
            frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::AlarmSetup), (*it)->name, (*it)->duration });
        }
    }
}

void AlarmEntryHandler::WorkerThread(std::stop_token token)
{
    while(!token.stop_requested())
    {
        for(auto& a : entries)
        {
            if(a->is_armed)
            {
                a->duration--;
                if(a->duration.count() <= 0)
                {
                    if(a->trigger == AlarmTrigger::Macro)
                    {
                        CustomMacro::Get()->SimulateKeypress(a->trigger_key, true);

                        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
                        std::unique_lock lock(frame->mtx);
                        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::AlarmTriggered), a->name });
                    }
                    a->is_armed = false;
                }
            }
        }
        std::unique_lock lock{ m };
        m_cv.wait_for(lock, token, 1000ms, []() {return 0 == 1;});
    }
}

std::chrono::seconds AlarmEntryHandler::ParseDurationStringToSeconds(const std::string& input)
{
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    std::chrono::duration<int> ret = std::chrono::seconds(0);
    if(sscanf(input.c_str(), "%dh%dm%ds", &hours, &minutes, &seconds) == 3)
    {
        ret = std::chrono::hours(hours) + std::chrono::minutes(minutes) + std::chrono::seconds(seconds);
	}
    else if(sscanf(input.c_str(), "%dh%dm", &hours, &minutes) == 2)
    {
        ret = std::chrono::hours(hours) + std::chrono::minutes(minutes);
    }    
    else if(sscanf(input.c_str(), "%dm%ds", &minutes, &seconds) == 2)
    {
        ret = std::chrono::minutes(minutes) + std::chrono::seconds(seconds);
    }
    else if(sscanf(input.c_str(), "%d[^hour]", &hours) == 1 && input.find("hour") != std::string::npos)
    {
        ret = std::chrono::hours(hours);
    }
    else if(sscanf(input.c_str(), "%d[^min]", &minutes) == 1 && input.find("min") != std::string::npos)
    {
        ret = std::chrono::minutes(minutes);
    }    
    else if(sscanf(input.c_str(), "%d[^sec]", &seconds) == 1 && input.find("sec") != std::string::npos)
    {
        ret = std::chrono::seconds(seconds);
    }    
    return ret;
}