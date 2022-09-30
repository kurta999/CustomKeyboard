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