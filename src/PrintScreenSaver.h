#pragma once

#include "utils/CSingleton.h"

#include <filesystem>
#include <string.h>
#include <Windows.h>
#include <future>

constexpr size_t max_timestamp_len = 80;

class PrintScreenSaver : public CSingleton < PrintScreenSaver >
{
    friend class CSingleton < PrintScreenSaver >;
public:
    void Init();
    void SaveScreenshot();

    std::string timestamp_format = "%Y.%m.%d %H.%M.%S";
    std::filesystem::path screenshot_path = "Screenshots";
    std::string screenshot_key = "F12";
private:
    void DoSave();
    std::future<void> screenshot_future;

    void FormatTimestamp(char* buf, uint8_t len);
    INT GetPixelDataOffsetForPackedDIB(const BITMAPINFOHEADER* BitmapInfoHeader);
    unsigned decodeBMP(std::vector<unsigned char>& image, unsigned& w, unsigned& h, const std::vector<unsigned char>& bmp);
};