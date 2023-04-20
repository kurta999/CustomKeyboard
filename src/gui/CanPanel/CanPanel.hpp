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

#define MAX_BITEDITOR_FIELDS      32





class CanTxEntry;
class CanRxData;
class CanByteEditorDialog;
class BitEditorDialog;
class CanLogForFrameDialog;
class CanUdsRawDialog;
class CanMap;
class IResultPanel;

using CanBitfieldInfo = std::vector<std::tuple<std::string, std::string, CanMap*>>;


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

class BitEditorDialog : public wxDialog
{
public:
    BitEditorDialog(wxWindow* parent);

    // [label] = value
    void ShowDialog(uint32_t frame_id, bool is_rx, CanBitfieldInfo& values);
    std::vector<std::string> GetOutput();

    enum class BitSelection
    {
        Decimal,
        Hex,
        Binary,
    };

    enum class ClickType
    {
        None,
        Ok,
        Close,
        Apply,
    };
    
    ClickType GetClickType() { return m_ClickType; }

    uint32_t GetFrameId() { return m_FrameId; }

protected:
    void OnApply(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnRadioButtonClicked(wxCommandEvent& event);
private:
    int m_Id = 0;
    uint8_t m_DataFormat = 0;
    wxRadioButton* m_IsDecimal = {};
    wxRadioButton* m_IsHex = {};
    wxRadioButton* m_IsBinary = {};
    wxStaticText* m_InputLabel[MAX_BITEDITOR_FIELDS] = {};
    wxTextCtrl* m_Input[MAX_BITEDITOR_FIELDS] = {};
    wxRadioButton* m_InputBit[MAX_BITEDITOR_FIELDS] = {};
    wxSizer* sizerTop = {};
    wxSizer* sizerMsgs = {};

    ClickType m_ClickType = ClickType::None;

    CanBitfieldInfo m_BitfieldInfo;
    BitSelection bit_sel = BitSelection::Decimal;
    uint32_t m_FrameId = {};

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(BitEditorDialog);
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
