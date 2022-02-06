#pragma once

#include "utils/CSingleton.hpp"
#include <string>
#ifdef _WIN32
#include <shlobj.h>
#endif

class SymlinkCreator : public CSingleton < SymlinkCreator >
{
    friend class CSingleton < SymlinkCreator >;

public:
    SymlinkCreator() = default;
    bool HandleKeypress(std::string& pressed_keys);
    void UnmarkFiles();

    bool is_enabled = 1;
    std::string mark_key = "DOWN";
    std::string place_symlink_key = "UP";
    std::string place_hardlink_key = "RIGHT";

private:
    void Mark();
    void Place(bool is_symlink);
    void GetSelectedItemsFromFileExplorer();
    std::wstring GetDestinationPathFromFileExplorer();
#ifdef _WIN32
    IFolderView2* GetFolderView2();
#endif
    std::vector<wchar_t*> selected_items;
};