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

    // !\brief Initialize CoinPrice
    void Init(void);    
    
    // !\brief Get coin price from web API
    void ExecuteApiRead(void);

    // !\brief Update prices
    void UpdatePrices(void);

    std::future<void> m_api_future;
    std::atomic<float> eth_buy;
    std::atomic<float> eth_sell;
    std::atomic<float> btc_buy;
    std::atomic<float> btc_sell;
};