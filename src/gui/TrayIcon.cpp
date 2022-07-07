#include "pch.hpp"

BEGIN_EVENT_TABLE(TrayIcon, wxTaskBarIcon)
EVT_TASKBAR_LEFT_DCLICK(TrayIcon::OnLeftDoubleClick)
EVT_MENU(TrayIcon::ID::OpenScreenshots, TrayIcon::OnOpenScreenshots)
EVT_MENU(TrayIcon::ID::ReloadConfig, TrayIcon::OnReload)
EVT_MENU(TrayIcon::ID::Exit, TrayIcon::OnQuit)
END_EVENT_TABLE()

int TrayIcon::max_backups = 0;

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
	{
		bool is_iconized = mainFrame->IsIconized();
		bool is_shown = mainFrame->IsShown();
		if(is_iconized)
		{
			mainFrame->Iconize(false);
			mainFrame->Show(true);
		}
		else
		{
			mainFrame->Show(!is_shown);
		}

		if(is_shown)
			mainFrame->Raise();
	}
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
	popup->Append(TrayIcon::ID::OpenScreenshots, wxT("Open screenshots"))->SetBitmap(wxArtProvider::GetBitmap(wxART_FLOPPY, wxART_OTHER, mainFrame->GetMainWindowOfCompositeControl()->FromDIP(wxSize(14, 14))));
	popup->AppendSeparator();
	popup->Append(TrayIcon::ID::ReloadConfig, wxT("Reload config"))->SetBitmap(wxArtProvider::GetBitmap(wxART_TIP, wxART_OTHER, mainFrame->GetMainWindowOfCompositeControl()->FromDIP(wxSize(14, 14))));
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

void TrayIcon::OnReload(wxCommandEvent& WXUNUSED(event))
{
	Settings::Get()->LoadFile();
	mainFrame->main_panel->UpdateKeybindings();
	mainFrame->config_panel->UpdateSubpanels();
	mainFrame->can_panel->RefreshSubpanels();
}

void TrayIcon::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	RemoveIcon();
	if (mainFrame)
		wxExit();
}