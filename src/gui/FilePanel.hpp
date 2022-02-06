#pragma once

#include <inttypes.h>
#include <wx/wx.h>
#include <wx/treelist.h>

#include <map>

#ifdef _WIN32
using ComparatorIntType = int64_t;  /* To be able to use uint64_t, you have to add a corresponding function to wxWidgets library */
#else
using ComparatorIntType = int32_t;
#endif

class MyComparator : public wxTreeListItemComparator
{
public: /* to made this comparator work, you need to change the return value of comparators to int64_t from int in wxWidgets main library */
	virtual ComparatorIntType Compare(wxTreeListCtrl* treelist, unsigned column, wxTreeListItem item1, wxTreeListItem item2) override
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
	ComparatorIntType GetSizeFromText(const wxString& text) const
	{
		wxString size;
		ComparatorIntType factor = 1;
		if(text.EndsWith(" GB", &size))
			factor = (ComparatorIntType)((int64_t)8 * (int64_t)1024 * (int64_t)1024 * (int64_t)1024);
		else if(text.EndsWith(" MB", &size))
			factor = (ComparatorIntType)((int64_t)8 * (int64_t)1024 * (int64_t)1024);
		else if(!text.EndsWith(" kB", &size))
			factor = (ComparatorIntType)((int64_t)8 * (int64_t)1024);
		else if(!text.EndsWith(" B", &size))
			factor = 8;
		unsigned long long n = 0;
		size.ToULongLong(&n);
		return n * factor;
	}
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
	wxTreeListCtrl* tree;
	MyComparator m_comparator;
private:

	std::map<size_t, std::unique_ptr<DirItems>> dir_map;
	wxDECLARE_EVENT_TABLE();
};