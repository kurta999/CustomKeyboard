#include "pch.hpp"

wxBEGIN_EVENT_TABLE(AlarmPanel, wxPanel)
wxEND_EVENT_TABLE()

AlarmPanel::AlarmPanel(wxFrame* parent) :
    wxPanel(parent, wxID_ANY)
{
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

    std::unique_ptr<AlarmEntryHandler>& alarm_entry = wxGetApp().alarm_entry;
    for (auto& i : alarm_entry->entries)
    {
        wxStaticText* text = new wxStaticText(this, NULL, i->name);
        text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString)); \
        alarms.push_back(text);
    }

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

void AlarmPanel::UpdateAlarmsDisplay()
{
    int pos = 0;
    std::unique_ptr<AlarmEntryHandler>& alarm_entry = wxGetApp().alarm_entry;
    for (auto& i : alarms)
    {
        i->SetLabelText(wxString::Format("%s - %s", alarm_entry->entries[pos]->name, utils::SecondsToHms(alarm_entry->entries[pos]->duration.count())));
        if (alarm_entry->entries[pos]->is_armed)
            i->SetForegroundColour(*wxRED);
        else
            i->SetForegroundColour(*wxBLACK);
        pos++;
    }

}

void AlarmPanel::On10MsTimer()
{
    if(m_showAlarmDialog)
	{
		m_showAlarmDialog = false;
        ShowAlarmDialogInternal();
	}

    UpdateAlarmsDisplay();
}