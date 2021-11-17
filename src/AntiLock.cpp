#include "pch.h"

void AntiLock::Process()
{
	if(is_enabled)
	{
		LASTINPUTINFO linput_info;
		linput_info.cbSize = sizeof(LASTINPUTINFO);
		linput_info.dwTime = 0;
		if(GetLastInputInfo(&linput_info) != 0)
		{
			DWORD last_activity_time = GetTickCount() - linput_info.dwTime;
			if(last_activity_time > (timeout * 1000))
			{
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
				LOGMSG(normal, "AntiLock executed");
			}
		}
	}
}