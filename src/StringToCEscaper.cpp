#include "pch.hpp"

void StringEscaper::EscapeString(std::string& input, bool escape_percent, bool insert_backslash_to_end)
{
    boost::algorithm::replace_all(input, "\\", "\\\\");
    boost::algorithm::replace_all(input, "\"", "\x5C\""); /* \x5C = \ ASCII code, I was not able to type write this here */
    if(escape_percent)
        boost::algorithm::replace_all(input, "%", "%%");
    if(insert_backslash_to_end)
        boost::algorithm::replace_all(input, "\x0D\x0A", "\x5C\x0D\x0A");
}