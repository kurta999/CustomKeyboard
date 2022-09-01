#include "pch.hpp"

#define CLEANUP() /* TODO: this is a fucking big anti pattern, refactor it - remove singletons!!! */\
    DirectoryBackup::Get()->backups.clear();\
    std::filesystem::remove_all(L"backup_source");\
    std::filesystem::remove_all(L"backup_dest");\
    std::filesystem::remove_all(L"backup_dest_2")\

class DirectoryBackupTest : public ::testing::Test {
protected:

    DirectoryBackupTest() {
    }

    virtual ~DirectoryBackupTest() {
    }

    virtual void SetUp() {
        
    }

    virtual void TearDown() 
    {
        DirectoryBackup::Get()->backups.clear();
        std::filesystem::remove_all(L"backup_source");
        std::filesystem::remove_all(L"backup_dest");
        std::filesystem::remove_all(L"backup_dest_2");
    }

};

void WriteExampleFile(std::filesystem::path p, const char* text)
{
    std::ofstream f(p, std::ofstream::binary);
    if(f)
    {
        f << text;
    }
}

TEST(DirectoryBackupTest, Test1_Basic)
{
    CLEANUP();
    std::unique_ptr<BackupEntry> backup = std::make_unique<BackupEntry>(BackupEntry{ L"backup_source", {L"backup_dest", L"backup_dest_2"}, {L"Debug", L"Release"}, 2, 1, 1 });
    std::filesystem::create_directory(backup->from);
    WriteExampleFile(backup->from / "first_txt.txt", "test string in first file");
    WriteExampleFile(backup->from / "second_txt.txt", "second txt string");

    DirectoryBackup::Get()->backup_time_format.clear();
    DirectoryBackup::Get()->backups.push_back(std::move(backup));
    DirectoryBackup::Get()->BackupFile(0);
    while(DirectoryBackup::Get()->IsInProgress())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    BackupEntry* b = DirectoryBackup::Get()->backups[0].get();
    EXPECT_TRUE(std::filesystem::exists(b->to[0] / b->from / "first_txt.txt"));
    EXPECT_TRUE(std::filesystem::exists(b->to[0] / b->from / "second_txt.txt"));
}

TEST(DirectoryBackupTest, Test_Symlink)
{
    CLEANUP();
    std::unique_ptr<BackupEntry> backup = std::make_unique<BackupEntry>(BackupEntry{ L"backup_source", {L"backup_dest", L"backup_dest_2"}, {L"Debug", L"Release"}, 2, 1, 1 });
    std::filesystem::create_directory(backup->from);
    WriteExampleFile(backup->from / "first_txt.txt", "test string in first file");
    WriteExampleFile(backup->from / "second_txt.txt", "second txt string");
    
    auto restore_path = std::filesystem::current_path();
    std::filesystem::current_path(backup->from);
    std::filesystem::create_symlink("first_txt.txt", "first_symlink.txt");
    std::filesystem::current_path(restore_path);

    DirectoryBackup::Get()->backup_time_format.clear();
    DirectoryBackup::Get()->backups.push_back(std::move(backup));
    DirectoryBackup::Get()->BackupFile(0);
    while(DirectoryBackup::Get()->IsInProgress())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    BackupEntry* b = DirectoryBackup::Get()->backups[0].get();
    EXPECT_TRUE(std::filesystem::exists(b->to[0] / b->from / "first_txt.txt"));
    EXPECT_TRUE(std::filesystem::exists(b->to[0] / b->from / "second_txt.txt"));
}