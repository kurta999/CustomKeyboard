#pragma once

#include <wx/wx.h>

class LogPanel : public wxPanel
{
public:
	LogPanel(wxFrame* parent);

	wxListBox* m_Log = nullptr;
	wxButton* m_ClearButton = nullptr;
private:
	wxDECLARE_EVENT_TABLE();
};