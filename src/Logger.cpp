#include "Logger.h"
#include <assert.h>

Logger::Logger()
{
	fLog = fopen("logfile.txt", "w");
	assert(fLog);
}

Logger::~Logger()
{
	fclose(fLog);
}