#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CanPanel, wxPanel)
EVT_GRID_CELL_CHANGED(CanPanel::OnCellValueChanged)
EVT_SIZE(CanPanel::OnSize)
wxEND_EVENT_TABLE()

using namespace std::chrono_literals;

CanGrid::CanGrid(wxWindow* parent)
{
    m_grid = new wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(800, 250), 0);

    // Grid
    m_grid->CreateGrid(1, Col_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(Col_Id, "ID");
    m_grid->SetColLabelValue(Col_DataSize, "Size");
    m_grid->SetColLabelValue(Col_Data, "Received Data");
    m_grid->SetColLabelValue(Col_Period, "Period");
    m_grid->SetColLabelValue(Col_Count, "Count");
    m_grid->SetColLabelValue(Col_Comment, "Comment");

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
    /*
    m_grid->SetColSize(Col_Id, 60);
    m_grid->SetColSize(Col_DataSize, 30);*/
    m_grid->SetColSize(Col_Data, 200);
    m_grid->SetColSize(Col_Comment, 200);
}

void CanGrid::AddRow(wxString id, wxString dlc, wxString data, wxString period, wxString count, wxString comment)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Id), id);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_DataSize), dlc);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Data), data);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Period), period);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Count), count);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Comment), comment);

    m_grid->SetCellEditor(cnt, Col_Id, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, Col_DataSize, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, Col_Period, new wxGridCellNumberEditor);
    cnt++;
}

void CanGrid::AddRow(std::unique_ptr<CanTxEntry>& e)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Id), wxString::Format("%X", e->id));
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_DataSize), wxString::Format("%lld", e->data.size()));

    std::string hex;
    boost::algorithm::hex(e->data.begin(), e->data.end(), std::back_inserter(hex));
    utils::separate<2, ' '>(hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Data), hex);

    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Period), wxString::Format("%d", e->period));
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Count), "0");
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Comment), e->comment);

    m_grid->SetCellEditor(cnt, Col_DataSize, new wxGridCellNumberEditor);
    m_grid->SetCellEditor(cnt, Col_Period, new wxGridCellNumberEditor);

    m_grid->SetReadOnly(cnt, Col_Count, true);

    grid_to_entry[cnt] = e.get();
    cnt++;
}

void CanGrid::UpdateTxCounter(uint32_t frame_id, size_t count)
{
    for(auto& i : grid_to_entry)
    {
        if(i.second->id == frame_id)
        {
            m_grid->SetCellValue(wxGridCellCoords(i.first, Col_Count), wxString::Format("%lld", count));
            break;
        }
    }
}

CanGridRx::CanGridRx(wxWindow* parent)
{
    m_grid = new wxGrid(parent, wxID_ANY, wxDefaultPosition, wxSize(800, 250), 0);

    // Grid
    m_grid->CreateGrid(1, Col_Max);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    m_grid->SetColLabelValue(Col_Id, "ID");
    m_grid->SetColLabelValue(Col_DataSize, "Size");
    m_grid->SetColLabelValue(Col_Data, "Received Data");
    m_grid->SetColLabelValue(Col_Period, "Period");
    m_grid->SetColLabelValue(Col_Count, "Count");
    m_grid->SetColLabelValue(Col_Comment, "Comment");

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

    m_grid->SetColSize(Col_Data, 200);
    m_grid->SetColSize(Col_Comment, 200);
}

void CanGridRx::AddRow(std::unique_ptr<CanRxData>& e)
{
    m_grid->AppendRows(1);
    int num_row = m_grid->GetNumberRows() - 1;
    m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Period), "0");
    m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Count), "1");
    //can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Comment), entry.second->comment); // TODO: add RX list
    rx_grid_to_entry[num_row] = e.get();

    m_grid->SetReadOnly(num_row, Col_Id);
    m_grid->SetReadOnly(num_row, Col_DataSize);
    m_grid->SetReadOnly(num_row, Col_Data);
    m_grid->SetReadOnly(num_row, Col_Period);
    m_grid->SetReadOnly(num_row, Col_Count);
    m_grid->SetReadOnly(num_row, Col_Id);
    m_grid->SetReadOnly(num_row, Col_Id);
}

