#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/stc/stc.h>
#include <wx/treelist.h>
#include <wx/grid.h>

#include <map>

enum
{
    Col_Id,
    Col_DataSize,
    Col_Data,
    Col_Period,
    Col_Count,
    Col_Comment,
    Col_Max
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

class CanPanel : public wxPanel
{
public:
	CanPanel(wxWindow* parent);

	void On10MsTimer();
    void RefreshSubpanels();

    void LoadTxList();
    void SaveTxList();
    void LoadRxList();
    void SaveRxList();
    void OnSize(wxSizeEvent& evt);
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
