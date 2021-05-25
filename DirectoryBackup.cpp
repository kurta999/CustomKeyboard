#include "DirectoryBackup.h"
#include "Logger.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <algorithm>
#include <future> 
#include <tuple>
#include <set>

#if defined ( _WIN32 )
#include <sys/stat.h>
#endif

void DirectoryBackup::DoBackup()
{
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	size_t file_count = 0;

	for(auto& b : backups)
	{
		auto folder_name = b->from.filename();
		auto folder_name_with_date = folder_name.generic_string();

		for(auto& t : b->to)
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
			if(files.size() >= (size_t)b->max_backups)
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

		for(auto& t : b->to)
		{
			std::filesystem::path destination_dir = t / folder_name_with_date;
			std::filesystem::create_directory(destination_dir);
			for(auto& p : std::filesystem::recursive_directory_iterator(b->from))
			{
				auto rel_path = p.path().lexically_proximate(b->from);

				bool is_file = std::filesystem::is_regular_file(p.path());

				if(b->IsInIgnoreList(rel_path.generic_u8string())) continue;
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
	}
	DBG("completed\n");
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

	MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
	frame->mtx.lock();
	frame->backup_result = std::make_tuple(1, dif, file_count);
	frame->mtx.unlock();
}

void DirectoryBackup::Init()
{

}

void DirectoryBackup::BackupFiles()
{
	backup_future = std::async(&DirectoryBackup::DoBackup, this);
}


/*
* 	size_t cnt = 0;
	constexpr time_t min_date = std::numeric_limits<time_t>::min();
	for(auto& f : std::filesystem::directory_iterator(to_backup))
	{
		time_t last_write = GetFileWriteTime(f.path());
		cnt++;
	}*/



#if 0
std::tuple<int64_t, size_t> DirectoryBackup::DoBackup()
{
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	std::filesystem::path from_backup = "C:\\Users\\Ati\\Desktop\\folder_from_backup";
	std::filesystem::path to_backup = "C:\\Users\\Ati\\Desktop\\folder_where_to_backup";

	auto folder_name = from_backup.filename();
	auto folder_name_with_date = from_backup.filename().generic_string();

	std::set<std::string> files;
	for(auto& f : std::filesystem::directory_iterator(to_backup))
	{
		files.emplace(f.path().generic_string());
	}

	if(files.size() >= MAX_BACKUP)
	{
		auto to_remove = *files.begin();
		std::filesystem::remove_all(to_remove);
	}

	time_t current_time;
	time(&current_time);
	std::tm* now = std::localtime(&current_time);
	char datetime[64];
	strftime(datetime, sizeof(datetime), "%Y_%m_%d %H_%M_%S", now);
	folder_name_with_date += std::string(datetime);

	std::filesystem::path destination_dir = to_backup / folder_name_with_date;
	std::filesystem::create_directory(destination_dir);
	size_t file_count = 0;
	for(auto& p : std::filesystem::recursive_directory_iterator(from_backup))
	{
		auto rel_path = p.path().lexically_proximate(from_backup);

		bool is_file = std::filesystem::is_regular_file(p.path());

		if(IsInIgnoreList(rel_path.generic_u8string())) continue;
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
	DBG("completed\n");
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
	return std::make_tuple(dif, file_count);
}
#endif

std::time_t GetFileWriteTime(const std::filesystem::path& filename)  /* This ugly shit can be removed in C++20 */
{
#if defined ( _WIN32 )
	{
		struct _stat64 fileInfo;
		if(_wstati64(filename.wstring().c_str(), &fileInfo) != 0)
		{
			throw std::runtime_error("Failed to get last write time.");
		}
		return fileInfo.st_mtime;
	}
#else
	{
		auto fsTime = std::filesystem::last_write_time(filename);
		return decltype (fsTime)::clock::to_time_t(fsTime);
	}
#endif
}