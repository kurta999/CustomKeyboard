#include "pch.hpp"
#include "../commitid.h"
#include <hidapi/hidapi.h>

#define HOTKEY_ID_TERMINAL 0x3000		/* any value between 0 and 0xBFFF */
//#define HOTKEY_ID_NUM_LOCK 0x3001		/* any value between 0 and 0xBFFF */

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(ID_Help, MyFrame::OnHelp)
EVT_MENU(ID_About, MyFrame::OnAbout)
EVT_MENU(ID_Quit, MyFrame::OnQuit)
EVT_MENU(wxID_OPEN, MyFrame::OnOpen)
EVT_MENU(wxID_SAVE, MyFrame::OnSave)
EVT_MENU(wxID_SAVEAS, MyFrame::OnSaveAs)
EVT_MENU(ID_DestroyAll, MyFrame::OnDestroyAll)
EVT_MENU(ID_CanLoadTxList, MyFrame::OnCanLoadTxList)
EVT_MENU(ID_CanSaveTxList, MyFrame::OnCanSaveTxList)
EVT_MENU(ID_CanLoadRxList, MyFrame::OnCanLoadRxList)
EVT_MENU(ID_CanSaveRxList, MyFrame::OnCanSaveRxList)
EVT_MENU(ID_CanLoadMapping, MyFrame::OnCanLoadMapping)
EVT_MENU(ID_CanSaveMapping, MyFrame::OnCanSaveMapping)
EVT_MENU(ID_CmdExecutorSave, MyFrame::OnSaveCmdExecutor)
EVT_MENU(ID_BsecSaveCache, MyFrame::OnSaveBsecCache)
EVT_MENU(ID_SaveEverything, MyFrame::OnSaveEverything)
EVT_SIZE(MyFrame::OnSize)
EVT_CLOSE(MyFrame::OnClose)
//EVT_CHAR_HOOK(MyFrame::OnKeyDown)
#ifdef _WIN32
EVT_HOTKEY(HOTKEY_ID_TERMINAL, MyFrame::OnHotkey)
#endif
//EVT_HOTKEY(HOTKEY_ID_NUM_LOCK, MyFrame::OnHotkey)  /* Numlock toggling doesn't work in this way */
wxEND_EVENT_TABLE()

