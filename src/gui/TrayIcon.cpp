#include "pch.hpp"

BEGIN_EVENT_TABLE(TrayIcon, wxTaskBarIcon)
EVT_TASKBAR_LEFT_DCLICK(TrayIcon::OnLeftDoubleClick)
EVT_MENU(TrayIcon::ID::OpenScreenshots, TrayIcon::OnOpenScreenshots)
EVT_MENU(TrayIcon::ID::OpenRootFolder, TrayIcon::OnOpenRootFolder)
EVT_MENU(TrayIcon::ID::ReloadConfig, TrayIcon::OnReload)
EVT_MENU(TrayIcon::ID::ToggleAntilock, TrayIcon::OnToggleAntiLock)
EVT_MENU(TrayIcon::ID::Exit, TrayIcon::OnQuit)
END_EVENT_TABLE()

int TrayIcon::max_backups = 0;
int TrayIcon::max_alarms = 0;

int MenuEventFilter::FilterEvent(wxEvent& event)
{
	const wxEventType t = event.GetEventType();
	auto obj = event.GetEventObject();
	if(t == wxEVT_MENU)
	{
		const int id = event.GetId();
		if(id >= TrayIcon::ID::DoBackup && id <= TrayIcon::ID::DoBackup + TrayIcon::max_backups)
		{
			int backup_id = id - TrayIcon::ID::DoBackup;
			DirectoryBackup::Get()->BackupFile(backup_id);
			return Event_Processed;
		}
		else if(id >= TrayIcon::ID::DoAlarm && id <= TrayIcon::ID::DoAlarm + TrayIcon::max_backups)
		{
			int alarm_id = id - TrayIcon::ID::DoAlarm;

			std::unique_ptr<AlarmEntryHandler>& alarm_entry = wxGetApp().alarm_entry;
			alarm_entry->HandleKeypress(alarm_entry->entries[alarm_id]->trigger_key, true);
			return Event_Processed;
		}
	}
	return Event_Skip;  /* Continue processing the event normally as well */
}

/*
TrayIcon::TrayIcon(void) : wxTaskBarIcon()
{	
	mainFrame = NULL;
}
*/

TrayIcon::~TrayIcon(void) 
{
	delete filter;
}

void TrayIcon::SetMainFrame(MyFrame* frame)
{
	mainFrame = frame;
	filter = new MenuEventFilter();
}

void TrayIcon::OnLeftDoubleClick(wxTaskBarIconEvent& event)
{
	if(mainFrame)
		mainFrame->ToggleForegroundVisibility();
}

wxMenu* TrayIcon::CreatePopupMenu()
{
	wxMenu* popup = new wxMenu;  /* no memory leak here, wxWidgets takes care about it */
	max_backups = 0;
	for(auto& i : DirectoryBackup::Get()->backups)
	{
		wxMenuItem* item = popup->Append(TrayIcon::ID::DoBackup + max_backups++, i->from.filename().generic_string());
		item->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW_DIR, wxART_OTHER, mainFrame->GetMainWindowOfCompositeControl()->FromDIP(wxSize(14, 14))));
		if(DirectoryBackup::Get()->IsInProgress())
			item->Enable(false);
	}
	popup->AppendSeparator();

	std::unique_ptr<AlarmEntryHandler>& alarm_entry = wxGetApp().alarm_entry;
	max_alarms = 0;
	for(auto& i : alarm_entry->entries)
	{
		wxMenuItem* item = popup->Append(TrayIcon::ID::DoAlarm + max_alarms++, i->name);
		item->SetBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_OTHER, mainFrame->GetMainWindowOfCompositeControl()->FromDIP(wxSize(14, 14))));
		if(DirectoryBackup::Get()->IsInProgress())
			item->Enable(false);
	}

	popup->AppendSeparator();
	popup->Append(TrayIcon::ID::OpenScreenshots, wxT("Open screenshots"))->SetBitmap(wxArtProvider::GetBitmap(wxART_FLOPPY, wxART_OTHER, mainFrame->GetMainWindowOfCompositeControl()->FromDIP(wxSize(14, 14))));
	popup->Append(TrayIcon::ID::OpenRootFolder, wxT("Open Root folder"))->SetBitmap(wxArtProvider::GetBitmap(wxART_HARDDISK, wxART_OTHER, mainFrame->GetMainWindowOfCompositeControl()->FromDIP(wxSize(14, 14))));
	popup->AppendSeparator();
	popup->Append(TrayIcon::ID::ReloadConfig, wxT("Reload config"))->SetBitmap(wxArtProvider::GetBitmap(wxART_TIP, wxART_OTHER, mainFrame->GetMainWindowOfCompositeControl()->FromDIP(wxSize(14, 14))));
	popup->AppendSeparator();
	popup->Append(TrayIcon::ID::ToggleAntilock, wxT("Toggle AntiLock"))->SetBitmap(wxArtProvider::GetBitmap(wxART_CDROM, wxART_OTHER, mainFrame->GetMainWindowOfCompositeControl()->FromDIP(wxSize(14, 14))));
	popup->AppendSeparator();
	popup->Append(TrayIcon::ID::Exit, wxT("E&xit"))->SetBitmap(wxArtProvider::GetBitmap(wxART_QUIT, wxART_OTHER, mainFrame->GetMainWindowOfCompositeControl()->FromDIP(wxSize(14, 14))));
	return popup;
}

void TrayIcon::OnOpenScreenshots(wxCommandEvent& WXUNUSED(event))
{
#ifdef _WIN32
	ShellExecuteW(NULL, NULL, PrintScreenSaver::Get()->screenshot_path.generic_wstring().c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
}

void TrayIcon::OnOpenRootFolder(wxCommandEvent& WXUNUSED(event))
{
#ifdef _WIN32
	wchar_t work_dir[1024] = {};
	GetCurrentDirectoryW(WXSIZEOF(work_dir) - 1, work_dir);
	ShellExecuteW(NULL, NULL, work_dir, NULL, NULL, SW_SHOWNORMAL);
#endif
}

void TrayIcon::OnReload(wxCommandEvent& WXUNUSED(event))
{
	LOG(LogLevel::Verbose, "reload 1");
	Settings::Get()->LoadFile();
	LOG(LogLevel::Verbose, "reload 2");
	std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
	can_handler->LoadFiles();
	LOG(LogLevel::Verbose, "reload 3");
	mainFrame->main_panel->UpdateKeybindings();
	LOG(LogLevel::Verbose, "reload 3-2");
	mainFrame->main_panel->UpdateWorkingDays();
	LOG(LogLevel::Verbose, "reload 4");
	mainFrame->config_panel->UpdateSubpanels();
	LOG(LogLevel::Verbose, "reload 5");
	mainFrame->can_panel->RefreshSubpanels();
	LOG(LogLevel::Verbose, "reload 6");
	mainFrame->cmd_panel->ReloadCommands();
	LOG(LogLevel::Normal, "Settings has been reloaded");
	mainFrame->SetCurrentPage(Settings::Get()->default_page);
}

void TrayIcon::OnToggleAntiLock(wxCommandEvent& WXUNUSED(event))
{
	AntiLock::Get()->is_suspended ^= 1;
	LOG(LogLevel::Verbose, "AntiLock suspended: {}", AntiLock::Get()->is_suspended);
}

void TrayIcon::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	RemoveIcon();
	if (mainFrame)
		wxExit();
}