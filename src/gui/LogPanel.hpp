#pragma once

#include <wx/wx.h>

class LogPanel : public wxPanel
{
public:
	LogPanel(wxFrame* parent);

	wxListBox* m_Log;
	wxButton* m_ClearButton = nullptr;
private:
	wxDECLARE_EVENT_TABLE();
};