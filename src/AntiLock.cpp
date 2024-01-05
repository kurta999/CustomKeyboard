#include "pch.hpp"

void AntiLock::LoadExclusions(const std::string& input)
{
    if(!input.empty())
    {
        std::vector<std::string> ignore_list;
        boost::split(ignore_list, input, [](char input) { return input == '|'; }, boost::algorithm::token_compress_on);
        AntiLock::Get()->exclusions = std::move(ignore_list);
        for(auto& i : AntiLock::Get()->exclusions)
        {
            LOG(LogLevel::Notification, "AntiLock exclusion: {}", i);
        }
    }
}

const std::string AntiLock::SaveExclusions() const
{
    std::string exclusions;
    for(auto& x : AntiLock::Get()->exclusions)
    {
        exclusions += x + '|';
    }
    if(!exclusions.empty() && exclusions.back() == '|')
        exclusions.pop_back();
    return exclusions;
}

bool AntiLock::IsSessionActive() const
{
#ifdef _WIN32
    typedef BOOL(PASCAL* WTSQuerySessionInformation)(HANDLE hServer, DWORD SessionId, WTS_INFO_CLASS WTSInfoClass, LPTSTR* ppBuffer, DWORD* pBytesReturned);
    typedef void (PASCAL* WTSFreeMemory)(PVOID pMemory);

    WTSINFOEXW* pInfo = NULL;
    WTS_INFO_CLASS wtsic = WTSSessionInfoEx;
    bool bRet = false;
    LPTSTR ppBuffer = NULL;
    DWORD dwBytesReturned = 0;
    LONG dwFlags = 0;
    WTSQuerySessionInformation pWTSQuerySessionInformation = NULL;
    WTSFreeMemory pWTSFreeMemory = NULL;

    HMODULE hLib = LoadLibrary(L"wtsapi32.dll");
    if(!hLib)
    {
        return false;
    }
    pWTSQuerySessionInformation = (WTSQuerySessionInformation)GetProcAddress(hLib, "WTSQuerySessionInformationW");
    if(pWTSQuerySessionInformation)
    {
        pWTSFreeMemory = (WTSFreeMemory)GetProcAddress(hLib, "WTSFreeMemory");
        if(pWTSFreeMemory != NULL)
        {
            DWORD dwSessionID = WTSGetActiveConsoleSessionId();
            if(pWTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, dwSessionID, wtsic, &ppBuffer, &dwBytesReturned))
            {
                if(dwBytesReturned > 0)
                {
                    pInfo = (WTSINFOEXW*)ppBuffer;
                    if(pInfo->Level == 1)
                    {
                        dwFlags = pInfo->Data.WTSInfoExLevel1.SessionFlags;
                    }
                    if(dwFlags == WTS_SESSIONSTATE_UNLOCK)
                    {
                        bRet = true;
                    }
                }
                pWTSFreeMemory(ppBuffer);
                ppBuffer = NULL;
            }
        }
    }
    if(hLib != NULL)
    {
        FreeLibrary(hLib);
    }
    return bRet;
#else
    return true;
#endif
}

void AntiLock::SimulateUserActivity()
{
#ifdef _WIN32
    if(is_suspended)
        return;

    std::string log_string = "AntiLock executed: ";
    POINT pos;
    GetCursorPos(&pos);
    m_StepForward = !m_StepForward;
    if(m_StepForward)
    {
        m_LastStep.x = utils::random_mt(5, 300);
        m_LastStep.y = utils::random_mt(5, 300);

        pos.x += m_LastStep.x;
        pos.y += m_LastStep.y;
    }
    else
    {
        pos.x -= (m_LastStep.x - utils::random_mt(1, 50));
        pos.y -= (m_LastStep.y - utils::random_mt(1, 50));
    }

    SetCursorPos(pos.x, pos.y);
    ShowCursor(TRUE);

    if(utils::random_mt(1, 3) == 2)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_SCROLL;
        input.ki.dwFlags = 0; // 0 = press
        SendInput(1, &input, sizeof(INPUT));
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));

        log_string += "VK_SCROLL, ";
    }

    if(utils::random_mt(1, 5) == 2)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_NUMLOCK;
        input.ki.dwFlags = 0; // 0 = press
        SendInput(1, &input, sizeof(INPUT));
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));

        log_string += "VK_NUMLOCK, ";
    }

    if(utils::random_mt(1, 5) == 2)
    {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.ki.wVk = MOUSEEVENTF_WHEEL;
        input.ki.dwFlags = 0; // 0 = press
        SendInput(1, &input, sizeof(INPUT));
        //input.ki.dwFlags = KEYEVENTF_KEYUP;
        //SendInput(1, &input, sizeof(INPUT));

        log_string += "MOUSEEVENTF_WHEEL";
    }
#else

#endif
    if(is_screensaver)
        StartScreenSaver();
    LOG(LogLevel::Normal, "{}", log_string);
}

void AntiLock::StartScreenSaver() const
{
#ifdef _WIN32
    static const std::string screensavers[] = { "PhotoScreensaver", "Mystify", "Ribbons" };
    int ret = utils::random_mt<int>(0, std::size(screensavers) - 1);

    ShellExecuteA(NULL, NULL, std::format("C:\\Windows\\System32\\{}.scr", screensavers[ret]).c_str(), NULL, "/s", SW_SHOWNORMAL);
#else

#endif
}

bool AntiLock::IsAnExclusion(const std::string& p) const
{
    for(auto& i : exclusions)
    {
        if(i.empty()) continue;
        if(std::search(p.begin(), p.end(), i.begin(), i.end()) != p.end())
        {
            return true;
        }
    }
    return false;
}

void AntiLock::Process()
{
    //LOG(LogLevel::Normal, "first");
	if(is_enabled || IdlePowerSaver::Get()->is_enabled)
	{
#ifdef _WIN32
		LASTINPUTINFO linput_info;
		linput_info.cbSize = sizeof(LASTINPUTINFO);
		linput_info.dwTime = 0;
        //LOG(LogLevel::Normal, "after is_enabled");
		if(GetLastInputInfo(&linput_info) != 0)
		{
            //LOG(LogLevel::Normal, "GetLastInputInfo");
            m_LastActivityTime = GetTickCount() - linput_info.dwTime;
            IdlePowerSaver::Get()->Process(m_LastActivityTime);
			if(m_LastActivityTime > (timeout * 1000) && is_enabled)
			{
                //LOG(LogLevel::Normal, "m_LastActivityTime > (timeout * 1000)");
                HWND foreground = GetForegroundWindow();
                if(foreground)
                {
                    //LOG(LogLevel::Normal, "foreground");
                    char window_title[256];
                    if(GetWindowTextA(foreground, window_title, sizeof(window_title)))
                    {
                        //LOG(LogLevel::Normal, "GetWindowTextA");
                        if(!IsAnExclusion(window_title))
                        {
                            //LOG(LogLevel::Normal, "IsAnExclusion");
                            if(IsSessionActive())
                            {
                                //LOG(LogLevel::Normal, "IsSessionActive");
                                if(GetTickCount() - m_LastExecution > timeout * 1000)
                                {
									//LOG(LogLevel::Normal, "GetTickCount() - m_LastExecution > 1000");
									m_LastExecution = GetTickCount();
									SimulateUserActivity();
								}
                            }
                        }
                    }
                }
			}
            else
            {
                m_LastExecution = 0;
            }
		}
#endif
	}
}