#include "pch.hpp"

wxBEGIN_EVENT_TABLE(BitEditorDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, BitEditorDialog::OnApply)
EVT_BUTTON(wxID_OK, BitEditorDialog::OnOk)
EVT_BUTTON(wxID_CLOSE, BitEditorDialog::OnCancel)
EVT_CLOSE(BitEditorDialog::OnClose)
wxEND_EVENT_TABLE()

BitEditorDialog::BitEditorDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Bit editor", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    sizerTop = new wxBoxSizer(wxVERTICAL);
    sizerMsgs = new wxStaticBoxSizer(wxVERTICAL, this, "&Bit editor");

    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_IsDecimal = new wxRadioButton(this, wxID_ANY, "Decimal");
    m_IsDecimal->Bind(wxEVT_RADIOBUTTON, &BitEditorDialog::OnRadioButtonClicked, this);
    h_sizer->Add(m_IsDecimal);
    m_IsHex = new wxRadioButton(this, wxID_ANY, "Hex");
    m_IsHex->Bind(wxEVT_RADIOBUTTON, &BitEditorDialog::OnRadioButtonClicked, this);
    h_sizer->Add(m_IsHex);
    m_IsBinary = new wxRadioButton(this, wxID_ANY, "Binary");
    m_IsBinary->Bind(wxEVT_RADIOBUTTON, &BitEditorDialog::OnRadioButtonClicked, this);
    h_sizer->Add(m_IsBinary);

    sizerMsgs->Add(h_sizer);
    sizerMsgs->AddSpacer(20);

    for(int i = 0; i != MAX_BITEDITOR_FIELDS; i++)
    {
        m_InputLabel[i] = new wxStaticText(this, wxID_ANY, "_");
        sizerMsgs->Add(m_InputLabel[i], 1, wxLEFT | wxEXPAND, 0);
        m_Input[i] = new wxTextCtrl(this, wxID_ANY, "_", wxDefaultPosition, wxSize(250, 25), 0);
        sizerMsgs->Add(m_Input[i], 1, wxLEFT | wxEXPAND, 0);
    }

    sizerTop->Add(sizerMsgs, wxSizerFlags(1).Expand().Border());

    // finally buttons to show the resulting message box and close this dialog
    sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCLOSE | wxOK), wxSizerFlags().Right().Border()); /* wxOK */

    sizerTop->SetMinSize(wxSize(200, 200));
    SetAutoLayout(true);
    SetSizer(sizerTop);
    sizerTop->Fit(this);
    //sizerTop->SetSizeHints(this);
    CentreOnScreen();
}

