#pragma once

#include "MainPanel.h"
#include "EscaperPanel.h"
#include "ParserPanel.h"
#include "LogPanel.h"
#include "Editor.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/stc/stc.h>
#include <wx/filepicker.h>

#include <mutex>


class MacroPanel : public wxPanel
{
public:
	MacroPanel(wxFrame* parent);
	wxStaticText* m_MousePos;

private:
	wxDECLARE_EVENT_TABLE();
};


class TrayIcon;

class MyFrame : public wxFrame
{
public:
	MyFrame(const wxString& title);
	~MyFrame()
	{
		// deinitialize the frame manager
		m_mgr.UnInit();
	}

	void OnHelp(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnSaveAs(wxCommandEvent& event);
	void OnDestroyAll(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnOverlockErrorCheck(wxTimerEvent& event);

	void SetIconTooltip(const wxString& str);

	MainPanel* main_panel;
	EditorPanel* editor_panel;
	EscaperPanel* escape_panel;
	MacroPanel* macro_panel;
	ParserPanel* parser_panel;
	LogPanel* log_panel;
	wxAuiNotebook* ctrl;
	std::mutex mtx;
	std::tuple<int, int64_t, size_t> backup_result;

	wxDECLARE_EVENT_TABLE();
private:
	void HandleNotifications();
	template<typename T> void ShowNotificaiton(const wxString& title, const wxString& message, int timeout, T&& fptr);

	wxIcon applicationIcon;
	TrayIcon* tray;
	wxAuiManager m_mgr;
	wxTimer* m_timer;
	wxTimer* m_octimer;
};