#pragma once

#include <wx/taskbar.h>

class MyFrame;

class NotificationIcon : public wxTaskBarIcon 
{
public:
	NotificationIcon(void)
	{

	}
	~NotificationIcon(void);

	void SetMainFrame(MyFrame* frame);

	void OnLeftDoubleClick(wxTaskBarIconEvent& event);
	void OnReload(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	virtual wxMenu* CreatePopupMenu();

	enum ID
	{
		Exit = 0,
		ReloadConfig
	};
protected:

	MyFrame* mainFrame;
private:
	DECLARE_EVENT_TABLE()

};