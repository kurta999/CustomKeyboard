#pragma once

#include "utils/CSingleton.hpp"
#include <string>

class SerialForwarder : public CSingleton < SerialForwarder >
{
    friend class CSingleton < SerialForwarder >;

public:
    SerialForwarder();
    ~SerialForwarder();

    void Init() { printf("fasz"); }
    // !\brief Replace path separators to opposite ones in clipboard 
    void Send(std::string& ip, uint16_t port, const char* data, size_t len);


private:
    // !\brief Replace path separators to opposite ones in given string
    // !\param str [in] Reference to string where separators will be replaced
    void ReplaceString(std::string& str);

    boost::asio::io_context io_context;
    std::unique_ptr<std::thread> m_worker = nullptr;
};