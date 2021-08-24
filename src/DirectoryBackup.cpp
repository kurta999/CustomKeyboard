#include "pch.h"

void DirectoryBackup::DoBackup(BackupEntry* backup)
{
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	auto folder_name = backup->from.filename();
	auto folder_name_with_date = folder_name.generic_string();

	if(!std::filesystem::exists(backup->from))
		return; /* if source directory doesn't exists, just do nothing */

	MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
	frame->show_backup_dlg = true;
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

	char* hash_buf = nullptr;
	if(backup->calculate_hash)
		hash_buf = new char[backup->hash_buf_size * 1024 * 1024];

	bool first_run_hash = false;
	size_t file_count = 0, files_size = 0;
	SHA256_CTX ctx_from;
	SHA256_CTX ctx_to;
	uint8_t hash_from[SHA256_BLOCK_SIZE];
	uint8_t hash_tmp[SHA256_BLOCK_SIZE];
	if(backup->calculate_hash)
		sha256_init(&ctx_from);
	bool fail = false;
	for(auto& t : backup->to)
	{
		if(backup->calculate_hash)
			sha256_init(&ctx_to);
		std::filesystem::path destination_dir = t / folder_name_with_date;
		std::filesystem::create_directory(destination_dir);
		for(auto& p : std::filesystem::recursive_directory_iterator(backup->from))
		{
			auto rel_path = p.path().lexically_proximate(backup->from);

			bool is_file = std::filesystem::is_regular_file(p.path());

			if(backup->IsInIgnoreList(rel_path.generic_u8string())) continue;
			//DBGW(L"f: %d, %s\n", is_file, p.path().c_str());

			if(!is_file)
			{
				std::filesystem::path destination_path = destination_dir / rel_path;
				std::filesystem::create_directory(destination_path);
			}
			else
			{
				std::filesystem::path destination_path = destination_dir / rel_path;
				std::filesystem::copy_file(p.path(), destination_path);

				if(backup->calculate_hash)
				{
					std::ifstream f(destination_path, std::ifstream::binary);
					f.peek();
					while(f.good())
					{
						std::streamsize chars_read = f.read(hash_buf, backup->hash_buf_size * 1024 * 1024).gcount();
						sha256_update(&ctx_to, (uint8_t*)hash_buf, chars_read);
					}
					f.close();
				}
			}
			
			if(!first_run_hash)  /* calculate source hash - only once */
			{
				file_count++;
				if(is_file)
					files_size += std::filesystem::file_size(p.path());
				if(backup->calculate_hash)
				{
					std::ifstream f(p.path(), std::ifstream::binary);
					f.peek();
					while(f.good())
					{
						std::streamsize chars_read = f.read(hash_buf, backup->hash_buf_size * 1024 * 1024).gcount();
						sha256_update(&ctx_from, (uint8_t*)hash_buf, chars_read);
					}
					f.close();
				}
			}
		}

		if(backup->calculate_hash)
		{
			if(!first_run_hash)
			{
				first_run_hash = true;
				sha256_final(&ctx_from, hash_from);
			}

			sha256_final(&ctx_to, hash_tmp);
			if(memcmp(hash_from, hash_tmp, sizeof(hash_from)) != 0)
			{
				DBG("Hash mismatch\n");
				LOGMSG(critical, "Hash mismatch, destination dir: {}", t.generic_string());
				fail = true;
				break;
			}
		}
	}
	if(hash_buf)
		delete[] hash_buf;

	std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
	int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
	
	{
		std::lock_guard<std::mutex> lock(frame->mtx);
		if(!fail)
			frame->pending_msgs.push_back({ (uint8_t)BackupCompleted, dif, file_count, files_size, &backup->to[0] });
		else
			frame->pending_msgs.push_back({ (uint8_t)BackupFailed });
		frame->show_backup_dlg = false;
	}
}

void DirectoryBackup::Init()
{

}

void DirectoryBackup::BackupFile(int id)
{
	if(id < backups.size())
	{
		if(backup_future.valid())
			backup_future.get();
		backup_future = std::async(&DirectoryBackup::DoBackup, this, backups[id]);
	}
}

bool DirectoryBackup::IsInProgress()
{
	bool ret = false;
	if(backup_future.valid())
		ret = backup_future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready;
	return ret;
}