void CanGridRx::UpdateRow(int num_row, uint32_t frame_id, std::unique_ptr<CanRxData>& e, std::string& comment)
{
    m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Id), wxString::Format("%X", frame_id));
    m_grid->SetCellValue(wxGridCellCoords(num_row, Col_DataSize), wxString::Format("%lld", e->data.size()));

    std::string hex;
    try
    {
        boost::algorithm::hex(e->data.begin(), e->data.end(), std::back_inserter(hex));
        utils::separate<2, ' '>(hex);
        m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Data), hex);
    }
    catch(...)  /* TODO: remove it after testing */
    {
        LOG(LogLevel::Critical, "Exception happend");
    }
    m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Period), wxString::Format("%d", e->period));
    m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Count), wxString::Format("%lld", e->count));
    m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Comment), comment);
}

CanPanel::CanPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	{
		wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Receive");
		static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
		static_box_sizer->GetStaticBox()->SetForegroundColour(*wxBLUE);

        can_grid_rx = new CanGridRx(this);

        can_grid_rx->m_grid->DeleteRows(0, can_grid_rx->m_grid->GetNumberRows());
        can_grid_rx->cnt = 0;

        static_box_sizer->Add(can_grid_rx->m_grid, 0, wxALL, 5);
        bSizer1->Add(static_box_sizer, wxSizerFlags(0).Top());
	}

	{
        wxStaticBoxSizer* const static_box_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Transmit");
        static_box_sizer->GetStaticBox()->SetFont(static_box_sizer->GetStaticBox()->GetFont().Bold());
        static_box_sizer->GetStaticBox()->SetForegroundColour(*wxBLUE);

        can_grid_tx = new CanGrid(this);
        RefreshTx();

        static_box_sizer->Add(can_grid_tx->m_grid, 0, wxALL, 5);
        bSizer1->Add(static_box_sizer, 0, wxALL, 5);

        wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
        m_SingleShot = new wxButton(this, wxID_ANY, "One Shot", wxDefaultPosition, wxDefaultSize);
        m_SingleShot->SetToolTip("Single shot mode for selected CAN frame");
        m_SingleShot->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                wxGrid* m_grid = can_grid_tx->m_grid;

                wxArrayInt rows = m_grid->GetSelectedRows();
                if(rows.empty()) return;

                for(auto& i : rows)
                {
                    CanTxEntry* entry = can_grid_tx->grid_to_entry[i];
                    entry->single_shot = true;
                    //m_grid->SetCellBackgroundColour(i, 0, *wxGREEN);
                }
            });
        h_sizer->Add(m_SingleShot);
        

        m_SendAll = new wxButton(this, wxID_ANY, "Send All", wxDefaultPosition, wxDefaultSize);
        m_SendAll->SetToolTip("Send all CAN frame");
        m_SendAll->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                wxGrid* m_grid = can_grid_tx->m_grid;

                for(auto& i : can_grid_tx->grid_to_entry)
                {
                    i.second->single_shot = false;
                    i.second->send = true;
                }
            });
        h_sizer->Add(m_SendAll);

        m_StopAll = new wxButton(this, wxID_ANY, "Stop All", wxDefaultPosition, wxDefaultSize);
        m_StopAll->SetToolTip("Stop all CAN frame");
        m_StopAll->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                wxGrid* m_grid = can_grid_tx->m_grid;

                for(auto& i : can_grid_tx->grid_to_entry)
                {
                    i.second->single_shot = false;
                    i.second->send = false;
                }
            });
        h_sizer->Add(m_StopAll);

        m_Add = new wxButton(this, wxID_ANY, "Add", wxDefaultPosition, wxDefaultSize);
        m_Add->SetToolTip("Add CAN frame to TX list");
        m_Add->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                CanEntryHandler* can_handler = wxGetApp().can_entry;
                wxGrid* m_grid = can_grid_tx->m_grid;

                std::unique_ptr<CanTxEntry> entry = std::make_unique<CanTxEntry>();
                entry->data = { 0, 0, 0, 0, 0, 0, 0, 0 };
                entry->id = 0x123;
                {
                    std::scoped_lock lock{ can_handler->m };
                    can_handler->entries.push_back(std::move(entry));
                }
                RefreshTx();
            });
        h_sizer->Add(m_Add);

        m_Copy = new wxButton(this, wxID_ANY, "Copy", wxDefaultPosition, wxDefaultSize);
        m_Copy->SetToolTip("Copy CAN frame");
        h_sizer->Add(m_Copy);

        m_Delete = new wxButton(this, wxID_ANY, "Delete", wxDefaultPosition, wxDefaultSize);
        m_Delete->SetToolTip("Delete CAN frame");
        h_sizer->Add(m_Delete);
        
        RefreshGuiIconsBasedOnSettings();

        bSizer1->Add(h_sizer);
	}	

	SetSizer(bSizer1);
	Show();
}

