#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>
#include <wx/treelist.h>
#include <wx/grid.h>
#include <wx/spinctrl.h>
#include <wx/filepicker.h>

#include <map>

#include "ICanResultPanel.hpp"

#include "CanSenderPanel.hpp"
#include "CanLogPanel.hpp"
#include "CanScriptPanel.hpp"


class CanTxEntry;
class CanRxData;
class CanByteEditorDialog;
class BitEditorDialog;
class CanLogForFrameDialog;
class CanUdsRawDialog;
class CanMap;
class IResultPanel;

class CanPanel : public wxPanel
{
public:
	CanPanel(wxWindow* parent);
    ~CanPanel();

    void On10MsTimer();
    void LoadTxList();
    void SaveTxList();
    void LoadRxList();
    void SaveRxList();    
    void LoadMapping();
    void SaveMapping();
    void RefreshSubpanels();

    CanSenderPanel* sender = nullptr;
    CanLogPanel* log = nullptr;
    CanScriptPanel* script = nullptr;
    wxAuiNotebook* m_notebook = nullptr;

private:
    void OnSize(wxSizeEvent& evt);
    void Changeing(wxAuiNotebookEvent& event);

    // !\brief AUI manager for subwindows
    wxAuiManager m_mgr;

	wxDECLARE_EVENT_TABLE();
};

class CanLogForFrameDialog : public wxDialog
{
public:
    CanLogForFrameDialog(wxWindow* parent);

    void ShowDialog(std::vector<std::string>& values);

protected:
    void OnApply(wxCommandEvent& event);
private:

    wxListBox* m_Log = nullptr;
    wxBoxSizer* sizerTop = nullptr;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CanLogForFrameDialog);
};
