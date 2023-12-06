#include "pch.hpp"

void PathSeparator::ReplaceClipboard(ReplaceType type)
{
	if(wxTheClipboard->Open())
	{
		if(wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			std::string input(data.GetText());

			switch(type)
			{
				case ReplaceType::PATH_SEPARATOR:
				{
					ReplaceString(input);
					break;
				}				
				case ReplaceType::WSL:
				{
					ReplaceStringFromWindowsToWsl(input);
					break;
				}
			}

			wxTheClipboard->SetData(new wxTextDataObject(input));
			MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
			{
				std::lock_guard lock(frame->mtx);
				frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::PathSeparatorsReplaced), std::move(input) });
			}
		}
		wxTheClipboard->Close();
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

void PathSeparator::ReplaceStringFromWindowsToWsl(std::string& str)
{
	size_t pos = str.find("/mnt/");
	if(pos != std::string::npos)
	{
		boost::algorithm::replace_all(str, "/", "\\");
		str.erase(0, 5); // erase mnt
		str[0] = std::toupper(str[0]);
		str.insert(1, ":");
	}
	else
	{
		boost::algorithm::replace_all(str, "\\", "/");
	}
}
