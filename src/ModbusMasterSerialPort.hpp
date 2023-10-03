#include "utils/CSingleton.hpp"
#include <atomic>
#include <condition_variable>
#include <string>

class ModbusMasterSerialPort : public SerialPortBase
{
public:
    ModbusMasterSerialPort();
    ~ModbusMasterSerialPort();

    void SetStopToken(std::stop_token& token) { m_stopToken = &token; }

    void Init();
    void OnUartDataReceived(const char* data, unsigned int len);
    void OnDataSent(CallbackAsyncSerial& serial_port);

    std::vector<uint8_t> ReadCoilStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::vector<uint8_t> ForceSingleCoil(uint8_t slave_id, uint16_t write_offset, bool status);
    std::vector<uint8_t> ReadInputStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::vector<uint16_t> ReadHoldingRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::vector<uint8_t> WriteHoldingRegister(uint8_t slave_id, uint16_t write_offset, uint16_t write_count, std::vector<uint16_t> buffer);
    std::vector<uint16_t> ReadInputRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);

    void SetHelper(IModbusHelper* helper) { m_Helper = helper; }

private:
    enum ResponseStatus : uint8_t
    {
        Ok,
        Timeout,
        CrcError
    };

    void AddCrcToFrame(std::vector<uint8_t>& vec);
    ResponseStatus NotifyAndWaitForResponse(const std::vector<uint8_t>& vec);
    ResponseStatus WaitForResponse();

    std::chrono::steady_clock::time_point last_tx_time;

    std::stop_token* m_stopToken = nullptr;

    // !\brief Mutex for entry handler
    std::mutex m_RecvMutex;

    std::condition_variable_any m_RecvCv;

    std::vector<uint8_t> m_RecvData;

    std::vector<uint8_t> m_SentData;

    bool m_LastDataCrcOk = true;

    IModbusHelper* m_Helper = nullptr;
};