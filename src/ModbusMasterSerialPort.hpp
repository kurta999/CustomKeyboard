#include "utils/CSingleton.hpp"
#include <atomic>
#include <condition_variable>
#include <string>

class ModbusMasterSerialPort : public SerialPortBase, public CSingleton < ModbusMasterSerialPort >
{
    friend class CSingleton < ModbusMasterSerialPort >;

public:
    ModbusMasterSerialPort();
    ~ModbusMasterSerialPort();

    void Init();
    void OnUartDataReceived(const char* data, unsigned int len);
    void OnDataSent(CallbackAsyncSerial& serial_port);

    std::vector<uint8_t> ReadCoilStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::vector<uint8_t> ForceSingleCoil(uint8_t slave_id, uint16_t write_offset, bool status);
    std::vector<uint8_t> ReadInputStatus(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::vector<uint16_t> ReadHoldingRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);
    std::vector<uint8_t> WriteHoldingRegister(uint8_t slave_id, uint16_t write_offset, uint16_t write_count, std::vector<uint16_t> buffer);
    std::vector<uint16_t> ReadInputRegister(uint8_t slave_id, uint16_t read_offset, uint16_t read_count);

private:
    std::chrono::steady_clock::time_point last_tx_time;

    // !\brief Mutex for entry handler
    std::mutex m_RecvMutex;

    std::condition_variable_any m_RecvCv;

    std::vector<uint8_t> m_RecvData;

    std::vector<uint8_t> m_SentData;

    bool m_LastDataCrcOk = true;
    // !\brief CAN Tx Queue
    //std::deque<std::shared_ptr<CanData>> m_TxQueue;

    // !\brief CAN Rx Queue
    //std::deque<std::shared_ptr<CanData>> m_RxQueue;

    enum ResponseStatus : uint8_t
    {
        Ok,
        Timeout,
        CrcError
    };

    ResponseStatus NotifyAndWaitForResponse();
    ResponseStatus WaitForResponse();
};