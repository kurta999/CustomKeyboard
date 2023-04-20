#include "pch.hpp"

wxBEGIN_EVENT_TABLE(DidPanel, wxPanel)
EVT_SIZE(DidPanel::OnSize)
EVT_GRID_CELL_CHANGED(DidPanel::OnCellValueChanged)
EVT_GRID_EDITOR_SHOWN(DidPanel::OnCellEditorShown)
EVT_CHAR_HOOK(DidPanel::OnKeyDown)
wxEND_EVENT_TABLE()

DidMap::iterator did_it;

enum class DidStateMachine
{
    Start,
    Send,
    Receive,
    Idle,
};

DidStateMachine can_sender_state = DidStateMachine::Idle;

DidGrid::DidGrid(wxWindow* parent)
{
    m_grid = new wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(1024, 600), 0);

    // Grid
    m_grid->CreateGrid(1, DidGridCol::Did_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(DidGridCol::Did_ID, "DID");
    m_grid->SetColLabelValue(DidGridCol::Did_Type, "Type");
    m_grid->SetColLabelValue(DidGridCol::Did_Name, "Name");
    m_grid->SetColLabelValue(DidGridCol::Did_Value, "Value");
    m_grid->SetColLabelValue(DidGridCol::Did_Len, "Len");
    m_grid->SetColLabelValue(DidGridCol::Did_MinVal, "Min");
    m_grid->SetColLabelValue(DidGridCol::Did_MaxVal, "Max");
    m_grid->SetColLabelValue(DidGridCol::Did_Timestamp, "Timestamp");

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

    m_grid->SetColSize(DidGridCol::Did_ID, 50);
    m_grid->SetColSize(DidGridCol::Did_Type, 100);
    m_grid->SetColSize(DidGridCol::Did_Name, 200);
    m_grid->SetColSize(DidGridCol::Did_Value, 250);
    m_grid->SetColSize(DidGridCol::Did_Timestamp, 135);
}

void DidGrid::AddRow(std::unique_ptr<DidEntry>& entry)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_ID), wxString::Format("%X", entry->id));
    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_Type), wxString::Format("%s", XmlDidLoader::GetStringFromType(entry->type)));
    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_Name), entry->name);
    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_Value), entry->value_str);
    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_Len), wxString::Format("%lld", entry->len));
    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_MinVal), wxString::Format("%s", entry->min));
    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_MaxVal), wxString::Format("%s", entry->max));

    wxString last_update_str;
    if(!entry->last_update.is_not_a_date_time())
    {
        last_update_str = boost::posix_time::to_iso_extended_string(entry->last_update);
    }

    if(!last_update_str.empty())
    {
        if(entry->nrc != 0)  /* TODO: create a function for this, because it's a duplicate */
        {
            switch(entry->nrc)
            {
                case 0x78:
                {
                    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_Value), "Pending... NRC 78");
                    m_grid->SetCellBackgroundColour(cnt, DidGridCol::Did_Value, *wxBLUE);
                    break;
                }
                default:
                {
                    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_Value), wxString::Format("NRC %X", entry->nrc));
                    m_grid->SetCellBackgroundColour(cnt, DidGridCol::Did_Value, *wxRED);

                    wxFont cell_font = m_grid->GetCellFont(cnt, Did_Name);
                    cell_font.SetWeight(wxFONTWEIGHT_NORMAL);
                    m_grid->SetCellFont(cnt, Did_Name, cell_font);
                    break;
                }
            }
        }
        else
        {
            m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_Value), entry->value_str);
            wxFont cell_font = m_grid->GetCellFont(cnt, Did_Name);
            cell_font.SetWeight(wxFONTWEIGHT_BOLD);
            m_grid->SetCellFont(cnt, Did_Name, cell_font);

            for(uint8_t i = 0; i != DidGridCol::Did_Max; i++)
                m_grid->SetCellBackgroundColour(cnt, i, (cnt & 1) ? 0xE6E6E6 : 0xFFFFFF);
        }
    }
    m_grid->SetCellValue(wxGridCellCoords(cnt, DidGridCol::Did_Timestamp), last_update_str);
    m_grid->SetReadOnly(cnt, DidGridCol::Did_Timestamp);
    
    grid_to_entry[cnt] = entry.get();
    did_to_row[entry->id] = cnt;
    cnt++;
}

