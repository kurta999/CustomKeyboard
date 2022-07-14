#include "pch.hpp"

bool SymlinkCreator::HandleKeypress(std::string& pressed_keys)
{
	bool ret = false;
	if(is_enabled)
	{
		if(pressed_keys == mark_key)
		{
			Mark();
			ret = true;
		}
		else if(pressed_keys == place_symlink_key)
		{
			Place(true);
			ret = true;
		}		
		else if(pressed_keys == place_hardlink_key)
		{
			Place(false);
			ret = true;
		}
	}
	return ret;
}

void SymlinkCreator::UnmarkFiles()
{
	m_SelectedItems.clear();
}

void SymlinkCreator::Mark()
{
	m_SelectedItems = utils::GetSelectedItemsFromFileExplorer();
	MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
	{
		std::lock_guard lock(frame->mtx);
		frame->pending_msgs.push_back({ !m_SelectedItems.empty() ? static_cast<uint8_t>(PopupMsgIds::LinkMark) : static_cast<uint8_t>(PopupMsgIds::LinkMarkError),
			static_cast<uint32_t>(m_SelectedItems.size()) });
	}
}

void SymlinkCreator::Place(bool is_symlink)
{
	std::wstring dest_path = utils::GetDestinationPathFromFileExplorer();
	if(!dest_path.empty() && !m_SelectedItems.empty())
	{
		std::filesystem::path dest = dest_path;
		if(is_symlink)
		{
			for(auto& item : m_SelectedItems)
			{
				std::filesystem::path dest_with_name = dest_path / std::filesystem::path(item).filename();
				try
				{
					if(std::filesystem::is_directory(item))
						std::filesystem::create_directory_symlink(item, dest_with_name);
					else
						std::filesystem::create_symlink(item, dest_with_name);
				}
				catch(std::filesystem::filesystem_error const& e)
				{
					LOG(LogLevel::Error, "Exception during creating symlinks ({}): {}", dest_with_name.generic_string(), e.what());
					break;
				}
			}

			MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
			{
				std::lock_guard lock(frame->mtx);
				frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::SymlinkCreated), static_cast<uint32_t>(m_SelectedItems.size()) });
			}
		}
		else
		{
			for(auto& item : m_SelectedItems)
			{
				std::filesystem::path dest_with_name = dest_path / std::filesystem::path(item).filename();
				try
				{
					std::filesystem::create_hard_link(item, dest_with_name);
				}
				catch(std::filesystem::filesystem_error const& e)
				{
					LOG(LogLevel::Error, "Exception during creating hardlinks ({}): {}", dest_with_name.generic_string(), e.what());
					break;
				}
			}

			MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
			{
				std::lock_guard lock(frame->mtx);
				frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::HardlinkCreated), static_cast<uint32_t>(m_SelectedItems.size()) });
			}
		}
	}
}