#include "pch.h"

void DirectoryBackup::DoBackup(BackupEntry* backup)
{
	size_t file_count = 0;
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	auto folder_name = backup->from.filename();
	auto folder_name_with_date = folder_name.generic_string();

	for(auto& t : backup->to)
	{
		if(!std::filesystem::exists(t))  /* not needed to go file checking when even the directory doesns't exists */
		{
			std::filesystem::create_directory(t);
			continue;
		}

		std::set<std::string> files;
		for(auto& f : std::filesystem::directory_iterator(t))
		{
			files.emplace(f.path().generic_string());
		}
		if(files.size() >= (size_t)backup->max_backups)
		{
			auto to_remove = *files.begin();
			std::filesystem::remove_all(to_remove);
		}
	}

	time_t current_time;
	time(&current_time);
	std::tm* now = std::localtime(&current_time);
	char datetime[64];
	strftime(datetime, sizeof(datetime), backup_time_format.c_str(), now);
	folder_name_with_date += std::string(datetime);

	for(auto& t : backup->to)
	{
		std::filesystem::path destination_dir = t / folder_name_with_date;
		std::filesystem::create_directory(destination_dir);
		for(auto& p : std::filesystem::recursive_directory_iterator(backup->from))
		{
			auto rel_path = p.path().lexically_proximate(backup->from);

			bool is_file = std::filesystem::is_regular_file(p.path());

			if(backup->IsInIgnoreList(rel_path.generic_u8string())) continue;
			DBGW(L"f: %d, %s\n", is_file, p.path().c_str());

			if(!is_file)
			{
				std::filesystem::path destination_path = destination_dir / rel_path;
				std::filesystem::create_directory(destination_path);
			}
			else
			{
				std::filesystem::path destination_path = destination_dir / rel_path;
				std::filesystem::copy_file(p.path(), destination_path);
			}
			file_count++;
		}
	}
	std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
	int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
	
	MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
	{
		std::lock_guard<std::mutex> lock(frame->mtx);
		frame->pending_msgs.push_back({ (uint8_t)BackupCompleted, dif, file_count, &backup->to[0] });
	}
}

void DirectoryBackup::DoBackups()
{
	for(auto& b : backups)
	{
		DoBackup(b);
	}
}

void DirectoryBackup::Init()
{

}

void DirectoryBackup::BackupFiles()
{
	if(backup_future.valid())
		backup_future.get();
	backup_future = std::async(&DirectoryBackup::DoBackups, this);
}

void DirectoryBackup::BackupFile(int id)
{
	if(backup_future.valid())
		backup_future.get();
	backup_future = std::async(&DirectoryBackup::DoBackup, this, backups[id]);
}