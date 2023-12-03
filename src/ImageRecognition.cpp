#include "pch.hpp"

namespace ImageRecognition
{
HMONITOR primaryMonitor;

bool FindImageOnScreen(const std::string& image_name, int& x, int& y)
{
#ifdef USE_BSEC
    cv::Mat templateImage = cv::imread(image_name, cv::IMREAD_COLOR);
    if(templateImage.empty()) {
        return false;
    }

    // Get the handle to the primary monitor
    primaryMonitor = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);


    // Get the handle to the secondary monitor, if it exists
    HMONITOR secondaryMonitor = NULL;
    int  monitor_count = GetSystemMetrics(SM_CMONITORS) > 1;
    if(monitor_count > 1) {
        EnumDisplayMonitors(NULL, NULL, [](HMONITOR monitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
            HMONITOR* secondaryMonitorPtr = reinterpret_cast<HMONITOR*>(lParam);
            if(monitor != primaryMonitor) {
                *secondaryMonitorPtr = monitor;
                return FALSE; // Stop enumeration
            }
            return TRUE; // Continue enumeration
            }, reinterpret_cast<LPARAM>(&secondaryMonitor));
    }

    // Capture a screenshot from the primary monitor
    HDC primaryMonitorDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
    HDC primaryScreenshotDC = CreateCompatibleDC(primaryMonitorDC);
    int primaryScreenWidth = GetDeviceCaps(primaryMonitorDC, HORZRES) * monitor_count;
    int primaryScreenHeight = GetDeviceCaps(primaryMonitorDC, VERTRES);
    HBITMAP primaryScreenshotBitmap = CreateCompatibleBitmap(primaryMonitorDC, primaryScreenWidth, primaryScreenHeight);
    SelectObject(primaryScreenshotDC, primaryScreenshotBitmap);
    BitBlt(primaryScreenshotDC, 0, 0, primaryScreenWidth, primaryScreenHeight, primaryMonitorDC, 0, 0, SRCCOPY | CAPTUREBLT);

    cv::Mat primaryScreenshotMat = cv::Mat(primaryScreenHeight, primaryScreenWidth, CV_8UC3);
    auto aaa = (BITMAPINFOHEADER{ sizeof(BITMAPINFOHEADER), primaryScreenshotMat.cols, -primaryScreenshotMat.rows, 1, 24, BI_RGB, 0, 0, 0, 0, });
    GetDIBits(primaryMonitorDC, primaryScreenshotBitmap, 0, primaryScreenshotMat.rows, primaryScreenshotMat.data, (BITMAPINFO*)&aaa, DIB_RGB_COLORS);
    SelectObject(primaryMonitorDC, primaryScreenshotBitmap);
    DeleteDC(primaryMonitorDC);
    DeleteObject(primaryScreenshotBitmap);
    ReleaseDC(NULL, primaryMonitorDC);


    /*
    Mat primaryScreenshotMat = Mat(primaryScreenHeight, primaryScreenWidth, CV_8UC4);
    GetBitmapBits(primaryScreenshotBitmap, primaryScreenHeight * primaryScreenWidth * 4, primaryScreenshotMat.data);
    */
    cv::Mat result;
    matchTemplate(primaryScreenshotMat, templateImage, result, cv::TM_CCOEFF_NORMED);

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    LOG(LogLevel::Debug, "Match: {:.1f}", maxVal);
    if(maxVal >= 0.7)
    {
        //cv::Rect matchRect(maxLoc.x, maxLoc.y, templateImage.cols, templateImage.rows);
        //rectangle(primaryScreenshotMat, matchRect, cv::Scalar(0, 255, 0), 2); // draw a green rectangle around the matched region
        //cout << "Template found at: (" << maxLoc.x << "," << maxLoc.y << ")" << endl;

        x = maxLoc.x;
        y = maxLoc.y;
        return true;
    }
#endif
    return false;
}

void MoveCursorAndClick(POINT pos)
{
#ifdef USE_BSEC
    SetCursorPos(pos.x, pos.y);

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(input));
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(input));
#endif
}

void GetAllWindowsFromProcessID(DWORD dwProcessID, std::vector <HWND>& vhWnds)
{
#ifdef USE_BSEC
    // find all hWnds (vhWnds) associated with a process id (dwProcessID)
    HWND hCurWnd = NULL;
    do
    {
        hCurWnd = FindWindowEx(NULL, hCurWnd, NULL, NULL);
        DWORD dwProcID = 0;
        GetWindowThreadProcessId(hCurWnd, &dwProcID);
        if(dwProcID == dwProcessID)
        {
            vhWnds.push_back(hCurWnd);  // add the found hCurWnd to the vector
            //wprintf(L"Found hWnd %d\n", hCurWnd);
        }
    } while(hCurWnd != NULL);
#endif
}

DWORD FindProcessId(const std::wstring& processName)
{
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if(processesSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    Process32First(processesSnapshot, &processInfo);
    if(!processName.compare(processInfo.szExeFile))
    {
        CloseHandle(processesSnapshot);
        return processInfo.th32ProcessID;
    }

    while(Process32Next(processesSnapshot, &processInfo))
    {
        if(!processName.compare(processInfo.szExeFile))
        {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
    }

    CloseHandle(processesSnapshot);
    return 0;
}

HWND GetHWNDByName(DWORD pid, const std::string& name)
{
    std::vector <HWND> vhWnds;
    GetAllWindowsFromProcessID(pid, vhWnds);
    for(auto& i : vhWnds)
    {
        char window_title[256];
        GetWindowTextA(i, window_title, sizeof(window_title));

        if(strstr(window_title, name.c_str()))
        {
            return i;
        }
    }
    return NULL;
}

void BringWindowToForeground(HWND hwnd)
{
    SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0); // restore the minimize window
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    //redraw to prevent the window blank.
    RedrawWindow(hwnd, NULL, 0, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

    /*
    HWND hCurWnd = ::GetForegroundWindow();
    DWORD dwMyID = ::GetCurrentThreadId();
    DWORD dwCurID = ::GetWindowThreadProcessId(hCurWnd, NULL);
    ::AttachThreadInput(dwCurID, dwMyID, TRUE);
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
    ::SetForegroundWindow(m_hWnd);
    ::SetFocus(m_hWnd);
    ::SetActiveWindow(m_hWnd);
    ::AttachThreadInput(dwCurID, dwMyID, FALSE);
    */
}

bool BringWindowToForegroundByName(const std::string& process_name, const std::string& window_name)
{
    const std::wstring process_namew = std::wstring(process_name.begin(), process_name.end());
    DWORD pid = FindProcessId(process_namew);
    if(pid == 0)
    {
        return false;
    }

    HWND chrome_window = GetHWNDByName(pid, window_name);
    if(chrome_window == 0)
    {
        return false;
    }

    BringWindowToForeground(chrome_window);
    return true;
}

void Minimize(HWND hwnd)
{
    SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}
}