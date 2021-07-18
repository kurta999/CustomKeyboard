#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>
#include <wx/treelist.h>

class ComTcpPanel : public wxPanel
{
public:
	ComTcpPanel(wxWindow* parent);

private:
	wxCheckBox* m_IsTcp;
	wxSpinCtrl* m_TcpPortSpin;
	wxCheckBox* m_IsCom;
	wxComboBox* m_serial;
	wxCheckBox* m_IsMinimizeOnExit;
	wxCheckBox* m_IsMinimizeOnStartup;
	wxSpinCtrl* m_DefaultPage;

	wxDECLARE_EVENT_TABLE();
};

class KeybrdPanel : public wxPanel
{
public:
	KeybrdPanel(wxWindow* parent);

	void OnTreeListChanged(wxTreeListEvent& evt);

private:
	wxTreeListCtrl* tree;
	wxTreeListCtrl* tree_binds;

	wxDECLARE_EVENT_TABLE();
};

class ConfigurationPanel : public wxPanel
{
public:
	ConfigurationPanel(wxWindow* parent);

	ComTcpPanel* comtcp_panel;
	KeybrdPanel* keybrd_panel;

private:
	wxDECLARE_EVENT_TABLE();
};