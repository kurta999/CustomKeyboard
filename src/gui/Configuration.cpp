#include "pch.h"

wxBEGIN_EVENT_TABLE(MacroRecordBoxDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, MacroRecordBoxDialog::OnApply)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MacroEditBoxDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, MacroEditBoxDialog::OnApply)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MacroAddBoxDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, MacroAddBoxDialog::OnApply)
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

MacroRecordBoxDialog::MacroRecordBoxDialog(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, "Message Box Test Dialog", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);

	wxString m_radioBox1Choices[] = { wxT("Keys only"), wxT("Mouse only"), wxT("Mouse and Keys") };
	m_RecordType = new wxRadioBox(this, wxID_ANY, wxT("Select recording type"), wxDefaultPosition, wxDefaultSize, WXSIZEOF(m_radioBox1Choices), m_radioBox1Choices, 1, wxRA_SPECIFY_COLS);
	m_RecordType->SetSelection(0);
	sizerTop->Add(m_RecordType, wxSizerFlags(1).Expand().Border());

	// miscellaneous other stuff
	wxSizer* const sizerFlags = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Note:");
	sizerFlags->Add(new wxStaticText(this, wxID_ANY, "Delay recording is not implemented between key events"), wxSizerFlags().Border());
	sizerTop->Add(sizerFlags, wxSizerFlags().Expand().Border());

	// finally buttons to show the resulting message box and close this dialog
	sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCLOSE), wxSizerFlags().Right().Border()); /* wxOK */

	SetSizerAndFit(sizerTop);
	CentreOnScreen();
}

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
	sizerFlags->Add(new wxStaticText(this, wxID_ANY, "TODO: add something here"), wxSizerFlags().Border());
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
	wxArrayString a;
	auto& map = CustomMacro::Get()->GetHidScanCodeMap();
	for(auto& i : map)
	{
		a.Add(i.first);
	}
	m_textMsg->AutoComplete(a);
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

void MacroRecordBoxDialog::ShowDialog(const wxString& str)
{
	m_RecordType->SetSelection(0);
	m_IsApplyClicked = false;
	ShowModal();
	DBG("isapply: %d", IsApplyClicked());
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
		comtcp_panel->Update();
	}
}

