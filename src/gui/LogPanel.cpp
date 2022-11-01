#include "pch.hpp"

wxBEGIN_EVENT_TABLE(LogPanel, wxPanel)
wxEND_EVENT_TABLE()

LogPanel::LogPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	Logger::Get()->SetLogHelper(this);
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* v_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxArrayString filters;
	filters.Add("All");
	filters.Add("Normal");
	filters.Add("Notification");
	filters.Add("Warning");
	filters.Add("Error");
	filters.Add("Critical");
	m_FilterLevel = new wxComboBox(this, wxID_ANY, "All", wxDefaultPosition, wxDefaultSize, filters, wxTE_PROCESS_ENTER | wxTE_READONLY);
	v_sizer->Add(m_FilterLevel, 0, wxALL, 5);
	m_FilterLevel->Bind(wxEVT_KEY_DOWN, [this](wxKeyEvent& event)
		{
			if(event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER)
			{
				ExecuteSearchInLogfile();
			}
			event.Skip();
		});

	m_FilterText = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	v_sizer->Add(m_FilterText, 0, wxALL, 5);
	m_FilterText->Bind(wxEVT_KEY_DOWN, [this](wxKeyEvent& event)
		{
			if(event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER)
			{
				ExecuteSearchInLogfile();
			}
			event.Skip();
		});
	m_ApplyFilter = new wxButton(this, wxID_ANY, wxT("Filter"), wxDefaultPosition, wxDefaultSize, 0);
	m_ApplyFilter->SetToolTip("Filter log messages from logfile");
	v_sizer->Add(m_ApplyFilter, 0, wxALL, 5);

	m_ApplyFilter->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			ExecuteSearchInLogfile();
		});

	m_Pause = new wxButton(this, wxID_ANY, wxT("Toggle auto-scroll"), wxDefaultPosition, wxDefaultSize, 0);
	m_Pause->SetToolTip("Disable auto-scroll");
	v_sizer->Add(m_Pause, 0, wxALL, 5);
	bSizer1->Add(v_sizer, 0, wxALL, 5);

	m_Pause->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			m_AutoScroll ^= 1;
			if(m_AutoScroll)
				m_Pause->SetBackgroundColour(wxNullColour);
			else
				m_Pause->SetBackgroundColour(*wxRED);
		});

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

	m_ClearButton = new wxButton(this, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
	m_ClearButton->SetToolTip("Clear log box");
	bSizer1->Add(m_ClearButton, 0, wxALL, 5);

	m_ClearButton->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			m_Log->Clear();
		});

	m_LogFilters.push_back({ "IdlePowerSaver" });

	this->SetSizerAndFit(bSizer1);
	this->Layout();
}

void LogPanel::ExecuteSearchInLogfile()
{
	std::string filter = m_FilterText->GetValue().ToStdString();
	std::string log_level = m_FilterLevel->GetValue().ToStdString();
	if(log_level == "All")
		log_level.clear();

	bool ret = Logger::Get()->SearchInLogFile(filter, log_level);
	if(ret)
		m_Log->ScrollLines(m_Log->GetCount());
}

template<typename T> bool LogPanel::IsFilterered(const T& file)
{
	for(auto& i : m_LogFilters)
	{
		if(i.empty()) continue;
		if(std::search(file.begin(), file.end(), i.begin(), i.end()) != file.end())
		{
			return true;
		}
	}
	return false;
}

void LogPanel::ClearEntries()
{
	m_Log->Clear();
}

void LogPanel::AppendLog(const std::string& file, const std::string& line, bool scroll_to_end)
{
	if(!IsFilterered(file))
	{
		m_Log->Append(wxString(line));
		if(scroll_to_end && m_AutoScroll)
			m_Log->ScrollLines(m_Log->GetCount());
	}
}

void LogPanel::AppendLog(const std::wstring& file, const std::wstring& line, bool scroll_to_end)
{
	if(!IsFilterered(file))
	{
		m_Log->Append(wxString(line));
		if(scroll_to_end && m_AutoScroll)
			m_Log->ScrollLines(m_Log->GetCount());
	}
}