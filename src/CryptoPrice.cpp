#include "pch.hpp"
#ifdef _WIN32
#include <atlstr.h>
#endif

constexpr uint64_t CRYPTO_PRICE_UPDATE = 5 * 60 * 1000;

CryptoPrice::CryptoPrice()
{

}

#ifdef _WIN32
CStringA ExecCmd(const wchar_t* cmd)
{
    CStringA strResult;
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe to get results from child's stdout.
    if(!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
        return strResult;

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing.
                              // Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = { 0 };
    BOOL fSuccess = CreateProcessW(L"C:\\windows\\system32\\cmd.exe", (LPWSTR)cmd, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if(!fSuccess)
    {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        return strResult;
    }

    bool bProcessEnded = false;
    for(; !bProcessEnded;)
    {
        // Give some timeslice (50 ms), so we won't waste 100% CPU.
        bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

        // Even if process exited - we continue reading, if
        // there is some data available over pipe.
        for(;;)
        {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if(!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if(!dwAvail) // No data available, return
                break;

            if(!::ReadFile(hPipeRead, buf, std::min((DWORD)sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
                // Error, the child process might ended
                break;

            buf[dwRead] = 0;
            strResult += buf;
        }
    } //for

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return strResult;
} //ExecCmd
#else
std::string exec(const char* cmd)
{
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#define WEXITSTATUS
#endif

    std::array<char, 512> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if(!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}
#endif

void CryptoPrice::ExecuteApiRead()
{
    std::vector<std::string> arr;
#ifdef _WIN32
    CStringA str = ExecCmd(L"/C curl --silent https://api.coinbase.com/v2/prices/ETH-USD/buy -: https://api.coinbase.com/v2/prices/ETH-USD/sell -: https://api.coinbase.com/v2/prices/BTC-USD/buy -: https://api.coinbase.com/v2/prices/BTC-USD/sell");
  
    boost::algorithm::split_regex(arr, str.GetString(), boost::regex("{\"data\":{\"base\":\""));
#else
    std::string buy_price_str = exec("curl https://api.coinbase.com/v2/prices/ETH-USD/buy -: https://api.coinbase.com/v2/prices/ETH-USD/sell -: https://api.coinbase.com/v2/prices/BTC-USD/buy -: https://api.coinbase.com/v2/prices/BTC-USD/sell");
    boost::algorithm::split_regex(arr, buy_price_str, boost::regex("{\"data\":{\"base\":\""));
#endif
    arr.erase(arr.begin());

    auto ExtractAmount = [](std::string& str, std::atomic<float>& out)
    {
        try
        {
            size_t pos = str.find("\"amount\":\"");
            if(pos != std::string::npos)
            {
                out = std::stof(&str[pos + 10]);
            }
        }
        catch(std::exception& e)
        {
            LOG(LogLevel::Error, "Exception: {}", e.what());
        }
    };

    ExtractAmount(arr[0], eth_buy);
    ExtractAmount(arr[1], eth_sell);
    ExtractAmount(arr[2], btc_buy);
    ExtractAmount(arr[3], btc_sell);
    is_pending = true;

    LOG(LogLevel::Notification, "Coin price successfully retreived! Buy, Sell - ETH: {}, {}, BTC: {}, {}", eth_buy, eth_sell, btc_buy, btc_sell);
    last_update = std::chrono::steady_clock::now();
}

void CryptoPrice::UpdatePrices(bool force)
{
    uint64_t dif = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_update).count();
    if(dif > CRYPTO_PRICE_UPDATE || force)
    {
        if(m_api_future.valid())
            if(m_api_future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready)
                return;
        m_api_future = std::async(&CryptoPrice::ExecuteApiRead, this);
    }
}