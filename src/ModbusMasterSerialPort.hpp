#include "utils/CSingleton.hpp"
#include <atomic>
#include <condition_variable>
#include <string>
/*
#pragma pack(push, 1)
class CanData
{
public:
    CanData(uint32_t frame_id_, uint8_t data_len, uint8_t* data_)
        : frame_id(frame_id_), data_len(data_len)
    {
        memset(data, 0, sizeof(data));
        if(data_len > sizeof(data))
            data_len = sizeof(data);

        if(data_)
            memcpy(data, data_, data_len);
    }
    uint32_t frame_id;
    uint8_t data_len;
    uint8_t data[8];
};
#pragma pack(pop)
*/
class ModbusMasterSerialPort : public CSingleton < ModbusMasterSerialPort >
{
    friend class CSingleton < ModbusMasterSerialPort >;

public:
    ModbusMasterSerialPort() = default;
    ~ModbusMasterSerialPort();

    void Init();
    void OnUartDataReceived(const char* data, unsigned int len);
    void UartReceiveThread(std::stop_token stop_token, std::condition_variable_any& cv, std::mutex& m);

    void SetEnabled(bool enable);
    bool IsEnabled();
    void SetComPort(uint16_t port);
    uint16_t GetComPort();

    void AddToTxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data);
    void AddToRxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data);
private:

    // !\brief Is serial port data receiving enabled?
    bool is_enabled = true;

    // !\brief COM port number
    uint16_t com_port = 5;

    // !\brief Forward received data from COM to a remote TCP server?
    bool forward_serial_to_tcp = false;

    // !\brief Remote TCP Server IP
    std::string remote_tcp_ip;

    // !\brief Remote TCP Server port
    uint16_t remote_tcp_port;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_worker = nullptr;

    // !\brief Conditional variable for main thread exiting
    std::condition_variable_any m_cv;

    // !\brief Mutex
    std::mutex m_mutex;

    std::chrono::steady_clock::time_point last_tx_time;

    bool waiting_for_response;

    // !\brief CAN Tx Queue
    //std::deque<std::shared_ptr<CanData>> m_TxQueue;

    // !\brief CAN Rx Queue
    //std::deque<std::shared_ptr<CanData>> m_RxQueue;
};