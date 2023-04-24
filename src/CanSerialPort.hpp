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
class CanSerialPort : public CSingleton < CanSerialPort >
{
    friend class CSingleton < CanSerialPort >;

public:
    CanSerialPort();
    ~CanSerialPort();

    // !\brief Initialize CanSerialPort
    void Init();

    void SetDeviceType(CanDeviceType device_type) { m_DeviceType = device_type; }
    CanDeviceType GetDeviceType() { return m_DeviceType; }

    void SetDevice(std::unique_ptr<ICanDevice>&& device);

    // !\brief Set this module enabled
    void SetEnabled(bool enable);

    // !\brief Is this module enabled?
    bool IsEnabled();

    // !\brief Set serial port
    void SetComPort(uint16_t port);

    // !\brief Get serial port
    uint16_t GetComPort();

    // !\brief Add CAN frame to TX queue
    void AddToTxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data);

    // !\brief Add CAN frame to RX queue
    void AddToRxQueue(uint32_t frame_id, uint8_t data_len, uint8_t* data);

    void SendPendingCanFrames(CallbackAsyncSerial& serial_port);

private:
    // !\brief Stops worker thread
    void DestroyWorkerThread();

    // !\brief Worker thread
    void WorkerThread(std::stop_token token);

    void NotifiyMainThread();

    // !\brief Called when data was received via serial port (called by boost::asio::read_some)
    // !\param serial_port [in] Pointer to received data
    // !\param len [in] Received data length
    void OnDataReceived(const char* data, unsigned int len);

    // !\brief Is serial port data receiving enabled?
    bool is_enabled = true;

    // !\brief COM port number
    uint16_t com_port = 5;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_worker;

    std::atomic<bool> is_notification_pending = false;

    // !\brief Conditional variable for main thread exiting
    std::condition_variable_any m_cv;

    // !\brief Mutex for main thread
    std::mutex m_mutex;

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