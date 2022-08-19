#include "pch.hpp"

wxBEGIN_EVENT_TABLE(FilePanel, wxPanel)
EVT_SIZE(FilePanel::OnSize)
EVT_TREELIST_ITEM_ACTIVATED(ID_DirList, FilePanel::OnItemActivated)
wxEND_EVENT_TABLE()

int MyComparator::Compare(wxTreeListCtrl* treelist, unsigned column, wxTreeListItem item1, wxTreeListItem item2)
{
	wxString text1 = treelist->GetItemText(item1, column), text2 = treelist->GetItemText(item2, column);
	switch(column)
	{
		case 0:
			return text1.CmpNoCase(text2);

		case 1:
		{
			int64_t result = GetSizeFromText(text1) - GetSizeFromText(text2);
			if(result > 0)
				result = 1;
			else if(result < 0)
				result = -1;
			return result;
		}
	}
	return 0;
}

int64_t MyComparator::GetSizeFromText(const wxString& text) const
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

void FilePanel::OnSize(wxSizeEvent& evt)
{
	evt.Skip();
}

void FilePanel::OnItemActivated(wxTreeListEvent& evt)
{
	wxTreeListItem item = evt.GetItem();
	const wxString& text = tree->GetItemText(item);

	std::list<std::wstring> vec_path;
	vec_path.push_front(text.ToStdWstring());

	wxTreeListItem item2 = item;
	while((item2 = tree->GetItemParent(item2)) != NULL)
	{
		const wxString& text2 = tree->GetItemText(item2);
		
		std::wstring textstr = text2.ToStdWstring();
		if(!textstr.empty())
			vec_path.push_front(textstr);
	}

	std::wstring final_path = m_DirText->GetValue().ToStdWstring();
	if(final_path[final_path.length() - 1] != L'\\')
		final_path.append(L"\\");

	std::list<std::wstring>::iterator it = vec_path.begin();
	std::advance(it, 1);

	for(; it != vec_path.end(); ++it)
		final_path += *it + L"\\";
#ifdef _WIN32
	std::wstring cmdline = std::wstring("/select,\"" + final_path);
	ShellExecuteW(NULL, L"open", L"explorer.exe", cmdline.c_str(), NULL, SW_NORMAL);
#else

#endif
}

