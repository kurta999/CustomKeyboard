#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CanLogPanel, wxPanel)
EVT_SIZE(CanLogPanel::OnSize)
EVT_CHAR_HOOK(CanLogPanel::OnKeyDown)
EVT_SPINCTRL(ID_CanLogLevelSpinCtrl, CanLogPanel::OnLogLevelChange)
wxEND_EVENT_TABLE()

CanLogPanel::CanLogPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;

    wxBoxSizer* v_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);

    static_box = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Log :: TX: 0, RX: 0, Total: 0");
    static_box->GetStaticBox()->SetFont(static_box->GetStaticBox()->GetFont().Bold());
    static_box->GetStaticBox()->SetForegroundColour(*wxBLUE);

    m_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxSize(800, 600), 0);

    // Grid
    m_grid->CreateGrid(1, CanLogGridCol::Log_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(CanLogGridCol::Log_Time, "Time");
    m_grid->SetColLabelValue(CanLogGridCol::Log_Direction, "Direction");
    m_grid->SetColLabelValue(CanLogGridCol::Log_Id, "Id");
    m_grid->SetColLabelValue(CanLogGridCol::Log_DataSize, "Size");
    m_grid->SetColLabelValue(CanLogGridCol::Log_Data, "Data");
    m_grid->SetColLabelValue(CanLogGridCol::Log_Comment, "Comment");

    // Columns
    m_grid->EnableDragColMove(true);
    m_grid->EnableDragColSize(true);
    m_grid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    m_grid->SetSelectionMode(wxGrid::wxGridSelectionModes::wxGridSelectRows);

    // Rows
    m_grid->EnableDragRowSize(true);
    m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);


    // Label Appearance

    // Cell Defaults
    m_grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    m_grid->HideRowLabels();

    m_grid->SetColSize(CanLogGridCol::Log_Data, 200);
    m_grid->SetColSize(CanLogGridCol::Log_Comment, 200);
    static_box->Add(m_grid);

    m_RecordingStart = new wxButton(this, wxID_ANY, "Record", wxDefaultPosition, wxDefaultSize);
    m_RecordingStart->SetToolTip("Start recording for received & sent CAN frames");
    m_RecordingStart->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
            can_handler->ToggleRecording(true, false);
        });
    h_sizer->Add(m_RecordingStart);

    m_RecordingPause = new wxButton(this, wxID_ANY, "Pause", wxDefaultPosition, wxDefaultSize);
    m_RecordingPause->SetToolTip("Suspend recording for received & sent CAN frames");
    m_RecordingPause->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
            can_handler->ToggleRecording(false, true);
        });
    h_sizer->Add(m_RecordingPause);

    m_RecordingStop = new wxButton(this, wxID_ANY, "Stop", wxDefaultPosition, wxDefaultSize);
    m_RecordingStop->SetToolTip("Suspend recording for received & sent CAN frames, clear everything");
    m_RecordingStop->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
            can_handler->ToggleRecording(false, false);
            inserted_until = 0;
        });
    h_sizer->Add(m_RecordingStop);

    m_RecordingClear = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize);
    m_RecordingClear->SetToolTip("Clear recording and reset frame counters");
    m_RecordingClear->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            ClearRecordingsFromGrid();

            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
            can_handler->ClearRecording();
        });
    h_sizer->Add(m_RecordingClear);
    //h_sizer->AddSpacer(35);

    m_AutoScrollBtn = new wxButton(this, wxID_ANY, wxT("Toggle auto-scroll"), wxDefaultPosition, wxDefaultSize, 0);
    m_AutoScrollBtn->SetToolTip("Toggle auto-scroll");
    m_AutoScrollBtn->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
        {
            m_AutoScroll ^= 1;
            if(m_AutoScroll)
                m_AutoScrollBtn->SetBackgroundColour(wxNullColour);
            else
                m_AutoScrollBtn->SetBackgroundColour(*wxRED);
        });
    h_sizer->AddSpacer(35);
    h_sizer->Add(m_AutoScrollBtn);

    h_sizer->AddSpacer(10);
    h_sizer->Add(new wxStaticText(this, wxID_ANY, "LogLevel:"));
    m_LogLevelCtrl = new wxSpinCtrl(this, ID_CanLogLevelSpinCtrl, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 1);
    h_sizer->Add(m_LogLevelCtrl);

    m_RecordingSave = new wxButton(this, wxID_ANY, "Save log", wxDefaultPosition, wxDefaultSize);
    m_RecordingSave->SetToolTip("Save recording to file");
    m_RecordingSave->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
