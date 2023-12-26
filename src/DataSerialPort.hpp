#pragma once

#include "utils/CSingleton.hpp"
#include <atomic>
#include <condition_variable>
#include <string>
#include <queue>
#include <inttypes.h>
#include <semaphore>
#include <boost/circular_buffer.hpp>

#include "SerialPortBase.hpp"

constexpr size_t MAX_DATA_LEN = 1024;

#pragma pack(push, 1)
class RawData
{
public:
    RawData(size_t data_len_, uint8_t* data_)
        : data_len(data_len_)
    {
        memset(data, 0, sizeof(data));
        if(data_len > sizeof(data))
            data_len = sizeof(data);

        if(data_)
            memcpy(data, data_, data_len_);
    }
    size_t data_len;
    uint8_t data[MAX_DATA_LEN];
};
#pragma pack(pop)

/* TODO: create asbtraction for this & SerialPort because it's the same - but no time currently */
class CallbackAsyncSerial;
class DataSerialPort : public SerialPortBase, public CSingleton < DataSerialPort >
{
    friend class CSingleton < DataSerialPort >;

public:
    DataSerialPort();
    ~DataSerialPort();

    // !\brief Initialize DataSerialPort
    void Init();

    // !\brief Add CAN frame to TX queue
    void AddToTxQueue(size_t data_len, uint8_t* data);

    // !\brief Add CAN frame to RX queue
    void AddToRxQueue(size_t data_len, uint8_t* data);

    // !\brief Send pending CAN Frames from the internal buffer
    void SendPendingDataFrames(CallbackAsyncSerial& serial_port);

    void ProcessReceivedFrames();

    void ClearBuffers();

private:
    // !\brief Called when data was received via serial port (called by boost::asio::read_some)
    // !\param serial_port [in] Pointer to received data
    // !\param len [in] Received data length
    void OnDataReceived(const char* data, unsigned int len);

    // !\brief On data sent
    void OnDataSent(CallbackAsyncSerial& serial_port);

    // !\brief Mutex for received data processing
    std::mutex m_RxMutex;

    // !\brief Circular buffer for received data
    std::vector<uint8_t> m_CircBuff;

    // !\brief CAN Tx Queue
    std::queue<std::shared_ptr<RawData>> m_TxQueue;
};