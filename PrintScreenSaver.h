#pragma once

#include "utils/CSingleton.h"

#include <filesystem>
#include <string.h>
#include <Windows.h>

constexpr size_t max_timestamp_len = 80;

class PrintScreenSaver : public CSingleton < PrintScreenSaver >
{
    friend class CSingleton < PrintScreenSaver >;
public:
    void Init();
    void SaveScreenshot();

    std::string timestamp_format;
    std::filesystem::path screenshot_path;
private:
    void FormatTimestamp(char* buf, uint8_t len);
    INT GetPixelDataOffsetForPackedDIB(const BITMAPINFOHEADER* BitmapInfoHeader);
    unsigned decodeBMP(std::vector<unsigned char>& image, unsigned& w, unsigned& h, const std::vector<unsigned char>& bmp);
};