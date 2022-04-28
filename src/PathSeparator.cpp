#include "pch.hpp"

void PathSeparator::ReplaceClipboard()
{
	if(wxTheClipboard->Open())
	{
		if(wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			std::string input(data.GetText());
			ReplaceString(input);

			wxTheClipboard->SetData(new wxTextDataObject(input));
			MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
			{
				std::lock_guard lock(frame->mtx);
				frame->pending_msgs.push_back({ PathSeparatorsReplaced, std::move(input) });
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
