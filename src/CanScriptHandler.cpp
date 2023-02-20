#include "pch.hpp"

/* This module isn't finished yet */

CanScriptHandler::CanScriptHandler(ICanResultPanel& result_panel) :
    m_Result(result_panel)
{
    m_operands["SetDataFrame"] = std::bind(&CanScriptHandler::SetDataFrame, this, 3, std::placeholders::_2);
    m_operands["SendFrame"] = std::bind(&CanScriptHandler::SendFrame, this, 2, std::placeholders::_2);
    m_operands["CheckFrameBlock"] = std::bind(&CanScriptHandler::CheckFrameBlock, this, 2, std::placeholders::_2);
    m_operands["Wait"] = std::bind(&CanScriptHandler::Wait, this, 2, std::placeholders::_2);
}

CanScriptHandler::~CanScriptHandler()
{

}

void CanScriptHandler::RunScript(const std::string& script)
{
    std::istringstream ss(script);
    std::string line;
    while(std::getline(ss, line, '\n'))
    {
        std::vector<std::string> params;
        boost::split(params, line, boost::is_any_of(" "));

        if(params.empty())
        {
            LOG(LogLevel::Normal, "Script ended. Params is empty.");
            break;
        }

        bool is_executed = false;
        for(auto& [name, func] : m_operands)
        {
            if(!memcmp(params[0].c_str(), name.c_str(), name.length()))
            {
                func(std::placeholders::_1, params);
                is_executed = true;
                break;
            }
        }

        if(!is_executed)
        {
            LOG(LogLevel::Warning, "Unknown script function name: {}", params[0]);
            m_Result.AddToLog(std::format("Unknown script function name: {}\n", params[0]));
        }
    }
    m_Result.AddToLog(std::format("Script finished\n"));
}

template <typename T> void CanScriptHandler::HandleBitWriting(uint32_t frame_id, uint8_t& pos, uint8_t offset, uint8_t size, uint8_t* byte_array, std::string& new_data)
{
    T raw_data;
    try
    {
        raw_data = static_cast<T>(std::stoll(new_data, nullptr, 16));
        set_bitfield(raw_data, offset, size, byte_array, sizeof(byte_array));
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Error, "Invalid input for pos {}. Exception: {}", pos, e.what());
    }

    //DBG("%s - %d, %x ......... %d\n", m.second->m_Name.c_str(), m.first, raw_data, m.second->m_Size);
    pos++;
}

