#pragma once

#include <filesystem>

class AlarmEntry;
class IAlarmEntryLoader
{
public:
    virtual ~IAlarmEntryLoader() = default;

    virtual bool Load(const std::filesystem::path& path, std::vector<std::unique_ptr<AlarmEntry>>& e) = 0;
    virtual bool Save(const std::filesystem::path& path, std::vector<std::unique_ptr<AlarmEntry>>& e) const = 0;
};