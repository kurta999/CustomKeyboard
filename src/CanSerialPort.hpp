#include "utils/CSingleton.hpp"
#include <atomic>
#include <condition_variable>
#include <string>
#include <semaphore>
#include <boost/circular_buffer.hpp>
#include <ICanDevice.hpp>

constexpr size_t MAX_CAN_FRAME_DATA_LEN = 8;

enum class CanDeviceType
{
    STM32,
    LAWICEL
};

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
    uint8_t data[MAX_CAN_FRAME_DATA_LEN];
};
#pragma pack(pop)

/* TODO: create asbtraction for this & SerialPort because it's the same - but no time currently */
class CallbackAsyncSerial;
class CanSerialPort : public SerialPortBase, public CSingleton < CanSerialPort >
{
    friend class CSingleton < CanSerialPort >;

public:
    CanSerialPort();
    ~CanSerialPort();

    // !\brief Initialize CanSerialPort
    void Init();

    // !\brief Set CAN Device Type
    void SetDeviceType(CanDeviceType device_type) { m_DeviceType = device_type; }

    // !\brief Get CAN Device Type
    CanDeviceType GetDeviceType() const { return m_DeviceType; }

    // !\brief Set internal CAN device
    void SetDevice(std::unique_ptr<ICanDevice>&& device);

    // !\brief Add CAN frame to TX queue
    void AddToTxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data);

    // !\brief Add CAN frame to RX queue
    void AddToRxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data);

    // !\brief Send pending CAN Frames from the internal buffer
    void SendPendingCanFrames(CallbackAsyncSerial& serial_port);

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
    boost::circular_buffer<char> m_CircBuff;

    // !\brief CAN Tx Queue
    std::queue<std::shared_ptr<CanData>> m_TxQueue;

    // !\brief CAN Device
    std::unique_ptr<ICanDevice> m_Device = nullptr;

    // !\brief CAN Device type
    CanDeviceType m_DeviceType = CanDeviceType::STM32;
};