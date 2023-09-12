#include "pch.hpp"

BackupEntry::BackupEntry(std::filesystem::path&& from_, std::vector<std::filesystem::path>&& to_, std::vector<std::wstring>&& ignore_list_, int max_backups_,
	bool compress_, bool calculate_hash_, size_t hash_buf_size_) :
	from(std::move(from_)), to(std::move(to_)), ignore_list(std::move(ignore_list_)), max_backups(max_backups_),
	m_Compress(compress_), calculate_hash(calculate_hash_), hash_buf_size(hash_buf_size_)
{
	if(!IsValid())
		return;
}

bool BackupEntry::IsValid() const
{
	if(!std::filesystem::exists(from))
	{
		LOG(LogLevel::Error, "Backup source directory \"{}\" doesn't exists!", from.generic_string());
		return false;
	}

	if(std::filesystem::is_regular_file(from))
	{
		LOG(LogLevel::Error, "Backup source directory \"{}\" is a regular file, it should be directory!", from.generic_string());
		return false;
	}
	return true;
}

bool BackupEntry::IsInIgnoreList(std::wstring&& p) const
{
	for(auto& i : ignore_list)
	{
		if(std::search(p.begin(), p.end(), i.begin(), i.end()) != p.end() && p.length() > 0 && i.length() > 0)
		{
			return true;
		}
	}
	return false;
}

void DirectoryBackup::Init()
{

}

void DirectoryBackup::LoadEntry(const std::string& from, const std::string& to, const std::string& ignore, int max_backups, bool compress_, bool calculate_hash, size_t buffer_size)
{
	std::filesystem::path from_path = from;

	std::vector<std::filesystem::path> to_path;
	boost::split(to_path, to, [](char input) { return input == '|'; }, boost::algorithm::token_compress_on);

	std::vector<std::wstring> ignore_list;
	std::wstring ignore_w;
	utils::MBStringToWString(ignore, ignore_w);
	boost::split(ignore_list, ignore, [](char input) { return input == '|'; }, boost::algorithm::token_compress_on);
	std::unique_ptr<BackupEntry> b = std::make_unique<BackupEntry>(std::move(from_path), std::move(to_path), std::move(ignore_list), max_backups, compress_, calculate_hash, buffer_size);

	DirectoryBackup::Get()->backups.push_back(std::move(b));
}

void DirectoryBackup::BackupFile(int id)
{
	if(id < backups.size())
	{
		if(backup_future.valid())
			backup_future.get();
		backup_future = std::async(&DirectoryBackup::DoBackup, this, backups[id].get());
	}
}

bool DirectoryBackup::IsInProgress() const
{
	bool ret = false;
	if(backup_future.valid())
		ret = backup_future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready;
	return ret;
}

void DirectoryBackup::Clear()
{
	backups.clear();
}