void CanPanel::On10MsTimer()
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::scoped_lock lock{ can_handler->m };
    for(auto& entry : can_handler->m_rxData)
    {
        bool found_in_grid = false;
        for(auto& i : can_grid_rx->rx_grid_to_entry)
        {
            if(can_grid_rx->rx_grid_to_entry[i.first] == entry.second.get())
            {
                std::string comment;
                auto it = can_handler->rx_entry_comment.find(entry.first);
                if(it != can_handler->rx_entry_comment.end())
                    comment = it->second;
                can_grid_rx->UpdateRow(i.first, entry.first, entry.second, comment);

                found_in_grid = true;
                break;
            }
        }

        if(!found_in_grid)
        {
            can_grid_rx->AddRow(entry.second);
        }
    }
}

void CanPanel::RefreshSubpanels()
{
    RefreshTx();
    RefreshRx();
    RefreshGuiIconsBasedOnSettings();
}

void CanPanel::RefreshTx()
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::scoped_lock lock{ can_handler->m };
    can_grid_tx->m_grid->DeleteRows(0, can_grid_tx->m_grid->GetNumberRows());
    can_grid_tx->cnt = 0;
    can_grid_tx->grid_to_entry.clear();
    for(auto& i : can_handler->entries)
    {
        can_grid_tx->AddRow(i);
    }
}

void CanPanel::RefreshRx()
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::scoped_lock lock{ can_handler->m };

    for(int i = 0; i != can_grid_rx->m_grid->GetNumberRows(); i++)
    {
        uint16_t frame_id = std::stoi(can_grid_rx->m_grid->GetCellValue(wxGridCellCoords(i, Col_Id)).ToStdString(), nullptr, 16);
        auto it = can_handler->rx_entry_comment.find(frame_id);
        if(it != can_handler->rx_entry_comment.end())
        {
            can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(i, Col_Comment), it->second);
        }
        else
        {
            can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(i, Col_Comment), "");
        }
    }
}

void CanPanel::RefreshGuiIconsBasedOnSettings()
{
    m_SingleShot->Enable(CanSerialPort::Get()->IsEnabled());
    m_SendAll->Enable(CanSerialPort::Get()->IsEnabled());
    m_StopAll->Enable(CanSerialPort::Get()->IsEnabled());
}

