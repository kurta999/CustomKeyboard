#include "pch.hpp"

void TerminalHotkey::SetKey(const std::string& key_str)
{
#ifdef _WIN32
	vkey = utils::GetVirtualKeyFromString(key_str);
	if(vkey == 0xFFFF)
	{
		LOG(LogLevel::Warning, "Invalid hotkey was specified for TerminalHotkey: {}", vkey);
		TerminalHotkey::Get()->is_enabled = false;
	}
#endif
	UpdateHotkeyRegistration();
}

std::string TerminalHotkey::GetKey()
{
	return utils::GetKeyStringFromVirtualKey(vkey);
}

void TerminalHotkey::UpdateHotkeyRegistration()
{
	MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
	if(frame)
		frame->RegisterTerminalHotkey(vkey);
}

void TerminalHotkey::Process()
{
#ifdef _WIN32 /* This is already done in Linux, so this implementation is Windows only */
	if(is_enabled)
	{
		std::wstring str = utils::GetDestinationPathFromFileExplorer();
		if(!str.empty())  /* User is in file explorer */
		{
			std::filesystem::path p(str);
			if(p.has_extension() && std::filesystem::is_regular_file(p))  /* If file is selected in explorer, it has to be removed */
				p.remove_filename();
			str = p.generic_wstring();
		}
		else
		{
			HWND foreground = GetForegroundWindow();
			if(foreground)
			{
				char window_title[256];
				GetWindowTextA(foreground, window_title, sizeof(window_title));
				if(!strncmp(window_title, "Program Manager", 16) || std::strlen(window_title) == 0)  /* User has desktop in focus or clicked on system tray*/
				{
					std::string dekstop_str = getenv("USERPROFILE") + std::string("/Desktop");
					str += std::wstring(dekstop_str.begin(), dekstop_str.end());
				}

			}
		}

		if(!str.empty())
			OpenTerminal(str);
	}
#endif
}

void TerminalHotkey::OpenTerminal(std::wstring& path)
{
#ifdef _WIN32
	switch(type)
	{
		case TerminalType::COMMAND_LINE:
		{
			ShellExecute(NULL, L"open", L"cmd", std::format(L"/k cd /d \"{}\"", path).c_str(), NULL, SW_SHOW);
			break;
		}		
		case TerminalType::POWER_SHELL:
		{
			ShellExecute(NULL, L"open", L"powershell", std::format(L"-NoExit -command \"& {{Set-Location {}}}\"", path).c_str(), NULL, SW_SHOW);
			break;
		}		
		case TerminalType::BASH_TERMINAL:
		{
			ShellExecute(NULL, L"open", L"wsl", std::format(L"--cd \"{}\"", path).c_str(), NULL, SW_SHOW);
			break;
		}
		default:  /* WINDOWS_TERMINAL */
		{
			ShellExecute(NULL, L"open", L"wt", std::format(L"/d \"{}\"", path).c_str(), NULL, SW_SHOW);
		}
	}
#endif
}