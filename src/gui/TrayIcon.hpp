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
	void OnOpenRootFolder(wxCommandEvent& event);
	void OnReload(wxCommandEvent& event);
	void OnToggleAntiLock(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	virtual wxMenu* CreatePopupMenu();

	enum ID
	{
		Exit = 0,
		ReloadConfig,
		OpenScreenshots,
		OpenRootFolder,
		ToggleAntilock,
		DoBackup = 1500,
		DoAlarm = 1600,
	};
	static int max_backups;
	static int max_alarms;
protected:
	MyFrame* mainFrame = nullptr;
private:
	MenuEventFilter* filter = nullptr;

	DECLARE_EVENT_TABLE()
};