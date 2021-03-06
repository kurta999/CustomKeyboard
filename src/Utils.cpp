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