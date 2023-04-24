#pragma once

#include <memory>
#include <tuple>
#include <string>
#include <any>

class Session;
using SharedSession = std::shared_ptr<Session>;
using TcpMessageReturn = std::tuple<bool, bool, std::string>;

class ITcpMessageExecutor
{
public:
    virtual ~ITcpMessageExecutor() {}

    virtual void SetCurrentSession(SharedSession session, size_t len) = 0;
    virtual TcpMessageReturn Process(std::any param) = 0;
};