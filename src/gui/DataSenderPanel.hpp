#pragma once

#include <wx/wx.h>

#include "IDataSender.hpp"

class DataSenderDataPanel : public wxPanel
{
public:
    DataSenderDataPanel(wxWindow* parent);
    void OnSize(wxSizeEvent& evt);

    wxListBox* m_Log = nullptr;

    void UpdateStatusIndicator();

    bool m_AutoScroll = true;

    wxButton* m_Send = nullptr;
    wxButton* m_Stop = nullptr;
    wxButton* m_Clear = nullptr;
    wxButton* m_SetDate = nullptr;
    wxStaticText* m_ErrorCount = nullptr;

private:
    wxDECLARE_EVENT_TABLE();
};

class DataSenderButtonPanel : public wxPanel
{
public:
    DataSenderButtonPanel(wxWindow* parent);
    void OnDataLoaded();
    void OnSize(wxSizeEvent& evt);

private:
    wxDECLARE_EVENT_TABLE();
};

class DataSenderPanel : public wxPanel, public IDataLogHelper
{
public:
	DataSenderPanel(wxFrame* parent);

    void OnDataLoaded();
    void On10MsTimer();
    void OnSize(wxSizeEvent& evt);

    wxAuiNotebook* m_notebook = nullptr;
    DataSenderDataPanel* data_panel = nullptr;
    DataSenderButtonPanel* button_panel = nullptr;

private:

	virtual void ClearEntries() override;
	virtual void AppendLog(DataEntry* entry, const std::string& line) override;
	virtual void OnError(uint32_t err_cnt, const std::string& line) override;

    // !\brief AUI manager for subwindows
    wxAuiManager m_mgr;


	wxDECLARE_EVENT_TABLE();
};
