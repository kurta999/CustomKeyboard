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
    void SetRemoteTcpIp(std::string & ip);
    std::string& GetRemoteTcpIp();
    void SetRemoteTcpPort(uint16_t remote_port);
    uint16_t GetRemoteTcpPort();

private:
    bool is_enabled = true;
    uint16_t com_port = 5;
    bool forward_serial_to_tcp = false;
    std::string remote_tcp_ip;
    uint16_t remote_tcp_port;
    std::unique_ptr<std::thread> m_worker = nullptr;
    std::atomic<bool> to_exit = false;
    std::condition_variable m_cv;
    std::mutex m_mutex;
};