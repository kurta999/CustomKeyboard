#pragma once

#include <wx/wx.h>

enum DidGridCol : int
{
    Did_ID,
    Did_Type,
    Did_Name,
    Did_Value,
    Did_Len,
    Did_MinVal,
    Did_MaxVal,
    Did_Timestamp,
    Did_Max
};

class DidEntry;

class DidGrid
{
public:
    DidGrid(wxWindow* parent);

    void AddRow(std::unique_ptr<DidEntry>& entry);

    wxGrid* m_grid = nullptr;

    std::map<uint16_t, DidEntry*> grid_to_entry;  /* Helper map for storing an additional ID to CanTxEntry */
    std::map<uint16_t, uint16_t> did_to_row;  /* Helper map for storing an additional ID to CanTxEntry */

    size_t cnt = 0;
};

class DidPanel : public wxPanel
{
public:
	DidPanel(wxFrame* parent);

    DidGrid* did_grid;
    wxStaticBoxSizer* static_box_grid = nullptr;

    wxButton* m_RefreshSelected = nullptr;
    wxButton* m_Abort = nullptr;
    wxButton* m_ClearDids = nullptr;
    wxButton* m_SaveCache = nullptr;

    void UpdateDidList();
    void WriteDid(uint16_t did, uint8_t* data_to_write, uint16_t size);
    void On100msTimer();

private:
    void OnSize(wxSizeEvent& evt);
    void OnCellValueChanged(wxGridEvent& ev);
    void OnCellEditorShown(wxGridEvent& ev);
    void OnKeyDown(wxKeyEvent& evt);

    bool is_dids_initialized = false;
    std::string search_pattern;

    uint16_t did_cnt = 0;

	wxDECLARE_EVENT_TABLE();
};
