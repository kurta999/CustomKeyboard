#pragma once

#include "utils/CSingleton.hpp"

class CanTxEntry
{
public:
    uint32_t id{};
    std::vector<uint8_t> data{};
    uint32_t period{};
    size_t count{};
    std::string comment{};
    std::chrono::steady_clock::time_point last_execution;
    bool send = false;
    bool single_shot = false;
};

class CanRxData
{
public:
    CanRxData(uint8_t* data_, uint8_t data_len)
    {
        data.insert(data.end(), data_, data_ + data_len);
        count++;
        last_execution = std::chrono::steady_clock::now();
    }
    std::vector<uint8_t> data{};
    uint32_t period{};
    size_t count{};
    std::chrono::steady_clock::time_point last_execution;
};

class ICanEntryLoader
{
public:
    virtual bool Load(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e) = 0;
    virtual bool Save(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e) = 0;
};

class XmlCanEntryLoader : public ICanEntryLoader
{
public:
    virtual ~XmlCanEntryLoader();

    bool Load(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e) override;
    bool Save(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e) override;
};

class ICanRxEntryLoader
{
public:
    virtual bool Load(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e) = 0;
    virtual bool Save(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e) = 0;
};

class XmlCanRxEntryLoader : public ICanRxEntryLoader
{
public:
    virtual ~XmlCanRxEntryLoader();

    bool Load(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e) override;
    bool Save(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e) override;
};
class CanEntryHandler
{
public:
    CanEntryHandler(ICanEntryLoader& loader, ICanRxEntryLoader& rx_loader);
    ~CanEntryHandler();

    void Init();

    void LoadTxList(std::filesystem::path& path);
    void SaveTxList(std::filesystem::path& path);
    void LoadRxList(std::filesystem::path& path);
    void SaveRxList(std::filesystem::path& path);

    std::vector<std::unique_ptr<CanTxEntry>> entries;
    std::unordered_map<uint32_t, std::unique_ptr<CanRxData>> m_rxData;  /* [frame_id] = rxData */
    std::unordered_map<uint32_t, std::string> rx_entry_comment;  /* [frame_id] = comment msg */

    std::filesystem::path default_tx_list;
    std::filesystem::path default_rx_list;
    std::mutex m;
private:
    ICanEntryLoader& m_CanEntryLoader;
    ICanRxEntryLoader& m_CanRxEntryLoader;

    std::atomic<bool> to_exit = false;
    std::unique_ptr<std::thread> t;
};