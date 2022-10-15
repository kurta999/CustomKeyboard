#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>
#include <wx/treelist.h>
#include <wx/grid.h>

#include <map>

enum CanSenderGridCol : int
{
    Sender_Id,
    Sender_DataSize,
    Sender_Data,
    Sender_Period,
    Sender_Count,
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

class CanGrid
{
public:
    CanGrid(wxWindow* parent);

    void AddRow(wxString id, wxString dlc, wxString data, wxString period, wxString count, wxString comment);
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
    void OnKeyDown(wxKeyEvent& evt);

    CanGrid* can_grid_tx = nullptr;
    CanGridRx* can_grid_rx = nullptr;

private:    
    void RefreshTx();
    void RefreshRx();
    void RefreshGuiIconsBasedOnSettings();

    void OnCellValueChanged(wxGridEvent& ev);

    wxStaticBoxSizer* static_box_tx = nullptr;
    wxStaticBoxSizer* static_box_rx = nullptr;

    wxTreeListCtrl* tree_receive = nullptr;
    wxDataViewCtrl* tree_t = nullptr;

    wxButton* m_SingleShot = nullptr;
    wxButton* m_SendAll = nullptr;
    wxButton* m_StopAll = nullptr;
    wxButton* m_Add = nullptr;
    wxButton* m_Copy = nullptr;
    wxButton* m_Delete = nullptr;

    wxString file_path_tx;
    wxString file_path_rx;

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
    void ClearRecordingsFromGrid();

    std::chrono::steady_clock::time_point start_time;
    bool is_something_inserted = false;
    std::size_t inserted_until = 0;
    wxStaticBoxSizer* static_box = nullptr;
    wxButton* m_RecordingStart = nullptr;
    wxButton* m_RecordingPause = nullptr;
    wxButton* m_RecordingStop = nullptr;
    wxButton* m_RecordingClear = nullptr;
    wxButton* m_RecordingSave = nullptr;
    size_t cnt = 0;
    std::string search_pattern;

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
    void RefreshSubpanels();

    CanSenderPanel* sender = nullptr;
    CanLogPanel* log = nullptr;

private:
    void OnSize(wxSizeEvent& evt);
    void Changeing(wxAuiNotebookEvent& event);

    // !\brief AUI manager for subwindows
    wxAuiManager m_mgr;

    wxAuiNotebook* m_notebook = nullptr;

	wxDECLARE_EVENT_TABLE();
};
