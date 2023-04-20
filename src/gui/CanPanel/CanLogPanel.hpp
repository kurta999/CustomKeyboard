#pragma once

#include <wx/wx.h>
#include <wx/grid.h>

#include <chrono>

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
