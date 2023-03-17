#pragma once

#include "IDidLoader.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

enum DidEntryType : uint8_t
{
    DET_UI8, DET_UI16, DET_UI32, DET_UI64, DET_STRING, DET_BYTEARRAY, DET_INVALID
};

using DidValueType = std::variant<uint8_t, uint16_t, uint32_t, uint64_t, std::string, std::vector<uint8_t>>;

class DidEntry
{
public:
    uint16_t id;
    DidEntryType type;
    std::string name;
    DidValueType value;
    std::string value_str;
    std::string min;  /* It's just informative right now */
    std::string max;  /* It's just informative right now */
    size_t len;
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

#if 0
    static inline std::map<DidEntryType, std::pair<int64_t, int64_t>> m_DidEntrySizes  /* TODO: use int128_t for size from boost::multiprecision */
    {
        {DET_UI8, {0, 1}},
        {DET_UI16, {std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()}},
        {DET_UI32, {std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max()}},
        {DET_STRING, {std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max()}},
        {DET_BYTEARRAY, {std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max()}},
        {CBT_I64, {std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max()}},
        {DET_INVALID, {0, 0}}
    };
#endif
};

class XmlDidCacheLoader : public IDidLoader
{
public:
    virtual ~XmlDidCacheLoader();

    bool Load(const std::filesystem::path& path, DidMap& m) override;
    bool Save(const std::filesystem::path& path, DidMap& m) override;

private:

};

class DidHandler
{
public:
    DidHandler(IDidLoader& loader, IDidLoader& cache_loader);
    ~DidHandler();

    void Init();

    void SaveChache();

    IDidLoader& m_loader;
    IDidLoader& m_cache_loader;

    DidMap m_DidList;

    void AddDidToReadQueue(uint16_t did);

    void WriteDid(uint16_t did, uint8_t* data_to_write, uint16_t size);

    void SetDidCompletionCallback(std::function<void(uint16_t)> callback);

    void OnIsoTpFrameReceived(uint8_t* data, size_t size);

    // !\brief Mutex for entry handler
    std::recursive_mutex m;

    std::vector<uint16_t> m_UpdatedDids;

private:

    bool SendUdsFrameAndWaitForResponse(std::vector<uint8_t> frame);

    bool WaitForResponse();

    void ProcessReadDidResponse(std::unique_ptr<DidEntry>& entry);

    void ProcessRejectedNrc(std::unique_ptr<DidEntry>& entry);

    void HandleDidReading();

    void HandleDidWriting();

    // !\brief Worker thread
    void WorkerThread(std::stop_token token);

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_worker;

    std::deque<uint16_t> m_PendingDidReads;

    std::map<uint16_t, std::string> m_PendingDidWrites;

    std::function<void(uint16_t)> m_DidCompletionCbk;

    uint8_t m_IsoTpBuffer[4096] = {};

    std::atomic<uint16_t> m_IsoTpBufLen{};
};