void BitEditorDialog::ShowDialog(uint32_t frame_id, bool is_rx, CanBitfieldInfo& values)
{
    if(values.size() > MAX_BITEDITOR_FIELDS)
    {
        values.resize(MAX_BITEDITOR_FIELDS);
        LOG(LogLevel::Warning, "Too much bitfields used for can frame mapping. FrameID: {:X}, Used: {}, Maximum supported: {}", frame_id, values.size(), MAX_BITEDITOR_FIELDS);
    }

    m_Id = 0;
    m_DataFormat = 0;
    m_BitfieldInfo = values;

    for(const auto& [label, value, frame] : values)
    {
        m_InputLabel[m_Id]->SetLabelText(label);

        m_InputLabel[m_Id]->SetForegroundColour(RGB_TO_WXCOLOR(frame->m_color));  /* input for red: 0x00FF0000, excepted input for wxColor 0x0000FF */
        m_InputLabel[m_Id]->SetBackgroundColour(RGB_TO_WXCOLOR(frame->m_bg_color));

        wxFont font;
        font.SetWeight(frame->m_is_bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
        font.Scale(1.0f);  /* Scale has to be set to default first */
        m_InputLabel[m_Id]->SetFont(font);
        font.Scale(frame->m_scale);
        font.SetFaceName("Segoe UI");
        m_InputLabel[m_Id]->SetFont(font);

        m_InputLabel[m_Id]->Show();
        m_InputLabel[m_Id]->SetToolTip(frame->m_Description);
        m_Input[m_Id]->SetValue(value);
        m_Input[m_Id]->Show();
        m_Id++;
    }

    for(int i = m_Id; i != MAX_BITEDITOR_FIELDS; i++)
    {
        m_InputLabel[i]->Hide();
        m_InputLabel[i]->SetToolTip("");
        m_Input[i]->Hide();
    }

    SetTitle(wxString::Format("Bit editor - %X (%s)", frame_id, is_rx ? "RX" : "TX"));
    bit_sel = BitSelection::Decimal;
    m_IsDecimal->SetValue(true);
    m_IsHex->SetValue(false);
    m_IsBinary->SetValue(false);
    m_FrameId = frame_id;

    sizerTop->Layout();
    sizerTop->Fit(this);

    m_ClickType = ClickType::None;
    int ret = ShowModal();
    DBG("ShowModal ret: %d\n", ret);
}

std::vector<std::string> BitEditorDialog::GetOutput()
{
    std::vector<std::string> ret;
    for(int i = 0; i != m_Id; i++)
    {
        std::string input_ret = m_Input[i]->GetValue().ToStdString();
        switch(bit_sel)
        {
        case BitSelection::Hex:
        {
            try
            {
                input_ret = std::to_string(std::stoll(input_ret, nullptr, 16));
            }
            catch(...)
            {
                LOG(LogLevel::Error, "Exception with std::stoll, str: {}", input_ret);
            }
            break;
        }
        case BitSelection::Binary:
        {
            try
            {
                input_ret = std::to_string(std::stoll(input_ret, nullptr, 2));
            }
            catch(...)
            {
                LOG(LogLevel::Error, "Exception with std::to_string, str: {}", input_ret);
            }
            break;
        }
        }

        DBG("input_ret: %s\n", input_ret.c_str());
        ret.push_back(input_ret);
    }
    return ret;
}

void BitEditorDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    DBG("OnApply %d\n", (int)m_ClickType)
        EndModal(wxID_APPLY);
    //Close();
    m_ClickType = ClickType::Apply;
}

void BitEditorDialog::OnOk(wxCommandEvent& event)
{
    DBG("OnOK %d\n", (int)m_ClickType);
    if(m_ClickType == ClickType::Close)
        return;
    EndModal(wxID_OK);
    //Close();
    m_ClickType = ClickType::Ok;
    event.Skip();
}

void BitEditorDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    DBG("OnCancel %d\n", (int)m_ClickType);
    EndModal(wxID_CLOSE);
    m_ClickType = ClickType::Close;
    //Close();
}

void BitEditorDialog::OnClose(wxCloseEvent& event)
{
    DBG("OnClose %d\n", (int)m_ClickType);
    m_ClickType = ClickType::Close;
    EndModal(wxID_CLOSE);
}

void BitEditorDialog::OnRadioButtonClicked(wxCommandEvent& event)
{
    if(event.GetEventObject() == dynamic_cast<wxObject*>(m_IsDecimal))
    {
        uint8_t cnt = 0;
        for(const auto& [label, value, frame] : m_BitfieldInfo)
        {
            m_Input[cnt]->SetValue(value);
            if(++cnt > m_Id)
                break;
        }
        bit_sel = BitSelection::Decimal;
    }
    else if(event.GetEventObject() == dynamic_cast<wxObject*>(m_IsHex))
    {
        uint8_t cnt = 0;
        for(const auto& [label, value, frame] : m_BitfieldInfo)
        {
            if(utils::is_number(value))
            {
                uint64_t decimal_val = std::stoll(value);
                std::string hex_str = std::format("{:X}", decimal_val);  /* std::to_chars gives lower case letters, I don't like it :/ */

                m_Input[cnt]->SetValue(hex_str);
                if(++cnt > m_Id)
                    break;
            }
        }
        bit_sel = BitSelection::Hex;
    }
    else if(event.GetEventObject() == dynamic_cast<wxObject*>(m_IsBinary))
    {
        uint8_t cnt = 0;
        for(const auto& [label, value, frame] : m_BitfieldInfo)
        {
            if(utils::is_number(value))
            {
                uint64_t decimal_val = std::stoll(value);
                std::string hex_str = std::format("{:b}", decimal_val);

                m_Input[cnt]->SetValue(hex_str);
                if(++cnt > m_Id)
                    break;
            }
        }
        bit_sel = BitSelection::Binary;
    }
}
