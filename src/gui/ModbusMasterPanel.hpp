#pragma once

class ModbusMasterPanel : public wxPanel
{
public:
	ModbusMasterPanel(wxWindow* parent);
	void UpdateSubpanels();

	wxAuiNotebook* m_notebook = nullptr;

private:
	void OnSize(wxSizeEvent& evt);
	void Changeing(wxAuiNotebookEvent& event);

	wxDECLARE_EVENT_TABLE();
};