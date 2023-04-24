#pragma once

#include <boost/asio.hpp>

#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "TcpMessageExecutor.hpp"

constexpr size_t SESSION_RECV_BUF_LEN = 1024;

class Session : public std::enable_shared_from_this<Session>
{
	friend class Server;
public:
	Session(boost::asio::io_service& io_service, std::mutex& io_mutex, std::unique_ptr<ITcpMessageExecutor>&& executor);

	~Session();

	// !\brief Send async message
	// !\param buffer [in] String buffer to send
	void SendAsync(const std::string& buffer);

	// !\brief Start async operations
	void StartAsync();

	// !\brief Stop async operations
	// !\param remove_from_session_list [in] Remove this session from session list?
	void StopAsync(bool remove_from_session_list = true);

	// !\brief Read handler for async_read_some
	// !\param error [in] Boost error code
	// !\param transferredBytes [in] Number of bytes received
	void HandleRead(const boost::system::error_code& error, std::size_t transferredBytes);

	// !\brief Handler for ASIO Deadline Timer
	// !\param error [in] Boost error code
	void HandleTransferTimer(const boost::system::error_code& error);

	// !\brief Write handler for async_write
	// !\param error [in] Boost error code
	void HandleWrite(const boost::system::error_code& error);

	// !\brief Pending messages waiting to be sent
	std::queue<std::string> pendingMessages;

	// !\brief Buffer for received data
	char receivedData[SESSION_RECV_BUF_LEN] = {};

	// !\brief Last sent data
	std::string sentData;

	// !\brief IP Address of session
	std::string sessionAddress;

	// !\brief Port of session
	unsigned short sessionPort = 0;

	// !\brief ASIO TCP socket of session
	boost::asio::ip::tcp::socket sessionSocket;

	// !\brief Mutex for IO operations
	std::mutex& m_IoMutex;

	// !\brief Is write in progress?
	bool writeInProgress = false;

	// !\brief Is session close pending?
	bool is_close_pending = false;

	// !\brief ASIO Deadline Timer for sending message chunks
	boost::asio::deadline_timer transferTimer;
	
	// !\brief Pointer to TCP message executor
	std::unique_ptr<ITcpMessageExecutor> m_msgExecutor;
};