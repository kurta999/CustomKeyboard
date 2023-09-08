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
	//template <typename T> bool IsFilterered(const T& file);

	virtual void ClearEntries() override;
	virtual void AppendLog(const std::string& file, const std::string& line, bool scroll_to_end = false) override;
	virtual void AppendLog(const std::wstring& file, const std::wstring& line, bool scroll_to_end = false) override;

	bool m_AutoScroll = true;

	wxComboBox* m_FilterLevel = nullptr;
	wxTextCtrl* m_FilterText = nullptr;
	wxButton* m_ApplyFilter = nullptr;
	wxButton* m_AutoScrollBtn = nullptr;
	wxButton* m_ClearButton = nullptr;
	wxComboBox* m_DefaultLogLevel = nullptr;
	wxButton* m_ApplyButton = nullptr;
	wxButton* m_FilterList = nullptr;

	wxDECLARE_EVENT_TABLE();
};