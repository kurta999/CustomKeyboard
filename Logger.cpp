
#include "Logger.h"


void Logger::Init(void)
{
	fLog = fopen("logfile.txt", "w");
}