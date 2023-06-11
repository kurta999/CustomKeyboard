#include "pch.hpp"

wxBEGIN_EVENT_TABLE(CanUdsRawDialog, wxDialog)
EVT_BUTTON(wxID_APPLY, CanUdsRawDialog::OnApply)
wxEND_EVENT_TABLE()

CanUdsRawDialog::CanUdsRawDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "CAN UDS Raw", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    sizerTop = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* sizerMsgs_sender = new wxStaticBoxSizer(wxVERTICAL, this, "&Send");
    wxBoxSizer* h_sizer_0 = new wxBoxSizer(wxHORIZONTAL);
    h_sizer_0->Add(new wxStaticText(this, wxID_ANY, "Sender FrameID:"));
    m_SenderId = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75, 25));
    h_sizer_0->Add(m_SenderId);
    sizerMsgs_sender->AddSpacer(5);

    h_sizer_0->Add(new wxStaticText(this, wxID_ANY, "Receiver FrameID:"));
    m_ReceiverId = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75, 25));
    h_sizer_0->Add(m_ReceiverId);

    h_sizer_0->Add(new wxStaticText(this, wxID_ANY, "Frame delay:"));
    m_DelayBetweenFrames = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75, 25));
    m_DelayBetweenFrames->SetToolTip("Delay between sending frames [ms]");
    h_sizer_0->Add(m_DelayBetweenFrames);

    h_sizer_0->Add(new wxStaticText(this, wxID_ANY, "Recv delay:"));
    m_RecvDelayFrames = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(75, 25));
    m_RecvDelayFrames->SetToolTip("Maximum waiting time for response [ms]");
    h_sizer_0->Add(m_RecvDelayFrames);
    sizerMsgs_sender->Add(h_sizer_0);

    sizerMsgs_sender->AddSpacer(10);

    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_ReadDid = new wxButton(this, wxID_ANY, "Read DID", wxDefaultPosition, wxDefaultSize);
    m_ReadDid->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            m_DataToSend->SetValue("10 03\r\n22 4000");
        });
    h_sizer->Add(m_ReadDid);

    m_WriteDid = new wxButton(this, wxID_ANY, "Write DID", wxDefaultPosition, wxDefaultSize);
    h_sizer->Add(m_WriteDid);
    m_WriteDid->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            m_DataToSend->SetValue("10 03\r\n2E 4000 0C\r\nDELAY 10\r\n22 4000");
        });

    m_EcuReset = new wxButton(this, wxID_ANY, "ECU Reset", wxDefaultPosition, wxDefaultSize);
    h_sizer->Add(m_EcuReset);
    m_EcuReset->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            m_DataToSend->SetValue("10 02");
        });

    m_Clear = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize);
    m_Clear->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            m_DataToSend->SetValue("");
            m_DataRecv->SetValue("");
        });

    h_sizer->Add(m_Clear);
    sizerMsgs_sender->Add(h_sizer);

    sizerMsgs_sender->AddSpacer(10);

    wxStaticText* data_to_send_text = new wxStaticText(this, wxID_ANY, "UDS Data to send:");
    data_to_send_text->SetToolTip("10 - Diagnostic Session Control\n11 - ECU Reset\n3E - Tester present\n22 - Read Data By Identifier\n23 - Read Memory By Address\n\
24 - Read Scaling Data By Identifier\n2A - Read Data By Identifier Periodic\n2E - Write Data By Identifier\n3D - Write Memory By Address\n14 - Delete all DTC\n\
19 - Read DTC Information");

    sizerMsgs_sender->Add(data_to_send_text);
    m_DataToSend = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, 100), wxTE_MULTILINE);
    sizerMsgs_sender->Add(m_DataToSend, 1, wxLEFT | wxEXPAND, 0);

    wxBoxSizer* h_sizer_3 = new wxBoxSizer(wxHORIZONTAL);
    m_SendBtn = new wxButton(this, wxID_ANY, "Send", wxDefaultPosition, wxDefaultSize);
    m_SendBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            HandleFrameSending();
        });

    h_sizer_3->AddStretchSpacer();
    h_sizer_3->Add(m_SendBtn);
    sizerMsgs_sender->Add(h_sizer_3);

    sizerTop->Add(sizerMsgs_sender, wxSizerFlags(1).Expand().Border());

    wxStaticBoxSizer* sizerMsgs_recv = new wxStaticBoxSizer(wxVERTICAL, this, "&Receive");
    wxStaticText* data_to_recv_text = new wxStaticText(this, wxID_ANY, "UDS Response:");
    data_to_recv_text->SetToolTip("11 - serviceNotSupported\n12 - subFunctionNotSupported\n14 - responseTooLong\n21 - busyRepeatReques\n22 - conditionsNotCorrect\n\
31 - requestOutOfRange\n33 - securityAccessDenied\n78 - ResponsePending\n7E - subFunctionNotSupportedInActiveSession\n7F -serviceNotSupportedInActiveSession");

    sizerMsgs_recv->Add(data_to_recv_text);
    m_DataRecv = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, 100), wxTE_MULTILINE);
    sizerMsgs_recv->Add(m_DataRecv, 1, wxLEFT | wxEXPAND, 0);
    sizerTop->Add(sizerMsgs_recv, wxSizerFlags(1).Expand().Border());

    // finally buttons to show the resulting message box and close this dialog
    sizerTop->Add(CreateStdDialogButtonSizer(wxCLOSE), wxSizerFlags().Right().Border()); /* wxOK */

    sizerTop->SetMinSize(wxSize(640, 480));
    SetAutoLayout(true);
    SetSizer(sizerTop);
    sizerTop->Fit(this);
    sizerTop->SetSizeHints(this);
    CentreOnScreen();
}

