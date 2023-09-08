#include "pch.hpp"

constexpr const char* LOG_FILENAME = "./logfile.txt";

Logger::Logger()
{
    fLog.open(LOG_FILENAME, std::ofstream::binary);
    assert(fLog);
}

void Logger::SetLogHelper(ILogHelper* helper)
{
    m_helper = helper;
}

void Logger::SetDefaultLogLevel(LogLevel level)
{
	m_DefaultLogLevel = level;
}

LogLevel Logger::GetDefaultLogLevel() const
{
	return m_DefaultLogLevel;
}

void Logger::SetLogLevelAsString(const std::string& level)
{
	m_DefaultLogLevel = StringToLogLevel(level);
}

const std::string Logger::GetLogLevelAsString()
{
	const std::string ret = LogLevelToString(m_DefaultLogLevel);
	return ret;
}

void Logger::SetLogFilters(const std::string& filter_list)
{
	m_LogFilters.clear();
	std::vector<std::string> filters;
	boost::split(filters, filter_list, [](char input) { return input == '|'; }, boost::algorithm::token_compress_on);
	for(auto& i : filters)
		m_LogFilters.push_back(i);
}

std::string Logger::GetLogFilters()
{
	std::string ret;
	for(auto& i : m_LogFilters)
	{
		ret += i + "|";
	}

	if(!ret.empty() && ret.back() == '|')
		ret.pop_back();
	return ret;
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
		char level[32] = {};
		char cppfile[64] = {};
		int linenumber;
		char funcname[256] = {};
		char logstr[512] = {};
		int ret = sscanf(line.c_str(), "%d.%d.%d %d:%d:%d.%d [%31[^]]] [%63[^:]:%d - %255[^]]] %511[^\n]",
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

void Logger::AppendPreinitedEntries()
{
	MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
	if(frame && frame->log_panel && m_helper)
	{
		std::unique_lock lock(m_mutex);
		for(auto& i : preinit_entries)
		{
			if(!i.message.empty())
				m_helper->AppendLog(i.file.ToStdString(), i.message.ToStdString(), true);
		}
		preinit_entries.clear();
	}
}

void Logger::Tick()
{
	AppendPreinitedEntries();
}

LogLevel Logger::StringToLogLevel(const std::string& level)
{
	LogLevel ret = LogLevel::Verbose;
	if(boost::algorithm::icontains(level, "Debug"))
		ret = LogLevel::Debug;
/*
	else if(level == "Verbose")
		ret = LogLevel::Verbose;
*/
	else if(boost::algorithm::icontains(level, "Normal"))
		ret = LogLevel::Normal;
	else if(boost::algorithm::icontains(level, "Notification"))
		ret = LogLevel::Notification;
	else if(boost::algorithm::icontains(level, "Warning"))
		ret = LogLevel::Warning;
	else if(boost::algorithm::icontains(level, "Error"))
		ret = LogLevel::Error;
	else if(boost::algorithm::icontains(level, "Critical"))
		ret = LogLevel::Critical;
	else
		LOG(LogLevel::Error, "Invalid log level: {}", level);
	return ret;
}

std::string Logger::LogLevelToString(LogLevel level)
{
	std::string ret = "Verbose";
	if(level == LogLevel::Debug)
		ret = "Debug";
/*
	else if(level == LogLevel::Verbose)
		ret = LogLevel::Verbose;
*/
	else if(level == LogLevel::Normal)
		ret = "Normal";
	else if(level == LogLevel::Notification)
		ret = "Notification";
	else if(level == LogLevel::Warning)
		ret = "Warning";
	else if(level == LogLevel::Error)
		ret = "Error";
	else if(level == LogLevel::Critical)
		ret = "Critical";
	else
		LOG(LogLevel::Error, "Invalid log level: {}", static_cast<int>(level));
	return ret;

}