void DirectoryBackup::DoBackup(BackupEntry* backup)
{
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	auto folder_name = backup->from.filename();
	std::wstring folder_name_with_date = folder_name.generic_wstring();

	if(!backup->IsValid())
		return;
#ifndef UNIT_TESTS
	MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
	frame->show_backup_dlg = true;
#endif
	is_cancelled = false;
	BackupRotation(backup);

	if(!backup_time_format.empty())
	{
		time_t current_time;
		time(&current_time);
		std::tm* now = std::localtime(&current_time);
		char datetime[64];
		strftime(datetime, sizeof(datetime), backup_time_format.c_str(), now);
		std::string date_tmp(datetime);

		folder_name_with_date += std::wstring(date_tmp.begin(), date_tmp.end());
	}
	else
	{
		LOG(LogLevel::Warning, "Backup time isn't set, backup functionality might not work as expected!");
	}

	std::unique_ptr<char[]> hash_buf = nullptr; 
	if(backup->calculate_hash)
		hash_buf = std::make_unique_for_overwrite<char[]>(backup->hash_buf_size * 1024 * 1024);

	bool first_run_hash = false;
	size_t file_count = 0, files_size = 0, dest_count = 0;
	SHA256_CTX ctx_from;
	SHA256_CTX ctx_to;
	uint8_t hash_from[SHA256_BLOCK_SIZE];
	uint8_t hash_tmp[SHA256_BLOCK_SIZE];
	if(backup->calculate_hash)
		sha256_init(&ctx_from);
	bool fail = false;

	std::chrono::steady_clock::time_point backup_start = std::chrono::steady_clock::now();
	for(auto& t : backup->to)
	{
		if(is_cancelled)
		{
			LOG(LogLevel::Normal, "Backup was cancelled by user");
			return;
		}

		if(backup->calculate_hash)
			sha256_init(&ctx_to);
		std::filesystem::path destination_dir = t / folder_name_with_date;

		std::error_code ec;
		std::filesystem::create_directory(destination_dir, ec);
		if(ec)
		{
			LOG(LogLevel::Error, "Error with create_directory ({}): {}", destination_dir.generic_string(), ec.message());
			fail = true;
			break;
		}
		for(auto& p : std::filesystem::recursive_directory_iterator(backup->from))
		{
			if(is_cancelled)
			{
				LOG(LogLevel::Normal, "Backup was cancelled by user");
				return;
			}

			auto rel_path = p.path().lexically_proximate(backup->from);
			bool is_file = std::filesystem::is_regular_file(p.path());

			if(backup->IsInIgnoreList(rel_path.generic_wstring())) continue;
			//DBGW(L"f: %d, %s\n", is_file, p.path().c_str());

			std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
			int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - backup_start).count();

			if(dif > 3500 || m_currentFile.empty())
			{
				std::lock_guard lock(m_TitleMutex);
				m_currentFile = p.path().generic_string();
				backup_start = std::chrono::steady_clock::now();
			}

			bool is_symlink = std::filesystem::is_symlink(p.path());
			if(is_symlink)
			{
				std::filesystem::path symlink_path; /* settings.ini */
				std::error_code ec;
				symlink_path = std::filesystem::read_symlink(p.path(), ec);
				if(ec)
				{
					LOG(LogLevel::Error, "Failed to read symlink ({}): {}", p.path().generic_string(), ec.message());
					fail = true;
					break;
				}

				std::filesystem::path destination_path = destination_dir / symlink_path;
				std::filesystem::path pointing_path = destination_dir / p.path().filename();

				auto path = std::filesystem::current_path();
				auto sim_base_path = pointing_path.lexically_relative(destination_dir);
				auto destination_symlink_path = path / destination_dir / sim_base_path;
				{
					std::error_code ec;
					std::filesystem::create_symlink(path / destination_path, destination_symlink_path);
					if(ec)
					{
						LOG(LogLevel::Error, "Error with create_symlink ({}, {}): {}", destination_path.generic_string(), destination_symlink_path.generic_string(), ec.message());
						fail = true;
						break;
					}
				}
				continue;
			}

			if(!is_file)
			{
				std::filesystem::path destination_path = destination_dir / rel_path;

				std::error_code ec;
				std::filesystem::create_directory(destination_path, ec);
				if(ec)
				{
					LOG(LogLevel::Error, "Error with create_directory ({}): {}", destination_path.generic_string(), ec.message());
					fail = true;
					break;
				}

				RestoreAttributes(p, destination_path);
			}
			else
			{  
				std::filesystem::path destination_path = destination_dir / rel_path;

				std::error_code ec;
				std::filesystem::copy_file(p.path(), destination_path, ec);
				if(ec)
				{
					LOG(LogLevel::Error, "Error with copy_file ({}): {}", destination_path.generic_string(), ec.message());
					fail = true;
					break;
				}
				if(backup->calculate_hash && std::filesystem::is_regular_file(p.path()))
				{
					std::ifstream f(destination_path, std::ifstream::binary);
					if(f)
					{
						f.peek();
						while(f.good())
						{
							std::streamsize chars_read = f.read(hash_buf.get(), backup->hash_buf_size * 1024 * 1024).gcount();
							sha256_update(&ctx_to, (uint8_t*)hash_buf.get(), chars_read);
						}
						f.close();
					}
					else
					{
						LOG(LogLevel::Error, "Failed to open file for calculating hash: \"{}\"", destination_path.generic_string());
					}
				}
			}
			
			if(!first_run_hash)  /* calculate source hash - only once */
			{
				file_count++;  /* File counter should be increased only once, at first entry */
				if(is_file)
					files_size += std::filesystem::file_size(p.path());
				if(backup->calculate_hash && std::filesystem::is_regular_file(p.path()))
				{
					std::ifstream f(p.path(), std::ifstream::binary);
					if(f)
					{
						f.peek();
						while(f.good())
						{
							std::streamsize chars_read = f.read(hash_buf.get(), backup->hash_buf_size * 1024 * 1024).gcount();
							sha256_update(&ctx_from, (uint8_t*)hash_buf.get(), chars_read);
						}
						f.close();
					}
					else
					{
						LOG(LogLevel::Error, "Failed to open file for calculating hash: \"{}\"", p.path().generic_string());
					}
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
				LOG(LogLevel::Critical, "Hash mismatch, destination dir: {}", t.generic_string());
				fail = true;
				break;
			}
		}

		if(backup->m_Compress && !fail)
		{
			m_currentFile = "Compressing";
			CompressAndRemoveFinalBackup(destination_dir);
		}
		dest_count++;
	}

	std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
	int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
#ifndef UNIT_TESTS
	{
		std::lock_guard lock(frame->mtx);
		if(!fail)
			frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::BackupCompleted), dif, file_count, files_size, dest_count, &backup->to[0] });
		else
			frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::BackupFailed), &backup->to[0] });
		frame->show_backup_dlg = false;
	}
