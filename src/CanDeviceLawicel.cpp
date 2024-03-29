#include "pch.hpp"

constexpr size_t CAN_SERIAL_RESPONSE_BUFFER_SIZE = 64;

constexpr const char MESSAGE_TRANSMIT_STANDARD_FRAME = 't';
constexpr const char MESSAGE_TRANSMIT_EXTENDED_FRAME = 'T';
constexpr const char MESSAGE_TRANSMIT_VERSION_INFO = 'V';

CanDeviceLawicel::CanDeviceLawicel(boost::circular_buffer<char>& CircBuff) :
    m_CircBuff(CircBuff)
{

}

CanDeviceLawicel::~CanDeviceLawicel()
{

}

void CanDeviceLawicel::ProcessReceivedFrames(std::mutex& rx_mutex)
{
    while(m_CircBuff.size() > 0)
    {
        uint8_t data_type = 0;
        boost::circular_buffer<char>::iterator it_start;
        boost::circular_buffer<char>::iterator it_end;
        for(boost::circular_buffer<char>::iterator i = m_CircBuff.begin(); i != m_CircBuff.end(); ++i)
        {
            char start_char = *i;
            if(start_char == MESSAGE_TRANSMIT_STANDARD_FRAME && it_start == m_CircBuff.end())
            {
                it_start = i;
                data_type = MESSAGE_TRANSMIT_STANDARD_FRAME;
            }            
            else if(start_char == MESSAGE_TRANSMIT_EXTENDED_FRAME && it_start == m_CircBuff.end())
            {
                it_start = i;
                data_type = MESSAGE_TRANSMIT_EXTENDED_FRAME;
            } 
            else if(start_char == MESSAGE_TRANSMIT_VERSION_INFO && it_start == m_CircBuff.end())
            {
                it_start = i;
                data_type = MESSAGE_TRANSMIT_VERSION_INFO;
            }

            if(start_char == '\r' && it_start != m_CircBuff.end())
            {
                it_end = ++i;
                break;
            }
        }

        if(it_start != m_CircBuff.end() && it_end != m_CircBuff.end())
        {
            char data[CAN_SERIAL_RESPONSE_BUFFER_SIZE];
            size_t data_len = it_end - it_start;
            if(data_len >= sizeof(data))
            {
                std::string hex;
                utils::ConvertHexBufferToString((const char*)data, data_len, hex);
                LOG(LogLevel::Warning, "Invalid CAN data received, {} is more than {}! Erasing circular buffer: {}", data_len, sizeof(data), hex);
                m_CircBuff.erase(m_CircBuff.begin(), it_end);
                return;
            }

            std::copy(it_start, it_end, data);
            m_CircBuff.erase(m_CircBuff.begin(), it_end);

            data[data_len] = 0;
            if(data_type == MESSAGE_TRANSMIT_VERSION_INFO)
            {
                LOG(LogLevel::Notification, "LAWICEL CANUSB version: {}", data);
                return;
            }

            char frame_id_str[9] = {};
            char frame_length = 0;
            uint8_t ret = 0;
            char response[64] = {};
            uint8_t newline = 0;

            if(data_type == MESSAGE_TRANSMIT_STANDARD_FRAME)  // (00:58 : 20.531) t3F4 7 80 83 00 00 00 00 00 66 90
                ret = sscanf(data, "%*c%03s%c%64[^\r]%c", frame_id_str, &frame_length, response, &newline);
            else
                ret = sscanf(data, "%*c%08s%c%64[^\r]%c", frame_id_str, &frame_length, response, &newline);
            if(ret == 4 && newline == '\r')
            {
                frame_length = frame_length - '0';

                std::string out;
                boost::algorithm::unhex(response, response + (frame_length * 2), std::back_inserter(out));
                uint32_t frame_id = std::stoi(frame_id_str, 0, 16);
                CanSerialPort::Get()->AddToRxQueue(frame_id, frame_length, (uint8_t*)out.c_str());
            }
            else
            {
                m_CircBuff.erase(m_CircBuff.begin(), m_CircBuff.end());
                LOG(LogLevel::Warning, "Invalid CAN data received! Erasing circular buffer");
            }
        }
        else  /* If no valid data was received, wait for the next iteration */
        {
            break;
        }
    }
}

size_t CanDeviceLawicel::PrepareSendDataFormat(const std::shared_ptr<CanData>& data_ptr, char* out, size_t max_size, bool& remove_from_queue)
{
    size_t send_size = 0;
    switch(device_state)
    {
        case 0:  /* Initial CR */
        {
            send_size = 1;
            memcpy(out, "\r", send_size);
            device_state++;
            std::this_thread::sleep_for(150ms);
            break;
        }
        case 1:  /* Get version */
        {
            send_size = 2;
            memcpy(out, "V\r", send_size);  
            device_state++;
            std::this_thread::sleep_for(150ms);
            break;
        }
        case 2:  /* CAN Baudrate 500Kbps */
        {
            send_size = 3;
            memcpy(out, "S6\r", send_size);
            device_state++;
            std::this_thread::sleep_for(50ms);
            break;
        }
        case 3:  /* Open CAN channel */
        {
            send_size = 2;
            memcpy(out, "O\r", send_size);
            device_state++;
            std::this_thread::sleep_for(200ms);
            break;
        }
        case 4:  /* Send data to CAN bus */
        {
            remove_from_queue = true;
            std::string out_hex;
            boost::algorithm::hex(data_ptr->data, data_ptr->data + data_ptr->data_len, std::back_inserter(out_hex));

            std::string out_str = std::format("{}{:X}{}{}\r", data_ptr->frame_id < 0x7FF ? 't' : 'T', data_ptr->frame_id, data_ptr->data_len, out_hex);
            send_size = out_str.length();
            memcpy(out, out_str.c_str(), out_str.length());
            break;
        }
        default:
        {
            assert(true);
            break;
        }
    }
    return send_size;
}
