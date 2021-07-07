#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>

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