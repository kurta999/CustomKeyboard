#include "pch.hpp"

#ifndef _WIN32
using boost::asio::ip::tcp;

class session
    : public std::enable_shared_from_this<session>
{
public:
    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        socket_.async_read_some(boost::asio::buffer(recv_buffer, sizeof(recv_buffer)),
            std::bind(&session::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    session(boost::asio::io_context& io_context)
        : socket_(io_context)
    {
    }
    
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred)
    {
        socket_.async_read_some(boost::asio::buffer(recv_buffer, sizeof(recv_buffer)),
            std::bind(&session::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }    
    
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if(socket_.is_open())
        {
            boost::system::error_code error;
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
            socket_.close(error);

            SerialPort::Get()->SimulateDataReception(recv_buffer, bytes_transferred);
        }
    }
    
private:
    tcp::socket socket_;
    std::string message_;
    char recv_buffer[256] = {};
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context)
        : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(boost::asio::ip::address::from_string(SerialForwarder::Get()->bind_ip), SerialForwarder::Get()->tcp_port), true)
    {
        start_accept();
    }

private:
    void handle_accept(const boost::system::error_code& error, std::shared_ptr<session> new_connection)
    {
        if(!error)
        {
            new_connection->start();
        }
        m_sessions.emplace(new_connection);
        start_accept();
    }

    void start_accept()
    {
        std::shared_ptr<session> new_connection = std::make_shared<session>(io_context_);
        acceptor_.async_accept(new_connection->socket(), std::bind(&tcp_server::handle_accept, this, std::placeholders::_1, new_connection));
    }

    std::set<std::shared_ptr<session>> m_sessions;
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};
#else
boost::asio::awaitable<void> SerialForwarder::echo(tcp_socket socket)
{
    try
    {
        char data[1024];
        for(;;)
        {
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data));
            co_await boost::asio::async_write(socket, boost::asio::buffer(data, n));
        }
    }
    catch(std::exception& e)
    {
        LOG(LogLevel::Error, "Exception: {}", e.what());
    }
}

boost::asio::awaitable<void> SerialForwarder::listener()
{
    auto executor = co_await boost::asio::this_coro::executor;
    tcp_acceptor acceptor(executor, { tcp::endpoint(boost::asio::ip::address::from_string(SerialForwarder::Get()->bind_ip), SerialForwarder::Get()->tcp_port) });
    for(;;)
    {
        auto socket = co_await acceptor.async_accept();
        boost::asio::co_spawn(executor, echo(std::move(socket)), boost::asio::detached);
    }
}
#endif

SerialForwarder::SerialForwarder()
{
    
}

void SerialForwarder::Init()
{
    if(is_enabled)
    {
        m_worker = std::make_unique<std::jthread>([this] {
#ifdef _WIN32
            try
            {
                boost::asio::co_spawn(io_context, listener(), boost::asio::detached);
                io_context.run();
                LOG(LogLevel::Notification, "iocontext finish");
            }
            catch(std::exception& e)
            {
                LOG(LogLevel::Error, "Boost exception: {}", e.what());
            }
            catch(...)
            {
                LOG(LogLevel::Error, "Unknown exception");
            }
#else
            try
            {
                tcp_server server(io_context);
                io_context.run();
                LOG(LogLevel::Notification, "iocontext finish");
            }
            catch(const boost::system::system_error& e)
            {
                LOG(LogLevel::Error, "Boost exception: {}", e.what());
            }
            catch(...)
            {
                LOG(LogLevel::Error, "Unknown exception");
            }
#endif
        });
        if(m_worker)
            utils::SetThreadName(*m_worker, "SerialForwarder");
    }
}

SerialForwarder::~SerialForwarder()
{
    io_context.stop();
    if(m_worker)
        m_worker.reset(nullptr);
}

void SerialForwarder::Send(std::string& ip, uint16_t port, const char* data, size_t len)
{
    boost::system::error_code ec;
    boost::asio::io_service ios;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip, ec), port);
    boost::asio::ip::tcp::socket socket(ios);
    if(ec)
        LOG(LogLevel::Error, "Failed to create endpoint from ip address: {}", ec.message());

    socket.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{ 200 }, ec);
    if(ec)
        LOG(LogLevel::Error, "set_option error: {}", ec.message());

    bool is_connected = false;
    socket.async_connect(endpoint, [&is_connected](const boost::system::error_code& ec)
        {
            if(!ec)
                is_connected = true;
        });
    ios.run_for(std::chrono::duration<int, std::milli>(50));

    if(is_connected)
    {
        socket.send(boost::asio::buffer(data, len), 0, ec);
        if(ec)
            LOG(LogLevel::Error, "Failed to forward serial over TCP: {}", ec.message());
        socket.close(ec);
    }
    else
    {
        LOG(LogLevel::Error, "Failed to connect to the remote server!");
    }
}