CanUdsRawDialog::~CanUdsRawDialog()
{
    if(m_isotp_future.valid())
        if(m_isotp_future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready)
            m_isotp_future.get();
}

void CanUdsRawDialog::ShowDialog()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    if(m_LastUdsSenderId == 0)
    {
        m_LastUdsSenderId = can_handler->GetDefaultEcuId();
        m_LastUdsReceiverId = can_handler->GetIsoTpResponseFrameId();
        m_LastDelayBetweenFrames = 100;
        m_LastRecvWaitingTime = 350;
        m_LastUdsInput.clear();
    }

    m_IsApplyClicked = false;
    m_SenderId->SetValue(wxString::Format("%X", m_LastUdsSenderId));
    m_ReceiverId->SetValue(wxString::Format("%X", m_LastUdsReceiverId));
    m_DelayBetweenFrames->SetValue(wxString::Format("%d", m_LastDelayBetweenFrames));
    m_RecvDelayFrames->SetValue(wxString::Format("%d", m_LastRecvWaitingTime));
    m_DataRecv->SetValue("");
    ShowModal();
}

uint32_t CanUdsRawDialog::GetSenderId()
{
    uint32_t ret = 0;
    try
    {
        ret = std::stoi(m_SenderId->GetValue().ToStdString(), 0, 16);
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
    }
    return ret;
}

uint32_t CanUdsRawDialog::GetReceiverId()
{
    uint32_t ret = 0;
    try
    {
        ret = std::stoi(m_ReceiverId->GetValue().ToStdString(), 0, 16);
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
    }
    return ret;
}

uint32_t CanUdsRawDialog::GetDelayBetweenFrames()
{
    uint32_t ret = 0;
    try
    {
        ret = std::stoi(m_DelayBetweenFrames->GetValue().ToStdString());
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
    }
    return ret;
}

uint32_t CanUdsRawDialog::GetWaitingTimeForFrames()
{
    uint32_t ret = 0;
    try
    {
        ret = std::stoi(m_RecvDelayFrames->GetValue().ToStdString());
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Warning, "stoi exception: {}", e.what());
    }
    return ret;
}

std::string CanUdsRawDialog::GetSentData()
{
    return m_DataToSend->GetValue().ToStdString();
}

void CanUdsRawDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    m_IsApplyClicked = true;
    Close();
}

bool SendIsoTpFrameGlobal(uint32_t sender_id, char* arr_to_send, uint16_t len)
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    can_handler->SendIsoTpFrame(sender_id, (uint8_t*)arr_to_send, len);
    return true;
}

void CanUdsRawDialog::HandleFrameSending()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;

    m_LastUdsSenderId = GetSenderId();
    m_LastUdsReceiverId = GetReceiverId();
    m_LastUdsInput = GetSentData();
    m_LastDelayBetweenFrames = GetDelayBetweenFrames();
    m_LastRecvWaitingTime = GetWaitingTimeForFrames();

    auto& uds_responses = can_handler->GetUdsRawBuffer();
    uds_responses.clear();  /* Clear every older request */

    uint32_t old_recv_frame_id = can_handler->GetIsoTpResponseFrameId();
    std::vector<std::string> lines;
    boost::split(lines, m_LastUdsInput, [](char input) { return input == '\n' || input == ';'; }, boost::algorithm::token_compress_on);
    for(auto& hex_str : lines)
    {
        int delay = 0;
        int delay_check = sscanf(hex_str.c_str(), "DELAY%*c%d%*c[^\n]", &delay);
        if(delay_check == 1)
        {
            {
                LOG(LogLevel::Notification, "Sending ISO-TP Frame, Delay: {}ms", delay);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            continue;
        }

        char byte_array[MAX_ISOTP_FRAME_LEN];
        boost::algorithm::erase_all(hex_str, " ");
        boost::algorithm::erase_all(hex_str, ".");
        utils::ConvertHexStringToBuffer(hex_str, std::span{ byte_array });

        uint16_t len = (hex_str.length() / 2);
        if(len == 0)
        {
            LOG(LogLevel::Warning, "Skipping IsoTP frame, input length is zero");
            continue;
        }

        m_isotp_future = std::async(&SendIsoTpFrameGlobal, m_LastUdsSenderId, byte_array, len);
        LOG(LogLevel::Notification, "Sending ISO-TP Frame, FrameID: {:X}, ResponseFrameID: {:X}, Len: {}", m_LastUdsSenderId, can_handler->GetIsoTpResponseFrameId(), len);

        std::this_thread::sleep_for(std::chrono::milliseconds(m_LastDelayBetweenFrames));
    }
    LOG(LogLevel::Warning, "Sending complete 0");
    std::this_thread::sleep_for(std::chrono::milliseconds(m_LastRecvWaitingTime));  /* TODO: use semaphore or something else */
    LOG(LogLevel::Warning, "Sending complete 1");

    while(can_handler->GetElapsedTimeSinceLastUdsFrame() < m_LastRecvWaitingTime)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    LOG(LogLevel::Warning, "Sending complete 2");

    std::string response;
    for(auto& i : uds_responses)
    {
        std::string tmp;
        utils::ConvertHexBufferToString(i.c_str(), i.length(), tmp);
        response += tmp + "\r\n";
    }
    uds_responses.clear();
    m_DataRecv->SetValue(response);

    if(old_recv_frame_id != m_LastUdsReceiverId)
        can_handler->SetIsoTpResponseFrame(old_recv_frame_id);
}