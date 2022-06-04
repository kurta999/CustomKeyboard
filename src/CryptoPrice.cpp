#include "pch.hpp"

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

CryptoPrice::CryptoPrice()
{

}


void CryptoPrice::ExecuteApiRead()
{
    std::string buy_price_str = exec("curl https://api.coinbase.com/v2/prices/ETH-USD/buy -: https://api.coinbase.com/v2/prices/ETH-USD/sell -: https://api.coinbase.com/v2/prices/BTC-USD/buy -: https://api.coinbase.com/v2/prices/BTC-USD/sell");
    
    std::vector<std::string> arr;
    boost::algorithm::split_regex(arr, buy_price_str, boost::regex("{\"data\":{\"base\":\""));
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
        catch(...)
        {

        }
    };

    ExtractAmount(arr[0], eth_buy);
    ExtractAmount(arr[1], eth_sell);
    ExtractAmount(arr[2], btc_buy);
    ExtractAmount(arr[3], btc_sell);
    
    LOGMSG(notification, "Coin price successfully retreived! Buy, Sell - ETH: {}, {}, BTC: {}, {}", eth_buy, eth_sell, btc_buy, btc_sell);
}

void CryptoPrice::UpdatePrices()
{
    if(m_api_future.valid())
        if(m_api_future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready)
            return;
    m_api_future = std::async(&CryptoPrice::ExecuteApiRead, this);
}

void CryptoPrice::Init()
{
    UpdatePrices();
}