void MyFrame::OnHelp(wxCommandEvent& event)
{
	wxMessageBox("This is a personal project for myself to improve my daily computer usage, particularly with programming and testing\n\
I've implemented things what I really needed to be more productive and accomplish things faster\n\
It's open source, because why not, maybe somebody will benefit from it one day.", "Help");
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxString platform = (sizeof(void*) == 4 ? " x86" : " x64");
	std::string wxwidgets_version = std::format("{}.{}.{}", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER);
	wxMessageBox(wxString("CustomKeyboard") + platform + " v" + COMMIT_TAG + " (" + COMMIT_ID + ")" + "\n\n"
"MIT License\n\
\n\
Copyright (c) 2021 - 2023 kurta999\n\
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
"HIDAPI: " + HID_API_VERSION_STR + "\n" +
"opencv: " + CV_VERSION + "\n" +
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

void MyFrame::OnKeyDown(wxKeyEvent& event)
{
	LOG(LogLevel::Notification, "OnKeyDown");
}

void MyFrame::OnHotkey(wxKeyEvent& evt)
{
	if(evt.GetId() == HOTKEY_ID_TERMINAL)
	{
		TerminalHotkey::Get()->Process();
	}
	/*
	else if(evt.GetId() == HOTKEY_ID_NUM_LOCK)
	{
		if(!wxGetKeyState(WXK_NUMLOCK))
		{
			int flags = evt.GetRawKeyFlags();
#ifdef _WIN32
			INPUT input = { 0 };
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = VK_NUMLOCK;
			input.ki.dwFlags = 0;
			SendInput(1, &input, sizeof(input));
			input.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &input, sizeof(input));
#endif
		}
	}
	*/
}

void MyFrame::OnSize(wxSizeEvent& event)
{
	wxSize a = event.GetSize();
	if(main_panel)
	{
		if(main_panel)
			main_panel->SetSize(a);
		if(config_panel)
		{
			config_panel->SetSize(a);
			if(config_panel->m_notebook)
				config_panel->m_notebook->SetSize(a);
			if(config_panel->comtcp_panel)
				config_panel->comtcp_panel->SetSize(a);
			if(config_panel->keybrd_panel)
				config_panel->keybrd_panel->SetSize(a);
			if(config_panel->backup_panel)
				config_panel->backup_panel->SetSize(a);
		}
		if(editor_panel)
		{
			editor_panel->SetSize(a);
			if(editor_panel->m_notebook)
				editor_panel->m_notebook->SetSize(a);
			if(editor_panel->gui_editor)
				editor_panel->gui_editor->SetSize(a);
			if(editor_panel->gui_cpp)
				editor_panel->gui_cpp->SetSize(a);
		}
		if(escape_panel)
			escape_panel->SetSize(a);
		if(debug_panel)
			debug_panel->SetSize(a);
		if(parser_panel)
			parser_panel->SetSize(a);
		if(log_panel)
			log_panel->SetSize(a);
		if(file_panel)
			file_panel->SetSize(a);
		if(can_panel)
		{
			can_panel->SetSize(a);
			if(can_panel->m_notebook)
				can_panel->m_notebook->SetSize(a);
			if(can_panel->sender)
				can_panel->sender->SetSize(a);
			if(can_panel->log)
				can_panel->log->SetSize(a);
			if(can_panel->script)
				can_panel->script->SetSize(a);
			can_panel->m_notebook->Layout();
		}
		if(modbus_master_panel)
			modbus_master_panel->SetSize(a);
		/*if (data_sender_panel)
			data_sender_panel->SetSize(a);*/
		if(cmd_panel)
			cmd_panel->SetSize(a);
		if(did_panel)
			did_panel->SetSize(a);
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

void MyFrame::OnCanLoadTxList(wxCommandEvent& event)
{
	if(can_panel)
		can_panel->LoadTxList();
}

void MyFrame::OnCanSaveTxList(wxCommandEvent& event)
{
	if(can_panel)
		can_panel->SaveTxList();
}

void MyFrame::OnCanLoadRxList(wxCommandEvent& event)
{
	if(can_panel)
		can_panel->LoadRxList();
}

void MyFrame::OnCanSaveRxList(wxCommandEvent& event)
{
	if(can_panel)
		can_panel->SaveRxList();
}

void MyFrame::OnCanLoadMapping(wxCommandEvent& event)
{
	if(can_panel)
		can_panel->LoadMapping();
}

void MyFrame::OnCanSaveMapping(wxCommandEvent& event)
{
	if(can_panel)
		can_panel->SaveMapping();
}

void MyFrame::OnSaveCmdExecutor(wxCommandEvent& event)
{
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
	bool ret = cmd->Save();
	if(ret)
	{
		std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

		char work_dir[1024] = {};
#ifdef _WIN32
		GetCurrentDirectoryA(sizeof(work_dir) - 1, work_dir);
#endif
		MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
		std::lock_guard lock(frame->mtx);
		frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::CommandsSaved), dif, std::string(work_dir) + "\\Cmds.xml"});
	}
}

void MyFrame::OnSaveBsecCache(wxCommandEvent& event)
{
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	std::unique_ptr<CmdExecutor>& cmd = wxGetApp().cmd_executor;
	bool ret = cmd->Save();
}

void MyFrame::OnSaveEverything(wxCommandEvent& event)
{
	Settings::Get()->SaveFile(false);
	std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;

	if(can_handler)
	{
		std::filesystem::path path = "TxList.xml";
		can_handler->SaveTxList(path);
		path = "RxList.xml";
		can_handler->SaveRxList(path);
		path = "FrameMapping.xml";
		can_handler->SaveMapping(path);
	}

	std::unique_ptr<CmdExecutor>& cmd_executor = wxGetApp().cmd_executor;
	if(cmd_executor)
		cmd_executor->Save();

	MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
	std::lock_guard lock(frame->mtx);
	frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::EverythingSaved) });
}

