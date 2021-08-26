#pragma once

#include "MainPanel.h"
#include "EscaperPanel.h"
#include "ParserPanel.h"
#include "LogPanel.h"
#include "Editor.h"
#include "Configuration.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/stc/stc.h>
#include <wx/filepicker.h>
#include <wx/progdlg.h>

#include <mutex>
#include <deque>

class MacroPanel : public wxPanel
{
public:
	MacroPanel(wxFrame* parent);
	wxStaticText* m_MousePos;

private:
	wxDECLARE_EVENT_TABLE();
};

class TrayIcon;

enum Msg : uint8_t
{
	ScreenshotSaved = 0,
	SettingsSaved,
	BackupCompleted,
	BackupFailed,
	StringEscaped,
	PathSeparatorsReplaced,
	LinkMark,
	LinkMarkError,
	SymlinkCreated,
	HardlinkCreated,
	MacroRecordingStarted,
	MacroRecordingStopped,
};

class MyFrame : public wxFrame
{
public:
	MyFrame(const wxString& title);
	~MyFrame()
	{
		// deinitialize the frame manager
		m_mgr.UnInit();
	}

	void SetIconTooltip(const wxString& str);

	MainPanel* main_panel = nullptr;
	ConfigurationPanel* config_panel;
	EditorPanel* editor_panel;
	EscaperPanel* escape_panel;
	MacroPanel* macro_panel;
	ParserPanel* parser_panel;
	LogPanel* log_panel = nullptr;
	wxAuiNotebook* ctrl;
	std::mutex mtx;
	std::deque<std::vector<std::any>> pending_msgs;
	wxProgressDialog* backup_prog = NULL;
	std::atomic<bool> show_backup_dlg;

	wxDECLARE_EVENT_TABLE();
private:
	void OnHelp(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnSaveAs(wxCommandEvent& event);
	void OnDestroyAll(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnTimer(wxTimerEvent& event);
	void HandleBackupProgressDialog();

private:
	void HandleNotifications();
	template<typename T> void ShowNotificaiton(const wxString& title, const wxString& message, int timeout, int flags, T&& fptr);

	wxIcon applicationIcon;
	TrayIcon* tray;
	wxAuiManager m_mgr;
	wxTimer* m_timer;
};