#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>

class GuiEditorMain : public wxPanel
{
public:
	GuiEditorMain(wxWindow* parent);
	void OnClick(wxMouseEvent& event);

private:
	void OnKeyDown(wxKeyEvent& event);
	void OnMouseMotion(wxMouseEvent& event);
	void OnMouseLeftDown(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);

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
	void Changeing(wxAuiNotebookEvent& event);

	GuiEditorMain* gui_editor;
	GuiEditorCpp* gui_cpp;

private:
	wxDECLARE_EVENT_TABLE();
};