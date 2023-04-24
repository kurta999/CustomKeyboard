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

    // !\brief Initialize TCP backend server for sensors
    void Init();

    // !\brief Start async operations
    void StartAsync(std::stop_token token);

    // !\brief Broadcast message to every session
    // !\param msg [in] TCP Server port
    void BroadcastMessage(const std::string& msg);
    
    // !\brief Is TCP backend server for sensors enabled?
    bool is_enabled = true;

    // !\brief TCP Server port
    uint16_t tcp_port = 2005;

private:
    
    // !\brief Create TCP acceptor
    // !\param port [in] TCP Server port
    bool CreateAcceptor(unsigned short port);

    // !\brief Stop async operations
    void StopAsync();

    // !\brief Start async accept
    void StartAccept();

    // !\brief Handle async accept
    // !\param error [in] Boost error code
    // !\param session [in] Shared pointer to current session
    void HandleAccept(const boost::system::error_code& error, SharedSession session);

    // !\brief Set of active sessions
    std::set<SharedSession> sessions;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_worker = nullptr;

    // !\brief ASIO Acceptor
    SharedAcceptor acceptor;

    // !\brief Mutex for IO operations
    std::mutex m_IoMutex;

    // !\brief IO Service
    boost::asio::io_service io_service;
};