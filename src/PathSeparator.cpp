#include "pch.h"

void PathSeparator::ReplaceClipboard()
{
	if(OpenClipboard(0))
	{
		char* text = (char*)GetClipboardData(CF_TEXT);
		if(text)
		{
			std::string input(text);
			ReplaceString(input);

			HGLOBAL clipbuffer;
			EmptyClipboard();
			clipbuffer = GlobalAlloc(GMEM_DDESHARE, input.length() + 1);
			if(clipbuffer)
			{
				char* buffer = static_cast<char*>(GlobalLock(clipbuffer));
				if(buffer)
					strncpy(buffer, input.c_str(), input.length() + 1);
				GlobalUnlock(clipbuffer);
				SetClipboardData(CF_TEXT, clipbuffer);
			}
		}
		CloseClipboard();
	}
}

void PathSeparator::ReplaceString(std::string& str)
{
	size_t pos = str.find('\\');
	if(pos != std::string::npos)
		boost::algorithm::replace_all(str, "\\", "/");
	else
		boost::algorithm::replace_all(str, "/", "\\");
}
