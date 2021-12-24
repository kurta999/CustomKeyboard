#pragma once

#include <wx/wx.h>
#include <wx/treelist.h>

#include <map>

#ifdef _WIN32
class MyComparator : public wxTreeListItemComparator
{
public: /* to made this comparator work, you need to change the return value of comparators to int64_t from int in wxWidgets main library */
	virtual int64_t Compare(wxTreeListCtrl* treelist, unsigned column, wxTreeListItem item1, wxTreeListItem item2) override
	{
		wxString text1 = treelist->GetItemText(item1, column), text2 = treelist->GetItemText(item2, column);
		switch(column)
		{
			case 0:
				return text1.CmpNoCase(text2);

			case 1:
				return GetSizeFromText(text1) - GetSizeFromText(text2);
		}
		return 0;
	}

private:
	int64_t GetSizeFromText(const wxString& text) const
	{
		wxString size;
		int64_t factor = 1;
		if(text.EndsWith(" GB", &size))
			factor = (int64_t)((int64_t)8 * (int64_t)1024 * (int64_t)1024 * (int64_t)1024);
		else if(text.EndsWith(" MB", &size))
			factor = (int64_t)((int64_t)8 * (int64_t)1024 * (int64_t)1024);
		else if(!text.EndsWith(" kB", &size))
			factor = (int64_t)((int64_t)8 * (int64_t)1024);
		else if(!text.EndsWith(" B", &size))
			factor = 8;
		unsigned long long n = 0;
		size.ToULongLong(&n);
		return n * factor;
	}
};
#endif
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
	wxTreeListCtrl* tree;
#ifdef _WIN32
	MyComparator m_comparator;
#endif
private:

	std::map<size_t, std::unique_ptr<DirItems>> dir_map;
	wxDECLARE_EVENT_TABLE();
};