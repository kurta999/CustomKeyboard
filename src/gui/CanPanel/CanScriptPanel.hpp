#pragma once

#include <wx/wx.h>

#include "ICanResultPanel.hpp"
#include <wx/filepicker.h>

class CanScriptHandler;
class CanScriptPanel : public wxPanel, public ICanResultPanel
{
public:
    CanScriptPanel(wxWindow* parent);
    ~CanScriptPanel();
    void OnFileDrop(wxDropFilesEvent& event);
    void OnFileSelected(wxFileDirPickerEvent& event);

    void AddToLog(std::string str) override;

    void On10MsTimer();

private:
    void OnKeyDown(wxKeyEvent& evt);
    void OnLogLevelChange(wxSpinEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void ClearRecordingsFromGrid();
    void HandleInputFileSelect(wxString& path);

    wxTextCtrl* m_Offset = nullptr;
    wxCheckBox* m_OnlyCreateVehicle = nullptr;
    wxCheckBox* m_VehCompInline = nullptr;
    wxCheckBox* m_SaveNumberPlate = nullptr;
    wxCheckBox* m_HideWhenAlphaIsSet = nullptr;
    wxCheckBox* m_ConvertItemName = nullptr;
    wxTextCtrl* m_Input = nullptr;
    wxTextCtrl* m_Output = nullptr;
    wxButton* m_RunButton = nullptr;
    wxButton* m_RunSelectedButton = nullptr;
    wxButton* m_Abort = nullptr;
    wxButton* m_ClearButton = nullptr;
    wxButton* m_ClearOutput = nullptr;
    wxFilePickerCtrl* m_FilePicker = nullptr;

    wxString path;

    std::unique_ptr<CanScriptHandler> m_Script;

    wxDECLARE_EVENT_TABLE();
};
