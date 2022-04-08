#include "pch.hpp"

#define SAFE_RELEASE(name) \
	if(name) \
		name->Release();

#ifdef _WIN32
IFolderView2* GetFolderView2()
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
#endif

std::wstring GetDestinationPathFromFileExplorer()
{
	std::wstring ret;
#ifdef _WIN32
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
#endif
	return ret;
}

void TerminalHotkey::Process()
{
#ifdef _WIN32 /* This is already done in Linux, so this implementation is Windows only */
	if(is_enabled && wxGetKeyState(vkey))
	{
		std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		int64_t dif = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - last_execution).count();
		if(dif < 500)  /* Avoid debouncing */
			return;

		std::wstring str = GetDestinationPathFromFileExplorer();
		if(!str.empty())
		{
			std::filesystem::path p(str);
			if(p.has_extension())  /* If file is selected in explorer, it has to be removed */
				p.remove_filename();
			str = p.generic_wstring();
			str.insert(0, L"/d ");
			last_execution = std::chrono::steady_clock::now();
			ShellExecute(NULL, L"open", L"wt", str.c_str(), NULL, SW_SHOW);
			/* swprintf(buf, L"/k cd /d %s", str.c_str()); - for cmd */
			/* swprintf(buf, L"/d %s", str.c_str()); - for wt */
		}
	}
#endif
}