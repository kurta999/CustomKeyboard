#pragma once

#include "utils/CSingleton.h"
#include <future> 
#include <tuple>
#include <filesystem>
#include <vector>

class BackupEntry
{
public:
    BackupEntry(std::filesystem::path&& from_, std::vector<std::filesystem::path>&& to_, std::vector<std::string>&& ignore_list_, int max_backups_) :
        from(std::move(from_)), to(std::move(to_)), ignore_list(std::move(ignore_list_)), max_backups(max_backups_)
    {

    }
    ~BackupEntry() = default;

    bool IsInIgnoreList(std::string&& p)
    {
        for(auto& i : ignore_list)
        {
            if(std::search(p.begin(), p.end(), i.begin(), i.end()) != p.end())
            {
                return true;
            }
        }
        return false;
    }

    std::filesystem::path from;
    std::vector<std::filesystem::path> to;
    std::vector<std::string> ignore_list;
    int max_backups;
};

class DirectoryBackup : public CSingleton < DirectoryBackup >
{
    friend class CSingleton < DirectoryBackup >;

public:
    friend class Settings;
    DirectoryBackup() = default;
    ~DirectoryBackup() = default;
    void Init(void);
    void BackupFiles();

    std::string backup_key;
private:
    std::vector< BackupEntry*> backups;
    void DoBackup();
    std::future<void> backup_future;
    std::string backup_time_format;
};