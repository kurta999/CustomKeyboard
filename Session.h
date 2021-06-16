#pragma once

#include <boost/asio.hpp>

#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

class Session : public std::enable_shared_from_this<Session>
{
	friend class Server;
public:
	Session(boost::asio::io_service& io_service);

	void SendAsync(const std::string& buffer);
	void StartAsync();
	void StopAsync();

	void HandleRead(const boost::system::error_code& error, std::size_t transferredBytes);
	void HandleTransferTimer(const boost::system::error_code& error);
	void HandleWrite(const boost::system::error_code& error);

	boost::asio::deadline_timer heartbeatTimer;
	std::vector<std::string> messageTokens;
	std::queue<std::string> pendingMessages;
	char receivedData[256];
	std::string sentData;
	std::string sessionAddress;
	unsigned short sessionPort;
	boost::asio::ip::tcp::socket sessionSocket;
	bool waitingForResponse;
	bool writeInProgress = false;
};