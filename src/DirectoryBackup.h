#pragma once

#include "utils/CSingleton.h"
#include <future> 
#include <tuple>
#include <filesystem>
#include <vector>

class BackupEntry
{
public:
    BackupEntry(std::filesystem::path&& from_, std::vector<std::filesystem::path>&& to_, std::vector<std::string>&& ignore_list_, int max_backups_,
        bool calculate_hash_, size_t hash_buf_size_) :
        from(std::move(from_)), to(std::move(to_)), ignore_list(std::move(ignore_list_)), max_backups(max_backups_), 
        calculate_hash(calculate_hash_), hash_buf_size(hash_buf_size_)
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
    int max_backups;  /* max backups for backup rotation in destination folder */
    bool calculate_hash;
    size_t hash_buf_size;  /* In megabytes */
};

class DirectoryBackup : public CSingleton < DirectoryBackup >
{
    friend class CSingleton < DirectoryBackup >;

public:
    friend class Settings;
    DirectoryBackup() = default;
    ~DirectoryBackup() = default;
    void Init(void);
    void BackupFile(int id);
    bool IsInProgress();

    std::string backup_time_format = "_%Y_%m_%d %H_%M_%S";
    std::vector< BackupEntry*> backups;
private:
    void DoBackups();
    void DoBackup(BackupEntry* backup);
    std::future<void> backup_future;
};