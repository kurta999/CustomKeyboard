#pragma once

#include "utils/CSingleton.hpp"

#include "Settings.hpp"
#include <boost/asio.hpp>

#include <inttypes.h>
#include <map>
#include <string>
#include <deque>

#include "Session.hpp"
#include <thread>

typedef std::shared_ptr<boost::asio::ip::tcp::acceptor> SharedAcceptor;
typedef std::shared_ptr<Session> SharedSession;

class Server : public CSingleton < Server >
{
    friend class CSingleton < Server >;
    friend class Session;

public:
    Server();
    ~Server();

    void Init();
    void StartAsync();

    bool CreateAcceptor(unsigned short port);
    void BroadcastMessage(const std::string& msg);
    
    bool is_enabled = true;
    uint16_t tcp_port = 2005;

private:
    void StopAsync();
    void HandleAccept(const boost::system::error_code& error, SharedSession session);
    void StartAccept();

    std::set<SharedSession> sessions;
    std::unique_ptr<std::thread> m_worker = nullptr;
    SharedAcceptor acceptor;
    boost::asio::io_service io_service;
};