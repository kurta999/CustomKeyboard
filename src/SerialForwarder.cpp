#include "pch.hpp"

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

        //message_ = "test string from server";
        //boost::asio::async_write(socket_, boost::asio::buffer(message_), std::bind(&session::handle_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
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
        }
    }
    
private:
    tcp::socket socket_;
    std::string message_;
    char recv_buffer[256];
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context)
        : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), 9999))  /* TODO: catch exception if port is already used or can't bind */
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
        start_accept();
    }

    void start_accept()
    {
        std::shared_ptr<session> new_connection = std::make_shared<session>(io_context_);
        acceptor_.async_accept(new_connection->socket(), std::bind(&tcp_server::handle_accept, this, std::placeholders::_1, new_connection));
    }


    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

SerialForwarder::SerialForwarder()
{
    m_worker = std::make_unique<std::thread>([this] {
            tcp_server server(io_context);
            io_context.run();
        });
}

SerialForwarder::~SerialForwarder()
{
    io_context.stop();
    if(m_worker && m_worker->joinable())
        m_worker->join();
}

void SerialForwarder::Send(std::string& ip, uint16_t port, const char* data, size_t len)
{
    boost::asio::io_service ios;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port); /* catch exception like failed connection (wrong ip or port) */
    boost::asio::ip::tcp::socket socket(ios);

    boost::system::error_code ec;
    socket.connect(endpoint, ec);
    if(ec)
        LOGMSG(error, "Failed to connec to remote TCP server: {}", ec.message());
    
    socket.send(boost::asio::buffer(data, len), 0);
    socket.close();

    if(ec)
        LOGMSG(error, "Failed to forward serial over TCP: {}", ec.message());
}