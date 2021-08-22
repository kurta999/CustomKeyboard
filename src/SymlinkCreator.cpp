#include "pch.h"

#define SAFE_RELEASE(name) \
	if(name) \
		name->Release();

bool SymlinkCreator::HandleKeypress(std::string& pressed_keys)
{
	bool ret = false;
	if(is_enabled)
	{
		if(pressed_keys == mark_key)
		{
			Mark();
			ret = true;
		}
		else if(pressed_keys == place_symlink_key)
		{
			Place(true);
			ret = true;
		}		
		else if(pressed_keys == place_hardlink_key)
		{
			Place(false);
			ret = true;
		}
	}
	return ret;
}

void SymlinkCreator::Mark()
{
	GetSelectedItemsFromFileExplorer();
	MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
	{
		std::lock_guard<std::mutex> lock(frame->mtx);
		frame->pending_msgs.push_back({ !selected_items.empty() ? (uint8_t)LinkMark : (uint8_t)LinkMarkError, (uint32_t)selected_items.size() });
	}
}

void SymlinkCreator::Place(bool is_symlink)
{
	std::wstring dest_path = GetDestinationPathFromFileExplorer();
	if(!dest_path.empty() && !selected_items.empty())
	{
		std::filesystem::path dest = dest_path;
		if(is_symlink)
		{
			for(PWSTR& item : selected_items)
			{
				std::filesystem::path dest_with_name = dest_path / std::filesystem::path(item).filename();
				try
				{
					if(std::filesystem::is_directory(item))
						std::filesystem::create_directory_symlink(item, dest_with_name);
					else
						std::filesystem::create_symlink(item, dest_with_name);
				}
				catch(std::filesystem::filesystem_error const& e)
				{
					LOGMSG(error, "Exception during creating symlinks: {}", e.what());
					break;
				}
			}

			MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
			{
				std::lock_guard<std::mutex> lock(frame->mtx);
				frame->pending_msgs.push_back({ (uint8_t)SymlinkCreated, (uint32_t)selected_items.size() });
			}
		}
		else
		{
			for(PWSTR& item : selected_items)
			{
				std::filesystem::path dest_with_name = dest_path / std::filesystem::path(item).filename();
				try
				{
					std::filesystem::create_hard_link(item, dest_with_name);
				}
				catch(std::filesystem::filesystem_error const& e)
				{
					LOGMSG(error, "Exception during creating hardlinks: {}", e.what());
					break;
				}
			}

			MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
			{
				std::lock_guard<std::mutex> lock(frame->mtx);
				frame->pending_msgs.push_back({ (uint8_t)HardlinkCreated, (uint32_t)selected_items.size() });
			}
		}
	}
}

IFolderView2* SymlinkCreator::GetFolderView2()
{
	IFolderView2* pfv = NULL;
	static bool inited = false;
	if(!inited)
	{
		if(SUCCEEDED(CoInitialize(0)))
			inited = true;
	}
	HWND hwndFind = GetForegroundWindow();
	IShellWindows* psw;
	if(SUCCEEDED(CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER, IID_IShellWindows, (void**)&psw)) && inited)
	{
		VARIANT v;
		V_VT(&v) = VT_I4;
		IDispatch* pdisp;
		BOOL fFound = FALSE;
		for(V_I4(&v) = 0; !fFound && psw->Item(v, &pdisp) == S_OK; V_I4(&v)++)
		{
			IWebBrowserApp* pwba;
			if(SUCCEEDED(pdisp->QueryInterface(IID_IWebBrowserApp, (void**)&pwba)))
			{
				HWND hwndWBA;
				if(SUCCEEDED(pwba->get_HWND((LONG_PTR*)&hwndWBA)) && hwndWBA == hwndFind)
				{
					fFound = TRUE;
					IServiceProvider* psp;
					if(SUCCEEDED(pwba->QueryInterface(IID_IServiceProvider, (void**)&psp)))
					{
						IShellBrowser* psb;
						if(SUCCEEDED(psp->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void**)&psb)))
						{
							IShellView* psv;
							if(SUCCEEDED(psb->QueryActiveShellView(&psv)))
							{
								if(SUCCEEDED(psv->QueryInterface(IID_IFolderView2, (void**)&pfv)))
								{

								}
							}
							SAFE_RELEASE(psv);
						}
						SAFE_RELEASE(psb);
					}
					SAFE_RELEASE(psp);
				}
				SAFE_RELEASE(pwba);
			}
			SAFE_RELEASE(pwba);
		}
		SAFE_RELEASE(pdisp);
	}
	SAFE_RELEASE(psw);
	return pfv;
}

void SymlinkCreator::GetSelectedItemsFromFileExplorer()
{
	IFolderView2* pfv = GetFolderView2();  /* null if clicked on desktop and not in file explorer */
	if(pfv)
	{
		selected_items.clear();

		IShellItemArray* sia = NULL;
		if(FAILED(pfv->GetSelection(FALSE, &sia)))
		{
			pfv->Release();
			return;
		}
		DWORD	num = 0;
		if(FAILED(sia->GetCount(&num)))
		{
			pfv->Release();
			sia->Release();
			return;
		}
		for(DWORD i = 0; i < num; i++)
		{
			IShellItem* si = NULL;
			if(FAILED(sia->GetItemAt(i, &si)))
			{
				pfv->Release();
				sia->Release();
				return;
			}

			PWSTR path = NULL;
			if(FAILED(si->GetDisplayName(SIGDN_FILESYSPATH, &path)))
			{
				pfv->Release();
				si->Release();
				continue;
			}

			if(path)
				selected_items.push_back(path);
		}
		pfv->Release();
	}
}

std::wstring SymlinkCreator::GetDestinationPathFromFileExplorer()
{
	std::wstring ret;
	IFolderView2* pfv = GetFolderView2();
	if(pfv)
	{
		IShellItemArray* sia = NULL;
		if(FAILED(pfv->GetSelection(TRUE, &sia)))
		{
			pfv->Release();
			return ret;
		}

		DWORD num = 0;
		if(FAILED(sia->GetCount(&num)))
		{
			pfv->Release();
			sia->Release();
			return ret;
		}

		IShellItem* si = NULL;
		if(FAILED(sia->GetItemAt(0, &si)))
		{
			pfv->Release();
			sia->Release();
			return ret;
		}

		PWSTR path = NULL;
		if(FAILED(si->GetDisplayName(SIGDN_FILESYSPATH, &path)))
		{
			pfv->Release();
			si->Release();
			return ret;
		}
		pfv->Release();

		if(path)
			ret = path;
	}
	return ret;
}

