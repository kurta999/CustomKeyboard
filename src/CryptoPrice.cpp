#include "pch.hpp"

constexpr uint64_t CRYPTO_PRICE_UPDATE = 5 * 60 * 1000;

CryptoPrice::CryptoPrice()
{

}

void CryptoPrice::ExecuteApiRead()
{
    std::vector<std::string> arr;
#ifdef _WIN32
    CStringA str = utils::ExecuteCmdWithoutWindow(L"/C curl --silent https://api.coinbase.com/v2/prices/ETH-USD/buy -: https://api.coinbase.com/v2/prices/ETH-USD/sell -: https://api.coinbase.com/v2/prices/BTC-USD/buy -: https://api.coinbase.com/v2/prices/BTC-USD/sell", 5000);
  
    boost::algorithm::split_regex(arr, str.GetString(), boost::regex("{\"data\":{\"base\":\""));
#else
    std::string buy_price_str = exec("curl https://api.coinbase.com/v2/prices/ETH-USD/buy -: https://api.coinbase.com/v2/prices/ETH-USD/sell -: https://api.coinbase.com/v2/prices/BTC-USD/buy -: https://api.coinbase.com/v2/prices/BTC-USD/sell");
    boost::algorithm::split_regex(arr, buy_price_str, boost::regex("{\"data\":{\"base\":\""));
#endif

    if(arr[0].find("not recognized") == std::string::npos) /* 'curl' is not recognized as an internal or external command, operable program or batch file. */
    {
        if(arr.begin() != arr.end())
            arr.erase(arr.begin());
        if(arr.size() == 4)
        {
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
        }
        else
        {
            LOG(LogLevel::Error, "Invalid number of requrests received from curl.");
        }
    }
    else
    {
        LOG(LogLevel::Error, "curl is not found on the system, coin price requests won't work!");
    }
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