#ifdef _WIN32
            const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

            if(!std::filesystem::exists("Can"))
                std::filesystem::create_directory("Can");
            std::string log_format = std::format("Can/CanLog_{:%Y.%m.%d_%H_%M_%OS}.csv", now);
            std::filesystem::path p(log_format);

            std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
            can_handler->SaveRecordingToFile(p);
#endif
        });

    v_sizer->Add(h_sizer);
    v_sizer->Add(m_RecordingSave);
    v_sizer->Add(static_box);

    SetSizerAndFit(v_sizer);
    Show();
}

void CanLogPanel::On10MsTimer()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    static std::string last_search_pattern;
    static uint64_t last_tx_cnt = 0, last_rx_cnt = 0;

    if(search_pattern.empty())
    {
        if(last_tx_cnt != can_handler->GetTxFrameCount() || last_rx_cnt != can_handler->GetRxFrameCount())
        {
            static_box->GetStaticBox()->SetLabelText(wxString::Format("Log :: TX: %lld, RX: %lld, Total: %lld", can_handler->GetTxFrameCount(), can_handler->GetRxFrameCount(),
                can_handler->GetTxFrameCount() + can_handler->GetRxFrameCount()));
        }
    }
    else
    {
        if(last_tx_cnt != can_handler->GetTxFrameCount() || last_rx_cnt != can_handler->GetRxFrameCount() || last_search_pattern != search_pattern)
        {
            static_box->GetStaticBox()->SetLabelText(wxString::Format("Log :: Filter: %s, TX: %lld, RX: %lld, Total: %lld", search_pattern,
                can_handler->GetTxFrameCount(), can_handler->GetRxFrameCount(), can_handler->GetTxFrameCount() + can_handler->GetRxFrameCount()));
        }
    }

    last_tx_cnt = can_handler->GetTxFrameCount();
    last_rx_cnt = can_handler->GetRxFrameCount();
    last_search_pattern = search_pattern;

    if(!can_handler->m_LogEntries.empty())
    {
        if(!is_something_inserted)
            inserted_until = 0;

        auto it = can_handler->m_LogEntries.begin();
        std::advance(it, inserted_until);
        if(it == can_handler->m_LogEntries.end())
        {
            //DBG("shit happend");
            return;
        }

        if(it != can_handler->m_LogEntries.end())
        {
            for(; it != can_handler->m_LogEntries.end(); ++it)
            {
                std::string comment;
                if((*it)->direction == 1)
                {
                    auto comment_it = can_handler->rx_entry_comment.find((*it)->frame_id);
                    if(comment_it != can_handler->rx_entry_comment.end())
                    {
                        comment = comment_it->second;
                    }
                }
                else
                {
                    for(auto& i : can_handler->entries)
                    {
                        if(i->id == (*it)->frame_id)
                        {
                            comment = i->comment;
                            break;
                        }
                    }
                }

                bool insert_row = false;
                if(search_pattern.empty())
                {
                    insert_row = true;
                }
                else
                {
                    if(boost::icontains(comment, search_pattern))
                        insert_row = true;
                }

                if(insert_row)
                    InsertRow((*it)->last_execution, (*it)->direction, (*it)->frame_id, (*it)->data, comment);
                inserted_until++;
            }
            //inserted_until = std::distance(can_handler->m_LogEntries.begin(), it);
            is_something_inserted = true;
        }
    }
}

void CanLogPanel::InsertRow(std::chrono::steady_clock::time_point& t1, uint8_t direction, uint32_t id, std::vector<uint8_t>& data, std::string& comment)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - can_handler->GetStartTime()).count();
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Time), wxString::Format("%.3lf", static_cast<double>(elapsed) / 1000.0));

    std::string hex;
    utils::ConvertHexBufferToString(data, hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Data), hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Direction), direction == CAN_LOG_DIR_TX ? "TX" : "RX");
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Id), wxString::Format("%X", id));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_DataSize), wxString::Format("%lld", data.size()));
    m_grid->SetCellValue(wxGridCellCoords(cnt, CanLogGridCol::Log_Comment), comment);

    if(m_AutoScroll)
        m_grid->ScrollLines(num_rows);

    for(uint8_t i = 0; i != CanLogGridCol::Log_Max; i++)
    {
        m_grid->SetReadOnly(cnt, i, true);
        m_grid->SetCellBackgroundColour(cnt, i, (direction == CAN_LOG_DIR_RX) ? 0xE6E6E6 : 0xFFFFFF);
    }

    cnt++;
}

