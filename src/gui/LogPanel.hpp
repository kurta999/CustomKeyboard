#pragma once

#include <wx/wx.h>
#include "ILogHelper.hpp"

class LogPanel : public wxPanel, public ILogHelper
{
public:
	LogPanel(wxFrame* parent);

	wxComboBox* m_FilterLevel;
	wxTextCtrl* m_FilterText;
	wxButton* m_Filter = nullptr;
	wxListBox* m_Log = nullptr;
	wxButton* m_ClearButton = nullptr;
private:
	void ExecuteSearchInLogfile();

	virtual void ClearEntries() override;
	virtual void AppendLog(std::string& line) override;

	wxDECLARE_EVENT_TABLE();
};