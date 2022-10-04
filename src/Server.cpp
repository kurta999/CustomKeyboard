#include "pch.hpp"

extern std::mutex mutex;

Server::Server()
{

}

Server::~Server()
{
    StopAsync();
    if(m_worker)
    {
        if(m_worker->joinable())
            m_worker->join();
        m_worker.reset(nullptr);
    }
}

void Server::Init(void)
{
    if(is_enabled)
    {
        std::scoped_lock lock(mutex);
        if(!CreateAcceptor(tcp_port))
        {
            LOG(LogLevel::Error, "createAcceptor fail!");
            return;
        }
        m_worker = std::make_unique <std::thread>(&Server::StartAsync, this);
        DatabaseLogic::Get()->GenerateGraphs();
    }
}

void Server::StartAsync()
{
    unsigned short port = acceptor->local_endpoint().port();
    io_service.reset();
    boost::system::error_code error;
    io_service.run(error);
    LOG(LogLevel::Verbose, "tcp backend io_service finish");
}

void Server::BroadcastMessage(const std::string& msg)
{
    if(is_enabled)
    {
        std::scoped_lock lock(mutex);
        for(auto& i : sessions)
        {
            i->SendAsync(msg);
        }
    }
}

bool Server::CreateAcceptor(unsigned short port)
{
    boost::system::error_code error;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
    acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(io_service);
    acceptor->open(endpoint.protocol(), error);
    if(error)
    {
        LOG(LogLevel::Error, "open error {} - {}", port, error.message());
        acceptor.reset();
        return false;
    }

    acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), error);
    if(error)
    {
        LOG(LogLevel::Error, "reuse_address error - {}", error.message());
        acceptor.reset();
        return false;
    }
    acceptor->bind(endpoint, error);
    if(error)
    {
        LOG(LogLevel::Error, "bind error - {}", error.message());
        acceptor.reset();
        return false;
    }
    acceptor->listen(boost::asio::socket_base::max_connections, error);
    if(error)
    {
        LOG(LogLevel::Error, "listen error - {}", error.message());
        acceptor.reset();
        return false;
    }
    StartAccept();
    return true;
}

void Server::StopAsync()
{
    if(is_enabled)
    {
        std::scoped_lock lock(mutex);
        if(acceptor)
        {
            acceptor->close();
            acceptor.reset();

            for(const auto& c : sessions)
            {
                c->StopAsync(false);
            }
            sessions.clear();
        }
        io_service.stop();
    }
}

void Server::StartAccept()
{
    SharedSession session = std::make_shared<Session>(io_service, std::make_unique<TcpMessageExecutor>());
    acceptor->async_accept(session->sessionSocket, std::bind(&Server::HandleAccept, this, std::placeholders::_1, session));
}

void Server::HandleAccept(const boost::system::error_code& error, SharedSession session)
{
    std::scoped_lock lock(mutex);
    if(acceptor)
    {
        if(!error)
        {
            session->StartAsync();
            StartAccept();
        }
    }
}
