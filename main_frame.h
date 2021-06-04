#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/stc/stc.h>
#include <wx/filepicker.h>

#include <mutex>

class MainPanel : public wxPanel
{
public:
	MainPanel(wxFrame* parent);

	wxButton* m_RefreshButton = nullptr;
	wxStaticText* m_textTemp;
	wxStaticText* m_textHum;
	wxStaticText* m_textCO2;
	wxStaticText* m_textVOC;
	wxStaticText* m_textPM25;
	wxStaticText* m_textPM10;
	wxStaticText* m_textLux;
	wxStaticText* m_textCCT;
private:

	wxDECLARE_EVENT_TABLE();
};

class GraphPanel : public wxPanel
{
public:
	GraphPanel(wxFrame* parent);

private:
	wxDECLARE_EVENT_TABLE();
};

class EscaperPanel : public wxPanel
{
public:
	EscaperPanel(wxFrame* parent);

	wxStyledTextCtrl* m_StyledTextCtrl = nullptr;
	wxCheckBox* m_IsEscapePercent = nullptr;
	wxCheckBox* m_IsBackslashAtEnd = nullptr;
	wxButton* m_OkButton = nullptr;
private:
	wxDECLARE_EVENT_TABLE();
};

class MacroPanel : public wxPanel
{
public:
	MacroPanel(wxFrame* parent);
	wxStaticText* m_MousePos;

private:
	wxDECLARE_EVENT_TABLE();
};

class ParserPanel : public wxPanel
{
public:
	ParserPanel(wxFrame* parent);
	void OnFileDrop(wxDropFilesEvent& event);
	void OnFileSelected(wxFileDirPickerEvent& event);

	wxCheckBox* m_IsModbus = nullptr;
	wxComboBox* m_PointerSize = nullptr;
	wxSpinCtrl* m_StructurePadding = nullptr;
	wxStyledTextCtrl* m_StyledTextCtrl = nullptr;
	wxTextCtrl* m_Output = nullptr;
	wxButton* m_OkButton = nullptr;
	wxFilePickerCtrl* m_FilePicker;

	wxString path;
private:
	wxDECLARE_EVENT_TABLE();
};

class LogPanel : public wxPanel
{
public:
	LogPanel(wxFrame* parent);

	wxListBox* m_Log;
	wxButton* m_ClearButton = nullptr;
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
	void OnClose(wxCloseEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnOverlockErrorCheck(wxTimerEvent& event);

	void SetIconTooltip(const wxString& str);

	MainPanel* main_panel;
	GraphPanel* graph_panel;
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