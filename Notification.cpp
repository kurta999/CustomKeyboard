#include "Notification.h"
#include "main_frame.h"

#include <wx/menu.h>

#include <array>
#include <variant>
#include "CustomMacro.h"
#include "Settings.h"

BEGIN_EVENT_TABLE(NotificationIcon, wxTaskBarIcon)
EVT_TASKBAR_LEFT_DCLICK(NotificationIcon::OnLeftDoubleClick)
EVT_MENU(NotificationIcon::ID::ReloadConfig, NotificationIcon::OnReload)
EVT_MENU(NotificationIcon::ID::Exit, NotificationIcon::OnQuit)
END_EVENT_TABLE()
/*
NotificationIcon::NotificationIcon(void) : wxTaskBarIcon()
{	
	mainFrame = NULL;
}
*/

NotificationIcon::~NotificationIcon(void) {
}

void NotificationIcon::SetMainFrame(MyFrame* frame)
{
	mainFrame = frame;
}

void NotificationIcon::OnLeftDoubleClick(wxTaskBarIconEvent& event) 
{
	if (mainFrame)
		mainFrame->Show(!mainFrame->IsShown());
}

wxMenu* NotificationIcon::CreatePopupMenu() 
{
	wxMenu* popup = new wxMenu;

	popup->Append(NotificationIcon::ID::ReloadConfig, wxT("Reload config"));
	popup->AppendSeparator();
	popup->Append(NotificationIcon::ID::Exit, wxT("E&xit"));
	return popup;
}

void NotificationIcon::OnReload(wxCommandEvent& WXUNUSED(event))
{
	Settings::Get()->LoadFile();
}

void NotificationIcon::OnQuit(wxCommandEvent& WXUNUSED(event)) 
{
	RemoveIcon();
	if (mainFrame)
		wxExit();
}