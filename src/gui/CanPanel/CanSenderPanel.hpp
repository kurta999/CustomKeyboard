#pragma once

#include <wx/wx.h>
#include <wx/grid.h>

#include <map>

class CanTxEntry;
class CanRxData;
class CanByteEditorDialog;
class BitEditorDialog;
class CanLogForFrameDialog;
class CanUdsRawDialog;
class CanMap;
class IResultPanel;

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
