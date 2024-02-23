#pragma once

#include <wx/wx.h>
#include <semaphore>

class AlarmPanel : public wxPanel
{
public:
	AlarmPanel(wxFrame* parent);

	void ShowAlarmDialog();
	void On10MsTimer();
	void WaitForAlarmSemaphore();
	const std::string& GetAlarmTime() const;

private:
	void ShowAlarmDialogInternal();

	std::binary_semaphore m_alarmSemaphore{0};
	bool m_showAlarmDialog = false;
	std::string m_DurationText;

	wxDECLARE_EVENT_TABLE();
};
