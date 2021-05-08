#pragma once

#include <wx/wx.h>
#include <wx/slider.h>
#include <wx/artprov.h>
#include <wx/spinctrl.h>
#include <wx/filepicker.h>
#include <wx/aui/aui.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/statline.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>
#include <wx/tglbtn.h>
#include <wx/srchctrl.h>
#include <wx/clrpicker.h>
#include <wx/fontpicker.h>
#include <wx/filepicker.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/timectrl.h>
#include <wx/calctrl.h>
#include <wx/treectrl.h>

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

	wxCheckBox* m_IsModbus = nullptr;
	wxComboBox* m_PointerSize = nullptr;
	wxSpinCtrl* m_StructurePadding = nullptr;
	wxStyledTextCtrl* m_StyledTextCtrl = nullptr;
	wxTextCtrl* m_Output = nullptr;
	wxButton* m_OkButton = nullptr;
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


class NotificationIcon;

class MyFrame : public wxFrame
{
public:
	MyFrame(const wxString& title);
	~MyFrame()
	{
		// deinitialize the frame manager
		m_mgr.UnInit();
	}

	void TimerEvent(wxTimerEvent& event);
	void OnClose(wxCloseEvent& event);

	void SetIconTooltip(const wxString& str);
	MainPanel* main_panel;
	EscaperPanel* escape_panel;
	MacroPanel* macro_panel;
	ParserPanel* parser_panel;
	LogPanel* log_panel;

	wxDECLARE_EVENT_TABLE();
private:
	wxIcon applicationIcon;
	NotificationIcon* notification;
	wxAuiManager m_mgr;
	wxTimer* m_timer;
};