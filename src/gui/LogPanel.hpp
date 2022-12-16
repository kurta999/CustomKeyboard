#pragma once

#include <wx/wx.h>
#include "ILogHelper.hpp"

class LogPanel : public wxPanel, public ILogHelper
{
public:
	LogPanel(wxFrame* parent);

	wxListBox* m_Log = nullptr;

private:
	void ExecuteSearchInLogfile();
	template <typename T> bool IsFilterered(const T& file);

	virtual void ClearEntries() override;
	virtual void AppendLog(const std::string& file, const std::string& line, bool scroll_to_end = false) override;
	virtual void AppendLog(const std::wstring& file, const std::wstring& line, bool scroll_to_end = false) override;

	bool m_AutoScroll = true;
	std::vector<wxString> m_LogFilters;

	wxComboBox* m_FilterLevel;
	wxTextCtrl* m_FilterText;
	wxButton* m_ApplyFilter = nullptr;
	wxButton* m_AutoScrollBtn = nullptr;
	wxButton* m_ClearButton = nullptr;

	wxDECLARE_EVENT_TABLE();
};