DidPanel::DidPanel(wxFrame* parent)
    : wxPanel(parent, wxID_ANY)
{
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

    static_box_grid = new wxStaticBoxSizer(wxHORIZONTAL, this, "&DID Managment");
    static_box_grid->GetStaticBox()->SetFont(static_box_grid->GetStaticBox()->GetFont().Bold());
    static_box_grid->GetStaticBox()->SetForegroundColour(*wxRED);

    did_grid = new DidGrid(this);

    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_RefreshDids = new wxButton(this, wxID_ANY, "Refresh", wxDefaultPosition, wxDefaultSize);
    m_RefreshDids->SetToolTip("Start refreshing process for DIDs)");
    m_RefreshDids->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            if(can_sender_state != DidStateMachine::Idle)  /* Abort */
            {
                m_RefreshDids->SetLabelText("Refresh");
                m_RefreshDids->SetBackgroundColour(wxNullColour);
                can_sender_state = DidStateMachine::Idle;
                did_cnt = 0;

                std::unique_ptr<DidHandler>& did_handler = wxGetApp().did_handler;
                did_it = did_handler->m_DidList.end();
            }
            else  /* Refresh */
            {
                m_RefreshDids->SetLabelText("Interrupt");
                m_RefreshDids->SetBackgroundColour(*wxRED);
                can_sender_state = DidStateMachine::Start;

                int num_rows = did_grid->m_grid->GetNumberRows();
                for(int i = 0; i != num_rows; i++)
                {
                    //wxFont cell_font = did_grid->m_grid->GetCellFont(i, Did_Name);
                    wxFont cell_font;
                    cell_font.SetWeight(wxFONTWEIGHT_NORMAL);
                    did_grid->m_grid->SetCellFont(i, Did_Name, cell_font);
                    did_grid->m_grid->SetCellBackgroundColour(did_cnt, DidGridCol::Did_Value, wxNullColour);
                }
                did_grid->m_grid->Update();
            }
        });
    h_sizer->Add(m_RefreshDids);    
    
    m_RefreshSelected = new wxButton(this, wxID_ANY, "Refresh selected", wxDefaultPosition, wxDefaultSize);
    m_RefreshSelected->SetToolTip("Start refreshing process for selected DIDs)");
    m_RefreshSelected->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::unique_ptr<DidHandler>& did_handler = wxGetApp().did_handler;
            wxGrid* m_grid = did_grid->m_grid;

            wxArrayInt rows = m_grid->GetSelectedRows();
            if(rows.empty()) return;

            for(auto& i : rows)
            {
                uint16_t did = std::stoi(did_grid->m_grid->GetCellValue(wxGridCellCoords(i, DidGridCol::Did_ID)).ToStdString(), nullptr, 16);
                std::unique_lock lock{ did_handler->m };  /* TODO: solve this deadlock if it's called while already being processed */
                did_handler->AddDidToReadQueue(did); 
            }
            did_handler->NotifyDidUpdate();
        });
    h_sizer->Add(m_RefreshSelected);

    m_ClearDids = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize);
    m_ClearDids->SetToolTip("Clear refreshed DID values)");
    m_ClearDids->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            int num_rows = did_grid->m_grid->GetNumberRows();
            for(int i = 0; i != num_rows; i++)
            {
                //wxFont cell_font = did_grid->m_grid->GetCellFont(i, Did_Name);
                wxFont cell_font;
                cell_font.SetWeight(wxFONTWEIGHT_NORMAL);
                did_grid->m_grid->SetCellFont(i, Did_Name, cell_font);

                did_grid->m_grid->SetCellValue(i, Did_Value, "");
                did_grid->m_grid->SetCellBackgroundColour(i, DidGridCol::Did_Value, wxNullColour);

                for(uint8_t x = 0; x != DidGridCol::Did_Max; x++)
                    did_grid->m_grid->SetCellBackgroundColour(i, x, (i & 1) ? 0xE6E6E6 : 0xFFFFFF);
            }
        });
    h_sizer->Add(m_ClearDids);

    wxBoxSizer* h_sizer_2 = new wxBoxSizer(wxHORIZONTAL);
    m_SaveCache = new wxButton(this, wxID_ANY, "Save cache", wxDefaultPosition, wxDefaultSize);
    m_SaveCache->SetToolTip("Save cached values for DIDs)");
    m_SaveCache->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
            std::unique_ptr<DidHandler>& did_handler = wxGetApp().did_handler;
            
            bool ret = did_handler->SaveChache();
            if(ret)
            {
                std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
                int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

                char work_dir[1024] = {};
#ifdef _WIN32
                GetCurrentDirectoryA(sizeof(work_dir) - 1, work_dir);
#endif
                MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
                std::lock_guard lock(frame->mtx);
                frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::DidCacheSaved), dif, std::string(work_dir) + "\\" + DID_CACHE_FILENAME });
            }
        });
    h_sizer_2->Add(m_SaveCache);

    static_box_grid->Add(did_grid->m_grid, 0, wxALL, 5);
    bSizer1->Add(static_box_grid, wxSizerFlags(0).Top());
    bSizer1->Add(h_sizer);
    bSizer1->Add(h_sizer_2);

    SetSizer(bSizer1);
    Layout();
}

