#include "pch.h"

wxBEGIN_EVENT_TABLE(ComTcpPanel, wxPanel)

wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(KeybrdPanel, wxPanel)
EVT_TREELIST_SELECTION_CHANGED(ID_AppBindList, KeybrdPanel::OnTreeListChanged)
wxEND_EVENT_TABLE()

ConfigurationPanel::ConfigurationPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxSize client_size = GetClientSize();

	wxAuiNotebook* m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(WINDOW_SIZE_X, WINDOW_SIZE_Y), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	comtcp_panel = new ComTcpPanel(m_notebook);
	keybrd_panel = new KeybrdPanel(m_notebook);
	m_notebook->AddPage(comtcp_panel, "Main settings", false, 0);
	m_notebook->AddPage(keybrd_panel, "Macro settings", false, 0);
}

/*
* 
* [TCP_Backend]
Enable=0
TCP_Port = 2005 # TCP Port for receiving measurements from sensors

[COM_Backend]
Enable=0
COM = 5 # Com port for UART where data received from STM32

[App]
MinimizeOnExit = 0
MinimizeOnStartup = 0
DefaultPage = 1*/

ComTcpPanel::ComTcpPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxArrayString array_serials;
	CEnumerateSerial::CPortAndNamesArray ports;
	CEnumerateSerial::UsingSetupAPI1(ports);
	for(auto& i : ports)
		array_serials.Add(wxString::Format("COM%d (%s)", i.first, i.second));

	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_IsTcp = new wxCheckBox(this, wxID_ANY, wxT("Enable TCP?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsTcp, 0, wxALL, 5);

	bSizer1->Add(new wxStaticText(this, wxID_ANY, wxT("TCP port:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	m_TcpPortSpin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0);
	bSizer1->Add(m_TcpPortSpin, 0, wxALL, 5);
	
	bSizer1->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND | wxALL, 5);

	m_IsCom = new wxCheckBox(this, wxID_ANY, wxT("Enable COM?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsCom, 0, wxALL, 5);

	bSizer1->Add(new wxStaticText(this, wxID_ANY, wxT("COM port:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	m_serial = new wxComboBox(this, wxID_ANY, "Select COM port", wxDefaultPosition, wxDefaultSize, array_serials);
	bSizer1->Add(m_serial);

	bSizer1->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND | wxALL, 5);

	m_IsMinimizeOnExit = new wxCheckBox(this, wxID_ANY, wxT("Minimize on exit?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsMinimizeOnExit, 0, wxALL, 5);

	m_IsMinimizeOnStartup = new wxCheckBox(this, wxID_ANY, wxT("Minimize on startup?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsMinimizeOnStartup, 0, wxALL, 5);

	bSizer1->Add(new wxStaticText(this, wxID_ANY, wxT("Default page:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	m_DefaultPage = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0);
	bSizer1->Add(m_DefaultPage, 0, wxALL, 5);
	
	SetSizer(bSizer1);
}

enum
{ 
	Col_Files = 1,
};

void KeybrdPanel::OnTreeListChanged(wxTreeListEvent& evt)
{
	wxTreeListItem item = evt.GetItem();
	wxTreeListItem root = tree->GetItemParent(item);
	wxString str = tree->GetItemText(item, 1);
	wxString str2 = tree->GetItemText(root, 0);
	DBG("sel: %d, %s - root: %s\n", evt.GetSelection(), str.ToStdString().c_str(), str2.ToStdString().c_str());
	
	std::unique_ptr<MacroContainer> ptr;
	auto& m = CustomMacro::Get()->GetMacros();
	for(auto& i : m)
	{
		if(i->name == str2.ToStdString())
		{
			for(auto& x : i->key_vec[str.ToStdString()])
			{
				KeyClass* p = x.get();
				if(dynamic_cast<KeyText*>(p))
					DBG("KeyText");
				if(dynamic_cast<KeyCombination*>(p))
					DBG("KeyCombination");
				if(dynamic_cast<KeyDelay*>(p))
					DBG("KeyDelay");
			}
		}
	}
	
}

KeybrdPanel::KeybrdPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	// Use wxTreeList as base
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxHORIZONTAL);

	tree = new wxTreeListCtrl(this, ID_AppBindList, wxDefaultPosition, wxSize(300, 400), wxTL_DEFAULT_STYLE);
	tree->AppendColumn("App name", tree->WidthFor("App nameApp nameApp name"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree->AppendColumn("Key bindings", tree->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	//tree->AppendColumn("C", tree->WidthFor("1,000,000 KiB"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	
	wxTreeListItem root = tree->GetRootItem();
	for(auto& m : CustomMacro::Get()->GetMacros())
	{
		wxTreeListItem item = tree->AppendItem(root, m->name.c_str()); 
		//tree->SetItemText(item, Col_Files, "test");
		for(auto& b : m->bind_name)
		{
			DBG("bind name: %s\n", b.second.c_str());
			wxTreeListItem bind_item = tree->AppendItem(item, b.second.c_str());
			tree->SetItemText(bind_item, Col_Files, b.first.c_str());
		}
		tree->Expand(item);
	}

	bSizer1->Add(tree, wxSizerFlags(2).Left());

	tree_binds = new wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(300, 400), wxTL_DEFAULT_STYLE);
	tree_binds->AppendColumn("App name", tree_binds->WidthFor("App nameApp nameApp name"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree_binds->AppendColumn("Key bindings", tree_binds->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);

	wxTreeListItem root2 = tree_binds->GetRootItem();
	for(auto& m : CustomMacro::Get()->GetMacros())
	{
		wxTreeListItem item = tree_binds->AppendItem(root2, m->name.c_str());
		tree_binds->SetItemText(item, Col_Files, "test");
		for(auto& b : m->bind_name)
		{
			DBG("bind name: %s\n", b.second.c_str());
			wxTreeListItem bind_item = tree_binds->AppendItem(item, b.second.c_str());
			tree_binds->SetItemText(bind_item, Col_Files, b.first.c_str());
		}
		tree_binds->Expand(item);
	}

	bSizer1->Add(tree_binds, wxSizerFlags(2).Top());

	SetSizer(bSizer1);
	Show();
}

