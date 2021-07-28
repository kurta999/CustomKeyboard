#include "pch.h"

wxBEGIN_EVENT_TABLE(MacroEditBoxDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, MacroEditBoxDialog::OnApply)
//EVT_BUTTON(wxID_CLOSE, TestMessageBoxDialog::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MacroAddBoxDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, MacroAddBoxDialog::OnApply)
//EVT_BUTTON(wxID_CLOSE, TestMessageBoxDialog::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ComTcpPanel, wxPanel)

wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(KeybrdPanel, wxPanel)
EVT_TREELIST_SELECTION_CHANGED(ID_AppBindListMain, KeybrdPanel::OnTreeListChanged_Main)
EVT_TREELIST_ITEM_CONTEXT_MENU(ID_AppBindListMain, KeybrdPanel::OnItemContextMenu_Main)
EVT_TREELIST_SELECTION_CHANGED(ID_MacroDetails, KeybrdPanel::OnTreeListChanged_Details)
EVT_TREELIST_ITEM_CONTEXT_MENU(ID_MacroDetails, KeybrdPanel::OnItemContextMenu_Details)
EVT_TREELIST_ITEM_ACTIVATED(ID_MacroDetails, KeybrdPanel::OnItemActivated)
wxEND_EVENT_TABLE()

MacroAddBoxDialog::MacroAddBoxDialog(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, "Message Box Test Dialog", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);

	// this sizer allows to configure the messages shown in the message box
	wxSizer* const sizerMsgs = new wxStaticBoxSizer(wxVERTICAL, this, "&Add new macro");

	sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Macro Key:"));
	m_macroKey = new wxTextCtrl(this, wxID_ANY, "a", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	wxArrayString a;
	auto& map = CustomMacro::Get()->GetHidScanCodeMap();
	for(auto& i : map)
	{
		a.Add(i.first);
	}
	m_macroKey->AutoComplete(a);
	sizerMsgs->Add(m_macroKey, wxSizerFlags(1).Expand().Border(wxBOTTOM));

	sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Macro Name:"));
	m_macroName = new wxTextCtrl(this, wxID_ANY, "a", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	sizerMsgs->Add(m_macroName, wxSizerFlags(1).Expand().Border(wxBOTTOM));
	sizerTop->Add(sizerMsgs, wxSizerFlags(1).Expand().Border());

	// miscellaneous other stuff
	wxSizer* const sizerFlags = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Note:");
	sizerFlags->Add(new wxStaticText(this, wxID_ANY, "Press SCROLL LOCK to get mouse coordinates relative to the active window"), wxSizerFlags().Border());
	sizerTop->Add(sizerFlags, wxSizerFlags().Expand().Border());

	// finally buttons to show the resulting message box and close this dialog
	sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCLOSE), wxSizerFlags().Right().Border()); /* wxOK */

	SetSizerAndFit(sizerTop);
	CentreOnScreen();
}

MacroEditBoxDialog::MacroEditBoxDialog(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, "Message Box Test Dialog", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);

	// this sizer allows to configure the messages shown in the message box
	wxSizer* const sizerMsgs = new wxStaticBoxSizer(wxVERTICAL, this, "&Macro settings");

	wxString choices[] = { wxT("Sequence"), wxT("Text"), wxT("Delay"), wxT("Mouse move"), wxT("Mouse click") };
	m_radioBox1 = new wxRadioBox(this, wxID_ANY, wxT("Action type"), wxDefaultPosition, wxDefaultSize, WXSIZEOF(choices), choices, 1, wxRA_SPECIFY_ROWS);
	sizerMsgs->Add(m_radioBox1, wxSizerFlags().Left());
	sizerMsgs->Add(new wxStaticText(this, wxID_ANY, "&Macro text:"));
	m_textMsg = new wxTextCtrl(this, wxID_ANY, "a", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	sizerMsgs->Add(m_textMsg, wxSizerFlags(1).Expand().Border(wxBOTTOM));
	sizerTop->Add(sizerMsgs, wxSizerFlags(1).Expand().Border());

	// miscellaneous other stuff
	wxSizer* const sizerFlags = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Note:");
	sizerFlags->Add(new wxStaticText(this, wxID_ANY, "Press SCROLL LOCK to get mouse coordinates relative to the active window"), wxSizerFlags().Border());
	sizerTop->Add(sizerFlags, wxSizerFlags().Expand().Border());

	// finally buttons to show the resulting message box and close this dialog
	sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCLOSE), wxSizerFlags().Right().Border()); /* wxOK */

	SetSizerAndFit(sizerTop);
	CentreOnScreen();
}

