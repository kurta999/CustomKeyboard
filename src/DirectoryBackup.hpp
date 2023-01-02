#pragma once

#include "utils/CSingleton.hpp"
#include <future> 
#include <tuple>
#include <filesystem>
#include <vector>

class BackupEntry
{
public:
    BackupEntry(std::filesystem::path&& from_, std::vector<std::filesystem::path>&& to_, std::vector<std::wstring>&& ignore_list_, int max_backups_,
        bool calculate_hash_, size_t hash_buf_size_);

    // !\brief Is constructed backup entry valid?
    bool IsValid();

    // !\brief Return true if the given file is in ignore list
    // !\param p [in] File to check
    bool IsInIgnoreList(std::wstring&& p);

    // !\brief Backup source path
    std::filesystem::path from;

    // !\brief Backup destination path vector (for multiple destinations)
    std::vector<std::filesystem::path> to;

    // !\brief Ignored file list
    std::vector<std::wstring> ignore_list;

    // !\brief max backups for backup rotation in destination folder
    int max_backups;

    // !\brief Calculate hash for backups (hash of destination folder)
    bool calculate_hash;

    // !\brief Hash buffer size [MB]
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

    // !\brief Construct backup entry from string
    void LoadEntry(const std::string& from, const std::string& to, const std::string& ignore, int max_backups, bool calculate_hash, size_t buffer_size);

    // !\brief Starts backup with given id
    // \param id [in] ID of backup entry to execute
    void BackupFile(int id);

    // !\brief Is backup in progess?
    bool IsInProgress();

    // !\brief Delete all backups from backup list
    void Clear();

    // !\brief Backup time format
    std::string backup_time_format = "_%Y_%m_%d %H_%M_%S";

    // !\brief Vector of backups
    std::vector<std::unique_ptr<BackupEntry>> backups;
    
    // !\brief Is backup cancelled?
    bool is_cancelled = false;

protected:
    // !\brief Backups given backup entry
    // !\param backup [in] Backup entry to execute
    void DoBackup(BackupEntry* backup);

    // !\brief Execute backup rotation (removing older backups)
    // !\param backup [in] Backup entry to execute
    void BackupRotation(BackupEntry* backup);

    // !\brief Future for backup async operations
    std::future<void> backup_future;

#ifdef UNIT_TESTS
    class DirectoryBackupTest;
    friend class DirectoryBackupTest;
#endif
};