FilePanel::FilePanel(wxFrame* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_DirText = new wxTextCtrl(this, wxID_ANY, "C:\\wxWidgets-3.2.0\\samples", wxDefaultPosition, wxSize(200, 20));
	bSizer1->Add(m_DirText);
	m_FileInfo = new wxStaticText(this, wxID_ANY, "No info available", wxDefaultPosition, wxSize(400, 20));
	bSizer1->Add(m_FileInfo);

	wxBoxSizer* bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_Generate = new wxButton(this, wxID_ANY, "Generate", wxDefaultPosition, wxDefaultSize);
	m_Generate->SetToolTip("Read dirs and get it's size");
	bSizer2->Add(m_Generate, 0, wxALL, 5);
	m_Generate->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			GenerateTree();
		});
	m_Clear = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize);
	m_Clear->SetToolTip("Clear generated tree to free up memory");
	bSizer2->Add(m_Clear, 0, wxALL, 5);
	m_Clear->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event)
		{
			ClearTree();
		});

	bSizer1->Add(bSizer2);

	tree = new wxTreeListCtrl(this, ID_DirList, wxDefaultPosition, wxSize(400, 400), wxTL_SINGLE);
	tree->AppendColumn("Directory entry", tree->WidthFor("Very long text for filenames AAAAAAAAAAA"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree->AppendColumn("Size", tree->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree->SetItemComparator(&m_comparator);

	bSizer1->Add(tree, wxSizerFlags(1).Left().Expand().Border(wxRIGHT, 30));
	SetSizerAndFit(bSizer1);
}

void FilePanel::GenerateTree()
{
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
	ClearTree();
	
	std::filesystem::path path = m_DirText->GetValue().ToStdString();
	if(!std::filesystem::exists(path))
	{
		LOG(LogLevel::Error, "Destination directory doesn't exists!");
		return;
	}

	size_t file_sizes = 0, dir_cnt = 0, file_cnt = 0;

	std::filesystem::path root_path = path.filename();
	if(root_path.empty())
		root_path = path;
	wxTreeListItem root = tree->GetRootItem();
	wxTreeListItem root_item = tree->AppendItem(root, root_path.generic_wstring().c_str());
	dir_map[std::filesystem::hash_value(path)] = std::make_unique<DirItems>(root_item, 0, 0);

	wxTreeListItem item = root_item;
	std::error_code ec;
	for(auto p = std::filesystem::recursive_directory_iterator(path, std::filesystem::directory_options::skip_permission_denied, ec); p != std::filesystem::recursive_directory_iterator(); ++p)
	{
		auto parent_p = p->path().parent_path();
		//DBG("path: %s, depth: %d, parent: %s\n", p->path().generic_string().c_str(), p.depth(), parent_p.generic_string().c_str());

		std::filesystem::path rel_path = p->path().lexically_proximate(path);
		std::filesystem::path curr_path = p->path();
		std::filesystem::path dir_without_filename = curr_path;
		dir_without_filename.remove_filename();
		dir_without_filename = std::filesystem::canonical(dir_without_filename);
		bool is_file = false;
		try
		{
			is_file = std::filesystem::is_regular_file(curr_path);
		}
		catch(...)
		{
			continue;
		}
		//DBGW(L"f: %d, %s\n", is_file, p.path().c_str());

		try
		{
			if(is_file)
			{
				size_t fsize = std::filesystem::file_size(curr_path);
				std::string str = utils::GetDataUnit(fsize);

				auto it = dir_map.find(std::filesystem::hash_value(parent_p));
				if(it != dir_map.end())
				{
					wxTreeListItem child_item = tree->AppendItem(it->second->item, curr_path.filename().generic_wstring().c_str());
					tree->SetItemText(child_item, 1, str.c_str());
					if(p.depth() < 1)
						tree->Expand(child_item);
				}
				else
				{
					wxTreeListItem child_item = tree->AppendItem(item, curr_path.filename().generic_wstring().c_str());
					tree->SetItemText(child_item, 1, str.c_str());
					if(p.depth() < 1)
						tree->Expand(child_item);
				}

				auto it2 = dir_map.find(std::filesystem::hash_value(dir_without_filename));
				if(it2 != dir_map.end())
				{
					it2->second->size += fsize;
					it2->second->filecount++;
				}
				else
					dir_map[std::filesystem::hash_value(dir_without_filename)] = std::make_unique<DirItems>((wxTreeListItem)NULL, fsize, 0);
				file_sizes += fsize;
				file_cnt++;

				auto parentppp = dir_without_filename;  /* update size of every parent directory */
				while(parentppp != path)
				{
					auto parentppp2 = parentppp.parent_path();
					auto it2 = dir_map.find(std::filesystem::hash_value(parentppp2));
					if(it2 != dir_map.end())
						it2->second->size += fsize;
					parentppp = parentppp2;
					//DBG("while remove: %s\n", parentppp.generic_string().c_str());
				}
				//DBG("fasz\n");
			}
			else
			{
				auto it = dir_map.find(std::filesystem::hash_value(parent_p));
				if(it != dir_map.end())
				{
					wxTreeListItem child_item = tree->AppendItem(it->second->item, curr_path.filename().generic_wstring().c_str());
					if(p.depth() < 1)
						tree->Expand(child_item);

					auto it2 = dir_map.find(std::filesystem::hash_value(curr_path));
					if(it2 != dir_map.end())
						it2->second->item = child_item;
					else
						dir_map[std::filesystem::hash_value(curr_path)] = std::make_unique<DirItems>(child_item, 0, 0);
				}
				else
				{
					wxTreeListItem child_item = tree->AppendItem(item, curr_path.filename().generic_wstring().c_str());
					if(p.depth() < 1)
						tree->Expand(child_item);

					auto it2 = dir_map.find(std::filesystem::hash_value(curr_path));
					if(it2 != dir_map.end())
						it2->second->item = child_item;
					else
						dir_map[std::filesystem::hash_value(curr_path)] = std::make_unique<DirItems>(child_item, 0, 0);

					item = child_item;
				}
				dir_cnt++;
			}
		}
		catch(...)
		{
			//LOG(LogLevel::Critical, "Exception: {}", boost::current_exception_diagnostic_information().c_str()); /* #include "boost/exception/diagnostic_information.hpp" */
			continue;
		}
	}

	for(auto& i : dir_map)  /* finally update base directories */
	{
		if(i.second->size != 0)
		{
			std::string str = utils::GetDataUnit(i.second->size);
			tree->SetItemText(i.second->item, 1, str.c_str());
		}
	}

	std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
	int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

	std::string result_str = std::format("{} file in {} directory has been parsed in {:.6f} ms ({})", file_cnt, dir_cnt, (double)dif / 1000000.0, utils::GetDataUnit(file_sizes));
	m_FileInfo->SetLabelText(result_str);
	LOG(LogLevel::Notification, result_str.c_str());
}

void FilePanel::ClearTree()
{
	tree->DeleteAllItems();
	dir_map.clear();
}