void MacroEditBoxDialog::ShowDialog(const wxString& str, uint8_t macro_type)
{
	m_radioBox1->SetSelection(macro_type);
	m_textMsg->SetLabel(str);
	m_IsApplyClicked = false;
	ShowModal();
	DBG("isapply: %d", IsApplyClicked());
}

void MacroAddBoxDialog::ShowDialog(const wxString& macro_key, const wxString& macro_name)
{
	m_macroKey->SetLabel(macro_key);
	m_macroName->SetLabel(macro_name);
	m_IsApplyClicked = false;
	ShowModal();
	DBG("isapply: %d", IsApplyClicked());
}

void ConfigurationPanel::Changeing(wxAuiNotebookEvent& event)
{
	int sel = event.GetSelection();
	if(sel == 0)
	{
		int sel = 0;
		wxArrayString array_serials;
		CEnumerateSerial::CPortAndNamesArray ports;
		CEnumerateSerial::UsingSetupAPI1(ports);

		for(auto& i : ports)
		{
			array_serials.Add(wxString::Format("COM%d (%s)", i.first, i.second));
			if(CustomMacro::Get()->com_port == i.first)
				sel = array_serials.GetCount() - 1;
		}

		comtcp_panel->m_IsCom->SetValue(CustomMacro::Get()->is_enabled);

		comtcp_panel->m_serial->Clear();
		comtcp_panel->m_serial->Insert(array_serials, WXSIZEOF(array_serials));
		comtcp_panel->m_serial->SetSelection(sel);
		comtcp_panel->m_IsTcp->SetValue(Server::Get()->is_enabled);
		comtcp_panel->m_TcpPortSpin->SetValue(Server::Get()->tcp_port);
		comtcp_panel->m_IsMinimizeOnExit->SetValue(Settings::Get()->minimize_on_exit);
		comtcp_panel->m_IsMinimizeOnStartup->SetValue(Settings::Get()->minimize_on_startup);
		comtcp_panel->m_DefaultPage->SetValue(Settings::Get()->default_page);
	}
}

