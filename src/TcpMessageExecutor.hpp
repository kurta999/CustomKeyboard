#pragma once

#include <tuple>

class ITcpMessageExecutor
{
public:
    //ITcpMessageExecutor(const char* ip, const char* recv_data, size_t len);

    virtual std::tuple<bool, bool> Process() = 0;
};

class TcpMessageExecutor : public ITcpMessageExecutor
{
public:
    TcpMessageExecutor(const char* ip, char* recv_data, size_t len);

    virtual std::tuple<bool, bool> Process() override;

private:
    void OpenExplorer();
    void ProcessSensors();

    char* m_recv_data;
    const char* m_ip;
    size_t m_len;
};

