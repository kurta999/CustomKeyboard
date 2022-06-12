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
    wxGrid* m_grid;
    
    std::map<uint16_t, CanTxEntry*> grid_to_entry;  /* Helper map for storing an additional ID to CanTxEntry */

    size_t cnt = 0;
};

class CanGridRx
{
public:
    CanGridRx(wxWindow* parent);

    wxGrid* m_grid;
    std::map<uint16_t, CanRxData*> rx_grid_to_entry;  /* Helper map for storing an additional ID to CanRxData */

    size_t cnt = 0;
};

class CanPanel : public wxPanel
{
public:
	CanPanel(wxWindow* parent);

	void On10MsTimer();
    void Refresh();
    void LoadTxList();
    void SaveTxList();
    void LoadRxList();
    void SaveRxList();

private:
	void OnCellValueChanged(wxGridEvent& ev);

	wxTreeListCtrl* tree_receive;
	wxDataViewCtrl* tree_t;

    CanGrid* can_grid_tx = nullptr;
    CanGridRx* can_grid_rx = nullptr;

	wxButton* m_SingleShot;
	wxButton* m_SendAll;
	wxButton* m_StopAll;
	wxButton* m_Add;
	wxButton* m_Copy;
	wxButton* m_Delete;

    wxString file_path_tx;
    wxString file_path_rx;
	wxDECLARE_EVENT_TABLE();
};
