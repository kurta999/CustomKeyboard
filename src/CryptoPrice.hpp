#pragma once

#include "utils/CSingleton.hpp"
#include <future>
#include <atomic>

class CryptoPrice : public CSingleton < CryptoPrice >
{
    friend class CSingleton < CryptoPrice >;

public:
    CryptoPrice();
    ~CryptoPrice() = default;
 
    // !\brief Update prices
    // !\param force [in] Force update even if timeout isn't happend
    void UpdatePrices(bool force = false);

    // !\brief Timepoint for last update
    std::chrono::steady_clock::time_point last_update;

    // !\brief Future for executing async crypto price reading
    std::future<void> m_api_future;

    // !\brief ETH Buy price
    std::atomic<float> eth_buy;

    // !\brief ETH Sell price
    std::atomic<float> eth_sell;

    // !\brief BTC Buy price
    std::atomic<float> btc_buy;

    // !\brief BTC Sell price
    std::atomic<float> btc_sell;

    // !\brief Is new data pending? (used by GUI - main panel)
    std::atomic<bool> is_pending;

private:
    // !\brief Get coin price from web API
    void ExecuteApiRead(void);
};