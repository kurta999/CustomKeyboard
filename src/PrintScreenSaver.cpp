#include "pch.h"

//#define SAVE_BMP_TOO

void PrintScreenSaver::Init()
{
    //SaveScreenshot();
}

void PrintScreenSaver::FormatTimestamp(char* buf, uint8_t len)
{
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buf, len, timestamp_format.c_str(), timeinfo);
    strncat(buf, ".png", 4);
}

void PrintScreenSaver::DoSave()
{
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

    OpenClipboard(NULL);
    HGLOBAL ClipboardDataHandle = (HGLOBAL)GetClipboardData(CF_DIB);
    if(!ClipboardDataHandle)
    {
        // Clipboard object is not a DIB, and is not auto-convertible to DIB
        CloseClipboard();
        return;
    }

    BITMAPINFOHEADER* BitmapInfoHeader = (BITMAPINFOHEADER*)GlobalLock(ClipboardDataHandle);
    assert(BitmapInfoHeader); // This can theoretically fail if mapping the HGLOBAL into local address space fails. Very pathological, just act as if it wasn't a bitmap in the clipboard.

    SIZE_T ClipboardDataSize = GlobalSize(ClipboardDataHandle);
    assert(ClipboardDataSize >= sizeof(BITMAPINFOHEADER)); // Malformed data. While older DIB formats exist (e.g. BITMAPCOREHEADER), they are not valid data for CF_DIB; it mandates a BITMAPINFO struct. If this fails, just act as if it wasn't a bitmap in the clipboard.

    INT PixelDataOffset = GetPixelDataOffsetForPackedDIB(BitmapInfoHeader);
    size_t TotalBitmapFileSize = sizeof(BITMAPFILEHEADER) + ClipboardDataSize;

    BITMAPFILEHEADER BitmapFileHeader = {};
    BitmapFileHeader.bfType = 0x4D42;
    BitmapFileHeader.bfSize = (DWORD)TotalBitmapFileSize; // Will fail if bitmap size is nonstandard >4GB
    BitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + PixelDataOffset;
    CloseClipboard();

    char buf[max_timestamp_len];
    FormatTimestamp(buf, max_timestamp_len);
#ifdef SAVE_BMP_TOO
    HANDLE FileHandle = CreateFileA((std::string(buf) + ".bmp").c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD dummy = 0;
        BOOL Success = true;
        Success &= WriteFile(FileHandle, &BitmapFileHeader, sizeof(BITMAPFILEHEADER), &dummy, NULL);
        Success &= WriteFile(FileHandle, BitmapInfoHeader, (DWORD)ClipboardDataSize, &dummy, NULL);
        Success &= CloseHandle(FileHandle);
        if(Success)
        {
            wprintf(L"File saved.\r\n");
        }
    }
#endif
    std::vector<unsigned char> png;
    std::vector<unsigned char> clipboard_bmp;
    std::vector<unsigned char> bmp_to_encode;
    std::copy((char*)(&BitmapFileHeader), (char*)(&BitmapFileHeader) + sizeof(BITMAPFILEHEADER), std::back_inserter(clipboard_bmp));
    std::copy((char*)(BitmapInfoHeader), (char*)(BitmapInfoHeader)+ClipboardDataSize, std::back_inserter(clipboard_bmp));

    unsigned w_, h_;
    unsigned error = decodeBMP(bmp_to_encode, w_, h_, clipboard_bmp);
    unsigned long error_code = lodepng::encode(png, bmp_to_encode, BitmapInfoHeader->biWidth, BitmapInfoHeader->biHeight, LCT_RGBA, 8);

    std::string save_path = screenshot_path.string() + "\\" + buf;
    lodepng::save_file(png, save_path.c_str());

    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

    LOGMSG(notification, "Image saved to {}", save_path);

    MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
    {
        std::lock_guard lock(frame->mtx);
        frame->pending_msgs.push_back({ (uint8_t)ScreenshotSaved, dif });
    }
}

void PrintScreenSaver::SaveScreenshot()
{
    if(screenshot_future.valid())
        screenshot_future.get();

    screenshot_future = std::async(&PrintScreenSaver::DoSave, this);
}

