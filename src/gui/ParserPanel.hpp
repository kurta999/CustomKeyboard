#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/filepicker.h>
#include <wx/stc/stc.h>

class ParserPanel : public wxPanel
{
public:
	ParserPanel(wxFrame* parent);
	void OnFileDrop(wxDropFilesEvent& event);
	void OnFileSelected(wxFileDirPickerEvent& event);
	void OnSize(wxSizeEvent& event);

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