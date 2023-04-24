#include "pch.hpp"

Session::Session(boost::asio::io_service& io_service, std::mutex& io_mutex, std::unique_ptr<ITcpMessageExecutor>&& executor) :
	sessionSocket(io_service), m_IoMutex(io_mutex), transferTimer(io_service), m_msgExecutor(std::move(executor))
{

}

Session::~Session()
{

}

void Session::HandleRead(const boost::system::error_code& error, std::size_t bytesTransferred)
{
	std::scoped_lock lock(m_IoMutex);
	if(!error)
	{
		receivedData[bytesTransferred] = 0;

		std::tuple<bool, bool, std::string> ret_val;
		{
			TcpMessageInjector msg_executor(*m_msgExecutor, shared_from_this(), bytesTransferred);
			ret_val = msg_executor.GetResult();

			if(!std::get<2>(ret_val).empty())
			{
				SendAsync(std::get<2>(ret_val));
				if(std::get<0>(ret_val))
					is_close_pending = true;
			}
			else
			{
				if(std::get<0>(ret_val))
					StopAsync();
			}
		}
	}
}

void Session::HandleTransferTimer(const boost::system::error_code& error)
{
	std::scoped_lock lock(m_IoMutex);
	if(!error)
	{
		if(!pendingMessages.empty())
		{
			SendAsync(pendingMessages.front());
			pendingMessages.pop();
		}
		else
		{
			if(is_close_pending)
			{
				StopAsync();
			}
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
		else
		{
			if(is_close_pending)
			{
				StopAsync();
			}
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
		transferTimer.expires_from_now(boost::posix_time::milliseconds(100));
		transferTimer.async_wait(std::bind(&Session::HandleTransferTimer, shared_from_this(), std::placeholders::_1));
	}
	else
	{
		sentData = buffer;
		writeInProgress = true;
		boost::asio::async_write(sessionSocket, boost::asio::buffer(sentData, sentData.length()), 
			std::bind(&Session::HandleWrite, shared_from_this(), std::placeholders::_1));
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
	
	sessionSocket.async_read_some(boost::asio::buffer(receivedData, sizeof(receivedData)), 
		std::bind(&Session::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	Server::Get()->sessions.emplace(shared_from_this());
}

void Session::StopAsync(bool remove_from_session_list)
{
	if(sessionSocket.is_open())
	{
		boost::system::error_code error;
		sessionSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
		sessionSocket.close(error);
		
		if(remove_from_session_list)
		{
			for(const auto& c : Server::Get()->sessions)
			{
				if(boost::algorithm::equals(c->sessionAddress, sessionAddress))
				{
					Server::Get()->sessions.erase(c);
					return;
				}
			}
		}
	}
}