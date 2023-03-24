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

#define MAX_BITEDITOR_FIELDS      32

enum CanSenderGridCol : int
{
    Sender_Id,
    Sender_DataSize,
    Sender_Data,
    Sender_Period,
    Sender_Count,
    Sender_LogLevel,
    Sender_FavouriteLevel,
    Sender_Comment,
    Sender_Max
};

enum CanLogGridCol : int
{
    Log_Time,
    Log_Direction,
    Log_Id,
    Log_DataSize,
    Log_Data,
    Log_Comment,
    Log_Max
};

class CanTxEntry;
class CanRxData;
class CanByteEditorDialog;
class BitEditorDialog;
class CanLogForFrameDialog;
class CanUdsRawDialog;
class CanMap;
class IResultPanel;

using CanBitfieldInfo = std::vector<std::tuple<std::string, std::string, CanMap*>>;

class CanGrid
{
public:
    CanGrid(wxWindow* parent);

    void AddRow(wxString id, wxString dlc, wxString data, wxString period, wxString count, wxString loglevel, wxString comment);
    void AddRow(std::unique_ptr<CanTxEntry>& e);
    void RemoveLastRow();
    void UpdateTxCounter(uint32_t frame_id, size_t count);
    wxGrid* m_grid = nullptr;
    
    std::map<uint16_t, CanTxEntry*> grid_to_entry;  /* Helper map for storing an additional ID to CanTxEntry */

    size_t cnt = 0;
};

class CanGridRx
{
public:
    CanGridRx(wxWindow* parent);

    void AddRow(std::unique_ptr<CanRxData>& e);
    void UpdateRow(int num_row, uint32_t frame_id, std::unique_ptr<CanRxData>& e, std::string& comment);
    void ClearGrid();

    wxGrid* m_grid = nullptr;
    std::map<uint16_t, CanRxData*> rx_grid_to_entry;  /* Helper map for storing an additional ID to CanRxData */

    size_t cnt = 0;
};

class CanSenderPanel : public wxPanel
{
public:
    CanSenderPanel(wxWindow* parent);
    void UpdatePanel();

    void On10MsTimer();
    void RefreshSubpanels();

    void LoadTxList();
    void SaveTxList();
    void LoadRxList();
    void SaveRxList();
    void LoadMapping();
    void SaveMapping();
    void OnKeyDown(wxKeyEvent& evt);
    void UpdateGridForTxFrame(uint32_t frame_id, uint8_t* buffer);

    CanGrid* can_grid_tx = nullptr;
    CanGridRx* can_grid_rx = nullptr;

private:    
    void RefreshTx();
    void RefreshRx();
    void RefreshGuiIconsBasedOnSettings();

    void OnCellValueChanged(wxGridEvent& ev);
    /*
    void OnCellLeftClick(wxGridEvent& ev);
    void OnCellLeftDoubleClick(wxGridEvent& ev);
    */
    void OnCellRightClick(wxGridEvent& ev);
    void OnGridLabelRightClick(wxGridEvent& ev);
    void OnSize(wxSizeEvent& evt);

    wxStaticBoxSizer* static_box_tx = nullptr;
    wxStaticBoxSizer* static_box_rx = nullptr;

    wxButton* m_SingleShot = nullptr;
    wxButton* m_SendSelected = nullptr;
    wxButton* m_StopSelected = nullptr;
    wxButton* m_SendAll = nullptr;
    wxButton* m_StopAll = nullptr;
    wxButton* m_Add = nullptr;
    wxButton* m_Copy = nullptr;
    wxButton* m_MoveUp = nullptr;
    wxButton* m_MoveDown = nullptr;
    wxButton* m_Delete = nullptr;
    wxButton* m_Edit = nullptr;
    wxButton* m_SendDataFrame = nullptr;
    wxButton* m_SendIsoTp = nullptr;
    wxButton* m_ClearRx = nullptr;

    std::string m_LastDataInput;
    std::string m_LastIsoTpwResponseIDInput;

    BitEditorDialog* m_BitfieldEditor = nullptr;
    CanLogForFrameDialog* m_LogForFrame = nullptr;
    CanUdsRawDialog* m_UdsRawDialog = nullptr;

    wxString file_path_tx;
    wxString file_path_rx;
    wxString file_path_mapping;

    std::string search_pattern_tx;
    std::string search_pattern_rx;

    wxDECLARE_EVENT_TABLE();
};

class CanLogEntry;
class CanLogPanel : public wxPanel
{
public:
    CanLogPanel(wxWindow* parent);
    
    void On10MsTimer();
    void InsertRow(std::chrono::steady_clock::time_point& t1, uint8_t direction, uint32_t id, std::vector<uint8_t>& data, std::string& comment);
    void UpdatePanel();

    wxGrid* m_grid = nullptr;

private:

    void OnKeyDown(wxKeyEvent& evt);
    void OnLogLevelChange(wxSpinEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void ClearRecordingsFromGrid();

    bool is_something_inserted = false;
    std::size_t inserted_until = 0;
    wxStaticBoxSizer* static_box = nullptr;
    wxButton* m_RecordingStart = nullptr;
    wxButton* m_RecordingPause = nullptr;
    wxButton* m_RecordingStop = nullptr;
    wxButton* m_RecordingClear = nullptr;
    wxButton* m_AutoScrollBtn = nullptr;
    wxButton* m_RecordingSave = nullptr;
    wxSpinCtrl* m_LogLevelCtrl = nullptr;

    size_t cnt = 0;
    std::string search_pattern;
    bool m_AutoScroll = true;
    wxDECLARE_EVENT_TABLE();
};

class CanScriptHandler;
class CanScriptPanel : public wxPanel, public ICanResultPanel
{
public:
    CanScriptPanel(wxWindow* parent);
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

class CanUdsRawDialog : public wxDialog
{
public:
    CanUdsRawDialog(wxWindow* parent);

    void ShowDialog();
    bool IsApplyClicked() { return m_IsApplyClicked; }

    uint32_t GetSenderId();
    uint32_t GetReceiverId();
    uint32_t GetDelayBetweenFrames();
    uint32_t GetWaitingTimeForFrames();
    std::string GetSentData();

    wxTextCtrl* m_SenderId = nullptr;
    wxTextCtrl* m_ReceiverId = nullptr;
    wxTextCtrl* m_DelayBetweenFrames = nullptr;
    wxTextCtrl* m_RecvDelayFrames = nullptr;
    wxButton* m_ReadDid = nullptr;
    wxButton* m_WriteDid = nullptr;
    wxButton* m_Clear = nullptr;
    wxTextCtrl* m_DataToSend = nullptr;
    wxButton* m_SendBtn = nullptr;
    wxTextCtrl* m_DataRecv = nullptr;
protected:
    void OnApply(wxCommandEvent& event);

private:
    void HandleFrameSending();

    wxBoxSizer* sizerTop = nullptr;
    
    uint32_t m_LastUdsSenderId = 0;
    uint32_t m_LastUdsReceiverId = 0;
    uint32_t m_LastDelayBetweenFrames = 0;
    uint32_t m_LastRecvWaitingTime = 0;
    std::string m_LastUdsInput;

    bool m_IsApplyClicked = false;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CanUdsRawDialog);
};