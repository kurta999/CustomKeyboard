#include "pch.hpp"

Logger::Logger()
{
	fLog = fopen("logfile.txt", "w");
	assert(fLog);
}

Logger::~Logger()
{
	fclose(fLog);
}