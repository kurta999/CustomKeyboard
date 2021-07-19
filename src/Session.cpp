#include "pch.h"

std::mutex mutex;

Session::Session(boost::asio::io_service& io_service) : sessionSocket(io_service)
{

}

void Session::HandleRead(const boost::system::error_code& error, std::size_t bytesTransferred)
{
	std::scoped_lock lock(mutex);
	if(!error)
	{
		receivedData[bytesTransferred] = 0;
		Sensors::Get()->ProcessIncommingData(receivedData, sessionSocket.remote_endpoint().address().to_string().c_str());
		StopAsync();
	}
}

void Session::HandleTransferTimer(const boost::system::error_code& error)
{
	std::scoped_lock lock(mutex);
	if(!error)
	{
		if(!pendingMessages.empty())
		{
			SendAsync(pendingMessages.front());
			pendingMessages.pop();
		}
	}
}

void Session::HandleWrite(const boost::system::error_code& error)
{
	writeInProgress = false;
	if(!error)
	{
		if(!pendingMessages.empty())
		{
			SendAsync(pendingMessages.front());
			pendingMessages.pop();
		}
	}
	else
	{
		pendingMessages = std::queue<std::string>();
	}
}

void Session::SendAsync(const std::string& buffer)
{
	if(writeInProgress)
	{
		pendingMessages.push(buffer);
	}
	else
	{
		sentData = buffer;
		writeInProgress = true;
		boost::asio::async_write(sessionSocket, boost::asio::buffer(sentData, sentData.length()), 
			boost::bind(&Session::HandleWrite, shared_from_this(), boost::asio::placeholders::error));
	}
}

void Session::StartAsync()
{
	boost::system::error_code error;
	boost::asio::ip::tcp::endpoint remoteEndpoint = sessionSocket.remote_endpoint(error);
	if(error)
	{
		StopAsync();
		return;
	}
	sessionAddress = remoteEndpoint.address().to_string(); 
	sessionPort = remoteEndpoint.port();
	
	for(const auto &c : Server::Get()->sessions)
	{
		if(boost::algorithm::equals(c->sessionAddress, sessionAddress))
		{
			Server::Get()->sessions.erase(c);
			return;
		}
	}
	
	sessionSocket.async_read_some(boost::asio::buffer(receivedData, sizeof(receivedData)), 
		boost::bind(&Session::HandleRead, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	Server::Get()->sessions.insert(shared_from_this());
}

void Session::StopAsync()
{
	if(sessionSocket.is_open())
	{
		boost::system::error_code error;
		sessionSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
		sessionSocket.close(error);
	}
}