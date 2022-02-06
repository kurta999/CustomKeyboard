#include "pch.hpp"

bool AntiLock::IsSessionActive()
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

#endif
}

void AntiLock::SimulateUserActivity()
{
#ifdef _WIN32
    POINT pos;
    GetCursorPos(&pos);
    step_forward ^= step_forward;
    if(step_forward)
    {
        pos.x += 5;
        pos.y += 5;
    }
    else
    {
        pos.x -= 6;
        pos.y -= 6;
    }
    SetCursorPos(pos.x, pos.y);
    ShowCursor(TRUE);

    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = VK_SCROLL;
    input.ki.dwFlags = 0; // 0 = press
    SendInput(1, &input, sizeof(INPUT));
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
#else

#endif
    LOGMSG(normal, "AntiLock executed");
}

void AntiLock::Process()
{
	if(is_enabled)
	{
#ifdef _WIN32
		LASTINPUTINFO linput_info;
		linput_info.cbSize = sizeof(LASTINPUTINFO);
		linput_info.dwTime = 0;
		if(GetLastInputInfo(&linput_info) != 0)
		{
			DWORD last_activity_time = GetTickCount() - linput_info.dwTime;
			if(last_activity_time > (timeout * 1000))
			{
                if(IsSessionActive())
                    SimulateUserActivity();
			}
		}
#else
        SimulateUserActivity();
#endif
	}
}