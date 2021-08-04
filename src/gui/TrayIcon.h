#pragma once

#include <wx/taskbar.h>

class MyFrame;

class TrayIcon : public wxTaskBarIcon
{
public:
	TrayIcon(void)
	{

	}
	~TrayIcon(void);

	void SetMainFrame(MyFrame* frame);

	void OnLeftDoubleClick(wxTaskBarIconEvent& event);
	void OnReload(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnBackup(wxCommandEvent& event);
	virtual wxMenu* CreatePopupMenu();

	enum ID
	{
		Exit = 0,
		ReloadConfig,
		DoBackup,
	};
protected:

	MyFrame* mainFrame;
private:
	DECLARE_EVENT_TABLE()

};