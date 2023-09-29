#pragma once

#include <wx/wx.h>
#include <wx/grid.h>

#define MAX_BITEDITOR_FIELDS      32

class CanMap;
using CanBitfieldInfo = std::vector<std::tuple<std::string, std::string, CanMap*>>;

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