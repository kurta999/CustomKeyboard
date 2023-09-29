#include "utils/CSingleton.hpp"
#include <atomic>
#include <condition_variable>
#include <string>

class SerialPort : public SerialPortBase, public CSingleton < SerialPort >
{
    friend class CSingleton < SerialPort >;

public:
    SerialPort() = default;
    ~SerialPort();

    // !\brief Initialize KeyboardSerialPort
    void Init();

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
    // !\brief Called when data was received via serial port (called by boost::asio::read_some)
    // !\param serial_port [in] Pointer to received data
    // !\param len [in] Received data length
    void OnDataReceived(const char* data, unsigned int len);

    // !\brief Is VM or WSL in focus?
    // !\return VM or WSL in focus?
    bool IsUsingVmOrWsl();

    // !\brief Forward received data from COM to a remote TCP server?
    bool forward_serial_to_tcp = false;

    // !\brief Remote TCP Server IP
    std::string remote_tcp_ip;

    // !\brief Remote TCP Server port
    uint16_t remote_tcp_port = 0;
};