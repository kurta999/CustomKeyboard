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
        text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
        text->SetClientData((void*)i.get());
        text->Bind(wxEVT_RIGHT_DOWN, &AlarmPanel::OnRightClick, this);

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

void AlarmPanel::OnRightClick(wxMouseEvent& event)
{
    auto obj = event.GetEventObject();
    std::unique_ptr<AlarmEntryHandler>& alarm_entry = wxGetApp().alarm_entry;

    wxStaticText* text = dynamic_cast<wxStaticText*>(obj);
    if (text == nullptr)
    {
        LOG(LogLevel::Error, "text is nullptr");
        return;
    }

    void* clientdata = text->GetClientData();
    if (clientdata == nullptr)
    {
        LOG(LogLevel::Error, "clientdata is nullptr");
        return;
    }

    AlarmEntry* e = reinterpret_cast<AlarmEntry*>(clientdata);
    wxMenu menu;
    menu.Append(ID_AlarmTrigger, "&Trigger")->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT, wxART_OTHER, FromDIP(wxSize(14, 14))));
    menu.Append(ID_AlarmCancel, "&Cancel")->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_OTHER, FromDIP(wxSize(14, 14))));
    int ret = GetPopupMenuSelectionFromUser(menu);

    switch (ret)
    {
        case ID_AlarmTrigger:
        {
            alarm_entry->SetupAlarm(e);
            break;
        }
        case ID_AlarmCancel:
        {
            alarm_entry->CancelAlarm(e);
            break;
        }
    }
    event.Skip();
}