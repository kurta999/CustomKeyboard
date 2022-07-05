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
	
	wxPGProperty* m_pgType = nullptr;
	wxPGProperty* m_pgId = nullptr;
	wxPGProperty* m_pgName = nullptr;
	wxPGProperty* m_pgLabel = nullptr;
	wxPGProperty* m_pgPos = nullptr;
	wxPGProperty* m_pgSize = nullptr;
	wxPGProperty* m_pgMinSize = nullptr;
	wxPGProperty* m_pgMaxSize = nullptr;
	wxPGProperty* m_pgForegroundColor = nullptr;
	wxPGProperty* m_pgBackgroundColor = nullptr;
	wxPGProperty* m_pgFont = nullptr;
	wxPGProperty* m_pgTooltip = nullptr;
	wxPGProperty* m_pgEnabled = nullptr;
	wxPGProperty* m_pgHidden = nullptr;
	wxPGProperty* m_pgButtonStyle = nullptr;
	wxPGProperty* m_pgSliderStyle = nullptr;
	wxPGProperty* m_pgStaticTextStyle = nullptr;
	wxPGProperty* m_pgTextCtrlStyle = nullptr;
	wxPGProperty* m_pgComboBoxStyle = nullptr;
	wxPGProperty* m_pgChoiseStyle = nullptr;
	wxPGProperty* m_pgCheckboxStyle = nullptr;
	wxPGProperty* m_pgGaugeStyle = nullptr;
	wxPropertyGrid* m_propertyGrid = nullptr;

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
	wxAuiNotebook* m_notebook = nullptr;
	GuiEditorMain* gui_editor = nullptr;
	GuiEditorCpp* gui_cpp = nullptr;

private:
	wxDECLARE_EVENT_TABLE();
};