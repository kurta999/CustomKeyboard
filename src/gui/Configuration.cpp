#include "pch.hpp"

wxBEGIN_EVENT_TABLE(ConfigurationPanel, wxPanel)
EVT_SIZE(ConfigurationPanel::OnSize)
wxEND_EVENT_TABLE()

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
	: wxDialog(parent, wxID_ANY, "Macro recording", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);

	const wxString m_radioBox1Choices[] = { wxT("Keys only"), wxT("Mouse only"), wxT("Mouse and Keys") };
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
	: wxDialog(parent, wxID_ANY, "Macro key", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
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
	: wxDialog(parent, wxID_ANY, "Macro editor", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);

	// this sizer allows to configure the messages shown in the message box
	wxSizer* const sizerMsgs = new wxStaticBoxSizer(wxVERTICAL, this, "&Macro settings");

	const wxString choices[] = { wxT("Sequence"), wxT("Text"), wxT("Delay"), wxT("Mouse move"), 
		wxT("Mouse interpolate"), wxT("Mouse press"), wxT("Mouse release"), wxT("Mouse click"), 
		wxT("Bash terminal"), wxT("Execute command"), wxT("Execute XML command") };
	m_radioBox1 = new wxRadioBox(this, wxID_ANY, wxT("Action type"), wxDefaultPosition, wxDefaultSize, WXSIZEOF(choices), choices, 1, wxRA_SPECIFY_COLS);
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
	
	m_timer = new wxTimer(this, ID_CheckKeypress);
	Connect(m_timer->GetId(), wxEVT_TIMER, wxTimerEventHandler(MacroEditBoxDialog::OnTimer), NULL, this);
	m_timer->Start(100, false);

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

void MacroEditBoxDialog::OnTimer(wxTimerEvent& event)
{
	if(IsShown() && (GetType() == 3 || GetType() == 4)) /* is shown edit macro dialog & mouse movement or mouse interpolate radio item is selected */
	{
#ifdef _WIN32
		if(GetAsyncKeyState(VK_SCROLL) & 1)
		{
			HWND foreground = GetForegroundWindow();
			POINT p;
			if(foreground && ::GetCursorPos(&p))
			{
				if(::ScreenToClient(foreground, &p))
				{
					wxString str = wxString::Format("%d,%d", p.x, p.y);
					m_textMsg->SetValue(str);
				}
			}
		}
#else

#endif
	}
}

void ConfigurationPanel::Changeing(wxAuiNotebookEvent& event)
{
	int sel = event.GetSelection();
	if(sel == m_notebook->FindPage(comtcp_panel))
	{
		comtcp_panel->Update();
	}
}

void ConfigurationPanel::UpdateSubpanels()
{
	comtcp_panel->UpdatePanel();
	keybrd_panel->UpdateMainTree();
	keybrd_panel->UpdateDetailsTree();
	backup_panel->UpdateMainTree();
}

ConfigurationPanel::ConfigurationPanel(wxWindow* parent)
	: wxScrolled<wxPanel>(parent, wxID_ANY)
{
	wxSize client_size = GetClientSize();

	m_notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	comtcp_panel = new ComTcpPanel(m_notebook);
	keybrd_panel = new KeybrdPanel(m_notebook);
	backup_panel = new BackupPanel(m_notebook);
	m_notebook->AddPage(comtcp_panel, "Main settings", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->AddPage(keybrd_panel, "Macro settings", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->AddPage(backup_panel, "Backup settings", false, wxArtProvider::GetBitmap(wxART_HARDDISK, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(ConfigurationPanel::Changeing), NULL, this);

	Layout();
	SetMinSize(wxSize(1, 1));
	SetScrollRate(1, 1);
	FitInside();
}

void ConfigurationPanel::OnSize(wxSizeEvent& evt)
{
	FitInside();
	evt.Skip(true);
}

ComTcpPanel::ComTcpPanel(wxWindow* parent)
	: wxScrolled<wxPanel>(parent, wxID_ANY)
{
	Bind(wxEVT_PAINT, &ComTcpPanel::OnPaint, this);
	SetMinSize(wxSize(1, 1));
	SetScrollRate(1, 1);

	wxArrayString array_serials;
	array_serials.Add("Select a serial port from list below");
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "&TCP Settings");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		wxBoxSizer* tcp_horizontal_1 = new wxBoxSizer(wxHORIZONTAL);
		m_IsTcp = new wxCheckBox(this, wxID_ANY, wxT("Enable TCP?"), wxDefaultPosition, wxDefaultSize, 0);
		tcp_horizontal_1->Add(m_IsTcp, 0, wxALL, 5);
		tcp_horizontal_1->Add(new wxStaticText(this, wxID_ANY, wxT("TCP port:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_TcpPortSpin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, 0);
		tcp_horizontal_1->Add(m_TcpPortSpin, 0, wxALL, 5);

		tcp_horizontal_1->Add(new wxStaticText(this, wxID_ANY, wxT("Graph Gen interval:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_GraphGenInterval = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, 0);
		tcp_horizontal_1->Add(m_GraphGenInterval, 0, wxALL, 5);

		tcp_horizontal_1->Add(new wxStaticText(this, wxID_ANY, wxT("Graph Resolution:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_GraphResolution = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, 0);
		tcp_horizontal_1->Add(m_GraphResolution, 0, wxALL, 5);

		tcp_horizontal_1->Add(new wxStaticText(this, wxID_ANY, wxT("Graph Integration:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_GraphIntegrationTime = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, 0);
		tcp_horizontal_1->Add(m_GraphIntegrationTime, 0, wxALL, 5);


		static_box_sizer->Add(tcp_horizontal_1);

		wxBoxSizer* tcp_horizontal_2 = new wxBoxSizer(wxHORIZONTAL);
		static_box_sizer->Add(tcp_horizontal_2);

		m_IsTcpForwarder = new wxCheckBox(this, wxID_ANY, wxT("Enable TCP forwarding?"), wxDefaultPosition, wxDefaultSize, 0);
		tcp_horizontal_2->Add(m_IsTcpForwarder, 0, wxALL, 5);


		tcp_horizontal_2->Add(new wxStaticText(this, wxID_ANY, wxT("TCP forward IP:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_TcpForwarderIp = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		tcp_horizontal_2->Add(m_TcpForwarderIp, 0, wxALL, 5);

		tcp_horizontal_2->Add(new wxStaticText(this, wxID_ANY, wxT("TCP forward port:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_TcpForwarderPort = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, 0);
		tcp_horizontal_2->Add(m_TcpForwarderPort, 0, wxALL, 5);

		bSizer1->Add(static_box_sizer);
	}

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&COM Settings");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		wxBoxSizer* com_vert_2 = new wxBoxSizer(wxVERTICAL);
		m_IsCom = new wxCheckBox(this, wxID_ANY, wxT("Enable COM?"), wxDefaultPosition, wxDefaultSize, 0);
		com_vert_2->Add(m_IsCom, 0, wxEXPAND | wxALL, 5);

		com_vert_2->Add(new wxStaticText(this, wxID_ANY, wxT("COM port:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

		m_serial = new wxComboBox(this, wxID_ANY, "Select COM port", wxDefaultPosition, wxDefaultSize, array_serials);
		com_vert_2->Add(m_serial);
		static_box_sizer->Add(com_vert_2);

		wxBoxSizer* com_vert_1 = new wxBoxSizer(wxVERTICAL);
		m_IsPerAppMacro = new wxCheckBox(this, wxID_ANY, wxT("Use per-application macros?"), wxDefaultPosition, wxDefaultSize, 0);
		com_vert_1->Add(m_IsPerAppMacro, 0, wxALL, 5);
		m_IsAdvancedMacro = new wxCheckBox(this, wxID_ANY, wxT("Use advanced macros?"), wxDefaultPosition, wxDefaultSize, 0);
		com_vert_1->Add(m_IsAdvancedMacro, 0, wxALL, 5);
		static_box_sizer->Add(com_vert_1, 0, wxEXPAND | wxALL, 5);

		bSizer1->Add(static_box_sizer);
	}

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&CAN Sender Settings");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* v_sizer = new wxBoxSizer(wxHORIZONTAL);

		wxBoxSizer* com_vert_2 = new wxBoxSizer(wxVERTICAL);
		m_IsCanSerial = new wxCheckBox(this, wxID_ANY, wxT("Enable COM?"), wxDefaultPosition, wxDefaultSize, 0);
		com_vert_2->Add(m_IsCanSerial, 0, wxEXPAND | wxALL, 5);

		com_vert_2->Add(new wxStaticText(this, wxID_ANY, wxT("COM port:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

		m_CanSerial = new wxComboBox(this, wxID_ANY, "Select COM port", wxDefaultPosition, wxDefaultSize, array_serials);
		com_vert_2->Add(m_CanSerial);

		com_vert_2->Add(new wxStaticText(this, wxID_ANY, wxT("Device type:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);

		wxArrayString array_can_devices;
		array_can_devices.Add("STM32");
		array_can_devices.Add("Lawicel");
		m_CanDeviceType = new wxComboBox(this, wxID_ANY, "Select COM port", wxDefaultPosition, wxDefaultSize, array_can_devices);
		m_CanDeviceType->SetSelection(0);
		com_vert_2->Add(m_CanDeviceType);

		wxBoxSizer* com_vert_1 = new wxBoxSizer(wxVERTICAL);

		m_CanDefaultTxList = new wxTextCtrl(this, wxID_ANY, "TxList.txt", wxDefaultPosition, wxDefaultSize);
		com_vert_1->Add(m_CanDefaultTxList);
		m_CanDefaultRxList = new wxTextCtrl(this, wxID_ANY, "RxList.txt", wxDefaultPosition, wxDefaultSize);
		com_vert_1->Add(m_CanDefaultRxList);
		m_CanDefaultFrameMapping = new wxTextCtrl(this, wxID_ANY, "FrameMapping.xml", wxDefaultPosition, wxDefaultSize);
		com_vert_1->Add(m_CanDefaultFrameMapping);
		m_CanDefaultEcuId = new wxTextCtrl(this, wxID_ANY, "8DC", wxDefaultPosition, wxDefaultSize);
		com_vert_1->Add(m_CanDefaultEcuId);

		v_sizer->Add(com_vert_2);
		v_sizer->Add(com_vert_1);

		h_sizer->Add(v_sizer);

		wxBoxSizer* v_sizer_2 = new wxBoxSizer(wxVERTICAL);
		m_CanAutoSend = new wxCheckBox(this, wxID_ANY, wxT("Auto send?"), wxDefaultPosition, wxDefaultSize, 0);
		v_sizer_2->Add(m_CanAutoSend);
		m_CanAutoRecord = new wxCheckBox(this, wxID_ANY, wxT("Auto record?"), wxDefaultPosition, wxDefaultSize, 0);
		v_sizer_2->Add(m_CanAutoRecord);

		v_sizer_2->Add(new wxStaticText(this, wxID_ANY, wxT("Default record level:"), wxDefaultPosition, wxDefaultSize, 0));
		m_CanDefaultRecordLevel = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0);
		v_sizer_2->Add(m_CanDefaultRecordLevel);

		v_sizer_2->Add(new wxStaticText(this, wxID_ANY, wxT("Default favourite level:"), wxDefaultPosition, wxDefaultSize, 0));
		m_CanDefaultFavLevel = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0);
		v_sizer_2->Add(m_CanDefaultFavLevel);


		h_sizer->Add(v_sizer_2);
		static_box_sizer->Add(h_sizer, 0, wxEXPAND | wxALL, 5);

		bSizer1->Add(static_box_sizer);
	}

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Serial listener");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		m_SerialForwarderIsEnabled = new wxCheckBox(this, wxID_ANY, wxT("Enable?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_SerialForwarderIsEnabled);
		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Binded IP:"), wxDefaultPosition, wxDefaultSize, 0));
		m_SerialForwarderBindIp = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_SerialForwarderBindIp);
		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Port:"), wxDefaultPosition, wxDefaultSize, 0));
		m_SerialForwarderPort = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, 0);
		static_box_sizer->Add(m_SerialForwarderPort);

		bSizer1->Add(static_box_sizer);
	}


	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Path separator replacer settings");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Key:"), wxDefaultPosition, wxDefaultSize, 0));
		m_PathSepReplacerKey = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_PathSepReplacerKey);
		bSizer1->Add(static_box_sizer);
	}

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Application settings");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		m_IsMinimizeOnExit = new wxCheckBox(this, wxID_ANY, wxT("Minimize on exit?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_IsMinimizeOnExit, 0, wxALL, 5);

		m_IsMinimizeOnStartup = new wxCheckBox(this, wxID_ANY, wxT("Minimize on startup?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_IsMinimizeOnStartup, 0, wxALL, 5);

		m_RememberWindowSize = new wxCheckBox(this, wxID_ANY, wxT("Remember window size?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_RememberWindowSize, 0, wxALL, 5);
		m_AlwaysOnNumlock = new wxCheckBox(this, wxID_ANY, wxT("Force numlock on?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_AlwaysOnNumlock, 0, wxALL, 5);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Default page:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_DefaultPage = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0);
		static_box_sizer->Add(m_DefaultPage, 0, wxALL, 5);

		m_UsedPages = new wxButton(this, wxID_ANY, "Used pages", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_UsedPages);
		m_UsedPages->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
			{
				std::string filters = Settings::Get()->ParseUsedPagesToString(Settings::Get()->used_pages);
				boost::algorithm::replace_all(filters, ",", "\n");

				wxTextEntryDialog d(this, "Enter below application title for Used pages", "Used pages", filters, wxOK | wxCANCEL | wxTE_MULTILINE);
				int ret_code = d.ShowModal();
				if(ret_code == wxID_OK)  /* OK */
				{
					std::string result = d.GetValue().ToStdString();
					boost::algorithm::replace_all(result, "\n", ",");

					Settings::Get()->used_pages = Settings::Get()->ParseUsedPagesFromString(result);
				}
			});


		bSizer1->Add(static_box_sizer);
	}

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Screenshot Settings");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Key:"), wxDefaultPosition, wxDefaultSize, 0));
		m_ScreenshotKey = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_ScreenshotKey);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Time format:"), wxDefaultPosition, wxDefaultSize, 0));
		m_ScreenshotDateFmt = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_ScreenshotDateFmt);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Path:"), wxDefaultPosition, wxDefaultSize, 0));
		m_ScreenshotPath = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_ScreenshotPath);
		bSizer1->Add(static_box_sizer);
	}


	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Backup Settings");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Time format::"), wxDefaultPosition, wxDefaultSize, 0));
		m_BackupDateFmt = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_BackupDateFmt);
		bSizer1->Add(static_box_sizer);
	}

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Sylink creator settings");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		m_IsSymlink = new wxCheckBox(this, wxID_ANY, wxT("Enable?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_IsSymlink);
		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Mark key:"), wxDefaultPosition, wxDefaultSize, 0));
		m_MarkSymlink = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_MarkSymlink);
		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Symlink key:"), wxDefaultPosition, wxDefaultSize, 0));
		m_CreateSymlink = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_CreateSymlink);
		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Hardlink key:"), wxDefaultPosition, wxDefaultSize, 0));
		m_CreateHardlink = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_CreateHardlink);
		bSizer1->Add(static_box_sizer);
	}

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&AntiLock");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		m_IsAntiLock = new wxCheckBox(this, wxID_ANY, wxT("Enable?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_IsAntiLock);
		m_IsScreensSaverAfterLock = new wxCheckBox(this, wxID_ANY, wxT("Screensaver after move?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_IsScreensSaverAfterLock);
		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Timeout [s]:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_AntiLockTimeout = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 9999, 0);
		static_box_sizer->Add(m_AntiLockTimeout);		
		m_AntiLockExclusions = new wxButton(this, wxID_ANY, "Exclusions", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_AntiLockExclusions);
		m_AntiLockExclusions->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
			{
				std::string filters = AntiLock::Get()->SaveExclusions();
				boost::algorithm::replace_all(filters, "|", "\n");

				wxTextEntryDialog d(this, "Enter below application title for AntiLock exclusion", "Enter exclusions", filters, wxOK | wxCANCEL | wxTE_MULTILINE);
				int ret_code = d.ShowModal();
				if(ret_code == wxID_OK)  /* OK */
				{
					std::string result = d.GetValue().ToStdString();
					boost::algorithm::replace_all(result, "\n", "|");

					AntiLock::Get()->LoadExclusions(result);
				}
			});

		bSizer1->Add(static_box_sizer);
	}

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Terminal HotKey");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		m_IsTerminalHotkey = new wxCheckBox(this, wxID_ANY, wxT("Enable?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_IsTerminalHotkey);
		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Hotkey:"), wxDefaultPosition, wxDefaultSize, 0));
		m_TerminalHotkey = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
		static_box_sizer->Add(m_TerminalHotkey);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Type:"), wxDefaultPosition, wxDefaultSize, 0));
		wxArrayString array_terminals;
		array_terminals.Add("Windows Terminal");
		array_terminals.Add("cmd.exe");
		array_terminals.Add("PowerShell");
		array_terminals.Add("Bash Terminal");
		m_TerminalType = new wxComboBox(this, wxID_ANY, "Windows Terminal", wxDefaultPosition, wxDefaultSize, array_terminals);
		m_TerminalType->SetSelection(0);
		static_box_sizer->Add(m_TerminalType);

		bSizer1->Add(static_box_sizer);
	}

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Idle Power Saver");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxRED);

		m_IsIdlePowerSaver = new wxCheckBox(this, wxID_ANY, wxT("Enable?"), wxDefaultPosition, wxDefaultSize, 0);
		static_box_sizer->Add(m_IsIdlePowerSaver);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Timeout [s]:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_IdlePowerSaverTimeout = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 9999, 0);
		static_box_sizer->Add(m_IdlePowerSaverTimeout);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Reduced Power [%]:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_IdlePowerSaverReducedPowerPercent = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
		static_box_sizer->Add(m_IdlePowerSaverReducedPowerPercent);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Min Load Threshold [%]:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_IdlePowerSaverMinLoadThreshold = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
		static_box_sizer->Add(m_IdlePowerSaverMinLoadThreshold);

		static_box_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Max Load Threshold [%]:"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALL, 5);
		m_IdlePowerSaverMaxLoadThreshold = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
		static_box_sizer->Add(m_IdlePowerSaverMaxLoadThreshold);

		bSizer1->Add(static_box_sizer);
	}

	UpdatePanel(); /* update panel for first time, because changeing isn't happen when user clicks to aui and it shows first panel */

	wxBoxSizer* v_sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_Ok = new wxButton(this, wxID_ANY, "Save", wxDefaultPosition, wxDefaultSize);
	m_Ok->SetToolTip("Save all settings to settings.ini file");
	v_sizer_1->Add(m_Ok, 0, wxALL, 5);

	m_Backup = new wxButton(this, wxID_ANY, "Backup", wxDefaultPosition, wxDefaultSize);
	m_Backup->SetToolTip("Backup settings.ini file");
	v_sizer_1->Add(m_Backup, 0, wxALL, 5);

	bSizer1->Add(v_sizer_1);

	m_Ok->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			CustomMacro::Get()->use_per_app_macro = m_IsPerAppMacro->IsChecked();
			CustomMacro::Get()->advanced_key_binding = m_IsAdvancedMacro->IsChecked();
			wxString com_str = m_serial->GetStringSelection();
			SerialPort::Get()->SetEnabled(m_IsCom->IsChecked());
			if(m_serial->GetSelection() > 0)
				SerialPort::Get()->SetComPort(atoi(&com_str.c_str().AsChar()[2]));
			SerialPort::Get()->SetForwardToTcp(m_IsTcpForwarder->IsChecked());
			SerialPort::Get()->SetRemoteTcpIp(m_TcpForwarderIp->GetValue().ToStdString());
			SerialPort::Get()->SetRemoteTcpPort(m_TcpForwarderPort->GetValue());
			
			std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
			wxString com_str_can = m_CanSerial->GetStringSelection();
			CanSerialPort::Get()->SetEnabled(m_IsCanSerial->IsChecked());
			if(m_CanSerial->GetSelection() > 0)
				CanSerialPort::Get()->SetComPort(atoi(&com_str_can.c_str().AsChar()[2]));
			
			CanSerialPort::Get()->SetDeviceType(static_cast<CanDeviceType>(m_CanDeviceType->GetSelection()));
			can_handler->default_tx_list = m_CanDefaultTxList->GetValue().ToStdString();
			can_handler->default_rx_list = m_CanDefaultRxList->GetValue().ToStdString();
			can_handler->default_mapping = m_CanDefaultFrameMapping->GetValue().ToStdString();
			
			can_handler->ToggleAutoSend(m_CanAutoSend->GetValue());
			can_handler->ToggleAutoRecord(m_CanAutoRecord->GetValue());
			can_handler->SetRecordingLogLevel(m_CanDefaultRecordLevel->GetValue());
			can_handler->SetFavouriteLevel(m_CanDefaultFavLevel->GetValue());
			can_handler->SetDefaultEcuId(std::stoi(m_CanDefaultEcuId->GetValue().ToStdString(), 0, 16));

			SerialTcpBackend::Get()->is_enabled = m_SerialForwarderIsEnabled->GetValue();
			SerialTcpBackend::Get()->bind_ip = m_SerialForwarderBindIp->GetValue().ToStdString();
			SerialTcpBackend::Get()->tcp_port = m_SerialForwarderPort->GetValue();
			Server::Get()->is_enabled = m_IsTcp->IsChecked();
			Server::Get()->tcp_port = static_cast<uint16_t>(m_TcpPortSpin->GetValue());
			Sensors::Get()->SetGraphGenerationInterval(static_cast<uint16_t>(m_GraphGenInterval->GetValue()));
			Sensors::Get()->SetGraphResolution(static_cast<uint16_t>(m_GraphResolution->GetValue()));
			Sensors::Get()->SetIntegrationTime(static_cast<uint16_t>(m_GraphIntegrationTime->GetValue()));

			PathSeparator::Get()->replace_key = m_PathSepReplacerKey->GetValue();
			Settings::Get()->minimize_on_exit = m_IsMinimizeOnExit->IsChecked();
			Settings::Get()->minimize_on_startup = m_IsMinimizeOnStartup->GetValue();
			Settings::Get()->remember_window_size = m_RememberWindowSize->GetValue();
			Settings::Get()->always_on_numlock = m_AlwaysOnNumlock->GetValue();
			Settings::Get()->default_page = static_cast<uint8_t>(m_DefaultPage->GetValue());
			PrintScreenSaver::Get()->screenshot_key = m_ScreenshotKey->GetValue(); /* in case of invalid key screenshot saving never will be triggered - nothing special.. using it for disabling */
			PrintScreenSaver::Get()->timestamp_format = m_ScreenshotDateFmt->GetValue();
			PrintScreenSaver::Get()->screenshot_path = m_ScreenshotPath->GetValue().ToStdString();
			DirectoryBackup::Get()->backup_time_format = m_BackupDateFmt->GetValue().ToStdString();
			SymlinkCreator::Get()->is_enabled = m_IsSymlink->IsChecked();
			SymlinkCreator::Get()->mark_key = m_MarkSymlink->GetValue().ToStdString();
			SymlinkCreator::Get()->place_symlink_key = m_CreateSymlink->GetValue().ToStdString();
			SymlinkCreator::Get()->place_hardlink_key = m_CreateHardlink->GetValue().ToStdString();
			AntiLock::Get()->is_enabled = m_IsAntiLock->GetValue();
			AntiLock::Get()->is_screensaver = m_IsScreensSaverAfterLock->GetValue();
			AntiLock::Get()->timeout = static_cast<uint32_t>(m_AntiLockTimeout->GetValue());
			TerminalHotkey::Get()->is_enabled = m_IsTerminalHotkey->GetValue();
			TerminalHotkey::Get()->SetKey(m_TerminalHotkey->GetValue().ToStdString());
			TerminalHotkey::Get()->type = static_cast<TerminalType>(m_TerminalType->GetSelection());

			IdlePowerSaver::Get()->is_enabled = m_IsIdlePowerSaver->IsChecked();
			IdlePowerSaver::Get()->timeout = m_IdlePowerSaverTimeout->GetValue();
			IdlePowerSaver::Get()->reduced_power_percent = m_IdlePowerSaverReducedPowerPercent->GetValue();
			IdlePowerSaver::Get()->min_load_threshold = m_IdlePowerSaverMinLoadThreshold->GetValue();
			IdlePowerSaver::Get()->max_load_threshold = m_IdlePowerSaverMaxLoadThreshold->GetValue();

			Settings::Get()->SaveFile(false);

			MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
			frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::SettingsSaved) });
		});

	m_Backup->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			std::ifstream in("settings.ini", std::ios::in | std::ios::binary);
			if(in)
			{
#ifdef _WIN32
				std::string in_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
				const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
				std::ofstream out(std::format("settings.ini_{:%Y.%m.%d_%H-%M-%OS}.ini", now), std::ofstream::binary);
				out << in_str;
				LOG(LogLevel::Notification, "Settings.ini has been successfully backed up");
#endif
			}
		});
	SetSizer(bSizer1);

	//SetWindowStyle(wxVSCROLL);
	//SetWindowStyle(wxHSCROLL);
	SetScrollbar(wxVERTICAL, 0, 16, 50);
	FitInside();
}

