#pragma once

#include <wx/wx.h>

class CanUdsRawDialog : public wxDialog
{
public:
    CanUdsRawDialog(wxWindow* parent);

    void ShowDialog();
    bool IsApplyClicked() { return m_IsApplyClicked; }

    uint32_t GetSenderId();
    uint32_t GetReceiverId();
    uint32_t GetDelayBetweenFrames();
    uint32_t GetWaitingTimeForFrames();
    std::string GetSentData();

    wxTextCtrl* m_SenderId = nullptr;
    wxTextCtrl* m_ReceiverId = nullptr;
    wxTextCtrl* m_DelayBetweenFrames = nullptr;
    wxTextCtrl* m_RecvDelayFrames = nullptr;
    wxButton* m_ReadDid = nullptr;
    wxButton* m_WriteDid = nullptr;
    wxButton* m_EcuReset = nullptr;
    wxButton* m_Clear = nullptr;
    wxTextCtrl* m_DataToSend = nullptr;
    wxButton* m_SendBtn = nullptr;
    wxTextCtrl* m_DataRecv = nullptr;
protected:
    void OnApply(wxCommandEvent& event);

private:
    void HandleFrameSending();

    wxBoxSizer* sizerTop = nullptr;

    uint32_t m_LastUdsSenderId = 0;
    uint32_t m_LastUdsReceiverId = 0;
    uint32_t m_LastDelayBetweenFrames = 0;
    uint32_t m_LastRecvWaitingTime = 0;
    std::string m_LastUdsInput;

    bool m_IsApplyClicked = false;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CanUdsRawDialog);
};