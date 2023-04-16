#include "pch.hpp"

constexpr size_t CAN_SERIAL_RESPONSE_BUFFER_SIZE = 64;

CanDeviceLawicel::CanDeviceLawicel(boost::circular_buffer<char>& CircBuff) :
    m_CircBuff(CircBuff)
{

}

CanDeviceLawicel::~CanDeviceLawicel()
{

}

void CanDeviceLawicel::ProcessReceivedFrames()
{
    while(m_CircBuff.size() > 0)
    {
        uint8_t data_type = 0;
        boost::circular_buffer<char>::iterator it_start;
        boost::circular_buffer<char>::iterator it_end;
        for(boost::circular_buffer<char>::iterator i = m_CircBuff.begin(); i != m_CircBuff.end(); ++i)
        {
            char start_char = *i;
            if(start_char == 't' && it_start == m_CircBuff.end())
            {
                it_start = i;
                data_type = 't';
            }            
            else if(start_char == 'T' && it_start == m_CircBuff.end())
            {
                it_start = i;
                data_type = 'T';
            } 
            else if(start_char == 'V' && it_start == m_CircBuff.end())
            {
                it_start = i;
                data_type = 'V';
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
                LOG(LogLevel::Warning, "Invalid CAN data received, too much! Erasing circular buffer");
                m_CircBuff.erase(m_CircBuff.begin(), it_end);
                return;
            }

            std::copy(it_start, it_end, data);
            m_CircBuff.erase(m_CircBuff.begin(), it_end);

            data[data_len] = 0;
            if(data_type == 'V')
            {
                LOG(LogLevel::Notification, "LAWICEL CANUSB version: {}", data);
                return;
            }

            char frame_id_str[9] = {};
            char frame_length = 0;
            uint8_t ret = 0;
            char response[64] = {};
            uint8_t newline = 0;

            if(data_type == 't')  // (00:58 : 20.531) t3F4 7 80 83 00 00 00 00 00 66 90
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
        else  /* If no valid data received, wait for next iteration */
        {
            break;
        }
    }
}

size_t CanDeviceLawicel::PrepareSendDataFormat(std::shared_ptr<CanData>& data_ptr, char* out, size_t max_size, bool& remove_from_queue)
{
    switch(device_state)
    {
        case 0:  /* Initial CR */
        {
            memcpy(out, "\r", 1);
            device_state++;
            std::this_thread::sleep_for(150ms);
            break;
        }
        case 1:  /* Get version */
        {
            memcpy(out, "V\r", 2);  
            device_state++;
            std::this_thread::sleep_for(150ms);
            break;
        }
        case 2:  /* CAN Baudrate 500Kbps */
        {
            memcpy(out, "S6\r", 3);  
            device_state++;
            std::this_thread::sleep_for(50ms);
            break;
        }
        case 3:  /* Open CAN channel */
        {
            memcpy(out, "O\r", 2);  
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
            memcpy(out, out_str.c_str(), out_str.length());
            return out_str.length();
        }
        default:
        {
            assert(true);
            break;
        }
    }
    return 0;
}