void CanLogPanel::ClearRecordingsFromGrid()
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows)
        m_grid->DeleteRows(0, num_rows);
    cnt = 0;

    is_something_inserted = false;
    inserted_until = 0;
}

void CanLogPanel::OnKeyDown(wxKeyEvent& evt)
{
    if(evt.ControlDown())
    {
        switch(evt.GetKeyCode())
        {
            case 'F':
            {
                wxWindow* focus = wxWindow::FindFocus();
                if(focus == m_grid)
                {
                    wxTextEntryDialog d(this, "Enter frame name for what you want to filter", "Frame filter");
                    d.SetValue(search_pattern);
                    int ret = d.ShowModal();
                    if(ret == wxID_OK)
                    {
                        std::string new_search_pattern = d.GetValue().ToStdString();
                        if(new_search_pattern != search_pattern)
                        {
                            search_pattern = new_search_pattern;
                            ClearRecordingsFromGrid();
                        }
                    }
                }
                break;
            }
            case 'C':
            {
                wxWindow* focus = wxWindow::FindFocus();
                if(focus == m_grid)
                {
                    wxArrayInt rows = m_grid->GetSelectedRows();
                    if(rows.empty()) return;

                    wxString str_to_copy;
                    for(auto& row : rows)
                    {
                        for(uint8_t col = 0; col < CanLogGridCol::Log_Max - 1; col++)
                        {
                            str_to_copy += m_grid->GetCellValue(row, col);
                            str_to_copy += '\t';
                        }
                        str_to_copy += '\n';
                    }
                    if(str_to_copy.Last() == '\n')
                        str_to_copy.RemoveLast();

                    if(wxTheClipboard->Open())
                    {
                        wxTheClipboard->SetData(new wxTextDataObject(str_to_copy));
                        wxTheClipboard->Close();
                        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
                        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::SelectedLogsCopied) });
                    }
                }
                break;
            }
        }
    }
}

void CanLogPanel::OnLogLevelChange(wxSpinEvent& evt)
{
    uint8_t new_log_level = static_cast<uint8_t>(evt.GetValue());
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    can_handler->SetRecordingLogLevel(new_log_level);
}

void CanLogPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}

wxBEGIN_EVENT_TABLE(CanLogForFrameDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, CanLogForFrameDialog::OnApply)
wxEND_EVENT_TABLE()

CanLogForFrameDialog::CanLogForFrameDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "CAN log for frame", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    sizerTop = new wxBoxSizer(wxVERTICAL);

    m_Log = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE | wxLB_HSCROLL | wxLB_NEEDED_SB);
    m_Log->Bind(wxEVT_LEFT_DCLICK, [this](wxMouseEvent& event)
        {
            wxClipboard* clip = new wxClipboard();
            clip->Clear();
            clip->SetData(new wxTextDataObject(m_Log->GetString(m_Log->GetSelection())));
            clip->Flush();
            clip->Close();
        });
    sizerTop->Add(m_Log, wxSizerFlags(1).Left().Expand());

    //sizerTop->Add(sizerMsgs, wxSizerFlags(1).Expand().Border());

    // finally buttons to show the resulting message box and close this dialog
    sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCLOSE), wxSizerFlags().Right().Border()); /* wxOK */

    sizerTop->SetMinSize(wxSize(640, 480));
    SetAutoLayout(true);
    SetSizer(sizerTop);
    sizerTop->Fit(this);
    sizerTop->SetSizeHints(this);
    CentreOnScreen();
}

void CanLogForFrameDialog::ShowDialog(std::vector<std::string>& values)
{
    m_Log->Clear();
    for(const auto i : values)
    {
        m_Log->Append(i);
    }
    ShowModal();
}

void CanLogForFrameDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    Close();
    m_Log->Clear();
}