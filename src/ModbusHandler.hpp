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

class EventLogEntry
{
public:
    EventLogEntry(std::vector<uint16_t>& data_, std::chrono::steady_clock::time_point timepoint) :
        last_execution(timepoint)
    {
        data = std::move(data_);
    }

    std::vector<uint16_t> data;
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

    // !\brief Save content of predefined registers
    void Save();

    // !\brief Set modbus helper GUI
    // !\param helper [in] Pointer to modbus helper
    void SetModbusHelper(IModbusHelper* helper);

    // !\brief Toggle this module
    // !\param enable [in] Is enabled?
    void SetEnabled(bool enable);

    // !\brief Return activity state of this module
    bool IsEnabled() const;

    // !\brief Toggle polling status
    // !\param is_active [in] Is polling status active?
    void SetPollingStatus(bool is_active);

    // !\brief Toggle automatic CAN frame recording
    // !\param toggle [in] Toggle auto recording?
    void ToggleAutoRecord(bool toggle) { auto_recording = toggle; }

    // !\param Is auto recording enabled?
    // !\return Is auto recording enabled?
    bool IsAutoRecord() const { return auto_recording; }

    // !\param Get CAN auto send state
    // !\return Is auto send toggled?
    bool IsAutoSend() const { return auto_send; }

    // !\brief Toggle automatic sending of all CAN frames which period isn't null
    // !\param toggle [in] Toggle auto send?
    void ToggleAutoSend(bool toggle);

    // !\brief Toggle recording
    // !\param toggle [in] Toggle recording?
    // !\param is_puase [in] Is pause?
    void ToggleRecording(bool toggle, bool is_pause);

    // !\brief Clear recorded frames
    void ClearRecording();

    // !\brief Save recorded data to file
    // !\param path [in] File path to save
    bool SaveRecordingToFile(std::filesystem::path& path);

    // !\brief Save special recorded data to file
    // !\param path [in] File path to save
    bool SaveSpecialRecordingToFile(std::filesystem::path& path);

    // !\brief Set modbus polling rate
    // !\param rate_ms [in] Polling rate [ms]
    void SetPollingRate(uint16_t rate_ms) { m_pollingRate = rate_ms; }

    // !\brief Get modbus polling rate
    // !\return Polling rate [ms]
    uint16_t GetPollingRate() const { return m_pollingRate; }

    // !\brief Get default modbus config name
    // !\return Default modbus config name
    void SetDefaultConfigName(const std::string& default_config) { m_DefaultConfigName = default_config; }

    // !\brief Get default modbus config name
    // !\return Default modbus config name
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
    // !\return Max recorded entries
    size_t GetMaxRecordedEntries() { return max_recorded_entries; }

    // !\brief Edit coil
    // !\param id [in] Coil Register ID
    // !\param value [in] Boolean value for given coil
    void EditCoil(size_t id, bool value) { m_pendingCoilWrites.push_back({ id, value }); }

    // !\brief Edit holding registers
     // !\param id [in] Holding Register ID
    // !\param value [in] New value for given holding register
    void EditHolding(size_t id, uint64_t value) { m_pendingHoldingWrites.push_back({ id, value }); }

    // !\brief Return reference to serial
    // !\return Reference to serial port
    ModbusMasterSerialPort& GetSerial() { return *m_Serial; }
    
    // !\brief Return TX Frame count
    // !\return TX Frame count
    size_t GetTxFrameCount() { return tx_frame_cnt; }

    // !\brief Return RX Frame count
    // !\return RX Frame count
    size_t GetRxFrameCount() { return rx_frame_cnt; }

    // !\brief Get error count
    // !\return Error count
    size_t GetErrFrameCount() { return err_frame_cnt; }

    // !\brief Slave ID
    uint8_t m_slaveId;

    // !\brief Coils
    ModbusItemType m_coils;

    // !\brief Input statuses
    ModbusItemType m_inputStatus;

    // !\brief Holding registers
    ModbusItemType m_Holding;

    // !\brief Input registers
    ModbusItemType m_Input;

    // !\brief Metadata regarding modbus registers
    NumModbusEntries m_numEntries;

    // !\brief Log entries for sent and received data
    std::vector<std::unique_ptr<ModbusLogEntry>> m_LogEntries;

    // !\brief Log entries for eventlog received over modbus
    std::vector<std::unique_ptr<EventLogEntry>> m_EventLogEntries;

    // !\brief Sending every can frame automatically at startup which period is not null? 
    bool auto_send = false;

    // !\brief Start recording automatically?
    bool auto_recording = false;

    // !\brief Is recording on?
    bool is_recoding = false;

    // !\brief Max recorded entries
    size_t max_recorded_entries = 30000;

    // !\brief Get Start time
    std::chrono::steady_clock::time_point GetStartTime() { return start_time; }

    // !\brief Start time
    std::chrono::steady_clock::time_point start_time;

    // !\brief Lock
    std::mutex m;

private:
    // !\brief Handle bool reading (coils, input statuses)
    void HandleBoolReading(std::vector<uint8_t>& reg, ModbusItemType& items, size_t num_items, ModbusItemPanel* panel);

    // !\brief Handle register reading (input & holding registers)
    void HandleRegisterReading(std::vector<uint16_t>& reg, ModbusItemType& items, size_t num_items, ModbusItemPanel* panel);

    // !\brief Suspends main thread execution until paused
    void WaitIfPaused();

    // !\brief Handle modbus register polling
    void HandlePolling();

    // !\brief Handle register writing
    void HandleWrites();

    // !\brief Worker thread itself
    void ModbusWorker(std::stop_token token);
    
    // !\brief Reference to modbus entry loader
    IModbusEntryLoader& m_ModbusEntryLoader;

    // !\brief Is enabled?
    bool is_enabled{ true };

    // !\brief Polling rate [ms]
    uint16_t m_pollingRate{ 500 };

    // !\brief Default configuration name
    std::string m_DefaultConfigName{" Modbus.xml "};

    // !\brief TX Frame count
    size_t tx_frame_cnt = 0;

    // !\brief RX Frame count
    size_t rx_frame_cnt = 0;

    // !\brief Error count
    size_t err_frame_cnt = 0;

    // !\brief Favourite level
    // !\details CAN entries (both TX & RX) which level is below this level, won't show on TX and RX list
    uint8_t m_DefaultFavouriteLevel = 0;

    // !\brief Pending coil writes
    std::vector<std::pair<int, bool>> m_pendingCoilWrites;

    // !\brief Pending holding register writes
    std::vector<std::pair<int, uint64_t>> m_pendingHoldingWrites;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_workerModbus;

    // !\brief Conditional variable for worker thread
    std::condition_variable_any cv;
    
    // !\brief Is main thread paused?
    bool m_isMainThreadPaused = false;

    // !\brief Is socket/serial open in progress?
    bool m_isOpenInProgress = false;

    // !\brief Is socket/serial close in progress?
    bool m_isCloseInProgress = false;

    // !\brief Serial/Socket handler
    std::unique_ptr<ModbusMasterSerialPort> m_Serial;

    // !\brief Modbus helper for GUI communication
    IModbusHelper* m_helper = nullptr;
};