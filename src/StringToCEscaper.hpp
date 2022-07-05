#pragma once

#include <string>

class StringEscaper
{
public:
    StringEscaper() = default;

    // !\brief Escape string to C format
    // !\param input [in] Input string
    // !\param escape_percent [in] Escape percent (%)?
    // !\param insert_backslash_to_end [in] Insert backslash at end of each line (\)?
    void EscapeString(std::string& input, bool escape_percent, bool insert_backslash_to_end);
};