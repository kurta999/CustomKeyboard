#pragma once

#include <wx/taskbar.h>

class MyFrame;

class MenuEventFilter : public wxEventFilter  /* without this event filter doing backups from tray is not possible due to dynamic menu ids */
{
public:
	MenuEventFilter()
	{
		wxEvtHandler::AddFilter(this);
	}
	virtual ~MenuEventFilter()
	{
		wxEvtHandler::RemoveFilter(this);
	}
	int FilterEvent(wxEvent& event) override;
};

class TrayIcon : public wxTaskBarIcon
{
public:
	TrayIcon(void)
	{

	}
	~TrayIcon(void);

	void SetMainFrame(MyFrame* frame);

	void OnLeftDoubleClick(wxTaskBarIconEvent& event);
	void OnOpenScreenshots(wxCommandEvent& event);
	void OnReload(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnBackup(wxCommandEvent& event);
	virtual wxMenu* CreatePopupMenu();

	enum ID
	{
		Exit = 0,
		ReloadConfig,
		OpenScreenshots,
		DoBackup = 1500,
	};
	static int max_backups;
protected:
	MyFrame* mainFrame;
private:
	MenuEventFilter* filter;

	DECLARE_EVENT_TABLE()
};