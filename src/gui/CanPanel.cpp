#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CanPanel, wxPanel)
EVT_GRID_CELL_CHANGED(CanPanel::OnCellValueChanged)
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

    m_grid->SetColLabelValue(0, "ID");
    m_grid->SetColLabelValue(1, "DLC");
    m_grid->SetColLabelValue(2, "Data");
    m_grid->SetColLabelValue(3, "Period");
    m_grid->SetColLabelValue(4, "Count");
    m_grid->SetColLabelValue(5, "Comment");

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

    m_grid->SetColSize(1, 10);
    m_grid->SetColSize(5, 100);
}

void CanGrid::AddRow(wxString id, wxString dlc, wxString data, wxString period, wxString count, wxString comment)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Id), id);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Dlc), dlc);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Data), data);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Period), period);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Count), count);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Comment), comment);

    cnt++;
}

void CanGrid::AddRow(std::unique_ptr<CanTxEntry>& e)
{
    int num_rows = m_grid->GetNumberRows();
    if(num_rows <= cnt)
        m_grid->AppendRows(1);

    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Id), wxString::Format("%X", e->id));
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Dlc), wxString::Format("%lld", e->data.size()));

    std::string hex;
    boost::algorithm::hex(e->data.begin(), e->data.end(), std::back_inserter(hex));
    utils::separate<2, ' '>(hex);
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Data), hex);

    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Period), wxString::Format("%d", e->period));
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Count), "0");
    m_grid->SetCellValue(wxGridCellCoords(cnt, Col_Comment), e->comment);

    grid_to_entry[cnt] = e.get();
    cnt++;
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

    m_grid->SetColLabelValue(0, "ID");
    m_grid->SetColLabelValue(1, "DLC");
    m_grid->SetColLabelValue(2, "Data");
    m_grid->SetColLabelValue(3, "Period");
    m_grid->SetColLabelValue(4, "Count");
    m_grid->SetColLabelValue(5, "Comment");

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

    m_grid->SetColSize(1, 10);
    m_grid->SetColSize(5, 100);
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
        Refresh();

        static_box_sizer->Add(can_grid_tx->m_grid, 0, wxALL, 5);
        bSizer1->Add(static_box_sizer, 0, wxALL, 5);

        wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
        m_SingleShot = new wxButton(this, wxID_ANY, "Single Shot", wxDefaultPosition, wxDefaultSize);
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

                can_handler->entries.push_back(std::make_unique<CanTxEntry>());
                Refresh();
            });
        h_sizer->Add(m_Add);

        m_Copy = new wxButton(this, wxID_ANY, "Copy", wxDefaultPosition, wxDefaultSize);
        m_Copy->SetToolTip("Copy CAN frame");
        h_sizer->Add(m_Copy);

        m_Delete = new wxButton(this, wxID_ANY, "Delete", wxDefaultPosition, wxDefaultSize);
        m_Delete->SetToolTip("Delete CAN frame");
        h_sizer->Add(m_Delete);
        
        bSizer1->Add(h_sizer);
	}	

	SetSizer(bSizer1);
	Show();
}

void CanPanel::On10MsTimer()
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    for(auto& entry : can_handler->m_rxData)
    {
        bool found_in_grid = false;
        for(auto& i : can_grid_rx->rx_grid_to_entry)
        {
            if(can_grid_rx->rx_grid_to_entry[i.first] == entry.second.get())
            {
                can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(i.first, Col_Id), wxString::Format("%X", entry.first));
                can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(i.first, Col_Dlc), wxString::Format("%lld", entry.second->data.size()));
                
                std::string hex;
                boost::algorithm::hex(entry.second->data.begin(), entry.second->data.end(), std::back_inserter(hex));
                utils::separate<2, ' '>(hex);
                can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(i.first, Col_Data), hex);

                can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(i.first, Col_Period), wxString::Format("%d", entry.second->period));
                can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(i.first, Col_Count), wxString::Format("%lld", entry.second->count));

                found_in_grid = true;
                break;
            }
        }

        if(!found_in_grid)
        {
            can_grid_rx->m_grid->AppendRows(1);
            int num_row = can_grid_rx->m_grid->GetNumberRows() - 1;
            can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Period), "0");
            can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Count), wxString::Format("%lld", entry.second->count));
            //can_grid_rx->m_grid->SetCellValue(wxGridCellCoords(num_row, Col_Comment), entry.second->comment); // TODO: add RX list
            can_grid_rx->rx_grid_to_entry[num_row] = entry.second.get();

            can_grid_rx->m_grid->SetReadOnly(num_row, Col_Id);
            can_grid_rx->m_grid->SetReadOnly(num_row, Col_Dlc);
            can_grid_rx->m_grid->SetReadOnly(num_row, Col_Data);
            can_grid_rx->m_grid->SetReadOnly(num_row, Col_Period);
            can_grid_rx->m_grid->SetReadOnly(num_row, Col_Count);
            can_grid_rx->m_grid->SetReadOnly(num_row, Col_Id);
            can_grid_rx->m_grid->SetReadOnly(num_row, Col_Id);
        }
    }
}

