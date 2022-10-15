#pragma once

#include <wx/wx.h>
#include "ILogHelper.hpp"

class LogPanel : public wxPanel, public ILogHelper
{
public:
	LogPanel(wxFrame* parent);

	wxComboBox* m_FilterLevel;
	wxTextCtrl* m_FilterText;
	wxButton* m_ApplyFilter = nullptr;
	wxButton* m_Pause = nullptr;
	wxListBox* m_Log = nullptr;
	wxButton* m_ClearButton = nullptr;
private:
	void ExecuteSearchInLogfile();

	virtual void ClearEntries() override;
	virtual void AppendLog(const std::string& line, bool scroll_to_end = false) override;
	virtual void AppendLog(const std::wstring& line, bool scroll_to_end = false) override;

	bool m_AutoScroll = true;

	wxDECLARE_EVENT_TABLE();
};