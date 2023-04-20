#include "pch.hpp"

#include <wx/headerctrl.h>

wxBEGIN_EVENT_TABLE(CanPanel, wxPanel)
EVT_SIZE(CanPanel::OnSize)
wxEND_EVENT_TABLE()

CanPanel::CanPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
    wxSize client_size = GetClientSize();

    m_mgr.SetManagedWindow(this);

    m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(Settings::Get()->window_size.x - 50, Settings::Get()->window_size.y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
    sender = new CanSenderPanel(this);
    log = new CanLogPanel(this);
    script = new CanScriptPanel(this);
    m_notebook->Freeze();
    m_notebook->AddPage(sender, "Sender", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->AddPage(log, "Log", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->AddPage(script, "Script", false, wxArtProvider::GetBitmap(wxART_PLUS, wxART_OTHER, FromDIP(wxSize(16, 16))));
    m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(CanPanel::Changeing), NULL, this);
    
    /* size: 1640x1080 */
    m_notebook->Split(0, wxLEFT);
    
    m_notebook->Thaw();
    m_notebook->SetAutoLayout(true);
    m_notebook->Layout();
    m_notebook->SetSize(m_notebook->GetSize());
    m_notebook->SetSelection(0);
}

CanPanel::~CanPanel()
{
    m_mgr.UnInit();  /* deinitialize the frame manager */
}

void CanPanel::RefreshSubpanels()
{
    sender->RefreshSubpanels();
}

void CanPanel::LoadTxList()
{
    sender->LoadTxList();
}

void CanPanel::SaveTxList()
{
    sender->SaveTxList();
}

void CanPanel::LoadRxList()
{
    sender->LoadRxList();
}

void CanPanel::SaveRxList()
{
    sender->SaveRxList();
}

void CanPanel::LoadMapping()
{
    sender->LoadMapping();
}

void CanPanel::SaveMapping()
{
    sender->SaveMapping();
}

void CanPanel::On10MsTimer()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    bool is_lock_ok = can_handler->m.try_lock();
    if(!is_lock_ok)
    {
        DBG("\n\nCanSenderPanel::On10MsTimer lock failed\n\n");
        return;
    }
    sender->On10MsTimer();
    log->On10MsTimer();
    can_handler->m.unlock();
}

void CanPanel::Changeing(wxAuiNotebookEvent& event)
{
    /*
    int sel = event.GetSelection();
    if(sel == 0)
    {
        comtcp_panel->Update();
    }
    */
}

void CanPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}