void DidPanel::UpdateDidList()
{
    std::unique_ptr<DidHandler>& did_handler = wxGetApp().did_handler;
    if(did_grid->m_grid->GetNumberRows())
        did_grid->m_grid->DeleteRows(0, did_grid->m_grid->GetNumberRows());
    did_grid->cnt = 0;

    did_handler->m_UpdatedDids.clear();
    for(auto& i : did_handler->m_DidList)
    {
        bool add_row = false;
        if(search_pattern.empty())
            add_row = true;
        else
        {
            if(boost::icontains(i.second->name, search_pattern))
                add_row = true;
        }

        if(add_row)
        {
            did_grid->AddRow(i.second);
            did_handler->m_UpdatedDids.push_back(i.first);
        }
    }
}

void DidPanel::On100msTimer()
{
    std::unique_ptr<DidHandler>& did_handler = wxGetApp().did_handler;
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;

    if(!is_dids_initialized)
    {
        for(auto& i : did_handler->m_DidList)
        {
            did_grid->AddRow(i.second);
        }
        is_dids_initialized = true;
        did_it = did_handler->m_DidList.end();
    }

    if(!did_handler->m_UpdatedDids.empty())
    {
        //std::unique_lock lock{ did_handler->m };
        for(auto& did : did_handler->m_UpdatedDids)
        {
            auto& did_it = did_handler->m_DidList[did];
            uint16_t did_row = did_grid->did_to_row[did];

            if(did_it->nrc != 0)
            {
                switch(did_it->nrc)
                {
                    case 0x78:
                    {
                        did_grid->m_grid->SetCellValue(wxGridCellCoords(did_row, DidGridCol::Did_Value), "Pending... NRC 78");
                        did_grid->m_grid->SetCellBackgroundColour(did_row, DidGridCol::Did_Value, *wxBLUE);
                        break;
                    }
                    default:
                    {
                        did_grid->m_grid->SetCellValue(wxGridCellCoords(did_row, DidGridCol::Did_Value), wxString::Format("NRC %X", did_it->nrc));
                        did_grid->m_grid->SetCellBackgroundColour(did_row, DidGridCol::Did_Value, *wxRED);

                        wxFont cell_font = did_grid->m_grid->GetCellFont(did_row, Did_Name);
                        cell_font.SetWeight(wxFONTWEIGHT_NORMAL);
                        did_grid->m_grid->SetCellFont(did_row, Did_Name, cell_font);
                        break;
                    }
                }
            }
            else
            {
                did_grid->m_grid->SetCellValue(wxGridCellCoords(did_row, DidGridCol::Did_Value), did_it->value_str);
                wxFont cell_font = did_grid->m_grid->GetCellFont(did_row, Did_Name);
                cell_font.SetWeight(wxFONTWEIGHT_BOLD);
                did_grid->m_grid->SetCellFont(did_row, Did_Name, cell_font);
                did_grid->m_grid->SetCellBackgroundColour(did_row, DidGridCol::Did_Value, (did_row & 1) ? 0xE6E6E6 : 0xFFFFFF);
            }

            if(!did_it->last_update.is_not_a_date_time())
            {
                wxString last_update_str = boost::posix_time::to_iso_extended_string(did_it->last_update);
                did_grid->m_grid->SetCellValue(wxGridCellCoords(did_row, DidGridCol::Did_Timestamp), last_update_str);
            }
        }
        did_handler->m_UpdatedDids.clear();
    }
}