void MyFrame::On10msTimer(wxTimerEvent& event)
{
	HandleAlwaysOnNumlock();
	if(can_panel)
		can_panel->On10MsTimer();
	if(modbus_master_panel)
		modbus_master_panel->On10MsTimer();

	Logger::Get()->Tick();
}

void MyFrame::On100msTimer(wxTimerEvent& event)
{
	HandleDebugPanelUpdate();
	AntiLock::Get()->Process();
	HandleNotifications();
	HandleBackupProgressDialog();
	HandleCryptoPriceUpdate();
	HandleDidPanelUpdate();
}

void MyFrame::HandleDebugPanelUpdate()
{
	int sel = ctrl->GetSelection();
#ifdef _WIN32
	HWND foreground = GetForegroundWindow();
#else
	bool foreground = true;
#endif
	if((sel == ctrl->FindPage(debug_panel) || MacroRecorder::Get()->IsRecordingMouse()) && foreground)
	{
		if(debug_panel)
			debug_panel->HandleUpdate();
	}
}

void MyFrame::HandleBackupProgressDialog()
{
	if(show_backup_dlg && backup_prog == NULL && !DirectoryBackup::Get()->is_cancelled)
	{
		backup_prog = new wxProgressDialog("Backing up files", 
			"Please wait while files being backed up\nIt can take a few minutes...Be patient", 100, 0, wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_SMOOTH);
		backup_prog->Show();
	}

	if(backup_prog != NULL)
	{
		try
		{
			std::lock_guard lock(DirectoryBackup::Get()->m_TitleMutex);
			std::string current_file = DirectoryBackup::Get()->m_currentFile;
			if(!current_file.empty())
				backup_prog->Pulse(wxString::Format("Please wait while files being backed up\nIt can take a few minutes...Be patient\nCurrent file: %s", current_file));
			if(backup_prog && backup_prog->WasCancelled())
			{
				backup_prog->Destroy();
				backup_prog = NULL;
				DirectoryBackup::Get()->is_cancelled = true;
			}
		}
		catch(std::exception& e) /* TODO: solve possible deadlock in the future, there are no time for tihs right now */
		{
			LOG(LogLevel::Error, "Exception: {}", e.what());
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
{
#ifdef _WIN32
	if(Settings::Get()->always_on_numlock && !wxGetKeyState(WXK_NUMLOCK))  /* WXK_NUMLOCK causes assert failure on linux */
	{
		INPUT input = { 0 };
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = VK_NUMLOCK;
		input.ki.dwFlags = 0;
		SendInput(1, &input, sizeof(input));
		input.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &input, sizeof(input));
	}
#endif
}

void MyFrame::HandleCryptoPriceUpdate()
{
	CryptoPrice::Get()->UpdatePrices();
	if(CryptoPrice::Get()->is_pending)
	{
		if(main_panel)
			main_panel->UpdateCryptoPrices(CryptoPrice::Get()->eth_buy, CryptoPrice::Get()->eth_sell, CryptoPrice::Get()->btc_buy, CryptoPrice::Get()->btc_sell);
		CryptoPrice::Get()->is_pending = false;
	}
}

void MyFrame::HandleDidPanelUpdate()
{
	if(did_panel)
		did_panel->On100msTimer();
}

void MyFrame::RegisterTerminalHotkey(int vkey)
{
#ifdef _WIN32
	wxWindow::UnregisterHotKey(HOTKEY_ID_TERMINAL);
	if(vkey != 0xFFFF)  /* Register hotkey only if specified key is valid */
	{
		bool ret = wxWindow::RegisterHotKey(HOTKEY_ID_TERMINAL, wxMOD_NONE, vkey);
		if(!ret)
			LOG(LogLevel::Error, "Failed to register terminal hotkey!");
	}
#endif
}

void MyFrame::ToggleForegroundVisibility()
{
	bool is_iconized = IsIconized();
	bool is_shown = IsShown();
	if(is_iconized)
	{
		Iconize(false);
		SetFocus();
		Raise();
		Show(true);
	}
	else
	{
		Show(!is_shown);
	}

	if(is_shown)
		Raise();
}

void MyFrame::SetCurrentPage(uint8_t page_id)
{
	if(page_id > ctrl->GetPageCount() - 1)
		page_id = ctrl->GetPageCount() - 1;

	ctrl->SetSelection(page_id);
}

void MyFrame::SetIconTooltip(const wxString &str)
{
#ifdef _WIN32
	if(!tray->SetIcon(wxIcon(wxT("aaaa")), str))
	{
		LOG(LogLevel::Error, "Could not set tray icon.");
	}
	SetIcon(wxICON(aaaa));
#else

#endif
}

MyFrame::MyFrame(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title)
{
	tray = new TrayIcon();
	tray->SetMainFrame(this);
	SetIconTooltip(wxT("No measurements"));
	
	m_mgr.SetManagedWindow(this);
	SetMinSize(wxSize(800, 600));

	m_mgr.SetFlags(wxAUI_MGR_ALLOW_FLOATING);
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_Quit, "E&xit\tCtrl-E", "Close program")->SetBitmap(wxArtProvider::GetBitmap(wxART_QUIT, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(wxID_OPEN, "&Open file\tCtrl-O", "Open file")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(wxID_SAVE, "&Save file\tCtrl-S", "Save file")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(wxID_SAVEAS, "&Save file As\tCtrl-S", "Save file As other")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuFile->Append(ID_DestroyAll, "&Destroy all widgets\tCtrl-W", "Destroy all widgets")->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_OTHER, FromDIP(wxSize(16, 16))));
	wxMenu* menuCan = new wxMenu;
	menuCan->Append(ID_CanLoadTxList, "&Load TX List", "Load CAN TX List")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuCan->Append(ID_CanSaveTxList, "&Save TX List", "Save CAN TX List")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuCan->Append(ID_CanLoadRxList, "&Load RX List", "Load CAN RX List")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuCan->Append(ID_CanSaveRxList, "&Save RX List", "Save CAN RX List")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuCan->Append(ID_CanLoadMapping, "&Load CAN mapping", "Load CAN mapping")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuCan->Append(ID_CanSaveMapping, "&Save CAN mapping", "Save CAN mapping")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuCan->Append(ID_CanSaveAll, "&Save all CAN", "Save TX,RX List & CAN mapping")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuCan->Append(ID_CmdExecutorSave, "&Save CMDs", "Save commands from CMD Executor")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuCan->Append(ID_BsecSaveCache, "&Save BSEC", "Save BSEC Cache")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuCan->Append(ID_SaveEverything, "&Save everything", "Save everything (CAN, CmdExecutor, Settings, etc)")->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(ID_About, "&About", "Read license")->SetBitmap(wxArtProvider::GetBitmap(wxART_HELP_PAGE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	menuHelp->Append(ID_Help, "&Read help\tCtrl-H", "Read description about this program")->SetBitmap(wxArtProvider::GetBitmap(wxART_HELP, wxART_OTHER, FromDIP(wxSize(16, 16))));
	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuCan, "&CAN");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);

	CreateStatusBar();
	wxString platform = (sizeof(void*) == 4 ? "x86" : "x64");
#ifdef DEBUG
	SetStatusText("CustomKeyboard " + platform + " v" + COMMIT_TAG + " DEBUG BUILD " + COMMIT_ID);
#else
	SetStatusText("CustomKeyboard " + platform + " v" + COMMIT_TAG);
#endif

	SetClientSize(Settings::Get()->window_size);

	UsedPages used_pages = Settings::Get()->used_pages;
	if(used_pages.main)
		main_panel = new MainPanel(this);
	if(used_pages.escaper)
		escape_panel = new EscaperPanel(this);
	if(used_pages.debug)
		debug_panel = new DebugPanel(this);
	if(used_pages.struct_parser)
		parser_panel = new ParserPanel(this);
	if(used_pages.file_browser)
		file_panel = new FilePanel(this);
	if(used_pages.cmd_executor)
		cmd_panel = new CmdExecutorPanelBase(this);
	if(used_pages.can)
		can_panel = new CanPanel(this);
	if(used_pages.did)
		did_panel = new DidPanel(this);
	if(used_pages.modbus_master)
		modbus_master_panel = new ModbusMasterPanel(this);
	if (used_pages.data_sender)
		data_sender_panel = new DataSenderPanel(this);
	if(used_pages.log)
		log_panel = new LogPanel(this);
	Logger::Get()->AppendPreinitedEntries();
	
	wxSize client_size = GetClientSize();
	ctrl = new wxAuiNotebook(this, wxID_ANY, wxPoint(client_size.x, client_size.y), FromDIP(wxSize(430, 200)), wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	ctrl->Freeze();
	if(used_pages.main)
		ctrl->AddPage(main_panel, "Main Page", false, wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if(used_pages.config)
	{
		config_panel = new ConfigurationPanel(this);
		ctrl->AddPage(config_panel, "Config", false, wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, FromDIP(wxSize(16, 16))));
	}
	if(used_pages.map_converter)
	{
		map_converter_panel = new MapConverterPanel(this);
		ctrl->AddPage(map_converter_panel, "Map", false, wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_OTHER, FromDIP(wxSize(16, 16))));
	}	
	if(used_pages.wxeditor)
	{
		editor_panel = new EditorPanel(ctrl);
		ctrl->AddPage(editor_panel, "wxEditor", false, wxArtProvider::GetBitmap(wxART_PASTE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	}
	if(used_pages.escaper)
		ctrl->AddPage(escape_panel, "C StrEscape", false, wxArtProvider::GetBitmap(wxART_LIST_VIEW, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if(used_pages.debug)
		ctrl->AddPage(debug_panel, "Debug Page", false, wxArtProvider::GetBitmap(wxART_HELP, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if(used_pages.struct_parser)
		ctrl->AddPage(parser_panel, "Sturct Parser", false, wxArtProvider::GetBitmap(wxART_EDIT, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if(used_pages.file_browser)
		ctrl->AddPage(file_panel, "File Browser", false, wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if(used_pages.cmd_executor)
		ctrl->AddPage(cmd_panel, "CMD Executor", false, wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if(used_pages.can)
		ctrl->AddPage(can_panel, "CAN Sender", false, wxArtProvider::GetBitmap(wxART_REMOVABLE, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if(used_pages.did)
		ctrl->AddPage(did_panel, "DID", false, wxArtProvider::GetBitmap(wxART_FIND, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if(used_pages.modbus_master)
		ctrl->AddPage(modbus_master_panel, "ModbusMaster", false, wxArtProvider::GetBitmap(wxART_PRINT, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if (used_pages.data_sender)
		ctrl->AddPage(data_sender_panel, "DataSender", false, wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_OTHER, FromDIP(wxSize(16, 16))));
	if(used_pages.log)
		ctrl->AddPage(log_panel, "Log", false, wxArtProvider::GetBitmap(wxART_TIP, wxART_OTHER, FromDIP(wxSize(16, 16))));
	ctrl->Thaw();

	SetCurrentPage(Settings::Get()->default_page);
	Show(!Settings::Get()->minimize_on_startup);

	m_10msTimer = new wxTimer(this, ID_10msTimer);
	Connect(m_10msTimer->GetId(), wxEVT_TIMER, wxTimerEventHandler(MyFrame::On10msTimer), NULL, this);
	m_10msTimer->Start(100, false);	
	m_100msTimer = new wxTimer(this, ID_100msTimer);
	Connect(m_100msTimer->GetId(), wxEVT_TIMER, wxTimerEventHandler(MyFrame::On100msTimer), NULL, this);
	m_100msTimer->Start(100, false);

	//wxWindow::RegisterHotKey(HOTKEY_ID_NUM_LOCK, wxMOD_NONE, VK_NUMLOCK);
	//SetClientSize(800, 600);

	is_initialized = true;
}

void MyFrame::HandleNotifications()
{
	std::scoped_lock lock(mtx);
	if(pending_msgs.size() > 0)
	{
		try
		{
			std::vector<std::any> ret = pending_msgs.front();
			std::underlying_type_t<PopupMsgIds> type = std::any_cast<std::underlying_type_t<PopupMsgIds>>(ret[0]);
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
				case ScreenshotSaveFailed:
				{
					ShowNotificaiton("Failed to save the screenshot!", "Error happend while trying to save the screenshot.",
						3, wxICON_ERROR, [this](wxCommandEvent& event)
						{
						});
					break;
				}
				case SettingsSaved:
				{
					ShowNotificaiton("Settings saved", "Settings has been successfully saved", 3, wxICON_INFORMATION, [this](wxCommandEvent& event)
						{
#ifdef _WIN32
							wchar_t work_dir[1024];
							GetCurrentDirectory(WXSIZEOF(work_dir) - 1, work_dir);
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
					size_t dest_count = std::any_cast<decltype(files_size)>(ret[4]);
					std::filesystem::path* p = std::any_cast<decltype(p)>(ret[5]);
					ShowNotificaiton("Backup complete", wxString::Format("Backed up %zu files (%s) to %zu places in %.3fms", file_count, utils::GetDataUnit(files_size), dest_count,
						(double)time_elapsed / 1000000.0),
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
				case TxListLoaded:
				case TxListSaved:
				case RxListLoaded:
				case RxListSaved:
				case FrameMappingLoaded:
				case FrameMappingSaved:
				{
					const wxString msg[] = { "TX List Loaded", "TX List Saved", "RX List Loaded", "RX List Saved", "Frame Mapping Loaded", "Frame Mapping Saved"};
 					ShowNotificaiton(msg[type - TxListLoaded], msg[type - TxListLoaded], 3, wxICON_INFORMATION, [this](wxCommandEvent& event)
						{
						});
					break;
				}
				case TxListLoadError:
				case RxListLoadError:
				case FrameMappingLoadError:
				{
					const wxString msg[] = { "TX List Load failed", "RX List Load failed", "Frame Mapping Load failed"};
					ShowNotificaiton(msg[type - TxListLoadError], msg[type - TxListLoadError], 3, wxICON_ERROR, [this](wxCommandEvent& event)
						{
						});
					break;
				}
				case CanLogSaved:
				{
					int64_t time_elapsed = std::any_cast<decltype(time_elapsed)>(ret[1]);
					std::string filename = std::any_cast<decltype(filename)>(ret[2]);
					ShowNotificaiton("CAN Log saved", wxString::Format("Can log saved in %.3fms\nPath: %s",
						(double)time_elapsed / 1000000.0, filename), 3, wxICON_INFORMATION, [this, filename](wxCommandEvent& event)
						{
#ifdef _WIN32
							char work_dir[1024];
							GetCurrentDirectoryA(sizeof(work_dir) - 1, work_dir);
							std::string abs_file = work_dir + std::string("\\") + filename;

							boost::algorithm::replace_all(abs_file, "/", "\\");  /* Fix for path separator */
							std::string cmdline = std::string("/select,\"" + abs_file + "\"");
							ShellExecuteA(NULL, "open", "explorer.exe", cmdline.c_str(), NULL, SW_NORMAL);
#endif
						});
					break;
				}				
				case ModbusLogSaved:
				{
					int64_t time_elapsed = std::any_cast<decltype(time_elapsed)>(ret[1]);
					std::string filename = std::any_cast<decltype(filename)>(ret[2]);
					ShowNotificaiton("Modbus Log saved", wxString::Format("Modbus log saved in %.3fms\nPath: %s",
						(double)time_elapsed / 1000000.0, filename), 3, wxICON_INFORMATION, [this, filename](wxCommandEvent& event)
						{
#ifdef _WIN32
							char work_dir[1024];
							GetCurrentDirectoryA(sizeof(work_dir) - 1, work_dir);
							std::string abs_file = work_dir + std::string("\\") + filename;

							boost::algorithm::replace_all(abs_file, "/", "\\");  /* Fix for path separator */
							std::string cmdline = std::string("/select,\"" + abs_file + "\"");
							ShellExecuteA(NULL, "open", "explorer.exe", cmdline.c_str(), NULL, SW_NORMAL);
#endif
						});
					break;
				}				
				case CommandsSaved:
				{
					int64_t time_elapsed = std::any_cast<decltype(time_elapsed)>(ret[1]);
					std::string filename = std::any_cast<decltype(filename)>(ret[2]);
					ShowNotificaiton("Commands saved", wxString::Format("Commands saved in %.3fms\nPath: %s",
						(double)time_elapsed / 1000000.0, filename), 3, wxICON_INFORMATION, [this, filename](wxCommandEvent& event)
						{
#ifdef _WIN32
							std::string cmdline = std::string("/select,\"" + filename);
							ShellExecuteA(NULL, "open", "explorer.exe", cmdline.c_str(), NULL, SW_NORMAL);
#endif
						});
					break;
				}
				case DidCacheSaved:
				{
					int64_t time_elapsed = std::any_cast<decltype(time_elapsed)>(ret[1]);
					std::string filename = std::any_cast<decltype(filename)>(ret[2]);
					ShowNotificaiton("DIDs cache saved", wxString::Format("DIDs cache saved in %.3fms\nPath: %s",
						(double)time_elapsed / 1000000.0, filename), 3, wxICON_INFORMATION, [this, filename](wxCommandEvent& event)
						{
#ifdef _WIN32
							std::string cmdline = std::string("/select,\"" + filename);
							ShellExecuteA(NULL, "open", "explorer.exe", cmdline.c_str(), NULL, SW_NORMAL);
#endif
						});
					break;
				}				
				case DidUpdated:
				{
					ShowNotificaiton("DID updated", wxString::Format("DID value has been updated!"), 3, wxICON_INFORMATION, [this](wxCommandEvent& event)
						{

						});
					break;
				}
				case SelectedLogsCopied:
				{
					ShowNotificaiton("Logs copied", wxString::Format("Selected logs copied to clipboard"), 3, wxICON_INFORMATION, [this](wxCommandEvent& event)
						{
						});
					break;
				}
				case EverythingSaved:
				{
					ShowNotificaiton("Configurations saved", wxString::Format("Every configuration has been saved"), 3, wxICON_INFORMATION, [this](wxCommandEvent& event)
						{
							wchar_t work_dir[1024] = {};
							GetCurrentDirectoryW(WXSIZEOF(work_dir) - 1, work_dir);
							ShellExecuteW(NULL, NULL, work_dir, NULL, NULL, SW_SHOWNORMAL);
						});
					break;
				}
			}
		}
		catch(const std::exception& e)
		{
			LOG(LogLevel::Error, "Exception: {}", e.what());
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