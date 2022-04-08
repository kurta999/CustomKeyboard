#include "pch.hpp"
#include "../commitid.h"

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(ID_Help, MyFrame::OnHelp)
EVT_MENU(ID_About, MyFrame::OnAbout)
EVT_MENU(ID_Quit, MyFrame::OnQuit)
EVT_MENU(wxID_OPEN, MyFrame::OnOpen)
EVT_MENU(wxID_SAVE, MyFrame::OnSave)
EVT_MENU(wxID_SAVEAS, MyFrame::OnSaveAs)
EVT_MENU(ID_DestroyAll, MyFrame::OnDestroyAll)
EVT_SIZE(MyFrame::OnSize)
EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MainPanel, wxPanel)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MacroPanel, wxPanel)
wxEND_EVENT_TABLE()

void MyFrame::OnHelp(wxCommandEvent& event)
{
	wxMessageBox("This is a simple program which made for improving my coding experience.\n\
		Feel free to re(use) it in any way what you want.", "Help");
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxString platform = (sizeof(void*) == 4 ? " x86" : " x64");
	std::string fmt_version = std::to_string(FMT_VERSION);
	std::string wxwidgets_version = fmt::format("{}.{}.{}", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER);
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
SOFTWARE." + "\n\nUsed 3rd party libraries:\n"
"SQLite: " + SQLITE_VERSION + "\n" +
"wxWidgets: " + wxwidgets_version + "\n" +
"boost: " + BOOST_LIB_VERSION + "\n" +
"lodepng: " + LODEPNG_VERSION_STRING + "\n" +
"fmt: " + fmt_version + "\n\n" +
"Build info:\n" +
"Compiler: " + BOOST_COMPILER + "\n"
"Built on: " + __TIMESTAMP__, "OK");
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

void MyFrame::OnSize(wxSizeEvent& event)
{
	wxSize a = event.GetSize();
	if(main_panel)
	{
		main_panel->SetSize(a);
		config_panel->SetSize(a);
		config_panel->m_notebook->SetSize(a);
		config_panel->comtcp_panel->SetSize(a);
		config_panel->keybrd_panel->SetSize(a);
		config_panel->backup_panel->SetSize(a);
		editor_panel->SetSize(a);
		editor_panel->m_notebook->SetSize(a);
		editor_panel->gui_editor->SetSize(a);
		editor_panel->gui_cpp->SetSize(a);
		escape_panel->SetSize(a);
		parser_panel->SetSize(a);
		log_panel->SetSize(a);
		file_panel->SetSize(a);
	}
	event.Skip(true);
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

void MyFrame::On10msTimer(wxTimerEvent& event)
{
	HandleAlwaysOnNumlock();
}

void MyFrame::On100msTimer(wxTimerEvent& event)
{
#ifdef DBG_FOREGROUND_PRINT
	{
		HWND foreground = GetForegroundWindow();
		if(foreground)
		{
			char window_title[256];
			GetWindowTextA(foreground, window_title, 256);
			DBG("fgw: %s\n", window_title);
		}
	}
#endif

#ifdef _WIN32
	int sel = ctrl->GetSelection();
	HWND foreground = GetForegroundWindow();
	if((sel == 4 || MacroRecorder::Get()->IsRecordingMouse()) && foreground)
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
						MacroRecorder::Get()->MarkMousePosition((LPPOINT*)&p);
					}
				}
			}
		}
	}
#else

#endif

#ifdef _WIN32
	AntiLock::Get()->Process();
	TerminalHotkey::Get()->Process();
#endif
	HandleNotifications();
	HandleBackupProgressDialog();
#ifdef _WIN32
	TerminalHotkey::Get()->Process();
#endif
}

void MyFrame::HandleBackupProgressDialog()
{
	if(show_backup_dlg && backup_prog == NULL)
	{
		backup_prog = new wxProgressDialog("Backing up files", 
			"Please wait while files being backed up\nIt can take a few minutes...Be patient", 100, 0, wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_SMOOTH);
		backup_prog->Show();
	}

	if(backup_prog != NULL)
	{
		backup_prog->Pulse();
		if(backup_prog && backup_prog->WasCancelled())
		{
			backup_prog->Destroy();
			backup_prog = NULL;
		}
	}

	if(!show_backup_dlg && backup_prog != NULL)
	{
		show_backup_dlg = false;
		backup_prog->Destroy();
		backup_prog = NULL;
	}
}

void MyFrame::HandleAlwaysOnNumlock()
{    /* TODO: rewrite this with LL Keyboard Hook in future */
	if(Settings::Get()->always_on_numlock && !wxGetKeyState(WXK_NUMLOCK))
	{
		#ifdef _WIN32
		INPUT input = { 0 };
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_NUMLOCK;
		input.ki.dwFlags = 0;
		SendInput(1, &input, sizeof(input));
		input.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &input, sizeof(input));
		#else
		/* TODO: Handle numlock on on linux */
		#endif
	}
}

void MyFrame::SetIconTooltip(const wxString &str)
{
#ifdef _WIN32
	if(!tray->SetIcon(wxIcon(wxT("aaaa")), str))
	{
		LOGMSG(error, "Could not set tray icon.");
	}
	SetIcon(wxICON(aaaa));
#else

#endif
}