void DidPanel::WriteDid(uint16_t did, uint8_t* data_to_write, uint16_t size)
{
    std::unique_ptr<DidHandler>& did_handler = wxGetApp().did_handler;
    did_handler->WriteDid(did, data_to_write, size);
    did_handler->NotifyDidUpdate();

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    std::lock_guard lock(frame->mtx);
    frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::DidUpdated) });

}
void DidPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip(true);
}

void DidPanel::OnCellValueChanged(wxGridEvent& ev)
{
    int row = ev.GetRow(), col = ev.GetCol();
    if(ev.GetEventObject() == dynamic_cast<wxObject*>(did_grid->m_grid))
    {
        std::unique_ptr<DidHandler>& did_handler = wxGetApp().did_handler;
        std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;

        wxString did_str = did_grid->m_grid->GetCellValue(row, DidGridCol::Did_ID);
        uint32_t did_id = std::stoi(did_str.ToStdString(), nullptr, 16);

        std::unique_ptr<DidEntry>& did_it = did_handler->m_DidList[did_id];

        uint32_t hex_val = 0;
        std::string hex_str;

        bool is_ok = true;
        try
        {
            hex_str = did_grid->m_grid->GetCellValue(row, DidGridCol::Did_Value);
            if(did_it->type == DET_UI8 || did_it->type == DET_UI16 || did_it->type == DET_UI32 || did_it->type == DET_UI64)
                hex_val = std::stoi(hex_str, nullptr, 16);
        }
        catch(const std::exception& e)
        {
            LOG(LogLevel::Error, "stoi exception: {}", e.what());
            //can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(row, CanSenderGridCol::Sender_LogLevel), wxString::Format("%d", can_handler->m_rxData[frame_id]->log_level));
            is_ok = false;
        }

        if(did_handler->m_DidList[did_id] && is_ok)
        {
            switch(did_it->type)
            {
                case DET_UI8:
                {
                    uint8_t val_to_write = static_cast<uint8_t>(hex_val);
                    WriteDid(did_it->id, &val_to_write, sizeof(val_to_write));
                    break;
                }
                case DET_UI16:
                {
                    uint16_t val_to_write = static_cast<uint8_t>(hex_val);
                    WriteDid(did_it->id, (uint8_t*)&val_to_write, sizeof(val_to_write));
                    break;
                }
                case DET_UI32:
                {
                    uint32_t val_to_write = static_cast<uint8_t>(hex_val);
                    WriteDid(did_it->id, (uint8_t*)&val_to_write, sizeof(val_to_write));
                    break;
                }
                case DET_STRING:
                {
                    if(hex_str.length() < did_it->len)
                    {
                        while(hex_str.length() < did_it->len)
                        {
                            hex_str += hex_str.back();
                        }
                    }
                    else
                    {
                        hex_str.erase(did_it->len - 1, hex_str.length() - did_it->len);
                    }
                    uint8_t* byte_array = (uint8_t*)const_cast<const char*>(hex_str.c_str());

                    WriteDid(did_it->id, byte_array, hex_str.length());
                    break;
                }
                case DET_BYTEARRAY:
                {
                    char byte_array[MAX_ISOTP_FRAME_LEN];
                    boost::algorithm::erase_all(hex_str, " ");
                    boost::algorithm::erase_all(hex_str, ".");
                    
                    uint16_t len = (hex_str.length() / 2);
                    if(len == 0)
                    {
                        LOG(LogLevel::Warning, "Skipping IsoTP frame, input length is zero");
                        break;
                    }

                    if(hex_str.length() < did_it->len)
                    {
                        while(hex_str.length() < did_it->len)
                        {
                            hex_str += hex_str.back();
                        }
                    }
                    else
                    {
                        hex_str.erase(did_it->len - 1, hex_str.length() - did_it->len);
                    }

                    utils::ConvertHexStringToBuffer(hex_str, std::span{ byte_array });

                    WriteDid(did_it->id, (uint8_t*)&byte_array, len);
                    break;
                }
            }
        }
        else
        {
            LOG(LogLevel::Error, "Invalid DID: {:X}", did_id);
        }
    }
}