void CanPanel::Refresh()
{
    CanEntryHandler* can_handler = wxGetApp().can_entry;
    can_grid_tx->m_grid->DeleteRows(0, can_grid_tx->m_grid->GetNumberRows());
    can_grid_tx->cnt = 0;
    can_grid_tx->grid_to_entry.clear();
    for(auto& i : can_handler->entries)
    {
        can_grid_tx->AddRow(i);
    }
}

void CanPanel::OnCellValueChanged(wxGridEvent& ev)
{
    int row = ev.GetRow(), col = ev.GetCol();
    wxString new_value = can_grid_tx->m_grid->GetCellValue(row, col);
    if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_rx->m_grid))
    {
        
    }
    else if(ev.GetEventObject() == dynamic_cast<wxObject*>(can_grid_tx->m_grid))
    {
        switch(col)
        {
            case Col_Id:
            {
                can_grid_tx->grid_to_entry[row]->id = std::stoi(new_value.ToStdString(), nullptr, 16);
                break;
            }
            case Col_Dlc:
            {
                uint32_t new_size = std::stoi(new_value.ToStdString());
                if(new_size > 8) return;

                can_grid_tx->grid_to_entry[row]->data.resize(new_size);
                break;
            }
            case Col_Data:
            {
                char bytes[128] = { 0 };
                std::string hex_str = new_value.ToStdString();
                boost::algorithm::erase_all(hex_str, " ");
                std::string hash = boost::algorithm::unhex(hex_str);
                std::copy(hash.begin(), hash.end(), bytes);

                can_grid_tx->grid_to_entry[row]->data.insert(can_grid_tx->grid_to_entry[row]->data.end(), bytes, bytes + 8);
                break;
            }
            case Col_Period:
            {
                if(new_value == "off")
                    new_value = "0";
                can_grid_tx->grid_to_entry[row]->period = std::stoi(new_value.ToStdString(), nullptr, 16);
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
    can_handler->LoadTxList(p);
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
}

void CanPanel::LoadRxList()
{
    wxFileDialog openFileDialog(this, _("Open RX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    file_path_rx = openFileDialog.GetPath();
}

void CanPanel::SaveRxList()
{
    wxFileDialog saveFileDialog(this, _("Save RX XML file"), "", "", "XML files (*.xml)|*.xml", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if(saveFileDialog.ShowModal() == wxID_CANCEL)
        return;
    file_path_rx = saveFileDialog.GetPath();
}

/*
wxString Strid = m_grid->GetCellValue(wxGridCellCoords(i, Col_Id));
wxString Strdata_len = m_grid->GetCellValue(wxGridCellCoords(i, Col_Dlc));
wxString Strdata = m_grid->GetCellValue(wxGridCellCoords(i, Col_Data));
wxString Strperiod = m_grid->GetCellValue(wxGridCellCoords(i, Col_Period));
wxString Strcount = m_grid->GetCellValue(wxGridCellCoords(i, Col_Count));
wxString Strcomment = m_grid->GetCellValue(wxGridCellCoords(i, Col_Comment));

char bytes[128] = { 0 };
std::string hex_str = Strdata.ToStdString();
boost::algorithm::erase_all(hex_str, " ");
std::string hash = boost::algorithm::unhex(hex_str);
std::copy(hash.begin(), hash.end(), bytes);

uint32_t frame_id = std::stoi(Strid.ToStdString(), nullptr, 16);
uint8_t data_len = std::stoi(Strdata_len.ToStdString());
CanSerialPort::Get()->AddToTxQueue(frame_id, data_len, (uint8_t*)&bytes);

if(Strperiod == "off")
Strperiod = "0";
uint32_t period = std::stoi(Strperiod.ToStdString());
*/