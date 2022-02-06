#pragma once

#include "utils/CSingleton.hpp"
#include <string>

class PathSeparator : public CSingleton < PathSeparator >
{
    friend class CSingleton < PathSeparator >;

public:
    PathSeparator() = default;

    // !\brief Replace path separators to opposite ones in clipboard 
    void ReplaceClipboard();

    // !\brief Path separator execution key
    std::string replace_key = "F11";

private:
    // !\brief Replace path separators to opposite ones in given string
    // !\param str [in] Reference to string where separators will be replaced
    void ReplaceString(std::string& str);
};