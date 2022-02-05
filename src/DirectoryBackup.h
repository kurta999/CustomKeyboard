#pragma once

#include "utils/CSingleton.h"
#include <future> 
#include <tuple>
#include <filesystem>
#include <vector>

class BackupEntry
{
public:
    BackupEntry(std::filesystem::path&& from_, std::vector<std::filesystem::path>&& to_, std::vector<std::wstring>&& ignore_list_, int max_backups_,
        bool calculate_hash_, size_t hash_buf_size_) :
        from(std::move(from_)), to(std::move(to_)), ignore_list(std::move(ignore_list_)), max_backups(max_backups_), 
        calculate_hash(calculate_hash_), hash_buf_size(hash_buf_size_)
    {

    }
    ~BackupEntry() = default;

    // \brief Return true if the given file is in ignore list
    // \param p [in] File to check
    bool IsInIgnoreList(std::wstring&& p)
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

    // \brief Backup source path
    std::filesystem::path from;

    // \brief Backup destination path vector (for multiple destinations)
    std::vector<std::filesystem::path> to;

    // \brief Ignored file list
    std::vector<std::wstring> ignore_list;

    // \brief max backups for backup rotation in destination folder
    int max_backups;

    // \brief Calculate hash for backups (hash of destination folder)
    bool calculate_hash;

    // \brief Hash buffer size [MB]
    size_t hash_buf_size;
};

class DirectoryBackup : public CSingleton < DirectoryBackup >
{
    friend class CSingleton < DirectoryBackup >;

public:
    friend class Settings;
    DirectoryBackup() = default;
    ~DirectoryBackup() = default;

    // !\brief Initialize DirectoryBackup
    void Init(void);

    // !\brief Starts backup with given id
    // \param id [in] ID of backup entry to execute
    void BackupFile(int id);

    // !\brief Is backup in progess?
    bool IsInProgress();

    // !\brief Delete all backups from backup list
    void Clear();

    // \brief Backup time format
    std::string backup_time_format = "_%Y_%m_%d %H_%M_%S";

    // \brief Vector of backups
    std::vector<BackupEntry*> backups;
private:
    // \brief Backups given backup entry
    // \param backup [in] Backup entry to execute
    void DoBackup(BackupEntry* backup);

    void BackupRotation(BackupEntry* backup);

    // \brief Future for backup async operations
    std::future<void> backup_future;
};