#pragma once

namespace ImageRecognition
{
    bool BringWindowToForegroundByName(const std::string& process_name, const std::string& window_name);

    bool FindImageOnScreen(const std::string& image_name, int& x, int& y);
    void MoveCursorAndClick(POINT pos);
}