#include "pch.hpp"

constexpr const char* FILE_EXPLORER_OPEN_FRAME = "expw";
constexpr size_t FILE_EXPLORER_OPEN_FRAME_LEN = std::char_traits<char>::length(FILE_EXPLORER_OPEN_FRAME);

constexpr const char* TCP_HTTP_HEADER = "HTTP/1.1 200 OK\r\n Content-type:text/html\r\n Connection: close\r\n\r\n";
constexpr size_t TCP_HTTP_HEADER_LEN = std::char_traits<char>::length(TCP_HTTP_HEADER);

TcpMessageExecutor::TcpMessageExecutor()
{
	m_cmds["MEAS_DATA"] = std::bind(&TcpMessageExecutor::HandleAirQualityData, this, nullptr);
	m_cmds["expw"] = std::bind(&TcpMessageExecutor::HandleOpenExplorer, this, nullptr);
	
	m_cmds["GET /graphs"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "Temperature.html");
	m_cmds["GET /Temperature"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "Temperature.html");
	m_cmds["GET /Humidity"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "Humidity.html");
	m_cmds["GET /CO2"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "CO2.html");
	m_cmds["GET /VOC"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "VOC.html");
	m_cmds["GET /PM25"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "PM25.html");
	m_cmds["GET /PM10"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "PM10.html");
	m_cmds["GET /Lux"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "Lux.html");
	m_cmds["GET /CCT"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "CCT.html");
	
	m_cmds["GET /Line%20Chart%20Multiple%20Axes_files/Chart.min.js.download"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "Chart.min.js.download");
	m_cmds["GET /Line%20Chart%20Multiple%20Axes_files/utils.js.download"] = std::bind(&TcpMessageExecutor::HandleGraphs, this, "utils.js.download");
}

void TcpMessageExecutor::SetCurrentSession(SharedSession session, size_t len)
{
	m_session = session;
	m_recv_data = m_session->receivedData;
	m_len = len;
}

TcpMessageReturn TcpMessageExecutor::HandleAirQualityData(std::any param)
{
	bool ret = Sensors::Get()->ProcessIncommingData(m_recv_data, m_len, m_session->sessionAddress.c_str());
	return std::make_tuple(true, true, "");
}

TcpMessageReturn TcpMessageExecutor::HandleOpenExplorer(std::any param)
{
	std::replace_if(m_recv_data, m_recv_data + m_len, [](char c) { return c == '/'; }, '\\');
#ifdef _WIN32 
	std::wstring params = std::wstring(m_recv_data + FILE_EXPLORER_OPEN_FRAME_LEN, m_recv_data + (strlen(m_recv_data)));
	std::wstring cmdline = std::wstring(std::string(1, Settings::Get()->shared_drive_letter) + ":" + params);
	ShellExecuteW(NULL, L"open", L"explorer.exe", cmdline.c_str(), NULL, SW_NORMAL);
#else

#endif
	LOG(LogLevel::Normal, L"Explorer open recv: {}", cmdline);
	return std::make_tuple(true, true, "");
}

TcpMessageReturn TcpMessageExecutor::HandleGraphs(std::any param)
{
	const char* file_on_disk = std::any_cast<const char*>(param);

	std::string to_send{ TCP_HTTP_HEADER };
	std::ifstream t(std::string("Graphs/") + file_on_disk, std::ifstream::binary);
	if(t)
	{
		t.seekg(0, std::ios::end);
		size_t size = t.tellg();
		t.seekg(0);
		to_send.resize(TCP_HTTP_HEADER_LEN + size);
		t.read(&to_send[TCP_HTTP_HEADER_LEN], size);
		t.close();
	}
	else
	{
		LOG(LogLevel::Error, "Failed to open {}", file_on_disk);
	}
	return std::make_tuple(true, true, to_send);
}

TcpMessageReturn TcpMessageExecutor::Process(std::any param)
{
	if(!m_recv_data)
		return std::make_tuple(false, false, "");

	for(auto &[name, func] : m_cmds)
	{
		if(!memcmp(m_recv_data, name.c_str(), name.length()))
		{
			return func(std::placeholders::_1);
		}
	}

	return std::make_tuple(true, false, "");
}