void ComTcpPanel::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);

	// this call is vital: it adjusts the dc to account for the current
	// scroll offset
	DoPrepareDC(dc);
	/*
	dc.SetPen(*wxRED_PEN);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawRectangle(0, 0, WIDTH, HEIGHT);
	*/
}

void ComTcpPanel::UpdatePanel()
{
	int sel = 0;
	int sel_can = 0;
	wxArrayString array_serials; /* TODO: remove can serials from keyboard serial port and vice versa */
#ifdef _WIN32
	CEnumerateSerial::CPortAndNamesArray ports;
	CEnumerateSerial::UsingSetupAPI1(ports);

	for(auto& i : ports)
	{
		array_serials.Add(wxString::Format("COM%d (%s)", i.first, i.second));
		if(SerialPort::Get()->GetComPort() == i.first)
			sel = array_serials.GetCount() - 1;
		if(CanSerialPort::Get()->GetComPort() == i.first)
			sel_can = array_serials.GetCount() - 1;
	}
#else
	array_serials.Add("empty");  /* TODO: serial iteration isn't implemented in linux and inserting empty array will cause assertation fail */
#endif
	
	m_IsPerAppMacro->SetValue(CustomMacro::Get()->use_per_app_macro);
	m_IsAdvancedMacro->SetValue(CustomMacro::Get()->advanced_key_binding);
	m_IsCom->SetValue(SerialPort::Get()->IsEnabled());

	m_serial->Clear();
	m_serial->Insert(array_serials, WXSIZEOF(array_serials));
	m_serial->SetSelection(sel);

	std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
	m_CanSerial->Clear();
	m_CanSerial->Insert(array_serials, WXSIZEOF(array_serials));
	m_CanSerial->SetSelection(sel_can);
	m_CanDeviceType->SetSelection(static_cast<int>(CanSerialPort::Get()->GetDeviceType()));

	m_IsCanSerial->SetValue(CanSerialPort::Get()->IsEnabled());
	m_CanDefaultTxList->SetValue(can_handler->default_tx_list.generic_string());
	m_CanDefaultRxList->SetValue(can_handler->default_rx_list.generic_string());
	m_CanDefaultFrameMapping->SetValue(can_handler->default_mapping.generic_string());
	m_CanDefaultEcuId->SetValue(wxString::Format("%X", can_handler->GetDefaultEcuId()));

	m_IsTcpForwarder->SetValue(SerialPort::Get()->IsForwardToTcp());
	m_TcpForwarderIp->SetValue(SerialPort::Get()->GetRemoteTcpIp());
	m_TcpForwarderPort->SetValue(SerialPort::Get()->GetRemoteTcpPort());
	m_SerialForwarderIsEnabled->SetValue(SerialTcpBackend::Get()->is_enabled);
	m_SerialForwarderBindIp->SetValue(SerialTcpBackend::Get()->bind_ip);
	m_SerialForwarderPort->SetValue(SerialTcpBackend::Get()->tcp_port);
	m_IsTcp->SetValue(Server::Get()->is_enabled);
	m_TcpPortSpin->SetValue(Server::Get()->tcp_port);

	m_GraphGenInterval->SetValue(Sensors::Get()->GetGraphGenerationInterval());
	m_GraphResolution->SetValue(Sensors::Get()->GetGraphResolution());
	m_GraphIntegrationTime->SetValue(Sensors::Get()->GetIntegrationTime());

	m_PathSepReplacerKey->SetValue(PathSeparator::Get()->replace_key);
	m_IsMinimizeOnExit->SetValue(Settings::Get()->minimize_on_exit);
	m_IsMinimizeOnStartup->SetValue(Settings::Get()->minimize_on_startup);
	m_RememberWindowSize->SetValue(Settings::Get()->remember_window_size);
	m_AlwaysOnNumlock->SetValue(Settings::Get()->always_on_numlock);
	m_DefaultPage->SetValue(Settings::Get()->default_page);
	m_ScreenshotKey->SetValue(PrintScreenSaver::Get()->screenshot_key);
	m_ScreenshotDateFmt->SetValue(PrintScreenSaver::Get()->timestamp_format);
	m_ScreenshotPath->SetValue(PrintScreenSaver::Get()->screenshot_path.generic_string());
	m_BackupDateFmt->SetValue(DirectoryBackup::Get()->backup_time_format);
	m_IsSymlink->SetValue(SymlinkCreator::Get()->is_enabled);
	m_MarkSymlink->SetValue(SymlinkCreator::Get()->mark_key);
	m_CreateSymlink->SetValue(SymlinkCreator::Get()->place_symlink_key);
	m_CreateHardlink->SetValue(SymlinkCreator::Get()->place_hardlink_key);
	m_IsAntiLock->SetValue(AntiLock::Get()->is_enabled);
	m_IsScreensSaverAfterLock->SetValue(AntiLock::Get()->is_screensaver);
	m_AntiLockTimeout->SetValue(AntiLock::Get()->timeout);
	m_IsTerminalHotkey->SetValue(TerminalHotkey::Get()->is_enabled);
	m_TerminalHotkey->SetValue(TerminalHotkey::Get()->GetKey());
	m_TerminalType->SetSelection(static_cast<int>(TerminalHotkey::Get()->type));

	m_IsIdlePowerSaver->SetValue(IdlePowerSaver::Get()->is_enabled);
	m_IdlePowerSaverTimeout->SetValue(IdlePowerSaver::Get()->timeout);
	m_IdlePowerSaverReducedPowerPercent->SetValue(IdlePowerSaver::Get()->reduced_power_percent);
	m_IdlePowerSaverMinLoadThreshold->SetValue(IdlePowerSaver::Get()->min_load_threshold);
	m_IdlePowerSaverMaxLoadThreshold->SetValue(IdlePowerSaver::Get()->max_load_threshold);
}

