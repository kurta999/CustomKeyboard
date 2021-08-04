#include "pch.h"

wxBEGIN_EVENT_TABLE(BackupPanel, wxPanel)
EVT_TREELIST_ITEM_CONTEXT_MENU(ID_BackupPanel, BackupPanel::OnItemContextMenu)
EVT_TREELIST_ITEM_ACTIVATED(ID_BackupPanel, BackupPanel::OnItemActivated)
wxEND_EVENT_TABLE()

void BackupPanel::OnItemContextMenu(wxTreeListEvent& evt)
{
	enum
	{
		Id_AddNewBackup,
		Id_Delete,
	};

	wxMenu menu;
	menu.Append(Id_AddNewBackup, "&Add new backup");
	menu.Append(Id_Delete, "&Delete");

	const wxTreeListItem item = evt.GetItem();
	wxTreeListItem root = tree->GetItemParent(item);
	if(root == NULL) return;
	wxTreeListItem root2 = tree->GetItemParent(root);

	wxTreeListItem child = tree->GetFirstChild(item);
	menu.Enable(Id_Delete, !(child == 0));

	int ret = tree->GetPopupMenuSelectionFromUser(menu);
	switch(ret)
	{
		case Id_AddNewBackup:
		{
			wxString root_str = tree->GetItemText(item, 0);
			wxString item_str = tree->GetItemText(item, 1);

			BackupEntry* p = new BackupEntry("C:\\folder_non_exists", std::vector<std::filesystem::path>{"C:\\backup"}, std::vector<std::string>({ ".gitignore", ".txt" }), 2);
			DirectoryBackup::Get()->backups.push_back(p);

			UpdateMainTree();
			break;
		}
		case Id_Delete:
		{
			wxClientData* itemdata = tree->GetItemData(root);
			if(!itemdata)
				itemdata = tree->GetItemData(item);
			wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
			uint16_t id = dret->GetValue();
			delete DirectoryBackup::Get()->backups[id];
			DirectoryBackup::Get()->backups.erase(DirectoryBackup::Get()->backups.begin() + id);
			UpdateMainTree();
			break;
		}
	}
}

void BackupPanel::OnItemActivated(wxTreeListEvent& evt)
{
	wxTreeListItem item = evt.GetItem();
	wxTreeListItem root = tree->GetItemParent(item);
	wxString type_str = tree->GetItemText(item, 0);
	if(root)
	{
		wxClientData* itemdata = tree->GetItemData(root);
		wxIntClientData<uint16_t>* dret = dynamic_cast<wxIntClientData<uint16_t>*>(itemdata);
		uint16_t id = dret->GetValue();

		if(type_str == "Source")
		{
			wxTextEntryDialog d(this, "Enter text", "Enter source", DirectoryBackup::Get()->backups[id]->from.generic_string(), wxOK | wxCANCEL);
			int ret_code = d.ShowModal();
			if(ret_code == 5100)  /* OK */
			{
				std::string str = d.GetValue().ToStdString();
				if(std::filesystem::exists(str))
				{
					DirectoryBackup::Get()->backups[id]->from = str;
					UpdateMainTree();
				}
				else
				{
					wxMessageDialog(this, "Given directory doesn't exists", "Error", wxOK).ShowModal();
				}
			}
		}
		else if(type_str == "Destination")
		{
			wxString str;
			for(auto& i : DirectoryBackup::Get()->backups[id]->to)
			{
				str += i.generic_string() + "\n";
			}
			if(str[str.length() - 1] == '\n')
				str.erase(str.length() - 1, str.length());
			wxTextEntryDialog d(this, "Enter below destiantion list where backup(s) will be placed", "Enter destination(s)", str, wxOK | wxCANCEL | wxTE_MULTILINE);
			int ret_code = d.ShowModal();
			if(ret_code == 5100)  /* OK */
			{
				std::string result = d.GetValue().ToStdString();
				std::vector<std::filesystem::path> new_destination_list;
				boost::split(new_destination_list, result, boost::is_any_of("\n"));

				DirectoryBackup::Get()->backups[id]->to = std::move(new_destination_list);
				UpdateMainTree();
			}
		}
		else if(type_str == "Ignore")
		{
			wxString str;
			for(auto& i : DirectoryBackup::Get()->backups[id]->ignore_list)
			{
				str += i + "\n";
			}
			if(str[str.length() - 1] == '\n')
				str.erase(str.length() - 1, str.length());
			wxTextEntryDialog d(this, "Enter below desired folder names which you want to ignore", "Ignore list", str, wxOK | wxCANCEL | wxTE_MULTILINE);
			int ret_code = d.ShowModal();
			if(ret_code == 5100)  /* OK */
			{ 
				std::string result = d.GetValue().ToStdString();
				std::vector<std::string> new_ignore_list;
				boost::split(new_ignore_list, result, boost::is_any_of("\n"));

				DirectoryBackup::Get()->backups[id]->ignore_list = std::move(new_ignore_list);
				UpdateMainTree();
			}
		}
		else if(type_str == "Max backups")
		{
			wxTextEntryDialog d(this, "Enter maximum number of backups", "Enter max backups", std::to_string(DirectoryBackup::Get()->backups[id]->max_backups), wxOK | wxCANCEL);
			d.SetTextValidator(wxFILTER_DIGITS);
			int ret_code = d.ShowModal();
			if(ret_code == 5100)  /* OK */
			{
				try
				{
					int max_backups = std::stoi(d.GetValue().ToStdString());
					DirectoryBackup::Get()->backups[id]->max_backups = max_backups;
					UpdateMainTree();
				}
				catch(...)
				{
					wxMessageDialog(this, "Given input isn't number", "Error", wxOK).ShowModal();
				}
			}
		}
	}
}

void BackupPanel::UpdateMainTree()
{
	tree->DeleteAllItems();
	wxTreeListItem root = tree->GetRootItem();
	tree->DeleteAllItems();
	uint16_t cnt = 0;
	for(auto& i : DirectoryBackup::Get()->backups)
	{
		wxTreeListItem item = tree->AppendItem(root, i->from.filename().generic_string().c_str(), -1, -1, new wxIntClientData(cnt++));
		wxTreeListItem bind_item = tree->AppendItem(item, "Source");
		tree->SetItemText(bind_item, 1, i->from.generic_string());
		bind_item = tree->AppendItem(item, "Destination");
		tree->SetItemText(bind_item, 1, i->to[0].generic_string());
		bind_item = tree->AppendItem(item, "Ignore");

		wxString str_ignore;
		for(auto& i : i->ignore_list)
		{
			str_ignore += i + " ";
			if(str_ignore.length() > 80)
				break;
		}
		tree->SetItemText(bind_item, 1, str_ignore);
		bind_item = tree->AppendItem(item, "Max backups");
		tree->SetItemText(bind_item, 1, std::to_string(i->max_backups));
		tree->Expand(item);
	}
}

BackupPanel::BackupPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* bSizer1 = new wxBoxSizer(wxHORIZONTAL);

	tree = new wxTreeListCtrl(this, ID_BackupPanel, wxDefaultPosition, wxSize(600, 800), wxTL_DEFAULT_STYLE);
	tree->AppendColumn("Backup name", tree->WidthFor("App nameApp nameApp name"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	tree->AppendColumn("Data", tree->WidthFor("Key bindingsKey bindings"), wxALIGN_RIGHT, wxCOL_RESIZABLE | wxCOL_SORTABLE);
	UpdateMainTree();

	bSizer1->Add(tree, wxSizerFlags(2).Left());
	SetSizer(bSizer1);
	Show();
}