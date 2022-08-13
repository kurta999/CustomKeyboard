#include "pch.hpp"

constexpr const char* FILE_EXPLORER_OPEN_FRAME = "expw";

TcpMessageExecutor::TcpMessageExecutor(const char* ip, char* recv_data, size_t len) :
	m_ip(ip), m_recv_data(recv_data), m_len(len)
{

}

std::tuple<bool, bool> TcpMessageExecutor::Process()
{
	if(!memcmp(m_recv_data, FILE_EXPLORER_OPEN_FRAME, std::char_traits<char>::length(FILE_EXPLORER_OPEN_FRAME)))
	{
		std::replace_if(m_recv_data, m_recv_data + m_len, [](char c) { return c == '/'; }, '\\');
#ifdef _WIN32
		std::wstring cmdline = std::wstring(L"Z:" + std::wstring(m_recv_data, m_recv_data + strlen(m_recv_data)));
		ShellExecuteW(NULL, L"open", L"explorer.exe", cmdline.c_str(), NULL, SW_NORMAL);
#else

#endif
		LOGW(LogLevel::Normal, L"Explorer open recv: {}", cmdline);
		return std::make_tuple(true, true);
	}

	bool ret = Sensors::Get()->ProcessIncommingData(m_recv_data, m_ip);
	return std::make_tuple(true, ret);
}