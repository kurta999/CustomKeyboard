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

    // !\brief Handle incomming keypress
    // !\param pressed_keys [in] Pressed keys as string
    bool HandleKeypress(std::string& pressed_keys);

    // !\brief Delete all marked files from list
    void UnmarkFiles();

    // !\brief Is symlink/hardlink creation enabled?
    bool is_enabled = 1;

    // !\brief Mark key (for items which from create symlink/hardlink)
    std::string mark_key = "DOWN";

    // !\brief Place key for symlink
    std::string place_symlink_key = "UP";

    // !\brief Place key for hardlink
    std::string place_hardlink_key = "RIGHT";

private:
    // !\brief Marks symlink source from file explorer
    void Mark();

    // !\brief Create sylink
    // !\param is_symlink [in] true = symlink, false = hardlink
    void Place(bool is_symlink);

    // !\brief Selected item's path
    std::vector<std::wstring> m_SelectedItems;
};