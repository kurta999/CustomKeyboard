#pragma once

#include "MainPanel.hpp"
#include "EscaperPanel.hpp"
#include "ParserPanel.hpp"
#include "LogPanel.hpp"
#include "Editor.hpp"
#include "Configuration.hpp"
#include "FilePanel.hpp"
#include "CanPanel.hpp"

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
#include <any>

class MacroPanel : public wxPanel
{
public:
	MacroPanel(wxFrame* parent);
	wxStaticText* m_MousePos;
	wxStaticText* m_ActiveWindowTitle;

private:
	wxDECLARE_EVENT_TABLE();
};

class TrayIcon;

enum PopupMsgIds : uint8_t
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
		m_mgr.UnInit();  /* deinitialize the frame manager */
	}

	void SetIconTooltip(const wxString& str);

	MainPanel* main_panel = nullptr;
	ConfigurationPanel* config_panel;
	EditorPanel* editor_panel;
	EscaperPanel* escape_panel;
	MacroPanel* macro_panel;
	ParserPanel* parser_panel;
	FilePanel* file_panel = nullptr;
	CanPanel* can_panel = nullptr;
	LogPanel* log_panel = nullptr;
	wxAuiNotebook* ctrl;
	std::mutex mtx;
	std::deque<std::vector<std::any>> pending_msgs;
	wxProgressDialog* backup_prog = NULL;
	std::atomic<bool> show_backup_dlg = false;

	wxDECLARE_EVENT_TABLE();
private:
	void OnHelp(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnSaveAs(wxCommandEvent& event);
	void OnDestroyAll(wxCommandEvent& event);
	void OnCanLoadTxList(wxCommandEvent& event);
	void OnCanSaveTxList(wxCommandEvent& event);
	void OnCanLoadRxList(wxCommandEvent& event);
	void OnCanSaveRxList(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnSize(wxSizeEvent& event);

	// \brief Fast timer for frame
	void On10msTimer(wxTimerEvent& event);

	// \brief Main timer for frame
	void On100msTimer(wxTimerEvent& event);

	// \brief Handles backup progress dialog
	void HandleBackupProgressDialog();

	// \brief Handles numlock to be always on
	void HandleAlwaysOnNumlock();

private:
	// \brief Handles notifications
	void HandleNotifications();

	// \brief Show notification
	template<typename T> void ShowNotificaiton(const wxString& title, const wxString& message, int timeout, int flags, T&& fptr);

	// \brief Application icon
	wxIcon applicationIcon;

	// \brief Tray
	TrayIcon* tray;

	// \brief AUI manager for subwindows
	wxAuiManager m_mgr;

	// \brief Fast main frame timer
	wxTimer* m_10msTimer;	
	
	// \brief Main frame timer
	wxTimer* m_100msTimer;
};