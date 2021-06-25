#pragma once

#include "utils/CSingleton.h"

#include "Settings.h"
#include <boost/asio.hpp>

#include <inttypes.h>
#include <map>
#include <string>
#include <deque>

#include "Session.h"
#include <thread>

typedef std::shared_ptr<boost::asio::ip::tcp::acceptor> SharedAcceptor;
typedef std::shared_ptr<Session> SharedSession;

class Server : public CSingleton < Server >
{
    friend class CSingleton < Server >;
    friend class Session;

public:
    Server() = default;
    ~Server()
    {
        StopAsync();
    }

    void Init(void);
    void StartAsync();

    bool CreateAcceptor(unsigned short port);
    void BroadcastMessage(const std::string& msg);
    
private:
    friend class Settings;

    void StopAsync();
    void HandleAccept(const boost::system::error_code& error, SharedSession session);
    void StartAccept();

    uint8_t is_enabled;
    uint16_t tcp_port = 0;
    std::set<SharedSession> sessions;
    std::thread *t = nullptr;
    SharedAcceptor acceptor;
    boost::asio::io_service io_service;
};