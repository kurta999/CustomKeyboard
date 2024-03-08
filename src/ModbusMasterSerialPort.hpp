#include "utils/CSingleton.hpp"
#include <atomic>
#include <condition_variable>
#include <string>
#include <expected>
#include <semaphore>

enum class ModbusError
{
    WrongId,
};

#define MODBUS_LOG_DIR_RX 0
#define MODBUS_LOG_DIR_TX 1

class ModbusMasterSerialPort : public SerialPortBase
{
public:
    ModbusMasterSerialPort();
    ~ModbusMasterSerialPort();

    void SetStopToken(std::stop_token& token) { m_stopToken = &token; }

    void Init();
    void OnUartDataReceived(const char* data, unsigned int len);
    void OnDataSent(CallbackAsyncSerial& serial_port);

    std::expected<std::vector<uint8_t>, ModbusError> ReadCoilStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::expected<std::vector<uint8_t>, ModbusError> ForceSingleCoil(uint8_t slave_id, uint16_t write_offset, bool status);
    std::expected<std::vector<uint8_t>, ModbusError> ReadInputStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::expected<std::vector<uint16_t>, ModbusError> ReadHoldingRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::expected<std::vector<uint16_t>, ModbusError> ReadHoldingRegisters(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::expected<std::vector<uint8_t>, ModbusError> WriteHoldingRegister(uint8_t slave_id, uint16_t write_offset, uint16_t write_count, std::vector<uint16_t> buffer);
    std::expected<std::vector<uint16_t>, ModbusError> ReadInputRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);

    void SetHelper(IModbusHelper* helper) { m_Helper = helper; }

    uint16_t m_ResponseTimeout = 5000;

private:

    enum ModbusFunctionCodes : uint8_t
    {
        FC_ReadCoilStatus = 1,
        FC_ReadInputStatus = 2,
        FC_ReadHoldingRegister = 3,
        FC_ReadInputRegister = 4,
        FC_ForceSingleCoil = 5,
        FC_WriteSingleRegister = 6,
        FC_WriteMultipleRegister = 16,
    };

    enum ResponseStatus : uint8_t
    {
        Ok,
        Timeout,
        ModbusError,
        CrcError
    };

    enum ModbusErrors
    {
        IllegalFunction = 129,  // 1 
        IllegalDataAddr,        // 2
        IllegalDataVal,         // 3
        SlaveFailure,
        Ack,
        SlaveBusy,
        Nack,
        MemParityError,
        GatewayUnavailable = 143,
        GatewayTargetFailed
    };

    std::map<uint8_t, size_t> modbusErrorCount;

    void SetupHeader(std::vector<uint8_t>& vec, uint8_t slave_id, uint16_t fcode, uint16_t len);
    void AddCrcToFrame(std::vector<uint8_t>& vec);
    ResponseStatus NotifyAndWaitForResponse(const std::vector<uint8_t>& vec);
    void DoCleanup(std::vector<uint8_t>& recv_data);
    bool WaitForResponse();

    std::chrono::steady_clock::time_point last_tx_time;

    std::stop_token* m_stopToken = nullptr;

    // !\brief Mutex for entry handler
    std::mutex m_RecvMutex;

    std::condition_variable_any m_RecvCv;

    std::vector<uint8_t> m_RecvData;

    std::vector<uint8_t> m_SentData;

    std::vector<uint8_t> m_LastSentData;

    bool m_LastDataCrcOk = true;

    uint16_t sequence_id = 0;

    IModbusHelper* m_Helper = nullptr;
};