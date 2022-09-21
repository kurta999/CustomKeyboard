#pragma once

#include <tuple>

class Session;
using SharedSession = std::shared_ptr<Session>;
using TcpMessageReturn = std::tuple<bool, bool, std::string>;

class ITcpMessageExecutor
{
public:
    virtual void SetCurrentSession(SharedSession session, size_t len) = 0;
    virtual TcpMessageReturn Process(std::any param) = 0;
};

class TcpMessageExecutor : public ITcpMessageExecutor
{
public:
    TcpMessageExecutor();
    virtual ~TcpMessageExecutor();

    virtual void SetCurrentSession(SharedSession session, size_t len) override;
    virtual TcpMessageReturn Process(std::any param) override;

private:
    TcpMessageReturn HandleOpenExplorer(std::any param);
    TcpMessageReturn HandleGraphs(std::any param);
    TcpMessageReturn HandleSensorsMeasurements(std::any param);

    char* m_recv_data;
    size_t m_len;
    SharedSession m_session;

    std::map<std::string, std::function<TcpMessageReturn(std::any)>> m_cmds;
};

class TcpMessageInjector
{
public:
    TcpMessageInjector(ITcpMessageExecutor& executor, SharedSession session, size_t len) :
        m_injector(executor)
    {
        m_injector.SetCurrentSession(session, len);
        m_result = m_injector.Process(std::placeholders::_1);
        m_injector.SetCurrentSession(nullptr, NULL);
    }

    TcpMessageReturn& GetResult()
    {
        return m_result;
    }

private:
    TcpMessageReturn m_result;
    ITcpMessageExecutor& m_injector;
};