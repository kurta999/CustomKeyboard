#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

typedef struct
{
	uint16_t id;
	const wxString* name;
	wxWindow* w;
} PropgripdParams_t;

class GuiEditorMain : public wxPanel
{
public:
	GuiEditorMain(wxWindow* parent);
	void OnClick(wxMouseEvent& event);
	void UpdatePropgrid(PropgripdParams_t&& params);
	
	wxPGProperty* m_pgType;
	wxPGProperty* m_pgId;
	wxPGProperty* m_pgName;
	wxPGProperty* m_pgLabel;
	wxPGProperty* m_pgPos;
	wxPGProperty* m_pgSize;
	wxPGProperty* m_pgMinSize;
	wxPGProperty* m_pgMaxSize;
	wxPGProperty* m_pgForegroundColor;
	wxPGProperty* m_pgBackgroundColor;
	wxPGProperty* m_pgFont;
	wxPGProperty* m_pgTooltip;
	wxPGProperty* m_pgEnabled;
	wxPGProperty* m_pgHidden;
	wxPGProperty* m_pgButtonStyle;
	wxPGProperty* m_pgSliderStyle;
	wxPGProperty* m_pgStaticTextStyle;
	wxPGProperty* m_pgTextCtrlStyle;
	wxPGProperty* m_pgComboBoxStyle;
	wxPGProperty* m_pgChoiseStyle;
	wxPGProperty* m_pgCheckboxStyle;
	wxPGProperty* m_pgGaugeStyle;
	wxPropertyGrid* m_propertyGrid;

private:
	void OnKeyDown(wxKeyEvent& event);
	void OnMouseMotion(wxMouseEvent& event);
	void OnMouseLeftDown(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnPropertyGridChange(wxPropertyGridEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnContextMenu(wxContextMenuEvent& evt);

	wxDECLARE_EVENT_TABLE();
};

class GuiEditorCpp : public wxPanel
{
public:
	GuiEditorCpp(wxWindow* parent);

	wxStyledTextCtrl* m_StyledTextCtrl;
private:
};

class EditorPanel : public wxPanel
{
public:
	EditorPanel(wxWindow* parent);
	~EditorPanel()
	{
		m_mgr.UnInit();
	}
	void Changeing(wxAuiNotebookEvent& event);

	wxAuiManager m_mgr;
	wxAuiNotebook* m_notebook;
	GuiEditorMain* gui_editor;
	GuiEditorCpp* gui_cpp;

private:
	wxDECLARE_EVENT_TABLE();
};