#include "pch.hpp"

constexpr int LOCA_RECV_BUFFER = 1024;

boost::asio::awaitable<void> SerialTcpBackend::echo(tcp_socket socket)
{
    try
    {
        char data[LOCA_RECV_BUFFER];
        for(;;)
        {
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data));

            SerialPort::Get()->SimulateDataReception(data, n);
        }
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Error, "Exception: {}", e.what());
    }
}

boost::asio::awaitable<void> SerialTcpBackend::listener()
{
    auto executor = co_await boost::asio::this_coro::executor;
    tcp_acceptor acceptor(executor, { tcp::endpoint(boost::asio::ip::address::from_string(SerialTcpBackend::Get()->bind_ip), SerialTcpBackend::Get()->tcp_port) });
    for(;;)
    {
        auto socket = co_await acceptor.async_accept();
        boost::asio::co_spawn(executor, echo(std::move(socket)), boost::asio::detached);
    }
}

SerialTcpBackend::SerialTcpBackend()
{
    
}

void SerialTcpBackend::Init()
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
            catch(const std::exception& e)
            {
                LOG(LogLevel::Error, "std exception: {}", e.what());
            }
            catch(...)
            {
                LOG(LogLevel::Error, "Unknown exception");
            }
        });
#endif
        if(m_worker)
            utils::SetThreadName(*m_worker, "SerialForwarder");
    }
}

SerialTcpBackend::~SerialTcpBackend()
{
    io_context.stop();
    if(m_worker)
        m_worker.reset(nullptr);
}