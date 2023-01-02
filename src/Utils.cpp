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

	uint16_t crc16_modbus(void* data, size_t len)
	{
		using crc16_modbus_t = boost::crc_optimal<16, 0x8005, 0xFFFF, 0, true, true>;
		crc16_modbus_t calc_result;
		calc_result.process_bytes(reinterpret_cast<void*>(data), len);
		uint16_t crc = calc_result.checksum();
		return crc;
	}

#ifndef UNIT_TESTS
#ifdef _WIN32
	static std::map<std::string, int> vkey_lookup  /* Only Fx keys for now */
	{
		{"F1", VK_F1},
		{"F2", VK_F2},
		{"F3", VK_F3},
		{"F4", VK_F4},
		{"F5", VK_F5},
		{"F6", VK_F6},
		{"F7", VK_F7},
		{"F8", VK_F8},
		{"F9", VK_F9},
		{"F10", VK_F10},
		{"F11", VK_F11},
		{"F12", VK_F12},
	};
#endif
	int GetVirtualKeyFromString(const std::string& key)
	{
		int ret = 0xFFFF;
#ifdef _WIN32
		auto it = vkey_lookup.find(key);
		if(it != vkey_lookup.end())
			ret = it->second;
#endif
		return ret;
	}

	std::string GetKeyStringFromVirtualKey(int key_code)
	{
		std::string key_name = "INVALID";
#ifdef _WIN32
		for(auto& i : vkey_lookup)
		{
			if(i.second == key_code)
			{
				key_name = i.first;
				break;
			}
		}
#endif
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

#ifdef _WIN32
	CStringA ExecuteCmdWithoutWindow(const wchar_t* cmd, uint32_t timeout)
	{
		CStringA strResult;
		HANDLE hPipeRead, hPipeWrite;

		SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
		saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
		saAttr.lpSecurityDescriptor = NULL;

		// Create a pipe to get results from child's stdout.
		if(!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
			return strResult;

		STARTUPINFOW si = { sizeof(STARTUPINFOW) };
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.hStdOutput = hPipeWrite;
		si.hStdError = hPipeWrite;
		si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing.
								  // Requires STARTF_USESHOWWINDOW in dwFlags.

		PROCESS_INFORMATION pi = { 0 };
		BOOL fSuccess = CreateProcessW(L"C:\\windows\\system32\\cmd.exe", (LPWSTR)cmd, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		if(!fSuccess)
		{
			CloseHandle(hPipeWrite);
			CloseHandle(hPipeRead);
			return strResult;
		}

		if(timeout != std::numeric_limits<uint32_t>::min())
		{
			bool bProcessEnded = false;
			for(; !bProcessEnded;)
			{
				// Give some timeslice (50 ms), so we won't waste 100% CPU.
				bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

				// Even if process exited - we continue reading, if
				// there is some data available over pipe.
				for(;;)
				{
					char buf[1024];
					DWORD dwRead = 0;
					DWORD dwAvail = 0;

					if(!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
						break;

					if(!dwAvail) // No data available, return
						break;

					if(!::ReadFile(hPipeRead, buf, std::min((DWORD)sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
						// Error, the child process might ended
						break;

					buf[dwRead] = 0;
					strResult += buf;
				}
			}
		}

		CloseHandle(hPipeWrite);
		CloseHandle(hPipeRead);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return strResult;
	} //ExecCmd
#else

#endif

	std::string exec(const char* cmd)
	{
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#define WEXITSTATUS
#endif

		std::array<char, 512> buffer;
		std::string result;
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
		if(!pipe)
		{
			throw std::runtime_error("popen() failed!");
		}
		while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
		{
			result += buffer.data();
		}
		return result;
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

	void ConvertHexBufferToString(const std::vector<uint8_t>& in, std::string& out)
	{
		std::string hex;
		try
		{
			boost::algorithm::hex(in.begin(), in.end(), std::back_inserter(out));
			utils::separate<2, ' '>(out);
		}
		catch(...)
		{
			LOG(LogLevel::Error, "Exception with boost::algorithm::hex");
		}
	}

	void ConvertHexBufferToString(const char* in, size_t len, std::string& out)
	{
		std::string hex;
		try
		{
			boost::algorithm::hex(in, in + len, std::back_inserter(out));
			utils::separate<2, ' '>(out);
		}
		catch(...)
		{
			LOG(LogLevel::Error, "Exception with boost::algorithm::hex");
		}
	}

	uint32_t ColorStringToInt(std::string& in)
	{
		uint32_t ret = 0;
		if(in == "red")
			ret = 0xFF0000;
		else if(in == "green")
			ret = 0x33FF33;
		else if(in == "blue")
			ret = 0x6495ED;
		else if(in == "orange")
			ret = 0xFF7F50;
		else if(in == "white")
			ret = 0xFFFFFF;
		else if(in == "black")
			ret = 0;
		else if(in == "pink")
			ret = 0xFF10F0;
		else
		{
			size_t offset = 0;
			if(in[0] == '#')
				offset = 1;
			if(sscanf(&in.c_str()[offset], "%x", &ret) != 1)
			{
				LOG(LogLevel::Error, "Invalid color format: {}", in);
			}
		}
		return ret;
	}

#ifndef _WIN32
	uint32_t GetTickCount()
	{
		struct timespec ts;
		unsigned theTick = 0U;
		clock_gettime(CLOCK_REALTIME, &ts);
		theTick = ts.tv_nsec / 1000000;
		theTick += ts.tv_sec * 1000;
		return theTick;
	}
#endif
}