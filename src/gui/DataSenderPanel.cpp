#include "pch.hpp"

wxBEGIN_EVENT_TABLE(DataSenderPanel, wxPanel)
EVT_SIZE(DataSenderPanel::OnSize)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(DataSenderDataPanel, wxPanel)
EVT_SIZE(DataSenderDataPanel::OnSize)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(DataSenderButtonPanel, wxPanel)
EVT_SIZE(DataSenderButtonPanel::OnSize)
wxEND_EVENT_TABLE()

DataSenderDataPanel::DataSenderDataPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* v_sizer = new wxBoxSizer(wxHORIZONTAL);

	m_Send = new wxButton(this, wxID_ANY, wxT("Send"), wxDefaultPosition, wxDefaultSize, 0);
	m_Send->SetToolTip("Start sending of messages");
	m_Send->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;
			data_sender->ToggleSending(true);
		});

	v_sizer->Add(m_Send);

	m_Stop = new wxButton(this, wxID_ANY, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0);
	m_Stop->SetToolTip("Stop sending of messages");
	m_Stop->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;

			data_sender->ToggleSending(false);
			DataSerialPort::Get()->ClearBuffers();
			UpdateStatusIndicator();
		});
	v_sizer->Add(m_Stop);

	m_Clear = new wxButton(this, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
	m_Clear->SetToolTip("Clear everything");
	m_Clear->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;

			data_sender->ToggleSending(false);
			data_sender->ResetCounters();
			DataSerialPort::Get()->ClearBuffers();
			m_Log->Clear();
			UpdateStatusIndicator();
		});
	v_sizer->Add(m_Clear);

	m_SetDate = new wxButton(this, wxID_ANY, wxT("Send actual date"), wxDefaultPosition, wxDefaultSize, 0);
	m_SetDate->SetToolTip("Send actual date");
	m_SetDate->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;

			const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
			std::string log_format = std::format("{:%Y-%m-%d-%H-%M-%OS}.csv", now);
			DataSerialPort::Get()->AddToTxQueue(log_format.length(), (uint8_t*)log_format.c_str());

			LOG(LogLevel::Normal, "Date sent: {}", log_format);
			event.Skip();
		});
	v_sizer->Add(m_SetDate);

	m_ErrorCount = new wxStaticText(this, wxID_ANY, "Error count:");
	v_sizer->Add(m_ErrorCount);

	bSizer1->Add(v_sizer, 0, wxALL, 5);

	m_Log = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE | wxLB_HSCROLL | wxLB_NEEDED_SB);
	m_Log->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent& event)
		{
			wxClipboard* clip = new wxClipboard();
			clip->Clear();
			clip->SetData(new wxTextDataObject(m_Log->GetString(m_Log->GetSelection())));
			clip->Flush();
			clip->Close();
		});
	bSizer1->Add(m_Log, wxSizerFlags(1).Left().Expand());

	SetSizerAndFit(bSizer1);

}

void DataSenderDataPanel::UpdateStatusIndicator()
{
	std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;
	m_ErrorCount->SetLabelText(wxString::Format("TX: %lld, RX: %lld, Error: %lld", data_sender->GetTxMsgCount(), data_sender->GetRxMsgCount(), data_sender->GetErrorCount()));
}

void DataSenderDataPanel::OnSize(wxSizeEvent& evt)
{
	evt.Skip(true);
}

DataSenderButtonPanel::DataSenderButtonPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	OnDataLoaded();

}

void DataSenderButtonPanel::OnDataLoaded()
{
	std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
	for(auto& entry : data_sender->entries)
	{
		entry->m_Text = new wxStaticText(this, wxID_ANY, entry->m_TextName, wxDefaultPosition, wxDefaultSize, 0);
		entry->m_Text->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
		entry->m_Text->SetForegroundColour(wxColor(240, 120, 0));
		entry->m_Text->SetToolTip(entry->m_comment);
		bSizer1->Add(entry->m_Text);
	}
	SetSizerAndFit(bSizer1);
}

void DataSenderButtonPanel::OnSize(wxSizeEvent& evt)
{
	evt.Skip(true);
}

DataSenderPanel::DataSenderPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;
	data_sender->SetLogHelper(this);

	wxSize client_size = GetClientSize();

	m_mgr.SetManagedWindow(this);

	m_notebook = new wxAuiNotebook(this, wxID_ANY, wxPoint(0, 0), wxSize(Settings::Get()->window_size.x - 50, Settings::Get()->window_size.y - 50), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	data_panel = new DataSenderDataPanel(this);
	button_panel = new DataSenderButtonPanel(this);
	m_notebook->Freeze();
	m_notebook->AddPage(data_panel, "Data", false, wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, FromDIP(wxSize(16, 16))));
	m_notebook->AddPage(button_panel, "Button", false, wxArtProvider::GetBitmap(wxART_HELP_SETTINGS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	//m_notebook->Connect(wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler(DataSenderPanel::Changeing), NULL, this);

	/* size: 1640x1080 */
	m_notebook->Split(0, wxLEFT);

	m_notebook->Thaw();
	m_notebook->SetAutoLayout(true);
	m_notebook->Layout();
	m_notebook->SetSize(m_notebook->GetSize());
	m_notebook->SetSelection(0);
}

void DataSenderPanel::OnDataLoaded()
{
	if(button_panel)
		button_panel->OnDataLoaded();
}

void DataSenderPanel::On10MsTimer()
{
	
}

void DataSenderPanel::ClearEntries()
{
	if(data_panel)
		data_panel->m_Log->Clear();
}

void DataSenderPanel::AppendLog(DataEntry* entry, const std::string& line)
{
	static size_t cnt = 0;
	if(data_panel && data_panel->m_Log)
	{
		data_panel->m_Log->Append(wxString(line));
		cnt++;
		if(1 && data_panel->m_AutoScroll)
			data_panel->m_Log->ScrollLines(data_panel->m_Log->GetCount());

		if (!(cnt % 10))
		{
			data_panel->UpdateStatusIndicator();
		}

		if(entry && entry->m_Text)
		{
			size_t pos = entry->m_LastResponse.find(entry->m_StartWith);
			std::string tmp = "invalid";
			if(pos != std::string::npos)
			{
				tmp = entry->m_LastResponse.substr(pos + entry->m_StartWith.length());
			}
			entry->m_Text->SetLabelText(wxString::Format("%s: %s", entry->m_TextName, tmp));
		}
	}
}

void DataSenderPanel::OnError(uint32_t err_cnt, const std::string& line)
{
	if(data_panel && data_panel->m_Log)
	{
		data_panel->UpdateStatusIndicator();
	}
}

void DataSenderPanel::OnSize(wxSizeEvent& evt)
{
	evt.Skip(true);
}