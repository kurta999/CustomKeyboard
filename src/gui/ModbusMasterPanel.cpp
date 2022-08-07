#include "pch.hpp"

wxBEGIN_EVENT_TABLE(ModbusMasterPanel, wxPanel)
EVT_SIZE(ModbusMasterPanel::OnSize)
wxEND_EVENT_TABLE()

ModbusMasterPanel::ModbusMasterPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxSize client_size = GetClientSize();

	m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(Settings::Get()->window_size.x - 50, Settings::Get()->window_size.y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

//	m_notebook->AddPage(comtcp_panel, "Main settings", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
	//m_notebook->AddPage(keybrd_panel, "Macro settings", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(ConfigurationPanel::Changeing), NULL, this);
}

void ModbusMasterPanel::UpdateSubpanels()
{

}

void ModbusMasterPanel::OnSize(wxSizeEvent& event)
{
	event.Skip(true);
}

void ModbusMasterPanel::Changeing(wxAuiNotebookEvent& event)
{
	int sel = event.GetSelection();
	if(sel == 0)
	{
		//comtcp_panel->Update();
	}
}