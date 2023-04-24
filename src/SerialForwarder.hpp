#pragma once

#include "utils/CSingleton.hpp"
#include <string>

#ifdef _WIN32
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
using boost::asio::ip::tcp;
using boost::asio::use_awaitable_t;
using tcp_acceptor = use_awaitable_t<>::as_default_on_t<tcp::acceptor>;
using tcp_socket = use_awaitable_t<>::as_default_on_t<tcp::socket>;
namespace this_coro = boost::asio::this_coro;
#endif

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
#ifdef _WIN32
    boost::asio::awaitable<void> echo(tcp_socket socket);
    
    boost::asio::awaitable<void> listener();
#endif
    // !\brief ASIO IO Context
    boost::asio::io_context io_context;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_worker = nullptr;

};