#pragma once

#include <inttypes.h>
#include <wx/wx.h>
#include <wx/treelist.h>

#include <map>

class MyComparator : public wxTreeListItemComparator
{
public:
	virtual int Compare(wxTreeListCtrl* treelist, unsigned column, wxTreeListItem item1, wxTreeListItem item2) override;

private:
	int64_t GetSizeFromText(const wxString& text) const;
};

class DirItems
{
public:
	DirItems(wxTreeListItem item_, size_t size_, size_t file_count_) :
		item(item_), size(size_), filecount(file_count_)
	{

	}

	wxTreeListItem item;
	size_t size;
	size_t filecount;
};

class FilePanel : public wxPanel
{
public:
	FilePanel(wxFrame* parent);
	void OnSize(wxSizeEvent& evt);
	void OnItemActivated(wxTreeListEvent& evt);
	void GenerateTree();
	void ClearTree();

	wxButton* m_OkButton = nullptr;
	wxTextCtrl* m_DirText = nullptr;
	wxStaticText* m_FileInfo = nullptr;
	wxButton* m_Generate = nullptr;
	wxButton* m_Clear = nullptr;
	wxTreeListCtrl* tree = nullptr;
	MyComparator m_comparator;
private:

	std::map<size_t, std::unique_ptr<DirItems>> dir_map;
	wxDECLARE_EVENT_TABLE();
};