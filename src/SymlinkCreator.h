#pragma once

#include "utils/CSingleton.h"
#include <string>
#include <shlobj.h>

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
    IFolderView2* GetFolderView2();

    std::vector<PWSTR> selected_items;
};