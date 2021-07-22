#include "pch.h"
#include "../commitid.h"

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(ID_Help, MyFrame::OnHelp)
EVT_MENU(ID_About, MyFrame::OnAbout)
EVT_MENU(ID_Quit, MyFrame::OnQuit)
EVT_MENU(wxID_OPEN, MyFrame::OnOpen)
EVT_MENU(wxID_SAVE, MyFrame::OnSave)
EVT_MENU(wxID_SAVEAS, MyFrame::OnSaveAs)
EVT_MENU(ID_DestroyAll, MyFrame::OnDestroyAll)
EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MainPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(EditorPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ConfigurationPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(EscaperPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MacroPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(LogPanel, wxPanel)
wxEND_EVENT_TABLE()

void MyFrame::OnHelp(wxCommandEvent& event)
{
	wxMessageBox("This is a simple program which made for improving my coding experience.\n\
		Feel free to re(use) it in any way what you want.", "Help");
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxString platform = (sizeof(void*) == 4 ? " x86" : " x64");
	wxMessageBox(wxString("CustomKeyboard") + platform + " v" + COMMIT_TAG + " (" + COMMIT_ID + ")" + "\n\n"
"MIT License\n\
\n\
Copyright (c) 2021 kurta999\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy\n\
of this software and associated documentation files (the \"Software\"), to deal\n\
in the Software without restriction, including without limitation the rights\n\
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n\
copies of the Software, and to permit persons to whom the Software is\n\
furnished to do so, subject to the following conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in all\n\
copies or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n\
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n\
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n\
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n\
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n\
SOFTWARE.", "OK");
}

void MyFrame::OnQuit(wxCommandEvent& event)
{
	wxExit();
}

void MyFrame::OnClose(wxCloseEvent& event)
{
	if(Settings::Get()->minimize_on_exit)
		Hide();
	else
		wxExit();
}

void MyFrame::OnOpen(wxCommandEvent& event)
{
	GuiEditor::Get()->OnOpen();
}

void MyFrame::OnSave(wxCommandEvent& event)
{
	GuiEditor::Get()->OnSave();
}

void MyFrame::OnSaveAs(wxCommandEvent& event)
{
	GuiEditor::Get()->OnSaveAs();
}

void MyFrame::OnDestroyAll(wxCommandEvent& event)
{
	GuiEditor::Get()->OnDestroyAll();
}

void MyFrame::OnTimer(wxTimerEvent& event)
{
	MinerWatchdog::Get()->CheckProcessRunning();
	int sel = ctrl->GetSelection();
	HWND foreground = GetForegroundWindow();
	if(sel == 3 && foreground)
	{
		POINT p;
		if(::GetCursorPos(&p))
		{
			if(::ScreenToClient(foreground, &p))
			{
				RECT rect;
				if(GetWindowRect(foreground, &rect))
				{
					int width = rect.right - rect.left;
					int height = rect.bottom - rect.top;
					wxString str = wxString::Format(wxT("Mouse: %d,%d\n\nRect: %d,%d"), p.x, p.y, width, height);
					macro_panel->m_MousePos->SetLabelText(str);
					if(GetAsyncKeyState(VK_SCROLL))
					{
						LOGMSG(normal, str.ToStdString().c_str());
					}
				}
			}
		}
	}
	HandleNotifications();
}

void MyFrame::OnOverlockErrorCheck(wxTimerEvent& event)
{
	MinerWatchdog::Get()->CheckOverclockErrors();
}

void MyFrame::SetIconTooltip(const wxString &str)
{
	if(!tray->SetIcon(wxIcon(wxT("aaaa")), str))
	{
		wxLogError("Could not set icon.");
	}
	SetIcon(wxICON(aaaa));
}

MyFrame::MyFrame(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition,
		wxSize(WINDOW_SIZE_X, WINDOW_SIZE_Y))
{
	tray = new TrayIcon();
	tray->SetMainFrame(this);
	SetIconTooltip(wxT("No measurements"));
	
	m_mgr.SetManagedWindow(this);

	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Quit, "E&xit\tCtrl-E", "Close program")->SetBitmap(wxArtProvider::GetBitmap(wxART_QUIT, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(wxID_OPEN, "&Open file\tCtrl-O", "Open file")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(wxID_SAVE, "&Save file\tCtrl-S", "Save file")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(wxID_SAVEAS, "&Save file As\tCtrl-S", "Save file As other")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(ID_DestroyAll, "&Destroy all widgets\tCtrl-W", "Destroy all widgets")->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_OTHER, FromDIP(wxSize(16, 16))));
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(ID_About, "&About\tCtrl-A", "Read license")->SetBitmap(wxArtProvider::GetBitmap(wxART_HELP_PAGE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuHelp->Append(ID_Help, "&Read help\tCtrl-H", "Read description about this program")->SetBitmap(wxArtProvider::GetBitmap(wxART_HELP, wxART_OTHER, FromDIP(wxSize(16, 16))));
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);

	CreateStatusBar();
	wxString platform = (sizeof(void*) == 4 ? "x86" : "x64");
	SetStatusText("CustomKeyboard " + platform + " v" + COMMIT_TAG);

	main_panel = new MainPanel(this);
	escape_panel = new EscaperPanel(this);
	macro_panel = new MacroPanel(this);
	parser_panel = new ParserPanel(this);
	log_panel = new LogPanel(this);

	wxSize client_size = GetClientSize();
	ctrl = new wxAuiNotebook(this, wxID_ANY, wxPoint(client_size.x, client_size.y), FromDIP(wxSize(430, 200)), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	ctrl->Freeze();
	ctrl->AddPage(main_panel, "Main page", false, wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_OTHER, FromDIP(wxSize(16, 16))));
	config_panel = new ConfigurationPanel(this);
	ctrl->AddPage(config_panel, "Config page", false, wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	editor_panel = new EditorPanel(ctrl);
	ctrl->AddPage(editor_panel, "wxEditor page", false, wxArtProvider::GetBitmap(wxART_PASTE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->AddPage(escape_panel, "C StrEscape", false, wxArtProvider::GetBitmap(wxART_LIST_VIEW, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->AddPage(macro_panel, "Custom Macro", false, wxArtProvider::GetBitmap(wxART_HELP, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->AddPage(parser_panel, "Sturct Parser", false, wxArtProvider::GetBitmap(wxART_EDIT, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->AddPage(log_panel, "Log", false, wxArtProvider::GetBitmap(wxART_TIP, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->Thaw();

	ctrl->SetSelection(Settings::Get()->default_page);
	Show(!Settings::Get()->minimize_on_startup);

	m_timer = new wxTimer(this, ID_UpdateMousePosText);
	Connect(m_timer->GetId(), wxEVT_TIMER, wxTimerEventHandler(MyFrame::OnTimer), NULL, this);
	m_timer->Start(100, false);
	m_octimer = new wxTimer(this, ID_OCTimer);
	Connect(m_octimer->GetId(), wxEVT_TIMER, wxTimerEventHandler(MyFrame::OnOverlockErrorCheck), NULL, this);
	m_octimer->Start(3000, false);
	backup_result = std::make_tuple(0, 0, 0);
}

MacroPanel::MacroPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	m_MousePos = new wxStaticText(this, wxID_ANY, "Pos: 0,0");
	bSizer1->Add(m_MousePos);

	this->SetSizer(bSizer1);
	this->Layout();
	/*
	this->Bind(wxEVT_CHAR_HOOK, &MyPanel::OnKeyDown, this);
	*/
}

void MyFrame::HandleNotifications()
{
	mtx.lock();
	if(std::get<0>(backup_result) != 0)
	{
		switch(std::get<0>(backup_result))
		{
		case 1:
		{
			int64_t time_elapsed = std::get<1>(backup_result);
			size_t file_count = std::get<2>(backup_result);
			ShowNotificaiton("Backup complete", wxString::Format("Backed up %d files in %.3fms", file_count, (double)time_elapsed / 1000000.0), 3, [this](wxCommandEvent& event)
				{

				});
			break;
		}
		case 2:
		{
			int64_t time_elapsed = std::get<1>(backup_result);
			ShowNotificaiton("Screenshot saved", wxString::Format("Screenshot saved in %.3fms", (double)time_elapsed / 1000000.0), 3, [this](wxCommandEvent& event)
				{
					char work_dir[256];
					GetCurrentDirectoryA(sizeof(work_dir), work_dir);
					strncat(work_dir, "\\Screenshots", 12);
					ShellExecuteA(NULL, NULL, work_dir, NULL, NULL, SW_SHOWNORMAL);
				});
			std::get<0>(backup_result) = 0;
			break;
		}
		}
	}
	mtx.unlock();
}

template<typename T> void MyFrame::ShowNotificaiton(const wxString& title, const wxString& message, int timeout, T&& fptr)
{
	wxNotificationMessageBase* m_notif = new wxGenericNotificationMessage(title, message, this, wxICON_INFORMATION);
	m_notif->Show(timeout);
	m_notif->Bind(wxEVT_NOTIFICATION_MESSAGE_CLICK, fptr);
}