void CanScriptHandler::ApplyEditingOnFrameId(uint32_t frame_id, const std::string field_name, std::string new_data)
{
    uint8_t cnt = 0;
    uint8_t byte_array[8] = {};

    if(m_FrameIDValues.contains(frame_id))
        memcpy(byte_array, (const void*)m_FrameIDValues[frame_id], sizeof(uint64_t));

    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    CanMapping& m_mapping = can_handler->GetMapping();
    if(m_mapping.contains(frame_id))
    {
        LOG(LogLevel::Normal, "MapSize: {}, NewDataSize: {}", m_mapping[frame_id].size(), new_data.size());
        for(auto& [offset, m] : m_mapping[frame_id])
        {
            if(m->m_Name == field_name)
            {
                switch(m->m_Type)
                {
                case CBT_BOOL:
                case CBT_UI8:
                {
                    HandleBitWriting<uint8_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_I8:
                {
                    HandleBitWriting<int8_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_UI16:
                {
                    HandleBitWriting<uint16_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_I16:
                {
                    HandleBitWriting<int16_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_UI32:
                {
                    HandleBitWriting<uint32_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_I32:
                {
                    HandleBitWriting<int32_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_UI64:
                {
                    HandleBitWriting<uint64_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                case CBT_I64:
                {
                    HandleBitWriting<int64_t>(frame_id, cnt, offset, m->m_Size, byte_array, new_data);
                    break;
                }
                }
            }
        }
        m_FrameIDValues[frame_id] = *(uint64_t*)byte_array;

        //AssignNewBufferToTxEntry(frame_id, byte_array, sizeof(byte_array));
    }
    DBG("ok");
}

CanScriptReturn CanScriptHandler::SetDataFrame(std::any required_params, OperandParams& params)
{
    if(!CheckParams(params.size(), std::any_cast<int>(required_params)))
        return;

    const std::string& field_name = params[1]; /* VehicleStates */
    const std::string& frame_value = params[2];
    m_FrameValues[field_name] = strtoull(frame_value.c_str(), NULL, 16);

    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    CanMapping& mapping = can_handler->GetMapping();

    uint32_t frame_id = 0;
    for(auto& i : mapping)
    {
        for(auto& x : i.second)
        {
            if(x.second->m_Name == field_name)
            {
                frame_id = i.first;
                break;
            }
        }

        if(frame_id != 0)
            break;
    }

    ApplyEditingOnFrameId(frame_id, field_name, frame_value);

    uint64_t data_to_send = m_FrameIDValues[frame_id];
    uint8_t size_in_bytes = 8;
    if(can_handler->m_frame_size_mapping.contains(frame_id))
        size_in_bytes = can_handler->m_frame_size_mapping[frame_id];

    //CanSerialPort::Get()->AddToTxQueue(frame_id, size_in_bytes, (uint8_t*)&data_to_send);

    std::string hex;
    utils::ConvertHexBufferToString((const char*)&data_to_send, size_in_bytes, hex);
    m_Result.AddToLog(std::format("SetDataFrame {} (FrameID: {:X}, Size: {}, Data: {})\n", field_name, frame_id, size_in_bytes, hex));

    /* update frame in local array and if it's present in the CAN panel, update it there too */
}

CanScriptReturn CanScriptHandler::SendFrame(std::any required_params, OperandParams& params)
{
    if(!CheckParams(params.size(), std::any_cast<int>(required_params)))
        return;

    const std::string& frame_name = params[1];

    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    uint32_t frame_id = can_handler->FindFrameIdOnMapByName(frame_name);
    if(frame_id != 0)
    {
        CanMapping& m_mapping = can_handler->GetMapping();


        uint64_t data_to_send = m_FrameIDValues[frame_id];
        uint8_t size_in_bytes = 8;
        if(can_handler->m_frame_size_mapping.contains(frame_id))
            size_in_bytes = can_handler->m_frame_size_mapping[frame_id];

        CanSerialPort::Get()->AddToTxQueue(frame_id, size_in_bytes, (uint8_t*)&data_to_send);

        std::string hex;
        utils::ConvertHexBufferToString((const char*)&data_to_send, size_in_bytes, hex);
        m_Result.AddToLog(std::format("SendFrame {} (FrameID: {:X}, Size: {}, Data: {})\n", frame_name, frame_id, size_in_bytes, hex));

    }
    else
    {
        m_Result.AddToLog(std::format("SendFrame {} failed (FrameID: {:X})\n", frame_name, frame_id));
    }
}

CanScriptReturn CanScriptHandler::CheckFrameBlock(std::any required_params, OperandParams& params)
{
    if(!CheckParams(params.size(), std::any_cast<int>(required_params)))
        return;


    const std::string& field_name = params[1]; /* Filed name, VehicleState */
    const std::string& frame_value = params[2];
    //m_FrameValues[field_name] = strtoull(frame_value.c_str(), NULL, 16);

    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    CanMapping& mapping = can_handler->GetMapping();

    uint32_t frame_id = 0;
    for(auto& i : mapping)
    {
        for(auto& x : i.second)
        {
            if(x.second->m_Name == field_name)
            {
                frame_id = i.first;
                break;
            }
        }

        if(frame_id != 0)
            break;
    }

}

CanScriptReturn CanScriptHandler::Wait(std::any required_params, OperandParams& params)
{
    if(!CheckParams(params.size(), std::any_cast<int>(required_params)))
        return;

    uint32_t sleep_ms = std::stoi(params[1]);
    m_Result.AddToLog(std::format("Wait {} ms\n", sleep_ms));
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
}