MyFrame::MyFrame(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, Settings::Get()->window_size)
{
	tray = new TrayIcon();
	tray->SetMainFrame(this);
	SetIconTooltip(wxT("No measurements"));
	
	m_mgr.SetManagedWindow(this);
	SetMinSize(wxSize(800, 600));

	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Quit, "E&xit\tCtrl-E", "Close program")->SetBitmap(wxArtProvider::GetBitmap(wxART_QUIT, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(wxID_OPEN, "&Open file\tCtrl-O", "Open file")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(wxID_SAVE, "&Save file\tCtrl-S", "Save file")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(wxID_SAVEAS, "&Save file As\tCtrl-S", "Save file As other")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(ID_DestroyAll, "&Destroy all widgets\tCtrl-W", "Destroy all widgets")->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_OTHER, FromDIP(wxSize(16, 16))));
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(ID_About, "&About", "Read license")->SetBitmap(wxArtProvider::GetBitmap(wxART_HELP_PAGE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuHelp->Append(ID_Help, "&Read help\tCtrl-H", "Read description about this program")->SetBitmap(wxArtProvider::GetBitmap(wxART_HELP, wxART_OTHER, FromDIP(wxSize(16, 16))));
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);

	CreateStatusBar();
	wxString platform = (sizeof(void*) == 4 ? "x86" : "x64");
#ifdef DEBUG
	SetStatusText("CustomKeyboard " + platform + " v" + COMMIT_TAG + " DEBUG BUILD " + COMMIT_ID);
#else
	SetStatusText("CustomKeyboard " + platform + " v" + COMMIT_TAG);
#endif

	main_panel = new MainPanel(this);
	escape_panel = new EscaperPanel(this);
	macro_panel = new MacroPanel(this);
	parser_panel = new ParserPanel(this);
	file_panel = new FilePanel(this);
	log_panel = new LogPanel(this);
	Logger::Get()->AppendPreinitedEntries();

	wxSize client_size = GetClientSize();
	ctrl = new wxAuiNotebook(this, wxID_ANY, wxPoint(client_size.x, client_size.y), FromDIP(wxSize(430, 200)), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	ctrl->Freeze();
	ctrl->AddPage(main_panel, "Main page", false, wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_OTHER, FromDIP(wxSize(16, 16))));
	config_panel = new ConfigurationPanel(this);
	ctrl->AddPage(config_panel, "Config page", false, wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	editor_panel = new EditorPanel(ctrl);
	ctrl->AddPage(editor_panel, "wxEditor page", false, wxArtProvider::GetBitmap(wxART_PASTE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->AddPage(escape_panel, "C StrEscape", false, wxArtProvider::GetBitmap(wxART_LIST_VIEW, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->AddPage(macro_panel, "Mouse Info", false, wxArtProvider::GetBitmap(wxART_HELP, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->AddPage(parser_panel, "Sturct Parser", false, wxArtProvider::GetBitmap(wxART_EDIT, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->AddPage(file_panel, "File browser", false, wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->AddPage(log_panel, "Log", false, wxArtProvider::GetBitmap(wxART_TIP, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->Thaw();

	if(Settings::Get()->default_page > ctrl->GetPageCount() - 1)
		Settings::Get()->default_page = ctrl->GetPageCount() - 1;

	ctrl->SetSelection(Settings::Get()->default_page);
	Show(!Settings::Get()->minimize_on_startup);

	m_10msTimer = new wxTimer(this, ID_10msTimer);
	Connect(m_10msTimer->GetId(), wxEVT_TIMER, wxTimerEventHandler(MyFrame::On10msTimer), NULL, this);
	m_10msTimer->Start(100, false);	
	m_100msTimer = new wxTimer(this, ID_100msTimer);
	Connect(m_100msTimer->GetId(), wxEVT_TIMER, wxTimerEventHandler(MyFrame::On100msTimer), NULL, this);
	m_100msTimer->Start(100, false);
}

MacroPanel::MacroPanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
	
	m_MousePos = new wxStaticText(this, wxID_ANY, "Pos: 0,0");
	bSizer1->Add(m_MousePos);

	this->SetSizer(bSizer1);
	this->Layout();
}

void MyFrame::HandleNotifications()
{
	std::lock_guard lock(mtx);
	if(pending_msgs.size() > 0)
	{
		std::vector<std::any> ret = pending_msgs.front();
		uint8_t type = std::any_cast<uint8_t>(ret[0]);
		switch(type)
		{
			case ScreenshotSaved:
			{
				int64_t time_elapsed = std::any_cast<decltype(time_elapsed)>(ret[1]);
				std::string filename = std::any_cast<decltype(filename)>(ret[2]);
				ShowNotificaiton("Screenshot saved", wxString::Format("Screenshot saved in %.3fms\nPath: %s", 
					(double)time_elapsed / 1000000.0, filename), 3, wxICON_INFORMATION, [this, filename](wxCommandEvent& event)
					{
#ifdef _WIN32
						std::string cmdline = std::string("/select,\"" + filename);
						ShellExecuteA(NULL, "open", "explorer.exe", cmdline.c_str(), NULL, SW_NORMAL);
#endif
					});
				break;
			}
			case SettingsSaved:
			{
				ShowNotificaiton("Settings saved", "Settings has been successfully saved", 3, wxICON_INFORMATION, [this](wxCommandEvent& event)
					{
#ifdef _WIN32
						wchar_t work_dir[1024];
						GetCurrentDirectory(sizeof(work_dir), work_dir);
						StrCatW(work_dir, L"\\settings.ini");
						ShellExecute(NULL, L"open", work_dir, NULL, NULL, SW_SHOW);
#endif
					});
				break;
			}
			case StringEscaped:
			{
				ShowNotificaiton("String escaped", "String has been escaped and placed to clipboard", 
					3, wxICON_INFORMATION, [this](wxCommandEvent& event)
					{
					});
				break;
			}
			case PathSeparatorsReplaced:
			{
				std::string path = std::any_cast<decltype(path)>(ret[1]);
				ShowNotificaiton("Path separator replaced", wxString::Format("New form is in the clipboard:\n%s", path.substr(0, 64)), 
					3, wxICON_INFORMATION, [this](wxCommandEvent& event)
					{
					});
				break;
			}
			case BackupCompleted:
			{
				int64_t time_elapsed = std::any_cast<decltype(time_elapsed)>(ret[1]);
				size_t file_count = std::any_cast<decltype(file_count)>(ret[2]);
				size_t files_size = std::any_cast<decltype(files_size)>(ret[3]);
				std::filesystem::path* p = std::any_cast<decltype(p)>(ret[4]);
				ShowNotificaiton("Backup complete", wxString::Format("Backed up %zu files (%s) in %.3fms", file_count, utils::GetDataUnit(files_size), (double)time_elapsed / 1000000.0),
					3, wxICON_INFORMATION, [this, p](wxCommandEvent& event)
					{
#ifdef _WIN32
						ShellExecuteA(NULL, NULL, p->generic_string().c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
					});
				break;
			}
			case BackupFailed:
			{
				std::filesystem::path* p = std::any_cast<decltype(p)>(ret[1]);
				ShowNotificaiton("Backup failed!",
					wxString::Format("Backup failed due to wrong checksum values\nMake sure that your drive is not damaged\nCheck log file for more info"), 
					3, wxICON_ERROR, [this, p](wxCommandEvent& event)
					{
#ifdef _WIN32
						ShellExecuteA(NULL, NULL, p->generic_string().c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
					});
				break;
			}	
			case LinkMark:
			{
				uint32_t files_marked = std::any_cast<decltype(files_marked)>(ret[1]);
				ShowNotificaiton("Selected files have been marked", wxString::Format("%d files has been marked.\n\
Press KEY %s in destination directory for creating symlinks\nPress KEY %s in destination directory for creating hardlinks\n\n\
If you accidentaly marked the files, click here to unmark them", 
					files_marked, SymlinkCreator::Get()->place_symlink_key, SymlinkCreator::Get()->place_hardlink_key), 
					3, wxICON_INFORMATION, [this](wxCommandEvent& event)
					{
						SymlinkCreator::Get()->UnmarkFiles();
					});
				break;
			}
			case LinkMarkError:
			{
				ShowNotificaiton("Error with symlink marking", "Error happend while marking source for link creation", 
					3, wxICON_ERROR, [this](wxCommandEvent& event)
					{
					});
				break;
			}
			case SymlinkCreated:
			case HardlinkCreated:
			{
				uint32_t files_marked = std::any_cast<decltype(files_marked)>(ret[1]);
				ShowNotificaiton(wxString::Format("%s has been created", type == SymlinkCreated ? "Simlinks" : "Hardlinks"),
					wxString::Format("%d %s has been created succesfully!\n", files_marked, type == SymlinkCreated ? "Simlinks" : "Hardlinks"), 
					3, wxICON_INFORMATION, [this](wxCommandEvent& event)
					{
					});
				break;
			}
			case MacroRecordingStarted:
			{
				ShowNotificaiton("Macro recording started", wxString::Format("\
Start typing & pressing key combinations\nFor getting mouse position, press SCROLL LOCK\nClick again on macro recording icon to stop the recording\n\
Note: Macro recording is in WIP phase, so problems can happen!"), 5, wxICON_INFORMATION, [this](wxCommandEvent& event)
					{
					});
				break;
			}
			case MacroRecordingStopped:
			{
				ShowNotificaiton("Macro recording stopped", wxString::Format("5 macro has been successfully recorded"), 3, wxICON_INFORMATION, [this](wxCommandEvent& event)
					{
					});
				break;
			}

		}
		pending_msgs.pop_front();
	}
}

template<typename T> void MyFrame::ShowNotificaiton(const wxString& title, const wxString& message, int timeout, int flags, T&& fptr)
{
	wxNotificationMessageBase* m_notif = new wxGenericNotificationMessage(title, message, this, flags);
	m_notif->Show(timeout);
	m_notif->Bind(wxEVT_NOTIFICATION_MESSAGE_CLICK, fptr);
}