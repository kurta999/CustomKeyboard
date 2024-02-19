#include "pch.hpp"

/* TODO: finish this ugly shit */
void ShowAlarmDialog()
{
    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    wxTextEntryDialog dlg(frame, "Enter execution delay in hh:mm format", "Execution");
    int ret = dlg.ShowModal();
    if (ret == wxID_OK)
    {

    }
}
