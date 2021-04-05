#include "Notification.h"
#include "main_frame.h"

#include <wx/menu.h>

const int NotificationIcon::PopupExitID = wxID_HIGHEST + 1;

BEGIN_EVENT_TABLE(NotificationIcon, wxTaskBarIcon)
EVT_TASKBAR_LEFT_DCLICK(NotificationIcon::OnLeftDoubleClick)
EVT_MENU(PopupExitID, NotificationIcon::OnQuit)
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

	popup->AppendSeparator();
	popup->Append(PopupExitID, wxT("E&xit"));
	return popup;
}

void NotificationIcon::OnQuit(wxCommandEvent& WXUNUSED(event)) 
{
	RemoveIcon();
	if (mainFrame)
		mainFrame->Close(true);
}