void KeybrdPanel::UpdateMainTree()
{
	tree->DeleteAllItems();
	wxTreeListItem root = tree->GetRootItem();
	for(auto& m : CustomMacro::Get()->GetMacros())
	{
		wxTreeListItem item = tree->AppendItem(root, m->app_name.c_str());
		for(auto& b : m->bind_name)
		{
			wxTreeListItem bind_item = tree->AppendItem(item, b.second.c_str());
			tree->SetItemText(bind_item, 1, b.first.c_str());
		}
		tree->Expand(item);
	}
}

void KeybrdPanel::UpdateDetailsTree(std::unique_ptr<IKey>* ptr)
{
	tree_details->DeleteAllItems();
	wxTreeListItem root2 = tree_details->GetRootItem();
	uint16_t cnt = 0;
	auto& m = CustomMacro::Get()->GetMacros();
	for(auto& i : m)
	{
		if(i->app_name == root_sel_str.ToStdString())
		{
			for(auto& x : i->key_vec[child_sel_str.ToStdString()])
			{
				IKey* p = x.get();
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

		UpdateDetailsTree();
	}
}

void KeybrdPanel::OnTreeListChanged_Details(wxTreeListEvent& evt)
{

}

void KeybrdPanel::OnItemContextMenu_Main(wxTreeListEvent& evt)
{
	wxMenu menu;
	menu.Append(Id_Macro_AddNewApplication, "&Add new application (below)")->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW_DIR, wxART_OTHER, FromDIP(wxSize(14, 14))));
	menu.Append(Id_Macro_AddNewMacroKey, "&Add new macro key (below)")->SetBitmap(wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_OTHER, FromDIP(wxSize(14, 14))));
	menu.Append(Id_Macro_Rename, "&Rename")->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT, wxART_OTHER, FromDIP(wxSize(14, 14))));
	menu.Append(Id_Macro_Delete, "&Delete", "&Delete selected item")->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_OTHER, FromDIP(wxSize(14, 14))));

	const wxTreeListItem item = evt.GetItem();
	wxTreeListItem root = tree->GetItemParent(item);
	if(root == NULL) return;
	wxTreeListItem root2 = tree->GetItemParent(root);

	wxTreeListItem child = tree->GetFirstChild(item);
	menu.Enable(Id_Macro_AddNewApplication, !(child == 0));

	int ret = tree->GetPopupMenuSelectionFromUser(menu);
	switch(ret)
	{
		case Id_Macro_AddNewApplication:
		{
			const wxString& root_str = tree->GetItemText(item, 0);
			const wxString& item_str = tree->GetItemText(item, 1);

			wxTextEntryDialog d(this, "Type new application macro name", "Caption");
			int ret = d.ShowModal();
			if(ret == wxID_OK)
			{
				auto& m = CustomMacro::Get()->GetMacros();

				std::vector<std::unique_ptr<MacroAppProfile>>::const_iterator i;
				for(i = m.begin(); i != m.end(); i++)
				{
					if(i->get()->app_name == root_str)
					{
						break;
					}
				}
				m.insert(i + 1, std::make_unique<MacroAppProfile>(d.GetValue().ToStdString()));
			}
			UpdateMainTree();
			break;
		}
		case Id_Macro_AddNewMacroKey:
		{
			wxTreeListItem root = tree->GetItemParent(item);
			wxTreeListItem root2 = tree->GetItemParent(root);
			const wxString& item_str = root2 ? tree->GetItemText(root, 0) : tree->GetItemText(item, 0);
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
				if(i->app_name == item_str)
				{
					if(i->key_vec.contains(macro_key))
					{
						wxMessageDialog(this, "Given key already has a binded macro!", "Error", wxOK).ShowModal();
						return;
					}

					auto pp = std::make_unique<MacroAppProfile>();
					i->bind_name[macro_key] = std::move(macro_name);
					UpdateMainTree();
					break;
				}
			}
			break;
		}
		case Id_Macro_Rename:
		{
			const wxString& name_str = tree->GetItemText(item, 0);
			wxTreeListItem root = tree->GetItemParent(item);
			wxTreeListItem root2 = tree->GetItemParent(root);
			if(root2) /* clicked on subitem, not on root item */
			{
				const wxString& key_str = tree->GetItemText(item, 1);
				add_dlg->ShowDialog(key_str, name_str);

				const wxString& macro_key = add_dlg->GetMacroKey();
				const wxString& macro_name = add_dlg->GetMacroName();
				if(!add_dlg->IsApplyClicked()) return;

				auto& m = CustomMacro::Get()->GetMacros();
				auto it = std::find_if(m.begin(), m.end(), [this, sel_str = this->root_sel_str](std::unique_ptr<MacroAppProfile>& x) { return x->app_name == sel_str; });
				if(it != m.end())
				{
					std::map<std::string, std::vector<std::unique_ptr<IKey>>>::iterator e = (*it)->key_vec.find(child_sel_str.ToStdString());
					if(e == (*it)->key_vec.end())
						throw std::logic_error("map item not found");

					std::string old_macrokey = child_sel_str.ToStdString();

					auto nodeHandler = (*it)->bind_name.extract(old_macrokey);
					nodeHandler.key() = macro_key;
					nodeHandler.mapped() = macro_name;
					(*it)->bind_name.insert(std::move(nodeHandler));

					auto nodeHandler2 = (*it)->key_vec.extract(old_macrokey);
					nodeHandler2.key() = macro_key;
					(*it)->key_vec.insert(std::move(nodeHandler2));

					UpdateMainTree();
				}
			}
			else
			{
				wxTextEntryDialog dialog(this, "Enter below desired application name for macro", "Edit application name", name_str);
				int ret = dialog.ShowModal();
				if(ret == wxID_OK)
				{
					auto& m = CustomMacro::Get()->GetMacros();
					auto it = std::find_if(m.begin(), m.end(), [&name_str](std::unique_ptr<MacroAppProfile>& x) { return x->app_name == name_str; });
					if(it != m.end())
					{
						(*it)->app_name = dialog.GetValue().ToStdString();
						UpdateMainTree();
					}
				}
			}
			break;
		}
		case Id_Macro_Delete:
		{
			wxMessageDialog dialog(this, "Are you sure want to delete selected macro?", "Deleting macro", wxCENTER | wxNO_DEFAULT | wxYES_NO | wxICON_INFORMATION);
			int ret = dialog.ShowModal();
			switch(ret)
			{
				case wxID_YES:
				{
					auto& m = CustomMacro::Get()->GetMacros();
					wxTreeListItem root = tree->GetItemParent(item);
					wxTreeListItem root2 = tree->GetItemParent(root);
					const wxString& root_str = tree->GetItemText(item, 0);

					if(root2 == NULL) /* clicked on one of the root items */
					{
						m.erase(std::remove_if(m.begin(), m.end(), [&root_str](std::unique_ptr<MacroAppProfile>& x) { return x->app_name == root_str; }));
					}
					else
					{
						const wxString& root_str = tree->GetItemText(root, 0);
						const wxString& key_str = tree->GetItemText(item, 1);
						for(auto& i : m)
						{
							if(i->app_name == root_str)
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
	wxMenu menu;
	menu.Append(Id_MacroDetails_AddNew, "&Add new macro (below)")->SetBitmap(wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_OTHER, FromDIP(wxSize(14, 14))));
	menu.Append(Id_MacroDetails_Clone, "&Clone (below)")->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_OTHER, FromDIP(wxSize(14, 14))));
	menu.Append(Id_MacroDetails_Delete, "&Delete macro(s)")->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_OTHER, FromDIP(wxSize(14, 14))));
	menu.Append(Id_MacroDetails_MoveUp, "&Move up")->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_UP, wxART_OTHER, FromDIP(wxSize(14, 14))));
	menu.Append(Id_MacroDetails_MoveDown, "&Move down")->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_OTHER, FromDIP(wxSize(14, 14))));

	const wxTreeListItem item = evt.GetItem();
	wxTreeListItem root = tree->GetItemParent(item);
	if(root == NULL) return;
	int ret = tree->GetPopupMenuSelectionFromUser(menu);
	switch(ret)
	{
		case Id_MacroDetails_AddNew:
		{
			TreeDetails_AddNewMacro();
			break;
		}
		case Id_MacroDetails_Clone:
		{
			TreeDetails_CloneMacro();
			break;
		}
		case Id_MacroDetails_Delete:
		{
			TreeDetails_DeleteSelectedMacros();
			break;
		}
		case Id_MacroDetails_MoveUp:
		{
			TreeDetails_MoveUpSelectedMacro();
			break;
		}
		case Id_MacroDetails_MoveDown:
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
		const wxString& str = tree_details->GetItemText(item, 1);
		const wxString& str2 = tree_details->GetItemText(root, 0);
		DBG("sel: %s - root: %s\n", str.ToStdString().c_str(), str2.ToStdString().c_str());

		DBG("main tree sel: child: %s - root: %s\n", child_sel_str.c_str().AsChar(), root_sel_str.c_str().AsChar());
		wxTreeListItem root2 = tree_details->GetRootItem();

		const wxString& type_str = tree_details->GetItemText(item, 0);
		const wxString& item_str = tree_details->GetItemText(item, 1);

		uint8_t sel = 0;  /* default: SEQUENCE */
		if(type_str == "TEXT")
			sel = 1;
		else if(type_str == "DELAY RANDOM" || type_str == "DELAY")
			sel = 2;
		else if(type_str == "MOUSE MOVE")
			sel = 3;		
		else if(type_str == "MOUSE INTERPOLATE")
			sel = 4;
		else if(type_str == "MOUSE PRESS")
			sel = 5;		
		else if(type_str == "MOUSE RELEASE")
			sel = 6;		
		else if(type_str == "MOUSE CLICK")
			sel = 7;
		else if(type_str == "BASH")
			sel = 8;		
		else if(type_str == "CMD")
			sel = 9;
		else if(type_str == "CMD_XML")
			sel = 10;
		edit_dlg->ShowDialog(item_str, sel);
	}
}

