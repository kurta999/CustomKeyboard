#include "TrayIcon.h"
#include "main_frame.h"

#include <wx/menu.h>

#include <array>
#include <variant>
#include "../CustomMacro.h"
#include "../Settings.h"

BEGIN_EVENT_TABLE(TrayIcon, wxTaskBarIcon)
EVT_TASKBAR_LEFT_DCLICK(TrayIcon::OnLeftDoubleClick)
EVT_MENU(TrayIcon::ID::ReloadConfig, TrayIcon::OnReload)
EVT_MENU(TrayIcon::ID::Exit, TrayIcon::OnQuit)
END_EVENT_TABLE()
/*
TrayIcon::TrayIcon(void) : wxTaskBarIcon()
{	
	mainFrame = NULL;
}
*/

TrayIcon::~TrayIcon(void) {
}

void TrayIcon::SetMainFrame(MyFrame* frame)
{
	mainFrame = frame;
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
	}
}

wxMenu* TrayIcon::CreatePopupMenu()
{
	wxMenu* popup = new wxMenu;

	popup->Append(TrayIcon::ID::ReloadConfig, wxT("Reload config"));
	popup->AppendSeparator();
	popup->Append(TrayIcon::ID::Exit, wxT("E&xit"));
	return popup;
}

void TrayIcon::OnReload(wxCommandEvent& WXUNUSED(event))
{
	Settings::Get()->LoadFile();
}

void TrayIcon::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	RemoveIcon();
	if (mainFrame)
		wxExit();
}