void DidPanel::OnCellEditorShown(wxGridEvent& ev)
{
    ev.Skip();
    return;

    int row = ev.GetRow(), col = ev.GetCol();
    if(ev.GetEventObject() == dynamic_cast<wxObject*>(did_grid->m_grid))
    {
        did_grid->m_grid->SetReadOnly(row, col);
        std::unique_ptr<DidHandler>& did_handler = wxGetApp().did_handler;
        wxString did_str = did_grid->m_grid->GetCellValue(row, DidGridCol::Did_ID);
        uint32_t did_id = std::stoi(did_str.ToStdString(), nullptr, 16);

        if(did_handler->m_DidList[did_id])
        {
            std::unique_ptr<DidEntry>& did_it = did_handler->m_DidList[did_id];
            switch(did_it->type)
            {
                case DET_STRING:
                {
                    wxTextEntryDialog d(this, "DID Value", "Value");
                    d.SetValue(did_it->value_str);
                    int ret = d.ShowModal();
                    if(ret == wxID_OK)
                    {

                    }
                    break;
                }
                case DET_BYTEARRAY:
                {
                    wxTextEntryDialog d(this, "DID Value", "Value");
                    d.SetValue(did_it->value_str);
                    int ret = d.ShowModal();
                    if(ret == wxID_OK)
                    {

                    }
                    break;
                }
            }
        }
    }
    did_grid->m_grid->SetReadOnly(row, col, false);
    ev.Skip();
}

void DidPanel::OnKeyDown(wxKeyEvent& evt)
{
    if(evt.ControlDown())
    {
        switch(evt.GetKeyCode())
        {
            case 'F':
            {
                wxWindow* focus = wxWindow::FindFocus();
                if(evt.GetEventObject() == dynamic_cast<wxObject*>(did_grid->m_grid) || 1)
                {
                    wxTextEntryDialog d(this, "Enter DID name for what you want to filter", "Search for DID name");
                    int ret = d.ShowModal();
                    if(ret == wxID_OK)
                    {
                        search_pattern = d.GetValue().ToStdString();
                        if(search_pattern.empty())
                            static_box_grid->GetStaticBox()->SetLabelText("DID Managment");
                        else
                            static_box_grid->GetStaticBox()->SetLabelText(wxString::Format("DID Managment - Search filter: %s", search_pattern));

                        UpdateDidList();
                    }
                    return;
                }
                break;
            }
        }
    }
    evt.Skip();
}