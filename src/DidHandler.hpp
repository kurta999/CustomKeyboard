#pragma once

#include "IDidLoader.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <semaphore>

#include "ICanObserver.hpp"

constexpr const char* DID_CACHE_FILENAME = "DidCache.xml";

enum DidEntryType : uint8_t
{
    DET_UI8, DET_UI16, DET_UI32, DET_UI64, DET_STRING, DET_BYTEARRAY, DET_INVALID
};

using DidValueType = std::variant<uint8_t, uint16_t, uint32_t, uint64_t, std::string, std::vector<uint8_t>>;

class DidEntry
{
public:
    uint16_t id = 0;
    DidEntryType type;
    std::string name;
    DidValueType value;
    std::string value_str;
    std::string min;  /* It's just informative right now */
    std::string max;  /* It's just informative right now */
    size_t len = 0;
    uint16_t nrc = 0;

    boost::posix_time::ptime last_update = boost::posix_time::not_a_date_time;
};

class XmlDidLoader : public IDidLoader
{
public:
    virtual ~XmlDidLoader();

    bool Load(const std::filesystem::path& path, DidMap& m) override;
    bool Save(const std::filesystem::path& path, DidMap& m) override;

    static DidEntryType GetTypeFromString(const std::string_view& input);
    static const std::string_view GetStringFromType(DidEntryType type);

private:
    static inline std::map<DidEntryType, std::string> m_DidEntryTypeMap
    {
        {DET_UI8, "uint8_t"},
        {DET_UI16, "uint16_t"},
        {DET_UI32, "uint32_t"},
        {DET_UI64, "uint64_t"},
        {DET_STRING, "string"},
        {DET_BYTEARRAY, "bytearray"},
        {DET_INVALID, "invalid"}
    };
};

class XmlDidCacheLoader : public IDidLoader
{
public:
    virtual ~XmlDidCacheLoader();

    bool Load(const std::filesystem::path& path, DidMap& m) override;
    bool Save(const std::filesystem::path& path, DidMap& m) override;

private:

};

class DidHandler : public ICanObserver
{
public:
    DidHandler(IDidLoader& loader, IDidLoader& cache_loader, CanEntryHandler* can_handler);
    virtual ~DidHandler();

    // !\brief Initialize DID handler
    void Init();

    // !\brief Save DID cache
    bool SaveChache();

    // !\brief Add DID to read queue which is being read by this handler
    // !\param did [in] DID to add
    void AddDidToReadQueue(uint16_t did);

    // !\brief Write DID with specified data
    // !\param did [in] DID to write
    // !\param data_to_write [in] Data to write
    // !\param size [in] Size of data to write
    void WriteDid(uint16_t did, uint8_t* data_to_write, uint16_t size);

    // !\brief Notify that a DID has been updated
    void NotifyDidUpdate();

    // !\brief Abort DID reading
    void AbortDidUpdate();

    void OnFrameOnBus(uint32_t frame_id, uint8_t* data, uint16_t size) override;
    void OnIsoTpDataReceived(uint32_t frame_id, uint8_t* data, uint16_t size) override;
     
    // !\brief DID list
    DidMap m_DidList;

    // !\brief Mutex for entry handler
    std::mutex m;

    // !\brief DIDs whose values have been updated
    std::vector<uint16_t> m_UpdatedDids;

private:
    // !\brief Send a UDS frame and wait for a response
    // !\param frame [in] Frame to send
    bool SendUdsFrameAndWaitForResponse(std::vector<uint8_t> frame);

    // !\brief Wait for a response from the ECU
    bool WaitForResponse();

    // !\brief Process a DID response
    // !\param entry [in] DID entry to process
    void ProcessReadDidResponse(std::unique_ptr<DidEntry>& entry);

    // !\brief Process a NRC response 
    void ProcessRejectedNrc(std::unique_ptr<DidEntry>& entry);

    // !\brief Handle DID reading
    void HandleDidReading(std::stop_token& token);

    // !\brief Handle DID writing
    void HandleDidWriting(std::stop_token& token);

    // !\brief Worker thread
    void WorkerThread(std::stop_token token);

    // !\brief Worker thread object
    std::unique_ptr<std::jthread> m_worker;

    // !\brief Condition variable waiting for CAN message
    std::condition_variable_any m_CanMessageCv;

    // !\brief Mutex for CAN message CV
    std::mutex m_CanMessageMutex;

    // !\brief Semaphore for worker thread
    std::binary_semaphore m_Semaphore;

    // !\brief DIDs whose values are being read
    std::deque<uint16_t> m_PendingDidReads;

    // !\brief DIDs whose values are being written
    std::map<uint16_t, std::string> m_PendingDidWrites;

    // !\brief Callback to call when a DID has been updated
    std::atomic<bool> m_IsAborted{ false };

    // !\brief Buffer for IsoTp frames
    uint8_t m_IsoTpBuffer[4096] = {};

    // !\brief Length of IsoTp buffer
    std::atomic<uint16_t> m_IsoTpBufLen{};

    // !\brief DID Loader
    IDidLoader& m_loader;

    // !\brief DID Cache Loader
    IDidLoader& m_cache_loader;

    // !\brief Local instance for CAN entry handler
    CanEntryHandler* m_can_handler = nullptr;
};