void CanPanel::OnCellValueChanged(wxGridEvent& ev)
{
    int row = ev.GetRow(), col = ev.GetCol();
    if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_rx->m_grid))
    {
        wxString new_value = can_grid_tx->m_grid->GetCellValue(row, col);
        switch(col)
        {
            case Col_Comment:
            {
                wxString frame_str = can_grid_rx->m_grid->GetCellValue(row, Col_Id);
                uint32_t frame_id = std::stoi(frame_str.ToStdString(), nullptr, 16);

                CanEntryHandler* can_handler = wxGetApp().can_entry;
                std::scoped_lock lock{ can_handler->m };
                can_handler->rx_entry_comment[frame_id] = std::move(new_value.ToStdString());
                break;
            }
        }
    }
    else if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_tx->m_grid))
    {
        wxString new_value = can_grid_tx->m_grid->GetCellValue(row, col);
        switch(col)
        {
            case Col_Id:
            {
                uint32_t frame_id = std::stoi(new_value.ToStdString(), nullptr, 16);
                CanEntryHandler* can_handler = wxGetApp().can_entry;
                for(auto& i : can_handler->entries)
                {
                    if(i->id == frame_id)
                    {
                        wxMessageDialog(this, "Given CAN Fame ID already added to the list!", "Error", wxOK).ShowModal();
                        can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, Col_Id), wxString::Format("%X", can_grid_tx->grid_to_entry[row]->id));
                        return;
                    }
                }
                can_grid_tx->grid_to_entry[row]->id = frame_id;
                break;
            }
            case Col_DataSize:
            {
                uint32_t new_size = std::stoi(new_value.ToStdString());
                if(new_size > 8)
                {
                    wxMessageDialog(this, "Max payload size is 8!", "Error", wxOK).ShowModal();
                    can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, Col_DataSize), wxString::Format("%lld", can_grid_tx->grid_to_entry[row]->data.size()));
                    return;
                }

                can_grid_tx->grid_to_entry[row]->data.resize(new_size);

                std::string hex;
                boost::algorithm::hex(can_grid_tx->grid_to_entry[row]->data.begin(), can_grid_tx->grid_to_entry[row]->data.end(), std::back_inserter(hex));
                utils::separate<2, ' '>(hex);
                can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, Col_Data), wxString(hex));
                break;
            }
            case Col_Data:
            {
                char bytes[128] = { 0 };
                std::string hex_str = new_value.ToStdString();
                boost::algorithm::erase_all(hex_str, " ");
                if(hex_str.length() > 16)
                    hex_str.erase(16, hex_str.length() - 16);
                std::string hash;
                try
                {
                    hash = boost::algorithm::unhex(hex_str);
                }
                catch(...)
                {
                    LOG(LogLevel::Error, "Exception with boost::algorithm::unhex, str: {}", hex_str);
                }
                std::copy(hash.begin(), hash.end(), bytes);
                can_grid_tx->grid_to_entry[row]->data.assign(bytes, bytes + (hex_str.length() / 2));

                std::string hex;
                boost::algorithm::hex(can_grid_tx->grid_to_entry[row]->data.begin(), can_grid_tx->grid_to_entry[row]->data.end(), std::back_inserter(hex));
                utils::separate<2, ' '>(hex);
                can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, col), wxString(hex));
                can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, Col_DataSize), wxString::Format("%lld", hex_str.length() / 2));
                break;
            }
            case Col_Period:
            {
                if(new_value == "off")
                    new_value = "0";
                int period = std::stoi(new_value.ToStdString());
                if(period < 0)
                {
                    wxMessageDialog(this, "Period can't be negative!", "Error", wxOK).ShowModal();
                    can_grid_tx->m_grid->SetCellValue(wxGridCellCoords(row, Col_Period), wxString::Format("%d", can_grid_tx->grid_to_entry[row]->period));
                    return;
                }
                can_grid_tx->grid_to_entry[row]->period = period;
                break;
            }
            case Col_Comment:
            {
                can_grid_tx->grid_to_entry[row]->comment = std::move(new_value.ToStdString());
                break;
            }
        }
    }
    ev.Skip();
}

void CanPanel::LoadTxList()
{
    wxFileDialog openFileDialog(this, _("Open TX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    file_path_tx = openFileDialog.GetPath();
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_tx.ToStdString();
    bool ret = can_handler->LoadTxList(p);
    RefreshTx();

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if(ret)
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::TxListLoaded) });
    else
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::TxListLoadError) });
}

void CanPanel::SaveTxList()
{
    wxFileDialog saveFileDialog(this, _("Save TX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveFileDialog.ShowModal() == wxID_CANCEL)
        return;
    file_path_tx = saveFileDialog.GetPath();
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_tx.ToStdString();
    can_handler->SaveTxList(p);

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::TxListSaved) });
}

void CanPanel::LoadRxList()
{
    wxFileDialog openFileDialog(this, _("Open RX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    file_path_rx = openFileDialog.GetPath();
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_rx.ToStdString();
    bool ret = can_handler->LoadRxList(p);
    RefreshRx();

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    if(ret)
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::RxListLoaded) });
    else
        frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::RxListLoadError) });
}
 
void CanPanel::SaveRxList()
{
    wxFileDialog saveFileDialog(this, _("Save RX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveFileDialog.ShowModal() == wxID_CANCEL)
        return;
    file_path_rx = saveFileDialog.GetPath();
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    std::filesystem::path p = file_path_rx.ToStdString();
    can_handler->SaveRxList(p);

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    frame->pending_msgs.push_back({ static_cast<uint8_t>(PopupMsgIds::RxListSaved) });
}

void CanPanel::OnSize(wxSizeEvent& evt)
{
    evt.Skip();
}