ConfigurationPanel::ConfigurationPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxSize client_size = GetClientSize();

	wxAuiNotebook* m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(Settings::Get()->window_size.x - 50, Settings::Get()->window_size.y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	comtcp_panel = new ComTcpPanel(m_notebook);
	keybrd_panel = new KeybrdPanel(m_notebook);
	backup_panel = new BackupPanel(m_notebook);
	m_notebook->AddPage(comtcp_panel, "Main settings", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->AddPage(keybrd_panel, "Macro settings", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->AddPage(backup_panel, "Backup settings", false, wxArtProvider::GetBitmap(wxART_HARDDISK, wxART_OTHER, FromDIP(wxSize(16, 16))));
#ifdef DEBUG
	m_notebook->SetSelection(1);
#endif
	m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(ConfigurationPanel::Changeing), NULL, this);
}

ComTcpPanel::ComTcpPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxArrayString array_serials;
	array_serials.Add("Select a serial port from list below");
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
	wxSizer* const sizer_box_tcp = new wxStaticBoxSizer(wxHORIZONTAL, this, "&TCP Settings");

	m_IsTcp = new wxCheckBox(this, wxID_ANY, wxT("Enable TCP?"), wxDefaultPosition, wxDefaultSize, 0);
	sizer_box_tcp->Add(m_IsTcp, 0, wxALL, 5);

	sizer_box_tcp->Add(new wxStaticText(this, wxID_ANY, wxT("TCP port:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	m_TcpPortSpin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, 0);
	sizer_box_tcp->Add(m_TcpPortSpin, 0, wxALL, 5);
	bSizer1->Add(sizer_box_tcp);

	wxSizer* const sizer_box_com = new wxStaticBoxSizer(wxHORIZONTAL, this, "&COM Settings");
	wxBoxSizer* com_vert_1 = new wxBoxSizer(wxVERTICAL);
	m_IsPerAppMacro = new wxCheckBox(this, wxID_ANY, wxT("Use per-application macros?"), wxDefaultPosition, wxDefaultSize, 0);
	com_vert_1->Add(m_IsPerAppMacro, 0, wxALL, 5);
	m_IsAdvancedMacro = new wxCheckBox(this, wxID_ANY, wxT("Use advanced macros?"), wxDefaultPosition, wxDefaultSize, 0);
	com_vert_1->Add(m_IsAdvancedMacro, 0, wxALL, 5);
	sizer_box_com->Add(com_vert_1, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* com_vert_2 = new wxBoxSizer(wxVERTICAL);
	m_IsCom = new wxCheckBox(this, wxID_ANY, wxT("Enable COM?"), wxDefaultPosition, wxDefaultSize, 0);
	com_vert_2->Add(m_IsCom, 0, wxEXPAND | wxALL, 5);

	com_vert_2->Add(new wxStaticText(this, wxID_ANY, wxT("COM port:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	m_serial = new wxComboBox(this, wxID_ANY, "Select COM port", wxDefaultPosition, wxDefaultSize, array_serials);
	com_vert_2->Add(m_serial);
	sizer_box_com->Add(com_vert_2);
	bSizer1->Add(sizer_box_com);

	wxSizer* const sizer_box_app = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Application settings");
	m_IsMinimizeOnExit = new wxCheckBox(this, wxID_ANY, wxT("Minimize on exit?"), wxDefaultPosition, wxDefaultSize, 0);
	sizer_box_app->Add(m_IsMinimizeOnExit, 0, wxALL, 5);

	m_IsMinimizeOnStartup = new wxCheckBox(this, wxID_ANY, wxT("Minimize on startup?"), wxDefaultPosition, wxDefaultSize, 0);
	sizer_box_app->Add(m_IsMinimizeOnStartup, 0, wxALL, 5);

	m_RememberWindowSize = new wxCheckBox(this, wxID_ANY, wxT("Remember window size?"), wxDefaultPosition, wxDefaultSize, 0);
	sizer_box_app->Add(m_RememberWindowSize, 0, wxALL, 5);

	sizer_box_app->Add(new wxStaticText(this, wxID_ANY, wxT("Default page:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

	m_DefaultPage = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0);
	sizer_box_app->Add(m_DefaultPage, 0, wxALL, 5);
	bSizer1->Add(sizer_box_app);

	wxSizer* const sizer_box_screenshot = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Screenshot Settings");
	sizer_box_screenshot->Add(new wxStaticText(this, wxID_ANY, wxT("Key:"), wxDefaultPosition, wxDefaultSize, 0));
	m_ScreenshotKey = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	sizer_box_screenshot->Add(m_ScreenshotKey);

	sizer_box_screenshot->Add(new wxStaticText(this, wxID_ANY, wxT("Time format:"), wxDefaultPosition, wxDefaultSize, 0));
	m_ScreenshotDateFmt = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	sizer_box_screenshot->Add(m_ScreenshotDateFmt);

	sizer_box_screenshot->Add(new wxStaticText(this, wxID_ANY, wxT("Path:"), wxDefaultPosition, wxDefaultSize, 0));
	m_ScreenshotPath = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	sizer_box_screenshot->Add(m_ScreenshotPath);
	bSizer1->Add(sizer_box_screenshot);

	wxSizer* const sizer_box_backup = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Backup Settings");
	sizer_box_backup->Add(new wxStaticText(this, wxID_ANY, wxT("Time format::"), wxDefaultPosition, wxDefaultSize, 0));
	m_BackupDateFmt = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	sizer_box_backup->Add(m_BackupDateFmt);
	bSizer1->Add(sizer_box_backup);

	wxSizer* const sizer_box_symlink = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Sylink creator settings");
	m_IsSymlink = new wxCheckBox(this, wxID_ANY, wxT("Enable?"), wxDefaultPosition, wxDefaultSize, 0);
	sizer_box_symlink->Add(m_IsSymlink);

	sizer_box_symlink->Add(new wxStaticText(this, wxID_ANY, wxT("Mark key:"), wxDefaultPosition, wxDefaultSize, 0));
	m_MarkSymlink = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	sizer_box_symlink->Add(m_MarkSymlink);
	sizer_box_symlink->Add(new wxStaticText(this, wxID_ANY, wxT("Symlink key:"), wxDefaultPosition, wxDefaultSize, 0));
	m_CreateSymlink = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	sizer_box_symlink->Add(m_CreateSymlink);
	sizer_box_symlink->Add(new wxStaticText(this, wxID_ANY, wxT("Hardlink key:"), wxDefaultPosition, wxDefaultSize, 0));
	m_CreateHardlink = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	sizer_box_symlink->Add(m_CreateHardlink);
	bSizer1->Add(sizer_box_symlink);

	UpdatePanel(); /* update panel for first time, because changeing isn't happen when user clicks to aui and it shows first panel */

	m_Ok = new wxButton(this, wxID_ANY, "Save", wxDefaultPosition, wxDefaultSize);
	m_Ok->SetToolTip("Save all settings to settings.ini file");
	bSizer1->Add(m_Ok, 0, wxALL, 5);
	m_Ok->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			CustomMacro::Get()->use_per_app_macro = m_IsPerAppMacro->IsChecked();
			CustomMacro::Get()->advanced_key_binding = m_IsAdvancedMacro->IsChecked();
			wxString com_str = m_serial->GetStringSelection();
			CustomMacro::Get()->is_enabled = m_IsCom->IsChecked();
			if(m_serial->GetSelection() > 0)
				CustomMacro::Get()->com_port = atoi(&com_str.c_str().AsChar()[2]);
			Server::Get()->is_enabled = m_IsTcp->IsChecked();
			Server::Get()->tcp_port = m_TcpPortSpin->GetValue();
			Settings::Get()->minimize_on_exit = m_IsMinimizeOnExit->IsChecked();
			Settings::Get()->minimize_on_startup = m_IsMinimizeOnStartup->GetValue();
			Settings::Get()->remember_window_size = m_RememberWindowSize->GetValue();
			Settings::Get()->default_page = m_DefaultPage->GetValue();
			PrintScreenSaver::Get()->screenshot_key = m_ScreenshotKey->GetValue(); /* in case of invalid key screenshot saving never will be triggered - nothing special.. using it for disabling */
			PrintScreenSaver::Get()->timestamp_format = m_ScreenshotDateFmt->GetValue();
			PrintScreenSaver::Get()->screenshot_path = m_ScreenshotPath->GetValue().ToStdString();
			DirectoryBackup::Get()->backup_time_format = m_BackupDateFmt->GetValue().ToStdString();
			SymlinkCreator::Get()->is_enabled = m_IsSymlink->IsChecked();
			SymlinkCreator::Get()->mark_key = m_MarkSymlink->GetValue().ToStdString();
			SymlinkCreator::Get()->place_symlink_key = m_CreateSymlink->GetValue().ToStdString();
			SymlinkCreator::Get()->place_hardlink_key = m_CreateHardlink->GetValue().ToStdString();

			Settings::Get()->SaveFile(false);
		});
	SetSizer(bSizer1);
}

void ComTcpPanel::UpdatePanel()
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

	m_IsPerAppMacro->SetValue(CustomMacro::Get()->use_per_app_macro);
	m_IsAdvancedMacro->SetValue(CustomMacro::Get()->advanced_key_binding);
	m_IsCom->SetValue(CustomMacro::Get()->is_enabled);

	m_serial->Clear();
	m_serial->Insert(array_serials, WXSIZEOF(array_serials));
	m_serial->SetSelection(sel);
	m_IsTcp->SetValue(Server::Get()->is_enabled);
	m_TcpPortSpin->SetValue(Server::Get()->tcp_port);
	m_IsMinimizeOnExit->SetValue(Settings::Get()->minimize_on_exit);
	m_IsMinimizeOnStartup->SetValue(Settings::Get()->minimize_on_startup);
	m_RememberWindowSize->SetValue(Settings::Get()->remember_window_size);
	m_DefaultPage->SetValue(Settings::Get()->default_page);
	m_ScreenshotKey->SetValue(PrintScreenSaver::Get()->screenshot_key);
	m_ScreenshotDateFmt->SetValue(PrintScreenSaver::Get()->timestamp_format);
	m_ScreenshotPath->SetValue(PrintScreenSaver::Get()->screenshot_path.generic_u8string());
	m_BackupDateFmt->SetValue(DirectoryBackup::Get()->backup_time_format);
	m_IsSymlink->SetValue(SymlinkCreator::Get()->is_enabled);
	m_MarkSymlink->SetValue(SymlinkCreator::Get()->mark_key);
	m_CreateSymlink->SetValue(SymlinkCreator::Get()->place_symlink_key);
	m_CreateHardlink->SetValue(SymlinkCreator::Get()->place_hardlink_key);
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
			//DBG("bind name: %s\n", b.second.c_str());
			wxTreeListItem bind_item = tree->AppendItem(item, b.second.c_str());
			tree->SetItemText(bind_item, 1, b.first.c_str());
		}
		tree->Expand(item);
	}
}

void KeybrdPanel::UpdateDetailsTree(std::unique_ptr<KeyClass>* ptr)
{
	tree_details->DeleteAllItems();
	wxTreeListItem root2 = tree_details->GetRootItem();
	uint16_t cnt = 0;
	auto& m = CustomMacro::Get()->GetMacros();
	for(auto& i : m)
	{
		if(i->name == root_sel_str.ToStdString())
		{
			for(auto& x : i->key_vec[child_sel_str.ToStdString()])
			{
				KeyClass* p = x.get();
				wxTreeListItem item = tree_details->AppendItem(root2, p->GetName(), -1, -1, new wxIntClientData(cnt++));
				std::string str = p->GenerateText(false);
				tree_details->SetItemText(item, 1, str);
				if(ptr && x == *ptr)
					tree_details->Select(item);
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

void KeybrdPanel::OnItemContextMenu_Main(wxTreeListEvent& evt)
{
	enum
	{
		Id_AddNewApplication,
		Id_AddNewMacroKey,
		Id_Delete,
	};

	wxMenu menu;
	menu.Append(Id_AddNewApplication, "&Add new application (below)");
	menu.Append(Id_AddNewMacroKey, "&Add new macro key (below)");
	menu.Append(Id_Delete, "&Delete");

	const wxTreeListItem item = evt.GetItem();
	wxTreeListItem root = tree->GetItemParent(item);
	if(root == NULL) return;
	wxTreeListItem root2 = tree->GetItemParent(root);

	wxTreeListItem child = tree->GetFirstChild(item);
	menu.Enable(Id_AddNewApplication, !(child == 0));

	int ret = tree->GetPopupMenuSelectionFromUser(menu);
	switch(ret)
	{
		case Id_AddNewApplication:
		{
			wxString root_str = tree->GetItemText(item, 0);
			wxString item_str = tree->GetItemText(item, 1);

			wxTextEntryDialog d(this, "Type new application macro name", "Caption");
			int ret = d.ShowModal();
			if(ret == 5100)
			{
				auto& m = CustomMacro::Get()->GetMacros();

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
			break;
		}
		case Id_AddNewMacroKey:
		{
			wxTreeListItem root = tree->GetItemParent(item);
			wxTreeListItem root2 = tree->GetItemParent(root);
			wxString item_str = root2 ? tree->GetItemText(root, 0) : tree->GetItemText(item, 0);
			add_dlg->ShowDialog("NUM_0", "macro name");

			std::string macro_key = add_dlg->GetMacroKey().ToStdString();
			std::string macro_name = add_dlg->GetMacroName().ToStdString();
			if(!add_dlg->IsApplyClicked()) return;

			if(CustomMacro::Get()->GetKeyScanCode(macro_key) == 0xFFFF)
			{
				wxMessageDialog(this, "Invalid key!", "Error", wxOK).ShowModal();
				return;
			}

			auto& m = CustomMacro::Get()->GetMacros();
			for(auto& i : m)
			{
				if(i->name == item_str)
				{
					if(i->key_vec.find(macro_key) != i->key_vec.end()) 
					{
						wxMessageDialog(this, "Given key already has a binded macro!", "Error", wxOK).ShowModal();
						return;
					}

					auto pp = std::make_unique<MacroContainer>();
					i->bind_name[macro_key] = std::move(macro_name);
					UpdateMainTree();
					break;
				}
			}
			break;
		}
		case Id_Delete:
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
			}
			break;
		}
	}
}

void KeybrdPanel::OnItemContextMenu_Details(wxTreeListEvent& evt)
{
	enum
	{
		Id_AddNew,
		Id_Clone,
		Id_Delete,
		Id_MoveUp,
		Id_MoveDown,
	};

	wxMenu menu;
	menu.Append(Id_AddNew, "&Add new macro (below)");
	menu.Append(Id_Clone, "&Clone (below)");
	menu.Append(Id_Delete, "&Delete macro(s)");
	menu.Append(Id_MoveUp, "&Move up");
	menu.Append(Id_MoveDown, "&Move down");

	const wxTreeListItem item = evt.GetItem();
	wxTreeListItem root = tree->GetItemParent(item);
	if(root == NULL) return;
	int ret = tree->GetPopupMenuSelectionFromUser(menu);
	switch(ret)
	{
		case Id_AddNew:
		{
			TreeDetails_AddNewMacro();
			break;
		}
		case Id_Clone:
		{
			TreeDetails_CloneMacro();
			break;
		}
		case Id_Delete:
		{
			TreeDetails_DeleteSelectedMacros();
			break;
		}
		case Id_MoveUp:
		{
			TreeDetails_MoveUpSelectedMacro();
			break;
		}
		case Id_MoveDown:
		{
			TreeDetails_MoveDownSelectedMacro();
			break;
		}
	}
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
	KeyClass* p = x[id].get(); /* TODO: use smart pointers here! */
	KeyClass* c(p->Clone());
	x.insert(x.begin() + ++id, std::unique_ptr<KeyClass>(c));
}

template <typename T> void AddOrModifyMacro(std::vector<std::unique_ptr<KeyClass>>& x, uint16_t id, bool add, std::string&& source_string)
{
	auto tmp = std::make_unique<T>(std::move(source_string));
	if(id >= x.size())
		x.push_back(std::move(tmp));
	else
		if(add)
			x.insert(x.begin() + ++id, std::move(tmp));
		else
			x[id] = std::move(tmp);
}

void KeybrdPanel::ManipulateMacro(std::vector<std::unique_ptr<KeyClass>>& x, uint16_t id, bool add)
{
	uint8_t edit_sel = edit_dlg->GetType();
	std::string edit_str = edit_dlg->GetText().ToStdString();

	switch(edit_sel)
	{
		case 0:
		{
			AddOrModifyMacro<KeyCombination>(x, id, add, std::move(edit_str));
			break;
		}
		case 1:
		{
			AddOrModifyMacro<KeyText>(x, id, add, std::move(edit_str));
			break;
		}
		case 2:
		{
			try
			{
				AddOrModifyMacro<KeyDelay>(x, id, add, std::move(edit_str));
			}
			catch(std::exception& e)
			{
				wxMessageDialog(this, fmt::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}
		case 3:
		{
			try
			{
				AddOrModifyMacro<MouseMovement>(x, id, add, std::move(edit_str));
			}
			catch(std::exception& e)
			{
				wxMessageDialog(this, fmt::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}
		case 4:
		{
			try
			{
				AddOrModifyMacro<MouseClick>(x, id, add, std::move(edit_str));
			}
			catch(std::exception& e)
			{
				wxMessageDialog(this, fmt::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
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

void MacroRecordBoxDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
	Close();
	m_IsApplyClicked = true;
}

void MacroEditBoxDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
	Close();
	m_IsApplyClicked = true;
}

void MacroAddBoxDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
	Close();
	m_IsApplyClicked = true;
}

void KeybrdPanel::OnKeyDown(wxKeyEvent& evt)
{
	if(evt.ControlDown())
	{
		switch(evt.GetKeyCode())
		{
			case 'A':
			{
				wxWindow* focus = wxWindow::FindFocus();
				if(focus == tree->GetView())
					tree->SelectAll();
				if(focus == tree_details->GetView())
					tree_details->SelectAll();
				break;
			}
			case WXK_UP:
			{
				TreeDetails_MoveUpSelectedMacro();
				break;
			}
			case WXK_DOWN:
			{
				TreeDetails_MoveDownSelectedMacro();
				break;
			}
		}
	}
	else
	{
		switch(evt.GetKeyCode())
		{
			case 127: /* DELETE */
			{
				TreeDetails_DeleteSelectedMacros();
				break;
			}
			case 'A':
			{
				TreeDetails_AddNewMacro();
				break;
			}	
			case 'C':
			case 'D':
			{
				TreeDetails_CloneMacro();
				break;
			}
		}
	}
	//DBG("Key: %d\n", evt.GetKeyCode());
	evt.Skip();
}

void KeybrdPanel::TreeDetails_DeleteSelectedMacros()
{
	std::set<KeyClass*> items_to_delete;
	wxTreeListItems items;
	tree_details->GetSelections(items);
	if(!items.empty())
	{
		std::vector<int> to_erase;
		for(auto& x : items)
		{
			wxClientData* itemdata = tree_details->GetItemData(x);
			wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
			uint16_t id = dret->GetValue();

			to_erase.push_back(id);
		}

		std::sort(to_erase.begin(), to_erase.end());
		auto& m = CustomMacro::Get()->GetMacros();
		for(auto& i : m)
		{
			if(i->name == root_sel_str.ToStdString())
			{
				std::vector<std::unique_ptr<KeyClass>>& c = i->key_vec[child_sel_str.ToStdString()];
				for(auto& j : boost::adaptors::reverse(to_erase))
				{
					c.erase(c.begin() + j);
				}
				break;
			}
		}
		UpdateDetailsTree();
	}
}

void KeybrdPanel::TreeDetails_AddNewMacro()
{
	wxTreeListItems items;
	tree_details->GetSelections(items);
	if(!items.empty())
	{
		if(items.size() > 1)
		{
			wxMessageDialog(this, "Too much items selected\nSelect only one item", "Error", wxOK).ShowModal();
			return;
		}

		wxTreeListItem& item = items[0];
		auto& m = CustomMacro::Get()->GetMacros();
		for(auto& i : m)
		{
			if(i->name == root_sel_str.ToStdString())
			{
				std::vector<std::unique_ptr<KeyClass>>& x = i->key_vec[child_sel_str.ToStdString()];
				std::unique_ptr<KeyClass>* ptr = nullptr;
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
				ptr = id >= x.size() ? &x[x.size() - 1] : &x[id + 1];
				UpdateDetailsTree(ptr);
				break;
			}
		}
	}
}

void KeybrdPanel::TreeDetails_CloneMacro()
{
	wxTreeListItems items;
	tree_details->GetSelections(items);
	if(!items.empty())
	{
		if(items.size() > 1)
		{
			wxMessageDialog(this, "Too much items selected\nSelect only one item", "Error", wxOK).ShowModal();
			return;
		}

		wxTreeListItem& item = items[0];
		uint16_t id;
		std::vector<std::unique_ptr<KeyClass>>* x = GetKeyClassByItem(item, id);
		if(x)
		{
			DuplicateMacro(*x, id);
			UpdateDetailsTree();
		}
	}
}

void KeybrdPanel::TreeDetails_MoveUpSelectedMacro()
{
	wxTreeListItems items;
	tree_details->GetSelections(items);
	if(!items.empty())
	{
		if(items.size() > 1)
		{
			wxMessageDialog(this, "Too much items selected\nSelect only one item", "Error", wxOK).ShowModal();
			return;
		}

		wxTreeListItem& item = items[0];
		uint16_t id;
		std::vector<std::unique_ptr<KeyClass>>* x = GetKeyClassByItem(item, id);
		if(x)
		{
			std::unique_ptr<KeyClass>* ptr;
			if((*x)[id] == x->front())
			{
				std::rotate(x->begin(), x->begin() + 1, x->end());
				ptr = &x->back();
			}
			else
			{
				std::swap((*x)[id], (*x)[id - 1]);
				ptr = &(*x)[id - 1];
			}
			UpdateDetailsTree(ptr);
		}
	}
}

void KeybrdPanel::TreeDetails_MoveDownSelectedMacro()
{
	wxTreeListItems items;
	tree_details->GetSelections(items);
	if(!items.empty())
	{
		if(items.size() > 1)
		{
			wxMessageDialog(this, "Too much items selected\nSelect only one item", "Error", wxOK).ShowModal();
			return;
		}

		wxTreeListItem& item = items[0];
		uint16_t id;
		std::vector<std::unique_ptr<KeyClass>>* x = GetKeyClassByItem(item, id);
		if(x)
		{
			std::unique_ptr<KeyClass>* ptr;
			if((*x)[id] == x->back())
			{
				std::rotate(x->rbegin(), x->rbegin() + 1, x->rend());
				ptr = &x->front();
			}
			else
			{
				std::swap((*x)[id], (*x)[id + 1]);
				ptr = &(*x)[id + 1];
			}
			UpdateDetailsTree(ptr);
		}
	}
}

void KeybrdPanel::TreeDetails_StartRecording()
{
	if(MacroRecorder::Get()->IsRecordingMouse() || MacroRecorder::Get()->IsRecordingKeyboard())
	{
		MacroRecorder::Get()->StopRecording();
		btn_record->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, FromDIP(wxSize(24, 24))));
		btn_record->SetToolTip("Record macro (keypresses and/or mouse events)");
		Bind(wxEVT_CHAR_HOOK, &KeybrdPanel::OnKeyDown, this);

		MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
		{
			std::lock_guard lock(frame->mtx);
			frame->pending_msgs.push_back({ (uint8_t)MacroRecordingStopped });
		}
		return;
	}
	wxTreeListItems items;
	tree_details->GetSelections(items);
	if(!items.empty())
	{
		if(items.size() > 1)
		{
			wxMessageDialog(this, "Too much items selected\nSelect only one item", "Error", wxOK).ShowModal();
			return;
		}

		wxTreeListItem& item = items[0];
		auto& m = CustomMacro::Get()->GetMacros();
		for(auto& i : m)
		{
			if(i->name == root_sel_str.ToStdString())
			{
				std::vector<std::unique_ptr<KeyClass>>& x = i->key_vec[child_sel_str.ToStdString()];
				std::unique_ptr<KeyClass>* ptr = nullptr;
				uint16_t id = std::numeric_limits<uint16_t>::max();
				bool push_back = true;
				if(item)  // if selected item exists and not root element
				{
					wxClientData* itemdata = tree_details->GetItemData(item);
					wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
					id = dret->GetValue();
					push_back = false;
				}
				record_dlg->ShowDialog("a");
				if(!record_dlg->IsApplyClicked()) return;

				uint8_t selection = record_dlg->GetType();
				CustomMacro::Get()->editing_macro = &x;

				bool kbd = false, mouse = false;
				switch(selection)
				{
				case 0:
					kbd = true;
					break;
				case 1:
					mouse = true;
					break;
				case 2:
					mouse = true;
					kbd = true;
					break;
				}
				MacroRecorder::Get()->StartRecording(kbd, mouse);
				btn_record->SetBitmap(wxArtProvider::GetBitmap(wxART_REMOVABLE, wxART_OTHER, FromDIP(wxSize(24, 24))));
				btn_record->SetToolTip("Stop macro recording");
				Unbind(wxEVT_CHAR_HOOK, &KeybrdPanel::OnKeyDown, this);

				MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
				{
					std::lock_guard lock(frame->mtx);
					frame->pending_msgs.push_back({ (uint8_t)MacroRecordingStarted });
				}
			}
		}
	}
}

KeybrdPanel::KeybrdPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxHORIZONTAL);

	tree = new wxTreeListCtrl(this, ID_AppBindListMain, wxDefaultPosition, wxSize(300, 400), wxTL_SINGLE);
	tree->AppendColumn("App name", tree->WidthFor("App nameApp nameApp name"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree->AppendColumn("Key bindings", tree->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	UpdateMainTree();

	bSizer1->Add(tree, wxSizerFlags(2).Left());

	tree_details = new wxTreeListCtrl(this, ID_MacroDetails, wxDefaultPosition, wxSize(300, 400), wxTL_MULTIPLE);
	tree_details->AppendColumn("Action type", tree_details->WidthFor("App nameApp nameApp name"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree_details->AppendColumn("Parameters", tree_details->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	bSizer1->Add(tree_details, wxSizerFlags(2).Top());

	Bind(wxEVT_CHAR_HOOK, &KeybrdPanel::OnKeyDown, this);

	wxBoxSizer* vertical_sizer = new wxBoxSizer(wxVERTICAL);
	btn_add = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_add->SetToolTip("Add new macro below selected one");
	btn_add->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			TreeDetails_AddNewMacro();
		});

	btn_record = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_record->SetToolTip("Record macro (keypresses and/or mouse events)");
	btn_record->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			TreeDetails_StartRecording();
		});

	btn_copy = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_COPY, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_copy->SetToolTip("Copy selected macro below selected one");
	btn_copy->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			TreeDetails_CloneMacro();
		});

	btn_delete = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_DEL_BOOKMARK, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_delete->SetToolTip("Delete selected macro");
	btn_delete->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			TreeDetails_DeleteSelectedMacros();
		});

	btn_up = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_GO_UP, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_up->SetToolTip("Move up selected macro");
	btn_up->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			TreeDetails_MoveUpSelectedMacro();
		});

	btn_down = new wxBitmapButton(this, wxID_ANY, wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_OTHER, FromDIP(wxSize(24, 24))));
	btn_down->SetToolTip("Move down selected macro");
	btn_down->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			TreeDetails_MoveDownSelectedMacro();
		});

	m_Ok = new wxButton(this, wxID_ANY, "Save", wxPoint(tree->GetSize().x + 20, tree->GetSize().y + 20), wxDefaultSize);
	m_Ok->SetToolTip("Save all settings to settings.ini file");
	m_Ok->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			Settings::Get()->SaveFile(false);
		});

	vertical_sizer->Add(btn_add);
	vertical_sizer->Add(btn_record);
	vertical_sizer->Add(btn_copy);
	vertical_sizer->Add(btn_delete);
	vertical_sizer->Add(btn_up);
	vertical_sizer->Add(btn_down);
	bSizer1->Add(vertical_sizer);

	record_dlg = new MacroRecordBoxDialog(this);
	edit_dlg = new MacroEditBoxDialog(this);
	add_dlg = new MacroAddBoxDialog(this);
	SetSizer(bSizer1);
	Show();
}

std::vector<std::unique_ptr<KeyClass>>* KeybrdPanel::GetKeyClassByItem(wxTreeListItem item, uint16_t& id)
{
	std::vector<std::unique_ptr<KeyClass>>* ret = nullptr;
	if(item.GetID() != NULL)
	{
		auto& m = CustomMacro::Get()->GetMacros();
		for(auto& i : m)
		{
			if(i->name == root_sel_str.ToStdString())
			{
				std::vector<std::unique_ptr<KeyClass>>* x = &i->key_vec[child_sel_str.ToStdString()];

				wxClientData* itemdata = tree_details->GetItemData(item);
				wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
				id = dret->GetValue();
				ret = x;
			}
		}
	}
	return ret;
}