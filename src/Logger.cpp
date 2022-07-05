#include "pch.hpp"

Logger::Logger()
{
    fLog.open("logfile.txt", std::ofstream::trunc);
    assert(fLog);
}