void KeybrdPanel::DuplicateMacro(std::vector<std::unique_ptr<IKey>>& x, uint16_t id)
{
	IKey* p = x[id].get(); /* TODO: use smart pointers here! */
	IKey* c(p->Clone());
	x.insert(x.begin() + ++id, std::unique_ptr<IKey>(c));
}

template <typename T> void AddOrModifyMacro(std::vector<std::unique_ptr<IKey>>& x, uint16_t id, bool add, std::string&& source_string)
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

void KeybrdPanel::ManipulateMacro(std::vector<std::unique_ptr<IKey>>& x, uint16_t id, bool add)
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
			catch(const std::exception& e)
			{
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}
		case 3:
		{
			try
			{
				AddOrModifyMacro<MouseMovement>(x, id, add, std::move(edit_str));
			}
			catch(const std::exception& e)
			{
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}		
		case 4:
		{
			try
			{
				AddOrModifyMacro<MouseInterpolate>(x, id, add, std::move(edit_str));
			}
			catch(const std::exception& e)
			{
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}
		case 5:
		{
			try
			{
				AddOrModifyMacro<MousePress>(x, id, add, std::move(edit_str));
			}
			catch(const std::exception& e)
			{
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}		
		case 6:
		{
			try
			{
				AddOrModifyMacro<MouseRelease>(x, id, add, std::move(edit_str));
			}
			catch(const std::exception& e)
			{
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}		
		case 7:
		{
			try
			{
				AddOrModifyMacro<MouseClick>(x, id, add, std::move(edit_str));
			}
			catch(const std::exception& e)
			{
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}		
		case 8:
		{
			try
			{
				AddOrModifyMacro<BashCommand>(x, id, add, std::move(edit_str));
			}
			catch(const std::exception& e)
			{
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}		
		case 9:
		{
			try
			{
				AddOrModifyMacro<CommandExecute>(x, id, add, std::move(edit_str));
			}
			catch(const std::exception& e)
			{
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
			}
			break;
		}
		case 10:
		{
			try
			{
				AddOrModifyMacro<CommandXml>(x, id, add, std::move(edit_str));
			}
			catch(const std::exception& e)
			{
				wxMessageDialog(this, std::format("Invalid input!\n{}", e.what()), "Error", wxOK).ShowModal();
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
		if(i->app_name == root_sel_str.ToStdString())
		{
			std::vector<std::unique_ptr<IKey>>& x = i->key_vec[child_sel_str.ToStdString()];
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
	std::set<IKey*> items_to_delete;
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
			if(i->app_name == root_sel_str.ToStdString())
			{
				std::vector<std::unique_ptr<IKey>>& c = i->key_vec[child_sel_str.ToStdString()];
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
			if(i->app_name == root_sel_str.ToStdString())
			{
				std::vector<std::unique_ptr<IKey>>& x = i->key_vec[child_sel_str.ToStdString()];
				std::unique_ptr<IKey>* ptr = nullptr;
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
		std::vector<std::unique_ptr<IKey>>* x = GetKeyClassByItem(item, id);
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
		std::vector<std::unique_ptr<IKey>>* x = GetKeyClassByItem(item, id);
		if(x)
		{
			std::unique_ptr<IKey>* ptr;
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
		std::vector<std::unique_ptr<IKey>>* x = GetKeyClassByItem(item, id);
		if(x)
		{
			std::unique_ptr<IKey>* ptr;
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
			std::unique_lock lock(frame->mtx);
			frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::MacroRecordingStopped) });
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
			if(i->app_name == root_sel_str.ToStdString())
			{
				std::vector<std::unique_ptr<IKey>>& x = i->key_vec[child_sel_str.ToStdString()];
				std::unique_ptr<IKey>* ptr = nullptr;
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
					std::unique_lock lock(frame->mtx);
					frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::MacroRecordingStarted) });
				}
			}
		}
	}
}

KeybrdPanel::KeybrdPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer0 = new wxBoxSizer(wxVERTICAL); 
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxHORIZONTAL);

	tree = new wxTreeListCtrl(this, ID_AppBindListMain, wxDefaultPosition, wxSize(300, 400), wxTL_SINGLE);
	tree->AppendColumn("App name", tree->WidthFor("App nameApp nameApp name"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree->AppendColumn("Key bindings", tree->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	UpdateMainTree();
	bSizer1->Add(tree, wxSizerFlags(5).Left().Expand());

	tree_details = new wxTreeListCtrl(this, ID_MacroDetails, wxDefaultPosition, wxSize(300, 400), wxTL_MULTIPLE);
	tree_details->AppendColumn("Action type", tree_details->WidthFor("App nameApp nameApp name"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree_details->AppendColumn("Parameters", tree_details->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	bSizer1->Add(tree_details, wxSizerFlags(5).Top().Expand());

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

	m_Ok = new wxButton(this, wxID_ANY, "Save", wxDefaultPosition, wxDefaultSize); // wxPoint(tree->GetSize().x + 20, tree->GetSize().y + 20)
	m_Ok->SetToolTip("Save all settings to settings.ini file");
	m_Ok->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			Settings::Get()->SaveFile(false);

			MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
			frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::SettingsSaved) });
		});

	vertical_sizer->Add(btn_add);
	vertical_sizer->Add(btn_record);
	vertical_sizer->Add(btn_copy);
	vertical_sizer->Add(btn_delete);
	vertical_sizer->Add(btn_up);
	vertical_sizer->Add(btn_down);
	bSizer1->Add(vertical_sizer, wxSizerFlags(1).Border(wxRIGHT, 1).Expand());

	bSizer0->Add(bSizer1, wxSizerFlags(1).Expand().Border(wxDOWN, 50));
	bSizer0->Add(m_Ok, wxSizerFlags(0).CenterHorizontal().Border(wxDOWN, 150));

	record_dlg = new MacroRecordBoxDialog(this);
	edit_dlg = new MacroEditBoxDialog(this);
	add_dlg = new MacroAddBoxDialog(this);
	SetSizerAndFit(bSizer0);
	Show();
}

std::vector<std::unique_ptr<IKey>>* KeybrdPanel::GetKeyClassByItem(wxTreeListItem item, uint16_t& id)
{
	std::vector<std::unique_ptr<IKey>>* ret = nullptr;
	if(item.GetID() != NULL)
	{
		auto& m = CustomMacro::Get()->GetMacros();
		for(auto& i : m)
		{
			if(i->app_name == root_sel_str.ToStdString())
			{
				std::vector<std::unique_ptr<IKey>>* x = &i->key_vec[child_sel_str.ToStdString()];

				wxClientData* itemdata = tree_details->GetItemData(item);
				wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
				id = dret->GetValue();
				ret = x;
			}
		}
	}
	return ret;
}