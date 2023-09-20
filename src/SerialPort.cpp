#include "pch.hpp"

SerialPort::~SerialPort()
{

}

void SerialPort::Init()
{
    auto recv_f = std::bind(&SerialPort::OnDataReceived, this, std::placeholders::_1, std::placeholders::_2);
    InitInternal("SerialPort", recv_f, nullptr);
}

void SerialPort::SetForwardToTcp(bool enable)
{
    forward_serial_to_tcp = enable;
}

bool SerialPort::IsForwardToTcp()
{
    return forward_serial_to_tcp;
}

void SerialPort::SetRemoteTcpIp(const std::string& ip)
{
    remote_tcp_ip = ip;
}

std::string& SerialPort::GetRemoteTcpIp()
{
    return remote_tcp_ip;
}

void SerialPort::SetRemoteTcpPort(uint16_t remote_port)
{
    remote_tcp_port = remote_port;
}

uint16_t SerialPort::GetRemoteTcpPort()
{
    return remote_tcp_port;
}

void SerialPort::SimulateDataReception(const char* data, unsigned int len)
{
    OnDataReceived(data, len);
}

void SerialPort::OnDataReceived(const char* data, unsigned int len)
{
    if(forward_serial_to_tcp)
    {
        if(IsUsingVmOrWsl())
            utils::SendTcpBlocking(remote_tcp_ip, remote_tcp_port, data, len);
        else
            CustomMacro::Get()->ProcessReceivedData(data, len);
    }
    else
    {
        CustomMacro::Get()->ProcessReceivedData(data, len);
    }
}

// \x00\x00\x00\x00\x00\x00\x00\x00\x00\x54\x00\x00\x00\x00\x00\x4C\x45
// \h(00 00 00 00 00 00 00 00 00 54 00 00 00 00 00 4C 45)

bool SerialPort::IsUsingVmOrWsl()
{
#ifdef _WIN32
    bool ret = false;
    char window_title[256];
    HWND foreground = GetForegroundWindow();
    GetWindowTextA(foreground, window_title, sizeof(window_title));
    if(boost::algorithm::contains(window_title, "Oracle VM VirtualBox") || boost::algorithm::contains(window_title, "(Ubuntu)"))
        ret = true;
    return ret;
#else
    return false;
#endif
}