// Returns the offset, in bytes, from the start of the BITMAPINFO, to the start of the pixel data array, for a packed DIB.
INT PrintScreenSaver::GetPixelDataOffsetForPackedDIB(const BITMAPINFOHEADER* BitmapInfoHeader)
{
    INT OffsetExtra = 0;

    if(BitmapInfoHeader->biSize == sizeof(BITMAPINFOHEADER) /* 40 */)
    {
        // This is the common BITMAPINFOHEADER type. In this case, there may be bit masks following the BITMAPINFOHEADER
        // and before the actual pixel bits (does not apply if bitmap has <= 8 bpp)
        if(BitmapInfoHeader->biBitCount > 8)
        {
            if(BitmapInfoHeader->biCompression == BI_BITFIELDS)
            {
                OffsetExtra += 3 * sizeof(RGBQUAD);
            }
            else if(BitmapInfoHeader->biCompression == 6 /* BI_ALPHABITFIELDS */)
            {
                // Not widely supported, but valid.
                OffsetExtra += 4 * sizeof(RGBQUAD);
            }
        }
    }

    if(BitmapInfoHeader->biClrUsed > 0)
    {
        // We have no choice but to trust this value.
        OffsetExtra += BitmapInfoHeader->biClrUsed * sizeof(RGBQUAD);
    }
    else
    {
        // In this case, the color table contains the maximum number for the current bit count (0 if > 8bpp)
        if(BitmapInfoHeader->biBitCount <= 8)
        {
            // 1bpp: 2
            // 4bpp: 16
            // 8bpp: 256
            OffsetExtra += sizeof(RGBQUAD) << BitmapInfoHeader->biBitCount;
        }
    }

    return BitmapInfoHeader->biSize + OffsetExtra;
}

//returns 0 if all went ok, non-0 if error
//output image is always given in RGBA (with alpha channel), even if it's a BMP without alpha channel
unsigned PrintScreenSaver::decodeBMP(std::vector<unsigned char>& image, unsigned& w, unsigned& h, const std::vector<unsigned char>& bmp)
{
    static const unsigned MINHEADER = 54; //minimum BMP header size

    if(bmp.size() < MINHEADER) return -1;
    if(bmp[0] != 'B' || bmp[1] != 'M') return 1; //It's not a BMP file if it doesn't start with marker 'BM'
    unsigned pixeloffset = bmp[10] + 256 * bmp[11]; //where the pixel data starts
    //read width and height from BMP header
    w = bmp[18] + bmp[19] * 256;
    h = (signed short)(bmp[22] + bmp[23] * 256);
    //read number of channels from BMP header
    if(bmp[28] != 24 && bmp[28] != 32) return 2; //only 24-bit and 32-bit BMPs are supported.
    unsigned numChannels = bmp[28] / 8;

    //The amount of scanline bytes is width of image times channels, with extra bytes added if needed
    //to make it a multiple of 4 bytes.
    unsigned scanlineBytes = w * numChannels;
    if(scanlineBytes % 4 != 0) scanlineBytes = (scanlineBytes / 4) * 4 + 4;

    unsigned dataSize = scanlineBytes * h;
    if(bmp.size() < dataSize + pixeloffset) return 3; //BMP file too small to contain all pixels

    image.resize(w * h * 4);

    /*
    There are 3 differences between BMP and the raw image buffer for LodePNG:
    -it's upside down
    -it's in BGR instead of RGB format (or BRGA instead of RGBA)
    -each scanline has padding bytes to make it a multiple of 4 if needed
    The 2D for loop below does all these 3 conversions at once.
    */
    for(unsigned y = 0; y < h; y++)
        for(unsigned x = 0; x < w; x++) {
            //pixel start byte position in the BMP
            unsigned bmpos = pixeloffset + (h - y - 1) * scanlineBytes + numChannels * x;
            //pixel start byte position in the new raw image
            unsigned newpos = 4 * y * w + 4 * x;
            if(numChannels == 3) {
                image[newpos + 0] = bmp[bmpos + 2]; //R
                image[newpos + 1] = bmp[bmpos + 1]; //G
                image[newpos + 2] = bmp[bmpos + 0]; //B
                image[newpos + 3] = 255;            //A
            }
            else {
                image[newpos + 0] = bmp[bmpos + 2]; //R
                image[newpos + 1] = bmp[bmpos + 1]; //G
                image[newpos + 2] = bmp[bmpos + 0]; //B
                image[newpos + 3] = bmp[bmpos + 3]; //A
            }
        }
    return 0;
}
