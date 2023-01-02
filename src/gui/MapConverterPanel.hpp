#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/filepicker.h>
#include <wx/stc/stc.h>

class MapConverterPanel : public wxPanel
{
public:
	MapConverterPanel(wxFrame* parent);
	void OnFileDrop(wxDropFilesEvent& event);
	void OnFileSelected(wxFileDirPickerEvent& event);
	void OnSize(wxSizeEvent& event);

	wxTextCtrl* m_Offset = nullptr;
	wxCheckBox* m_OnlyCreateVehicle = nullptr;
	wxCheckBox* m_VehCompInline = nullptr;
	wxCheckBox* m_SaveNumberPlate = nullptr;
	wxCheckBox* m_HideWhenAlphaIsSet = nullptr;
	wxCheckBox* m_ConvertItemName = nullptr;
	wxTextCtrl* m_Input= nullptr;
	wxTextCtrl* m_Output = nullptr;
	wxButton* m_OkButton = nullptr;
	wxButton* m_ClearButton = nullptr;
	wxFilePickerCtrl* m_FilePicker = nullptr;

	wxString path;
private:
	void HandleInputFileSelect(wxString& path);

	wxDECLARE_EVENT_TABLE();
};