#endif
}

void DirectoryBackup::BackupRotation(BackupEntry* backup)
{
	for(auto& t : backup->to)
	{
		if(!std::filesystem::exists(t))  /* not needed to go file checking when even the directory doesns't exists */
		{
			std::error_code ec;
			std::filesystem::create_directory(t, ec);
			if(ec)
			{
				LOG(LogLevel::Error, "Error with create_directory ({}): {}", t.generic_string(), ec.message());
			}
			continue;
		}

		std::vector<std::wstring> files;
		for(auto& f : std::filesystem::directory_iterator(t))
		{
			std::filesystem::path folder_name = f.path().filename();
			std::filesystem::path src_folder_name = backup->from.filename();

			if(folder_name.generic_wstring().starts_with(src_folder_name.generic_wstring()))  /* Add only directories, which starts with backup source folder name */
				files.push_back(f.path().generic_wstring());
		}
		if(files.size() >= static_cast<size_t>(backup->max_backups))
		{
			std::sort(files.begin(), files.end());
			auto& to_remove = *files.begin();
			std::error_code ec;
			std::filesystem::remove_all(to_remove, ec);
			if(ec)
			{
				LOG(LogLevel::Error, "Error with remove_all ({}): {}", std::string(to_remove.begin(), to_remove.end()), ec.message());
			}
		}
	}
}

void DirectoryBackup::RestoreAttributes(const std::filesystem::path& src, const std::filesystem::path& dst)
{
#ifdef _WIN32
	DWORD attributes = GetFileAttributesA(src.generic_string().c_str());
	if(attributes & FILE_ATTRIBUTE_HIDDEN)
	{
		SetFileAttributesA(dst.generic_string().c_str(), attributes);
	}
#endif
}

bool DirectoryBackup::CompressAndRemoveFinalBackup(const std::filesystem::path& dst)
{
	bool ret = true;
	std::filesystem::path dest_dir_name = dst.filename();

	std::string cmdline = std::format("7z a \"{}\" \"{}\"", dst.generic_string(), dst.generic_string());
	std::wstring cmdlinew(cmdline.begin(), cmdline.end());

	std::string result = utils::exec(cmdline.c_str());
	if(result.find("Everything is Ok"))
	{
		std::filesystem::remove_all(dst);
	}
	else
	{
		LOG(LogLevel::Error, "Failed to compress backup with command line arguments: {}", cmdline);
		ret = false;
	}
	return ret;
}
