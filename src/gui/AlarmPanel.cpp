#include "pch.hpp"

wxBEGIN_EVENT_TABLE(AlarmPanel, wxPanel)
wxEND_EVENT_TABLE()

AlarmPanel::AlarmPanel(wxFrame* parent) :
    wxPanel(parent, wxID_ANY)
{
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

    this->SetSizer(bSizer1);
    this->Layout();
}

void AlarmPanel::ShowAlarmDialogInternal()
{
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    wxTextEntryDialog dlg(frame, "Enter execution delay in hh:mm format", "Execution");
    int ret = dlg.ShowModal();
    if(ret == wxID_OK)
    {
        std::string input = dlg.GetValue().ToStdString();
        m_DurationText = input;
    }
    m_alarmSemaphore.release();
}

const std::string& AlarmPanel::GetAlarmTime() const
{
    return m_DurationText;
}

void AlarmPanel::ShowAlarmDialog()
{
	m_showAlarmDialog = true;
}

void AlarmPanel::WaitForAlarmSemaphore()
{
    m_alarmSemaphore.acquire();
}

void AlarmPanel::On10MsTimer()
{
    if(m_showAlarmDialog)
	{
		m_showAlarmDialog = false;
        ShowAlarmDialogInternal();
	}
}