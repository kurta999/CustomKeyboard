#pragma once

#include "IModbusEntry.hpp"

class XmlModbusEnteryLoader : public IModbusEntryLoader
{
public:
    virtual ~XmlModbusEnteryLoader() = default;

    bool Load(const std::filesystem::path& path, uint8_t& slave_id, ModbusItemType& coils, ModbusItemType& input_status,
        ModbusItemType& holding, ModbusItemType& input, NumModbusEntries& num_entries) override;
    bool Save(const std::filesystem::path& path, uint8_t& slave_id, ModbusItemType& coils, ModbusItemType& input_status,
        ModbusItemType& holding, ModbusItemType& input, NumModbusEntries& num_entries) const override;
};

typedef enum
{
    MB_ERR_OK,
    MB_ERR_CRC,
    MB_ERR_TIMEOUT,
    MB_ERR_ILLEGAL_FUNCTION = 129,
    MB_ERR_ILLEGAL_DATA_ADDRESS = 130,
    MB_ERR_ILLEGAL_DATA_VALUE = 131,
    MB_ERR_SLAVE_DEVICE_FAILURE = 132,
    MB_ERR_ACK = 133,
    MB_ERR_SLAVE_DEVICE_BUSY = 134,
    MB_ERR_NAK,
    MB_ERR_MEMORY_PARITY_ERROR,
    MB_ERR_GATEWAY_UNAVAILABLE = 143,
    MB_ERR_GATEWAY_TARGET_FAILED = 144,
} ModbusErorrType;

class ModbusLogEntry
{
public:
    ModbusLogEntry(uint8_t dir, uint8_t fc, ModbusErorrType error, uint8_t* data_, size_t data_len, std::chrono::steady_clock::time_point& timepoint) :
        direction(dir), fcode(fc), error_type(error), last_execution(timepoint)
    {
        if(data_ && data_len)
            data.insert(data.end(), data_, data_ + data_len);
    }

    std::vector<uint8_t> data;
    uint8_t direction;
    uint8_t fcode;
    ModbusErorrType error_type;
    std::chrono::steady_clock::time_point last_execution;
};

class ModbusMasterSerialPort;
class ModbusEntryHandler
{
public:
    ModbusEntryHandler(XmlModbusEnteryLoader& loader);
    ~ModbusEntryHandler();

    // !\brief Initialize entry handler
    void Init();

    void Save();

    void SetModbusHelper(IModbusHelper* helper);

    void SetEnabled(bool enable);
    bool IsEnabled() const;

    void SetPollingStatus(bool is_active);

    void ToggleAutoRecord(bool toggle) { auto_recording = toggle; }

    // !\param Is auto recording enabled?
    // !\return Is auto recording enabled?
    bool IsAutoRecord() const { return auto_recording; }

    bool IsAutoSend() const { return auto_send; }

    void ToggleAutoSend(bool toggle);
    void ToggleRecording(bool toggle, bool is_pause);
    void ClearRecording();
    bool SaveRecordingToFile(std::filesystem::path& path);

    void SetPollingRate(uint16_t rate_ms) { m_pollingRate = rate_ms; }
    uint16_t GetPollingRate() const { return m_pollingRate; }

    void SetDefaultConfigName(const std::string& default_config) { m_DefaultConfigName = default_config; }
    const std::string& GetDefaultConfigName() const { return m_DefaultConfigName; }

    // !\brief Get favourite level
    uint8_t GetFavouriteLevel() const { return m_DefaultFavouriteLevel; }

    // !\brief Set favourite level
    // !\param favourite_level [in] Favourite level
    void SetFavouriteLevel(uint8_t favourite_level) { m_DefaultFavouriteLevel = favourite_level; }

    // !\brief Set max recorded entries
    // !\param max_entries [in] Max recorded entries
    void SetMaxRecordedEntries(size_t max_entries) { max_recorded_entries = max_entries; }

    // !\brief Get max recorded entries
    size_t GetMaxRecordedEntries() { return max_recorded_entries; }

    void EditCoil(size_t id, bool value) { m_pendingCoilWrites.push_back({ id, value }); }
    void EditHolding(size_t id, uint64_t value) { m_pendingHoldingWrites.push_back({ id, value }); }

    ModbusMasterSerialPort& GetSerial() { return *m_Serial; }
    
    size_t GetTxFrameCount() { return tx_frame_cnt; }
    size_t GetRxFrameCount() { return rx_frame_cnt; }
    size_t GetErrFrameCount() { return err_frame_cnt; }

    uint8_t m_slaveId;
    ModbusItemType m_coils;
    ModbusItemType m_inputStatus;
    ModbusItemType m_Holding;
    ModbusItemType m_Input;
    NumModbusEntries m_numEntries;

    std::vector<std::unique_ptr<ModbusLogEntry>> m_LogEntries;

    // !\brief Sending every can frame automatically at startup which period is not null? 
    bool auto_send = false;

    // !\brief Start recording automatically?
    bool auto_recording = false;

    // !\brief Is recording on?
    bool is_recoding = false;

    size_t max_recorded_entries = 30000;

    std::chrono::steady_clock::time_point GetStartTime() { return start_time; }

    std::chrono::steady_clock::time_point start_time;

    std::mutex m;

private:
    void HandleBoolReading(std::vector<uint8_t>& reg, ModbusItemType& items, size_t num_items, ModbusItemPanel* panel);
    void HandleRegisterReading(std::vector<uint16_t>& reg, ModbusItemType& items, size_t num_items, ModbusItemPanel* panel);

    void WaitIfPaused();
    void HandlePolling();
    void HandleWrites();
    void ModbusWorker(std::stop_token token);
    
    IModbusEntryLoader& m_ModbusEntryLoader;

    bool is_enabled{ true };
    uint16_t m_pollingRate{ 500 };

    std::string m_DefaultConfigName{" Modbus.xml "};

    size_t tx_frame_cnt = 0;
    size_t rx_frame_cnt = 0;
    size_t err_frame_cnt = 0;

    // !\brief Favourite level
    // !\details CAN entries (both TX & RX) which level is below this level, won't show on TX and RX list
    uint8_t m_DefaultFavouriteLevel = 0;

    std::vector<std::pair<int, bool>> m_pendingCoilWrites;
    std::vector<std::pair<int, uint64_t>> m_pendingHoldingWrites;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_workerModbus;

    std::condition_variable_any cv;
    
    bool m_isMainThreadPaused = false;

    bool m_isOpenInProgress = false;

    bool m_isCloseInProgress = false;

    std::unique_ptr<ModbusMasterSerialPort> m_Serial;

    IModbusHelper* m_helper = nullptr;
};