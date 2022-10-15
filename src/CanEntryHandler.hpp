#pragma once

#include "utils/CSingleton.hpp"
#include <filesystem>

constexpr uint8_t CAN_LOG_DIR_TX = 0;
constexpr uint8_t CAN_LOG_DIR_RX = 1;

class CanEntryBase
{
public:
    CanEntryBase() = default;
    CanEntryBase(uint8_t* data_, uint8_t data_len)
    {
        if(data_ && data_len)
            data.insert(data.end(), data_, data_ + data_len);
    }

    CanEntryBase(const CanEntryBase& from) :
        data(from.data)
    {

    }
    std::vector<uint8_t> data{};
    std::chrono::steady_clock::time_point last_execution;
};

class CanEntryTransmitInfo
{
public:
    CanEntryTransmitInfo() = default;

    CanEntryTransmitInfo(const CanEntryTransmitInfo& from) :
        period(from.period)
    {

    }

    uint32_t period{};
    size_t count{};
};

class CanTxEntry : public CanEntryBase, public CanEntryTransmitInfo
{
public:
    CanTxEntry() = default;
    ~CanTxEntry() = default;
    CanTxEntry(const CanTxEntry& from) : 
        CanEntryBase(from), id(from.id + 1), CanEntryTransmitInfo(from), comment(from.comment)
    { 

    }
    uint32_t id{};
    std::string comment{};
    bool send = false;
    bool single_shot = false;
};

class CanRxData : public CanEntryBase, public CanEntryTransmitInfo
{
public:
    CanRxData(uint8_t* data_, uint8_t data_len) :
        CanEntryBase(data_, data_len)
    {
        count = 1;
    }
};

class CanLogEntry : public CanEntryBase
{
public:
    CanLogEntry(uint8_t dir, uint32_t frame_id_, uint8_t* data_, uint8_t data_len, std::chrono::steady_clock::time_point& timepoint) :
        CanEntryBase(data_, data_len)
    {
        frame_id = frame_id_ & 0x1FFFFFFF;
        direction = dir & 1;
        last_execution = timepoint;
    }
    union
    {
        uint32_t frame_id_and_direction;
        struct
        {
            uint32_t frame_id : 29;
            uint8_t direction : 1;  /* 0 = sent, 1 = received */
        };
    };
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

    // !\brief Initialize entry handler
    void Init();

    // !\brief Called when a can frame was sent
    void OnFrameSent(uint32_t frame_id, uint8_t data_len, uint8_t* data);

    // !\brief Called when a can frame was received
    void OnFrameReceived(uint32_t frame_id, uint8_t data_len, uint8_t* data);
    
    // !\brief Toggle automatic sending of all CAN frames which period isn't null
    // !\param toggle [in] Toggle auto send?
    void ToggleAutoSend(bool toggle);    
    
    // !\brief Toggle recording
    // !\param toggle [in] Toggle recording?
    // !\param is_puase [in] Is pause?
    void ToggleRecording(bool toggle, bool is_puase);

    // !\brief Clear recorded frames
    void ClearRecording();

    // !\brief Load TX list from a file
    // !\param path [in] File path to load
    // !\return Is load was successfull?
    bool LoadTxList(std::filesystem::path& path);

    // !\brief Save TX list to a file
    // !\param path [in] File path to save
    bool SaveTxList(std::filesystem::path& path);

    // !\brief Load RX list from a file
    // !\param path [in] File path to load
    // !\return Is load was successfull?
    bool LoadRxList(std::filesystem::path& path);

    // !\brief Save RX list to a file
    // !\param path [in] File path to save
    bool SaveRxList(std::filesystem::path& path);
    
    // !\brief Save recorded data to file
    // !\param path [in] File path to save
    bool SaveRecordingToFile(std::filesystem::path& path);

    // !\brief Return TX Frame count
    // !\return TX Frame count
    uint64_t GetTxFrameCount() { return tx_frame_cnt; }

    // !\brief Return RX Frame count
    // !\return RX Frame count
    uint64_t GetRxFrameCount() { return rx_frame_cnt; }

    // !\brief Vector of CAN TX entries
    std::vector<std::unique_ptr<CanTxEntry>> entries;

    // !\brief Vector of CAN RX entries
    std::unordered_map<uint32_t, std::unique_ptr<CanRxData>> m_rxData;  /* [frame_id] = rxData */

    // !\brief Frame ID comment
    std::unordered_map<uint32_t, std::string> rx_entry_comment;  /* [frame_id] = comment msg */

    // !\brief CAN Log entries (both TX & RX)
    std::vector<std::unique_ptr<CanLogEntry>> m_LogEntries;

    // !\brief Path to default TX list
    std::filesystem::path default_tx_list;
    
    // // !\brief Path to default RX list
    std::filesystem::path default_rx_list;

    // !\brief Mutex for entry handler
    std::mutex m;

private:
    // !\brief Reference to can TX entry loader
    ICanEntryLoader& m_CanEntryLoader;

    // !\brief Reference to can RX entry loader
    ICanRxEntryLoader& m_CanRxEntryLoader;

    // !\brief Sending every can frame automatically at startup which period is not null? 
    bool auto_send = false;

    // !\brief Is recording on?
    bool is_recoding = false;

    // !\brief TX Frame count
    uint64_t tx_frame_cnt = 0;

    // !\brief RX Frame count
    uint64_t rx_frame_cnt = 0;

    // !\brief Exit worker thread?
    std::atomic<bool> to_exit = false;

    // !\brief Worker thread
    std::unique_ptr<std::thread> m_worker;

    // !\brief Starting time
    std::chrono::steady_clock::time_point start_time;
};