#include "pch.hpp"

wxBEGIN_EVENT_TABLE(DebugPanel, wxPanel)
wxEND_EVENT_TABLE()

DebugPanel::DebugPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_MousePos = new wxStaticText(this, wxID_ANY, "Pos: 0,0");
	m_MousePos->SetToolTip("Mouse position relative to active window");
	bSizer1->Add(m_MousePos);
	m_ActiveWindowTitle = new wxStaticText(this, wxID_ANY, "Window: \"NULL\"");
	m_ActiveWindowTitle->SetToolTip("Name of foreground (active) window");
	bSizer1->Add(m_ActiveWindowTitle);

	m_SaveScreenshot = new wxButton(this, wxID_ANY, "Save screenshot", wxDefaultPosition, wxDefaultSize);
	m_SaveScreenshot->SetToolTip("Save screenshot from clipboard to a .png file");
    m_SaveScreenshot->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
			PrintScreenSaver::Get()->SaveScreenshot();
        });
	bSizer1->Add(m_SaveScreenshot);	
	
	m_PathSeparatorReplace = new wxButton(this, wxID_ANY, "Path Separator Replace", wxDefaultPosition, wxDefaultSize);
	m_PathSeparatorReplace->SetToolTip("Replace path separators in clipboard");
	m_PathSeparatorReplace->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
			PathSeparator::Get()->ReplaceClipboard();
        });
	bSizer1->Add(m_PathSeparatorReplace);	
	
	m_KeyTopress = new wxTextCtrl(this, wxID_ANY, "NUM_1", wxDefaultPosition, wxSize(150, 50), wxTE_MULTILINE);
	bSizer1->Add(m_KeyTopress);

	m_SimulateKeypress = new wxButton(this, wxID_ANY, "Simulate keypress", wxDefaultPosition, wxDefaultSize);
	m_SimulateKeypress->SetToolTip("Simulate keypress on second keyboard");
	m_SimulateKeypress->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
			if(keypress_future.valid())
				keypress_future.get();

			keypress_future = std::async([key = m_KeyTopress->GetValue().ToStdString()] {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				CustomMacro::Get()->SimulateKeypress(key);
				});

        });
	bSizer1->Add(m_SimulateKeypress);

	m_TextPanel = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(310, 310), wxTE_MULTILINE);
	bSizer1->Add(m_TextPanel);

	wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);

	h_sizer->Add(new wxStaticText(this, wxID_ANY, "CPU Min:"));
	h_sizer->AddSpacer(5);
	m_CpuMinPowerPercent = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 100);
	h_sizer->Add(m_CpuMinPowerPercent);
	h_sizer->AddSpacer(5);

	h_sizer->Add(new wxStaticText(this, wxID_ANY, "CPU Max: "));
	h_sizer->AddSpacer(5);
	m_CpuMaxPowerPercent = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 100);
	h_sizer->Add(m_CpuMaxPowerPercent);
	h_sizer->AddSpacer(5);

	m_CpuPowerRefresh = new wxButton(this, wxID_ANY, "Refresh", wxDefaultPosition, wxDefaultSize);
	m_CpuPowerRefresh->SetToolTip("Refresh actual CPU power percent");
	m_CpuPowerRefresh->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			m_CpuMinPowerPercent->SetValue(IdlePowerSaver::Get()->GetCpuMinPowerPercent());
			m_CpuMaxPowerPercent->SetValue(IdlePowerSaver::Get()->GetCpuMaxPowerPercent());
		});
	h_sizer->Add(m_CpuPowerRefresh);

	m_CpuPowerApply = new wxButton(this, wxID_ANY, "Apply", wxDefaultPosition, wxDefaultSize);
	m_CpuPowerApply->SetToolTip("Apply new CPU power percent values");
	m_CpuPowerApply->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
		{
			uint8_t min_percent = m_CpuMinPowerPercent->GetValue();
			uint8_t max_percent = m_CpuMaxPowerPercent->GetValue();

			IdlePowerSaver::Get()->SetCpuPowerPercent(min_percent, max_percent);
		});
	h_sizer->Add(m_CpuPowerApply);

	bSizer1->AddSpacer(10);
	bSizer1->Add(h_sizer);

	this->SetSizer(bSizer1);
	this->Layout();
}

void DebugPanel::HandleUpdate()
{
#ifdef _WIN32
	POINT p;
	HWND foreground = GetForegroundWindow();
	if(::GetCursorPos(&p))
	{
		if(::ScreenToClient(foreground, &p))
		{
			RECT rect;
			if(GetWindowRect(foreground, &rect))
			{
				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;
				wxString str = wxString::Format(wxT("Mouse: %d,%d - Rect: %d,%d"), p.x, p.y, width, height);
				m_MousePos->SetLabelText(str);
				if(GetAsyncKeyState(VK_SCROLL))
				{
					LOG(LogLevel::Normal, str.ToStdString().c_str());
					MacroRecorder::Get()->MarkMousePosition((LPPOINT*)&p);
				}
			}
		}
	}

	char window_title[256];
	if(GetWindowTextA(foreground, window_title, sizeof(window_title)))
	{
		wxString str = wxString::Format(wxT("Window: \"%s\""), window_title);
		m_ActiveWindowTitle->SetLabelText(str);
	}
#else

#endif
}