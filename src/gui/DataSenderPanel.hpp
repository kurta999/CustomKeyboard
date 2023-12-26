#pragma once

#include <wx/wx.h>

#include "IDataSender.hpp"

class DataSenderPanel : public wxPanel, public IDataLogHelper
{
public:
	DataSenderPanel(wxFrame* parent);

	wxListBox* m_Log = nullptr;

private:

	virtual void ClearEntries() override;
	virtual void AppendLog(const std::string& line) override;
	virtual void OnError(uint32_t err_cnt, const std::string& line) override;

	void UpdateStatusIndicator();

	bool m_AutoScroll = true;

	wxButton* m_Send = nullptr;
	wxButton* m_Stop = nullptr;
	wxButton* m_Clear = nullptr;
	wxButton* m_SetDate = nullptr;
	wxStaticText* m_ErrorCount = nullptr;

	wxDECLARE_EVENT_TABLE();
};
