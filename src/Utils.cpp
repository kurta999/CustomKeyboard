#include "pch.hpp"

namespace utils
{
	bool is_number(const std::string& s)
	{
		return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
	}

    void MadeTextFromKeys(std::string& chr)
    {
        if(chr == "SPACE")
            chr = " ";
        else if(chr == "z")
            chr = "y";
        else if(chr == "Z")
            chr = "Y";
        else if(chr == "y")
            chr = "z";
        else if(chr == "Y")
            chr = "Z";
    }

	std::string GetDataUnit(size_t input)
	{
		float fInput = static_cast<float>(input);

		if(fInput < 1024)
		{
			return std::format("{} B", (size_t)fInput);
		}

		fInput /= 1024;
		if(fInput < 1024)
		{
			return std::format("{:.2f} kB", fInput);
		}

		fInput /= 1024;
		if(fInput < 1024)
		{
			return std::format("{:.2f} MB", fInput);
		}

		fInput /= 1024;
		if(fInput < 1024)
		{
			return std::format("{:.2f} GB", fInput);
		}

		fInput /= 1024;
		if(fInput < 1024)
		{
			return std::format("{:.2f} TB", fInput);
		}

		return std::string("X");
	}

	size_t MBStringToWString(const std::string& src, std::wstring& dest)
	{
		wchar_t* wstr = new wchar_t[src.length() + 1];
		size_t ret = std::mbstowcs(wstr, src.c_str(), src.length());
		wstr[src.length()] = 0;
		dest = wstr;
		delete[] wstr;
		return ret;
	}

	size_t WStringToMBString(const std::wstring& src, std::string& dest)
	{
		char* str = new char[src.length() + 1];
		size_t ret = std::wcstombs(str, src.c_str(), src.length());
		str[src.length()] = 0;
		dest = str;
		delete[] str;
		return ret;
	}
#ifndef UNIT_TESTS
	static std::map<std::string, wxKeyCode> vkey_lookup  /* Only Fx keys for now */
	{
		{"F1", WXK_F1},
		{"F2", WXK_F2},
		{"F3", WXK_F3},
		{"F4", WXK_F4},
		{"F5", WXK_F5},
		{"F6", WXK_F6},
		{"F7", WXK_F7},
		{"F8", WXK_F8},
		{"F9", WXK_F9},
		{"F10", WXK_F10},
		{"F11", WXK_F11},
		{"F12", WXK_F12},
	};

	wxKeyCode GetVirtualKeyFromString(const std::string& key)
	{
		wxKeyCode ret = WXK_NONE;
		auto it = vkey_lookup.find(key);
		if(it != vkey_lookup.end())
			ret = it->second;
		return ret;
	}

	std::string GetKeyStringFromVirtualKey(wxKeyCode key_code)
	{
		std::string key_name = "INVALID";
		for(auto& i : vkey_lookup)
		{
			if(i.second == key_code)
			{
				key_name = i.first;
				break;
			}
		}
		return key_name;
	}

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

	std::vector<std::wstring> GetSelectedItemsFromFileExplorer()
	{
		std::vector<std::wstring> selected_items;
#ifdef _WIN32
		IFolderView2* pfv = GetFolderView2();  /* null if clicked on desktop and not in file explorer */
		if(pfv)
		{
			IShellItemArray* sia = NULL;
			if(FAILED(pfv->GetSelection(FALSE, &sia)))
			{
				pfv->Release();
				return {};
			}
			DWORD	num = 0;
			if(FAILED(sia->GetCount(&num)))
			{
				pfv->Release();
				sia->Release();
				return {};
			}
			for(DWORD i = 0; i < num; i++)
			{
				IShellItem* si = NULL;
				if(FAILED(sia->GetItemAt(i, &si)))
				{
					pfv->Release();
					sia->Release();
					return {};
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
		return selected_items;
#endif
	}

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

#endif
	/*
	template<typename _Rep, typename _Period>
	void sleep_for(const std::chrono::duration<_Rep, _Period>& duration, const std::mutex& mutex, const std::condition_variable& cv, const std::stop_token& stop_token)
	{
		std::unique_lock lock(m_mutex);
		std::stop_callback stop_wait{ stop_token, [&cv]() { cv.notify_one(); } };
		cv.wait_for(lock, 10ms, [&stop_token]() { return stop_token.stop_requested(); });
	}
	*/
}