ConfigurationPanel::ConfigurationPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxSize client_size = GetClientSize();

	wxAuiNotebook* m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(WINDOW_SIZE_X - 50, WINDOW_SIZE_Y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	comtcp_panel = new ComTcpPanel(m_notebook);
	keybrd_panel = new KeybrdPanel(m_notebook);
	m_notebook->AddPage(comtcp_panel, "Main settings", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->AddPage(keybrd_panel, "Macro settings", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->SetSelection(1);
	m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(ConfigurationPanel::Changeing), NULL, this);
}

ComTcpPanel::ComTcpPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxArrayString array_serials;
	array_serials.Add("Select a serial port from list below");
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_IsTcp = new wxCheckBox(this, wxID_ANY, wxT("Enable TCP?"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_IsTcp, 0, wxALL, 5);

	bSizer1->Add(new wxStaticText(this, wxID_ANY, wxT("TCP port:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	m_TcpPortSpin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, 0);
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
	
	m_Ok = new wxButton(this, wxID_ANY, "Save", wxDefaultPosition, wxDefaultSize);
	bSizer1->Add(m_Ok, 0, wxALL, 5);
	m_Ok->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			wxString com_str = m_serial->GetStringSelection();
			CustomMacro::Get()->is_enabled = m_IsCom->IsChecked();
			if(m_serial->GetSelection() > 0)
				CustomMacro::Get()->com_port = atoi(&com_str.c_str().AsChar()[2]);
			Server::Get()->is_enabled = m_IsTcp->IsChecked();
			Server::Get()->tcp_port = m_TcpPortSpin->GetValue();
			Settings::Get()->minimize_on_exit = m_IsMinimizeOnExit->IsChecked();
			Settings::Get()->minimize_on_startup = m_IsMinimizeOnStartup->GetValue();
			Settings::Get()->default_page = m_DefaultPage->GetValue();

		});
	SetSizer(bSizer1);
}

void KeybrdPanel::UpdateMainTree()
{
	tree->DeleteAllItems();
	wxTreeListItem root = tree->GetRootItem();
	for(auto& m : CustomMacro::Get()->GetMacros())
	{
		wxTreeListItem item = tree->AppendItem(root, m->name.c_str());
		for(auto& b : m->bind_name)
		{
			DBG("bind name: %s\n", b.second.c_str());
			wxTreeListItem bind_item = tree->AppendItem(item, b.second.c_str());
			tree->SetItemText(bind_item, 1, b.first.c_str());
		}
		tree->Expand(item);
	}
}

void KeybrdPanel::UpdateDetailsTree()  // TODO: this is waste of resources, save or pass iterator instead of looping
{
	tree_details->DeleteAllItems();
	wxTreeListItem root2 = tree_details->GetRootItem();
	uint16_t cnt = 0;
	std::unique_ptr<MacroContainer> ptr;
	auto& m = CustomMacro::Get()->GetMacros();
	for(auto& i : m)
	{
		if(i->name == root_sel_str.ToStdString())
		{
			for(auto& x : i->key_vec[child_sel_str.ToStdString()])
			{
				KeyClass* p = x.get();
				std::string key;
				CustomMacro::Get()->GenerateReadableTextFromMap(x, false,
					[this, &key, &root2, &cnt](std::string& macro_str, std::string* macro_typename) mutable  -> void
					{
						wxTreeListItem item;
						item = tree_details->AppendItem(root2, *macro_typename, -1, -1, new wxIntClientData(cnt++));
						tree_details->SetItemText(item, 1, macro_str);
						key += macro_str;
					});
			}
			break;
		}
	}
}

void KeybrdPanel::OnTreeListChanged_Main(wxTreeListEvent& evt)
{
	wxTreeListItem item = evt.GetItem();
	wxTreeListItem root = tree->GetItemParent(item);
	if(root)
	{
		child_sel_str = tree->GetItemText(item, 1);
		root_sel_str = tree->GetItemText(root, 0);
		DBG("sel: %d, child: %s - root: %s\n", evt.GetSelection(), child_sel_str.c_str().AsChar(), root_sel_str.c_str().AsChar());

		UpdateDetailsTree();
	}
}

void KeybrdPanel::OnTreeListChanged_Details(wxTreeListEvent& evt)
{

}

void KeybrdPanel::OnItemContextMenu_Main(wxTreeListEvent& event)
{
	enum
	{
		Id_AddNew,
		Id_Delete,
		Id_MoveUp,
		Id_MoveDown,
	};

	auto a = wxWindow::FindFocus();
	bool b = wxWindow::HasFocus();

	wxMenu menu;
	menu.Append(Id_AddNew, "&Add new (below)");
	menu.Append(Id_Delete, "&Delete");
	//menu.AppendSeparator();
	//menu.Append(Id_MoveUp, "Move up");
	//menu.Append(Id_MoveDown, "Move down");

	const wxTreeListItem item = event.GetItem();
	int ret = tree->GetPopupMenuSelectionFromUser(menu);
	switch(ret)
	{
		case 0:
		{
			wxTreeListItem root = tree->GetItemParent(item);
			wxTreeListItem root2 = tree->GetItemParent(root);
			wxString root_str = tree->GetItemText(item, 0);
			wxString parent_str = tree->GetItemText(root, 0);
			wxString item_str = tree->GetItemText(item, 1);

			if(root2 == NULL) /* clicked on one of the root items */
			{
				wxTextEntryDialog d(this, "Type new application macro name", "Caption");
				int ret = d.ShowModal();
				if(ret == 5100)
				{
					auto& m = CustomMacro::Get()->GetMacros();

					// std::vector<std::unique_ptr<MacroContainer>>

					std::vector<std::unique_ptr<MacroContainer>>::const_iterator i;
					for(i = m.begin(); i != m.end(); i++)
					{
						if(i->get()->name == root_str)
						{
							break;
						}
					}
					m.insert(i + 1, std::make_unique<MacroContainer>(d.GetValue().ToStdString()));
				}
				DBG("ret: %d\n", ret);  // 5100 - ok, 5101 - cancel
				UpdateMainTree();
			}
			else
			{
				add_dlg->ShowDialog("Add new macro", "name");

				std::string macro_key = add_dlg->GetMacroKey().ToStdString();
				std::string macro_name = add_dlg->GetMacroName().ToStdString();
				if(!add_dlg->IsApplyClicked()) return;

				if(CustomMacro::Get()->GetKeyScanCode(macro_key) == 0xFFFF)
					DBG("err");
				auto& m = CustomMacro::Get()->GetMacros();
				
				for(auto& i : m)
				{
					if(i->name == parent_str)
					{
						if(i->key_vec.find(macro_key) != i->key_vec.end())
							DBG("err");

						auto pp = std::make_unique<MacroContainer>();
						i->bind_name[macro_key] = std::move(macro_name);
						UpdateMainTree();
						break;
					}
				}

			}
			break;
		}
		case 1:
		{
			wxMessageDialog dialog(this, "Are you sure want to delete selected macro?", "Deleting macro", wxCENTER | wxNO_DEFAULT | wxYES_NO | wxICON_INFORMATION);
			int ret = dialog.ShowModal();
			switch(ret)
			{
				case 5103:  /* Yes */
				{
					auto& m = CustomMacro::Get()->GetMacros();
					wxTreeListItem root = tree->GetItemParent(item);
					wxTreeListItem root2 = tree->GetItemParent(root);
					wxString root_str = tree->GetItemText(item, 0);

					if(root2 == NULL) /* clicked on one of the root items */
					{
						m.erase(std::remove_if(m.begin(), m.end(), [&root_str](std::unique_ptr<MacroContainer>& x) { return x->name == root_str; }));
					}
					else
					{
						wxString root_str = tree->GetItemText(root, 0);
						wxString key_str = tree->GetItemText(item, 1);
						for(auto& i : m)
						{
							if(i->name == root_str)
							{
								std::string str = key_str.ToStdString();
								i->key_vec.erase(str);
								i->bind_name.erase(str);
								break;
							}
						}
					}
					UpdateMainTree();
					break;
				}
				case 5104:  /* No */
				{
					break;
				}
			}
			break;
		}
	}
}

void KeybrdPanel::OnItemContextMenu_Details(wxTreeListEvent& event)
{
	DBG("context detail");
}

void KeybrdPanel::ShowAddDialog()
{
	edit_dlg->ShowDialog("Write macro text here", 0);
}

void KeybrdPanel::ShowEditDialog(wxTreeListItem item)
{
	wxTreeListItem root = tree_details->GetItemParent(item);
	if(root)
	{
		wxString str = tree_details->GetItemText(item, 1);
		wxString str2 = tree_details->GetItemText(root, 0);
		DBG("sel: %s - root: %s\n", str.ToStdString().c_str(), str2.ToStdString().c_str());

		DBG("main tree sel: child: %s - root: %s\n", child_sel_str.c_str().AsChar(), root_sel_str.c_str().AsChar());
		wxTreeListItem root2 = tree_details->GetRootItem();

		wxString type_str = tree_details->GetItemText(item, 0);
		wxString item_str = tree_details->GetItemText(item, 1);

		uint8_t sel = 0;  /* default: SEQUENCE */
		if(type_str == "TEXT")
			sel = 1;
		else if(type_str == "DELAY RANDOM" || type_str == "DELAY")
			sel = 2;
		else if(type_str == "MOUSE MOVE")
			sel = 3;
		else if(type_str == "MOUSE CLICK")
			sel = 4;
		edit_dlg->ShowDialog(item_str, sel);
	}
}

void KeybrdPanel::DuplicateMacro(std::vector<std::unique_ptr<KeyClass>>& x, uint16_t id)
{
	KeyClass* p = x[id].get();
	//std::unique_ptr<KeyClass> = std::make_unique(&p);

	//KeyClass* p2 = new KeyClass(p);

	KeyClass* c(p->Clone());
	x.insert(x.begin() + ++id, std::unique_ptr<KeyClass>(c));
}

void KeybrdPanel::ManipulateMacro(std::vector<std::unique_ptr<KeyClass>>& x, uint16_t id, bool add)
{
	//KeyClass* p = x.at(id).get();
	DBG("aa ");

	uint8_t edit_sel = edit_dlg->GetType();
	std::string edit_str = edit_dlg->GetText().ToStdString();

	switch(edit_sel)
	{
		case 0:
		{
			std::vector<uint16_t> keys;
			boost::char_separator<char> sep("+");
			boost::tokenizer< boost::char_separator<char> > tok(edit_str, sep);
			for(boost::tokenizer< boost::char_separator<char> >::iterator beg = tok.begin(); beg != tok.end(); ++beg)
			{
				DBG("Token: %s\n", beg->c_str());
				std::string key_code = *beg;

				uint16_t key = CustomMacro::Get()->GetKeyScanCode(key_code);
				if(key == 0xFFFF)
				{
					LOGMSG(error, "Invalid key found in settings.ini: {}", key_code);
				}
				keys.push_back(key);
			}

			auto tmp = std::make_unique<KeyCombination>(std::move(keys));
			if(id >= x.size())
				x.push_back(std::move(tmp));
			else
				if(add)
					x.insert(x.begin() + ++id, std::move(tmp));
				else
					x[id] = std::move(tmp);
			break;
		}
		case 1:
		{
			auto tmp = std::make_unique<KeyText>(std::move(edit_str));
			if(id >= x.size())
				x.push_back(std::move(tmp));
			else
				if(add)
					x.insert(x.begin() + ++id, std::move(tmp));
				else
					x[id] = std::move(tmp);
			break;
		}
		case 2:
		{
			size_t separator_pos = edit_str.find("-");
			if(separator_pos != std::string::npos)
			{
				uint32_t min_delay = static_cast<uint32_t>(std::stoi(edit_str));
				uint32_t max_delay = static_cast<uint32_t>(std::stoi(&edit_str[separator_pos + 1]));

				x[id] = std::make_unique<KeyDelay>(min_delay, max_delay);
			}
			else
				x[id] = std::make_unique<KeyDelay>(boost::lexical_cast<uint32_t>(edit_str));
			break;
		}
		case 3:
		{
			POINT pos;
			if(sscanf(edit_str.c_str(), "%ld%*c%ld", &pos.x, &pos.y) == 2)
				x[id] = std::make_unique<MouseMovement>((LPPOINT*)&pos);
			break;
		}
		case 4:
		{
			uint16_t mouse_button = 0xFFFF;
			if(edit_str == "L" || edit_str == "LEFT")
				mouse_button = MOUSEEVENTF_LEFTDOWN;
			if(edit_str == "R" || edit_str == "RIGHT")
				mouse_button = MOUSEEVENTF_RIGHTDOWN;
			if(edit_str == "M" || edit_str == "MIDDLE")
				mouse_button = MOUSEEVENTF_MIDDLEDOWN;
			if(mouse_button != 0xFFFF)
				x[id] = std::make_unique<MouseClick>(mouse_button);
			break;
		}
	}
}

void KeybrdPanel::OnItemActivated(wxTreeListEvent& evt)
{
	wxTreeListItem item = evt.GetItem();

	auto& m = CustomMacro::Get()->GetMacros();
	for(auto& i : m)
	{
		if(i->name == root_sel_str.ToStdString())
		{
			std::vector<std::unique_ptr<KeyClass>>& x = i->key_vec[child_sel_str.ToStdString()];
			wxClientData* itemdata = tree_details->GetItemData(item);
			wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
			uint16_t id = dret->GetValue();

			ShowEditDialog(item);
			if(!edit_dlg->IsApplyClicked()) return;

			ManipulateMacro(x, id, false);
			UpdateDetailsTree();
			break;
		}
	}
}

void MacroEditBoxDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
	DBG("apply");
	Close();
	m_IsApplyClicked = true;
}

void MacroAddBoxDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
	DBG("apply");
	Close();
	m_IsApplyClicked = true;
}

