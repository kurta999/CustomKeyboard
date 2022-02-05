#pragma once

#include "utils/CSingleton.h"

#include <filesystem>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <future>

constexpr size_t max_timestamp_len = 80;

class PrintScreenSaver : public CSingleton < PrintScreenSaver >
{
    friend class CSingleton < PrintScreenSaver >;
public:
    // \brief Initialize function
    void Init();

    // \brief Execute screenshot saving
    void SaveScreenshot();

    // \brief Screenshot timestamp format in filename
    std::string timestamp_format = "%Y.%m.%d %H.%M.%S";

    // \brief Screenshots path (relative to application directory)
    std::filesystem::path screenshot_path = "Screenshots";

    // \brief Screenshot key
    std::string screenshot_key = "F12";
private:
    void DoSave();
    std::future<void> screenshot_future;

    void FormatTimestamp(char* buf, uint8_t len);
#ifdef _WIN32
    INT GetPixelDataOffsetForPackedDIB(const BITMAPINFOHEADER* BitmapInfoHeader);
    unsigned decodeBMP(std::vector<unsigned char>& image, unsigned& w, unsigned& h, const std::vector<unsigned char>& bmp);
#endif
};