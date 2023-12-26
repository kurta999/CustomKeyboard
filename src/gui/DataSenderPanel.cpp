#include "pch.hpp"

wxBEGIN_EVENT_TABLE(DataSenderPanel, wxPanel)
wxEND_EVENT_TABLE()

DataSenderPanel::DataSenderPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;
	data_sender->SetLogHelper(this);

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

	this->SetSizerAndFit(bSizer1);
	this->Layout();
}

void DataSenderPanel::ClearEntries()
{
	m_Log->Clear();
}

void DataSenderPanel::AppendLog(const std::string& line)
{
	static size_t cnt = 0;
	if(m_Log)
	{
		m_Log->Append(wxString(line));
		cnt++;
		if(1 && m_AutoScroll)
			m_Log->ScrollLines(m_Log->GetCount());

		if (!(cnt % 10))
		{
			UpdateStatusIndicator();
		}
	}
}

void DataSenderPanel::OnError(uint32_t err_cnt, const std::string& line)
{
	if(m_Log)
	{
		UpdateStatusIndicator();
	}
}

void DataSenderPanel::UpdateStatusIndicator()
{
	std::unique_ptr<DataSender>& data_sender = wxGetApp().data_sender;
	m_ErrorCount->SetLabelText(wxString::Format("TX: %lld, RX: %lld, Error: %lld", data_sender->GetTxMsgCount(), data_sender->GetRxMsgCount(), data_sender->GetErrorCount()));
}