KeybrdPanel::KeybrdPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	// Use wxTreeList as base
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxHORIZONTAL);

	tree = new wxTreeListCtrl(this, ID_AppBindListMain, wxDefaultPosition, wxSize(300, 400), wxTL_DEFAULT_STYLE);
	tree->AppendColumn("App name", tree->WidthFor("App nameApp nameApp name"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree->AppendColumn("Key bindings", tree->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	UpdateMainTree();

	bSizer1->Add(tree, wxSizerFlags(2).Left());
	
	tree_details = new wxTreeListCtrl(this, ID_MacroDetails, wxDefaultPosition, wxSize(300, 400), wxTL_DEFAULT_STYLE);
	tree_details->AppendColumn("Action type", tree_details->WidthFor("App nameApp nameApp name"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree_details->AppendColumn("Parameters", tree_details->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	bSizer1->Add(tree_details, wxSizerFlags(2).Top());

	wxBoxSizer* vertical_sizer = new wxBoxSizer(wxVERTICAL);
	btn_add = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_add->SetToolTip("Add new macro below selected one");
	btn_add->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			wxTreeListItem item = tree_details->GetSelection();

			auto& m = CustomMacro::Get()->GetMacros();
			for(auto& i : m)
			{
				if(i->name == root_sel_str.ToStdString())
				{
					std::vector<std::unique_ptr<KeyClass>>& x = i->key_vec[child_sel_str.ToStdString()];
					uint16_t id = std::numeric_limits<uint16_t>::max();
					bool push_back = true;
					if(item)  // if selected item exists and not root element
					{
						wxClientData* itemdata = tree_details->GetItemData(item);
						wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
						id = dret->GetValue();
						push_back = false;
					}
					
					ShowAddDialog();
					if(!edit_dlg->IsApplyClicked()) return;

					ManipulateMacro(x, id, true);
					UpdateDetailsTree();
					break;
				}
			}
		});

	btn_copy = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_COPY, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_copy->SetToolTip("Copy selected macro below selected one");
	btn_copy->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			wxTreeListItem item = tree_details->GetSelection();
			if(item == NULL) return;

			auto& m = CustomMacro::Get()->GetMacros();
			for(auto& i : m)
			{
				if(i->name == root_sel_str.ToStdString())
				{
					std::vector<std::unique_ptr<KeyClass>>& x = i->key_vec[child_sel_str.ToStdString()];
					wxClientData* itemdata = tree_details->GetItemData(item);
					wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
					uint16_t id = dret->GetValue();

					DuplicateMacro(x, id);
					UpdateDetailsTree();
					break;
				}
			}

		});

	btn_delete = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_DEL_BOOKMARK, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_delete->SetToolTip("Delete selected macro");
	btn_delete->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			wxTreeListItem item = tree_details->GetSelection();
			if(item.GetID() == NULL) return;

			auto& m = CustomMacro::Get()->GetMacros();
			for(auto& i : m)
			{
				if(i->name == root_sel_str.ToStdString())
				{
					std::vector<std::unique_ptr<KeyClass>>& x = i->key_vec[child_sel_str.ToStdString()];

					wxClientData* itemdata = tree_details->GetItemData(item);
					wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
					uint16_t id = dret->GetValue();
					x.erase(x.begin() + id);
				}
			}
			UpdateDetailsTree();
		});

	btn_up = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_GO_UP, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_up->SetToolTip("Move up selected macro");
	btn_up->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			wxTreeListItem item = tree_details->GetSelection();
			if(item.GetID() == NULL) return;

			auto& m = CustomMacro::Get()->GetMacros();
			for(auto& i : m)
			{
				if(i->name == root_sel_str.ToStdString())
				{
					std::vector<std::unique_ptr<KeyClass>>& x = i->key_vec[child_sel_str.ToStdString()];

					wxClientData* itemdata = tree_details->GetItemData(item);
					wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
					uint16_t id = dret->GetValue();
					
					if(x[id] == x.front())
						std::rotate(x.begin(), x.begin() + 1, x.end());
					else
						std::swap(x[id], x[id - 1]);
				}
			}
			UpdateDetailsTree();
		});
	
	btn_down = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_down->SetToolTip("Move down selected macro");
	btn_down->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			wxTreeListItem item = tree_details->GetSelection();
			if(item.GetID() == NULL) return;

			auto& m = CustomMacro::Get()->GetMacros();
			for(auto& i : m)
			{
				if(i->name == root_sel_str.ToStdString())
				{
					std::vector<std::unique_ptr<KeyClass>>& x = i->key_vec[child_sel_str.ToStdString()];

					wxClientData* itemdata = tree_details->GetItemData(item);
					wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
					uint16_t id = dret->GetValue();

					if(x[id] == x.back())
						std::rotate(x.rbegin(), x.rbegin() + 1, x.rend());
					else
						std::swap(x[id], x[id + 1]);
				}
			}
			UpdateDetailsTree();
		});

	vertical_sizer->Add(btn_add);
	vertical_sizer->Add(btn_copy);
	vertical_sizer->Add(btn_delete);
	vertical_sizer->Add(btn_up);
	vertical_sizer->Add(btn_down);
	bSizer1->Add(vertical_sizer);

	edit_dlg = new MacroEditBoxDialog(this);
	add_dlg = new MacroAddBoxDialog(this);
	SetSizer(bSizer1);
	Show();
}

