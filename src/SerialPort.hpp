#include "utils/CSingleton.hpp"
#include <atomic>
#include <condition_variable>
#include <string>

class SerialPort : public CSingleton < SerialPort >
{
    friend class CSingleton < SerialPort >;

public:
    SerialPort() = default;
    ~SerialPort();

    // !\brief Initialize KeyboardSerialPort
    void Init();

    // !\brief Set this module enabled
    void SetEnabled(bool enable);

    // !\brief Is this module enabled?
    bool IsEnabled();

    // !\brief Set serial port
    void SetComPort(uint16_t port);

    // !\brief Get serial port
    uint16_t GetComPort();

    // !\brief Toggle TCP forwarding of received data
    void SetForwardToTcp(bool enable);

    // !\brief Is TCP forwarding enabled?
    bool IsForwardToTcp();

    // !\brief Set remote TCP IP
    void SetRemoteTcpIp(const std::string & ip);

    // !\brief Get remote TCP IP
    std::string& GetRemoteTcpIp();

    // !\brief Set remote TCP Port
    void SetRemoteTcpPort(uint16_t remote_port);

    // !\brief Get remote TCP Port
    uint16_t GetRemoteTcpPort();

    // !\brief Simulate data reception
    void SimulateDataReception(const char* data, unsigned int len);

private:
    // !\brief Stops worker thread
    void DestroyWorkerThread();

    // !\brief Called when data was received via serial port (called by boost::asio::read_some)
    // !\param serial_port [in] Pointer to received data
    // !\param len [in] Received data length
    void OnDataReceived(const char* data, unsigned int len);

    // !\brief Worker thread
    void WorkerThread();

    // !\brief Is VM or WSL in focus?
    // !\return VM or WSL in focus?
    bool IsUsingVmOrWsl();

    // !\brief Is serial port data receiving enabled?
    bool is_enabled = true;

    // !\brief COM port number
    uint16_t com_port = 5;

    // !\brief Forward received data from COM to a remote TCP server?
    bool forward_serial_to_tcp = false;

    // !\brief Remote TCP Server IP
    std::string remote_tcp_ip;

    // !\brief Remote TCP Server port
    uint16_t remote_tcp_port = 0;

    // !\brief Worker thread
    std::unique_ptr<std::thread> m_worker = nullptr;

    // !\brief Exit working thread?
    std::atomic<bool> to_exit = false;

    // !\brief Conditional variable for main thread exiting
    std::condition_variable m_cv;

    // !\brief Mutex
    std::mutex m_mutex;
};