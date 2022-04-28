#pragma once

#include "utils/CSingleton.hpp"
#include <string>

class SerialForwarder : public CSingleton < SerialForwarder >
{
    friend class CSingleton < SerialForwarder >;

public:
    SerialForwarder();
    ~SerialForwarder();

    void Init();

    // !\brief Send data to remote server - this is blocking function
    void Send(std::string& ip, uint16_t port, const char* data, size_t len);

    // !\brief Enabled?
    bool is_enabled = false;

    // !\brief Listening IP address (0.0.0.0 = any, by default)
    std::string bind_ip = "0.0.0.0";

    // !\brief Listening port (if no port-forwarding or redirecting happens, should be same as "RemoteTcpPort" config entry)
    uint16_t tcp_port = 10000;
private:
    // !\brief ASIO IO Context
    boost::asio::io_context io_context;

    // !\brief Worker thread
    std::unique_ptr<std::thread> m_worker = nullptr;

};