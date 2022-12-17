#include "pch.hpp"

constexpr const char* LOG_FILENAME = "logfile.txt";

Logger::Logger()
{
    fLog.open(LOG_FILENAME, std::ofstream::binary);
    assert(fLog);
}

void Logger::SetLogHelper(ILogHelper* helper)
{
    m_helper = helper;
}

bool Logger::SearchInLogFile(std::string_view filter, std::string_view log_level)
{
	std::ifstream in(LOG_FILENAME);
	if(!in)
	{
		LOG(LogLevel::Error, "Failed to open log file ({}) for search", LOG_FILENAME);
		return false;
	}
	if(!m_helper)
	{
		LOG(LogLevel::Error, "m_helper is nullptr");
		return false;
	}

	m_helper->ClearEntries();
	std::string line;
	while(std::getline(in, line, '\n'))  /* "2022.08.12.591 15:54:00 [Warning] [CorsairHid.cpp:59 - CorsairHid::ExecuteInitSequence] 5 \n" */
	{
		int year, month, day, hour, minute, second, millisecond;
		char level[32];
		char cppfile[64];
		int linenumber;
		char funcname[256];
		char logstr[512];
		int ret = sscanf(line.c_str(), "%d.%d.%d %d:%d:%d.%d [%32[^]]] [%64[^:]:%d - %256[^]]] %512[^\n]",
			&year, &month, &day, &hour, &minute, &second, &millisecond, level, cppfile, &linenumber, funcname, logstr);
		if(ret == 12)
		{
			std::string_view logline(logstr);
			std::string_view levelline(level);

			if(!filter.empty() && boost::algorithm::ifind_first(logline, filter).begin() == logline.end()) continue;  /* Skip if no match */
			if(!log_level.empty() && boost::algorithm::ifind_first(levelline, log_level).begin() == levelline.end()) continue;  /* Skip if no match */
			m_helper->AppendLog(cppfile, line);
		}
	}
	return true;
}