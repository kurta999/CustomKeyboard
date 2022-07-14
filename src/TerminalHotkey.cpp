#include "pch.hpp"

void TerminalHotkey::Process()
{
#ifdef _WIN32 /* This is already done in Linux, so this implementation is Windows only */
	if(is_enabled && wxGetKeyState(vkey))
	{
		std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		int64_t dif = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - last_execution).count();
		if(dif < 500)  /* Avoid bouncing */
			return;

		std::wstring str = utils::GetDestinationPathFromFileExplorer();
		if(!str.empty())  /* User is in file explorer */
		{
			std::filesystem::path p(str);
			if(p.has_extension())  /* If file is selected in explorer, it has to be removed */
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
				if(!strncmp(window_title, "Program Manager", 16))  /* User has desktop in focus */
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
	last_execution = std::chrono::steady_clock::now();
#ifdef _WIN32
	ShellExecute(NULL, L"open", L"wt", std::format(L"/d \"{}\"", path).c_str(), NULL, SW_SHOW);
#endif

	/* swprintf(buf, L"/k cd /d %s", str.c_str()); - for cmd */
	/* swprintf(buf, L"/d %s", str.c_str()); - for wt */
}