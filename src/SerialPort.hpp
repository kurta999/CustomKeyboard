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

    void Init();
    void DestroyWorkingThread();
    void OnUartDataReceived(const char* data, unsigned int len);
    void UartReceiveThread(std::atomic<bool>& to_exit, std::condition_variable& cv, std::mutex& m);

    void SetEnabled(bool enable);
    bool IsEnabled();
    void SetComPort(uint16_t port);
    uint16_t GetComPort();
    void SetForwardToTcp(bool enable);
    bool IsForwardToTcp();
    void SetRemoteTcpIp(const std::string & ip);
    std::string& GetRemoteTcpIp();
    void SetRemoteTcpPort(uint16_t remote_port);
    uint16_t GetRemoteTcpPort();

private:
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
    uint16_t remote_tcp_port;

    // !\brief Worker thread
    std::unique_ptr<std::thread> m_worker = nullptr;

    // !\brief Exit working thread?
    std::atomic<bool> to_exit = false;

    // !\brief Conditional variable for main thread exiting
    std::condition_variable m_cv;

    // !\brief Mutex
    std::mutex m_mutex;
};