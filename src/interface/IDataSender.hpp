#pragma once

#include <filesystem>

class DataEntry;

class IDataEntryLoader
{
public:
    virtual ~IDataEntryLoader() = default;

    virtual bool Load(const std::filesystem::path& path, std::vector<std::unique_ptr<DataEntry>>& e) = 0;
    virtual bool Save(const std::filesystem::path& path, std::vector<std::unique_ptr<DataEntry>>& e) const = 0;
};

class IDataLogHelper
{
public:
    virtual ~IDataLogHelper() = default;

    virtual void ClearEntries() = 0;
    virtual void AppendLog(const std::string& line) = 0;
    virtual void OnError(uint32_t err_cnt, const std::string& line) = 0;
};

enum class dataEntryType
{
    Hex,
    String
};

enum class dataEntrySendType
{
    Auto,
    Trigger
};

class DataEntryBase
{
public:
    DataEntryBase() = default;
    DataEntryBase(uint8_t* data_, size_t data_len, uint8_t* response_data_, size_t response_data_len)
    {
        if (data_ && data_len)
            data.insert(data.end(), data_, data_ + data_len);
        if (response_data_ && response_data_len)
            response.insert(response.end(), response_data_, response_data_ + response_data_len);
    }

    DataEntryBase(const DataEntryBase& from) :
        data(from.data)
    {

    }
    std::vector<uint8_t> data{};
    std::vector<uint8_t> response{};
    std::chrono::steady_clock::time_point lastExecution;
};

class DataEntry : public DataEntryBase
{
public:
    DataEntry() = default;

    DataEntry(uint8_t* data_, size_t data_len, uint8_t* response_data_, size_t response_data_len, dataEntryType type, dataEntrySendType send_type,
        size_t step, uint32_t period, uint32_t response_timeout, std::string& comment) :
        DataEntryBase(data_, data_len, response_data_, response_data_len), m_sendType(send_type),
        m_maxSendCount(step), m_type(type), m_period(period), m_responseTimeout(response_timeout), m_comment(comment)
    {

    }

    // !\brief Comment for frame
    std::string m_comment{};

    // !\brief Max send count
    size_t m_maxSendCount{};

    // !\brief How many times frame was sent
    size_t m_sentTimes{};

    // !\brief Period of sending
    uint32_t m_period{};

    // !\brief Response timeout
    uint32_t m_responseTimeout{};

    // !\brief Add CRC to frame?
    int m_addCrc{};

    // !\brief Type of data
    dataEntryType m_type;

    // !\brief Type of sending
    dataEntrySendType m_sendType{ dataEntrySendType::Auto };

    // !\brief Has frame to be sent periodically?
    bool m_send{ false };
};

class XmlDataEntryLoader : public IDataEntryLoader
{
public:
    virtual ~XmlDataEntryLoader() = default;

    bool Load(const std::filesystem::path& path, std::vector<std::unique_ptr<DataEntry>>& e) override;
    bool Save(const std::filesystem::path& path, std::vector<std::unique_ptr<DataEntry>>& e) const override;
};
