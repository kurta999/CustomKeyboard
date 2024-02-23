#pragma once

#include "IAlarmEntryLoader.hpp"
#include <filesystem>
#include <string>

enum class AlarmTrigger
{
    Macro,
    Invalid,
};

AlarmTrigger AlarmStringToTrigger(const std::string& in);
const std::string AlarmTriggerToString(AlarmTrigger trigger);

class AlarmEntry
{
public:
    AlarmEntry(const std::string& name_, AlarmTrigger trigger_, const std::string& trigger_key_, const std::string execute_, bool show_dialog_) :
        name(name_), trigger(trigger_), trigger_key(trigger_key_), execute(execute_), show_dialog(show_dialog_)
    {

    }

    std::string name;
    AlarmTrigger trigger;
    std::string trigger_key;
    std::string execute;
    bool show_dialog;
    bool is_armed{ false };
    std::chrono::seconds duration{ 0 };
};

class XmlAlarmEntryLoader : public IAlarmEntryLoader
{
public:
    virtual ~XmlAlarmEntryLoader() = default;

    bool Load(const std::filesystem::path& path, std::vector<std::unique_ptr<AlarmEntry>>& e) override;
    bool Save(const std::filesystem::path& path, std::vector<std::unique_ptr<AlarmEntry>>& e) const override;
};

class AlarmEntryHandler
{
public:
    AlarmEntryHandler(IAlarmEntryLoader& loader);
    ~AlarmEntryHandler();

    // !\brief Initialize entry handler
    void Init();

    void WorkerThread(std::stop_token token);

    bool Load();

    bool LoadAlarms(std::filesystem::path& path);

    bool SaveAlarms(std::filesystem::path& path);

    void HandleKeypress(const std::string& key);

    // !\brief Default alarms XML name
    std::filesystem::path default_alarms = "Alarms.xml";

    // !\brief Vector of Alarm entries
    std::vector<std::unique_ptr<AlarmEntry>> entries;

private:
    std::chrono::seconds ParseDurationStringToSeconds(const std::string& input);

    // !\brief Reference to Alarm entry loader
    IAlarmEntryLoader& m_AlarmEntryLoader;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_worker;

    // !\brief Conditional variable for main thread exiting
    std::condition_variable_any m_cv;

    // !\brief Mutex for entry handler
    std::mutex m;
};