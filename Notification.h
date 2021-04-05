#ifndef NOTIFICATIONICON_H
#define NOTIFICATIONICON_H

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
	void OnQuit(wxCommandEvent& event);
	virtual wxMenu* CreatePopupMenu();
protected:
	static const int PopupExitID;

	MyFrame* mainFrame;
private:
	DECLARE_EVENT_TABLE()

};

#endif