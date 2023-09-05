#include "pch.hpp"

CanScriptHandler::CanScriptHandler(ICanResultPanel& result_panel) :
    m_Result(result_panel)
{
    m_operands["SetFrameField"] = std::bind(&CanScriptHandler::SetFrameField, this, 3, std::placeholders::_2);
    m_operands["SetDataFrame"] = std::bind(&CanScriptHandler::SetFrameField, this, 3, std::placeholders::_2);
    m_operands["SetFrameFieldRaw"] = std::bind(&CanScriptHandler::SetFrameFieldRaw, this, 3, std::placeholders::_2);
    m_operands["SetDataFrameRaw"] = std::bind(&CanScriptHandler::SetFrameFieldRaw, this, 3, std::placeholders::_2);
    m_operands["SendFrame"] = std::bind(&CanScriptHandler::SendFrame, this, 2, std::placeholders::_2);
    m_operands["WaitForFrame"] = std::bind(&CanScriptHandler::WaitForFrame, this, 3, std::placeholders::_2);
    m_operands["CheckFrameBlock"] = std::bind(&CanScriptHandler::WaitForFrame, this, 3, std::placeholders::_2);
    m_operands["Sleep"] = std::bind(&CanScriptHandler::Sleep, this, 2, std::placeholders::_2);
    m_operands["Wait"] = std::bind(&CanScriptHandler::Sleep, this, 2, std::placeholders::_2);

    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    can_handler->RegisterObserver(this);
}

CanScriptHandler::~CanScriptHandler()
{
    std::unique_ptr<CanEntryHandler>& can_handler = wxGetApp().can_entry;
    if(can_handler.get())
        can_handler->UnregisterObserver(this);

    AbortRunningScript();
}

void CanScriptHandler::ExecuteScript(std::string script)
{
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    std::istringstream ss(script);
    std::string line;
    size_t line_cnt = 0;
    while(std::getline(ss, line, '\n') && !m_IsAborted)
    {
        line_cnt++;

        boost::algorithm::trim_right(line);
        std::vector<std::string> params;
        boost::split(params, line, boost::is_any_of(" "));

        if(params[0].starts_with("#"))  /* Skip comments */
            continue;

        if(params[0].length() == 0)
        {
            LOG(LogLevel::Verbose, "Skipping empty line at line: {}", line_cnt);
            continue;
        }

        bool is_executed = false;
        for(auto& [name, func] : m_operands)
        {
            if(!memcmp(params[0].c_str(), name.c_str(), params[0].length()))
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

    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

    m_Result.AddToLog(std::format("Script finished {}. Execution took {:.6f} ms\n", m_IsAborted ? "by abort" : "successfully", (double)dif / 1000000.0));
    m_IsAborted = false;
}

void CanScriptHandler::AbortRunningScript()
{
    m_IsAborted = true;
    m_WaitingFrame = std::numeric_limits<uint32_t>::max();
    m_WaitingFrameReceived = true;
    cv.notify_all();

    raw_frame_blocks.clear();
    m_FrameIDValues.clear();

    if(m_FutureHandle.valid())
        if(m_FutureHandle.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready)
            m_FutureHandle.get();
    //LOG(LogLevel::Normal, "Running script aborted");
}

void CanScriptHandler::RunScript(std::string script)
{
    if(m_FutureHandle.valid())
        if(m_FutureHandle.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready)
            m_FutureHandle.get();

    m_FutureHandle = std::async(&CanScriptHandler::ExecuteScript, this, script);
}

bool CanScriptHandler::IsScriptRunning() const
{
    bool ret = false;
    if(m_FutureHandle.valid())
        if(m_FutureHandle.wait_for(std::chrono::nanoseconds(1)) != std::future_status::ready)
            ret = true;
    return ret;
}

void CanScriptHandler::OnFrameOnBus(uint32_t frame_id, uint8_t* data, uint16_t size)
{
    if(m_WaitingFrame == frame_id && m_WaitingFrame != std::numeric_limits<uint32_t>::max() && !m_WaitingFrameReceived)
    {
        std::unique_lock lk(cv_m);
        m_WaitingFrameReceived = true;
        m_WaitingFrameData.assign(data, data + size);
        cv.notify_all();
    }
}

void CanScriptHandler::OnIsoTpDataReceived(uint32_t frame_id, uint8_t* data, uint16_t size)
{

}

template <typename T> void CanScriptHandler::HandleBitWriting(uint32_t frame_id, uint8_t& pos, uint8_t offset, uint8_t size, uint8_t* byte_array, std::string& new_data)
{
    T raw_data;
    try
    {
        raw_data = static_cast<T>(std::stoll(new_data, nullptr, 16));
        set_bitfield(raw_data, offset, size, byte_array, 64);
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
        memcpy(byte_array, (uint64_t*)&m_FrameIDValues[frame_id], sizeof(uint64_t));

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

        uint32_t reversed_1 = boost::endian::endian_reverse(*(uint32_t*)byte_array);
        uint32_t reversed_2 = boost::endian::endian_reverse(*(uint32_t*)&byte_array[4]);
        m_FrameIDValues[frame_id] = ((uint64_t)(uint64_t)reversed_1 << 32 | (uint64_t)reversed_2);

        can_handler->AssignNewBufferToTxEntry(frame_id, byte_array, sizeof(byte_array));

        MyFrame* frame = ((MyFrame*)(wxGetApp().GetTopWindow()));
        if(frame && frame->can_panel)
            frame->can_panel->sender->UpdateGridForTxFrame(frame_id, byte_array);
    }
    DBG("ok");
}

CanScriptReturn CanScriptHandler::SetFrameField(std::any required_params, OperandParams& params)
{
    if(!CheckParams(params.size(), std::any_cast<int>(required_params)))
        return;

    const std::string& field_name = params[1];
    const std::string& frame_value = params[2];

    try
    {
        //m_FrameValues[field_name] = strtoull(frame_value.c_str(), NULL, 16);
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Error, "Invalid SetFrameField script parameter, strtoull exception: {} (Input: {})", e.what(), frame_value);
        return;
    }

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

    if(!frame_id)
    {
        LOG(LogLevel::Error, "FrameID {} isn't found", frame_id);
        return;
    }

    ApplyEditingOnFrameId(frame_id, field_name, frame_value);

    uint64_t data_to_send = m_FrameIDValues[frame_id];
    uint8_t size_in_bytes = 8;
    if(can_handler->m_frame_size_mapping.contains(frame_id))
        size_in_bytes = can_handler->m_frame_size_mapping[frame_id];

    uint8_t array_to_send[8];
    memcpy(array_to_send, (uint8_t*)&data_to_send, sizeof(array_to_send));
    std::reverse(std::begin(array_to_send), std::end(array_to_send));

    std::string hex;
    utils::ConvertHexBufferToString((const char*)array_to_send, size_in_bytes, hex);
    m_Result.AddToLog(std::format("SetDataFrame {} (FrameID: {:X}, Size: {}, Data: {})\n", field_name, frame_id, size_in_bytes, hex));
}

CanScriptReturn CanScriptHandler::SetFrameFieldRaw(std::any required_params, OperandParams& params)
{
	/* TODO: finish this */
    if(!CheckParams(params.size(), std::any_cast<int>(required_params)))
        return;

    const std::string& field_name = params[1];
    std::string frame_value = params[2].starts_with("0x") ? params[2].substr(2) : params[2];

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

    if(!frame_id)
    {
        LOG(LogLevel::Error, "FrameID {} isn't found", frame_id);
        return;
    }

    if(frame_value.length() > 16)
    {
        LOG(LogLevel::Error, "Frame value is too long! Max supported bytes for a single frame is 8, but current are: {}", frame_value.length());
        return;
    }

    uint8_t array_to_send[8];
    std::string hex_str;

    if((frame_value.length() & 1))  /* Increase it's length if it's odd */
        frame_value += frame_value.back();

    utils::ConvertHexStringToBuffer(frame_value, std::span{ array_to_send });

    raw_frame_blocks[frame_id] = std::vector<uint8_t>(array_to_send, array_to_send + sizeof(array_to_send));

    std::string hex;
    utils::ConvertHexBufferToString((const char*)array_to_send, sizeof(array_to_send), hex);
    m_Result.AddToLog(std::format("SetDataFrameRaw {} (FrameID: {:X}, Size: {}, Data: {})\n", field_name, frame_id, sizeof(array_to_send), hex));
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

        if(m_FrameIDValues.contains(frame_id))
        {
            uint64_t data_to_send = m_FrameIDValues[frame_id];
            uint8_t size_in_bytes = 8;
            if(can_handler->m_frame_size_mapping.contains(frame_id))
                size_in_bytes = can_handler->m_frame_size_mapping[frame_id];

            CanSerialPort::Get()->AddToTxQueue(frame_id, size_in_bytes, (uint8_t*)&data_to_send);

            std::string hex;
            utils::ConvertHexBufferToString((const char*)&data_to_send, size_in_bytes, hex);
            m_Result.AddToLog(std::format("SendFrame {} (FrameID: {:X}, Size: {}, Data: {})\n", frame_name, frame_id, size_in_bytes, hex));
        }
        else if(raw_frame_blocks.contains(frame_id))
        {
            std::vector<uint8_t> to_send = raw_frame_blocks[frame_id];
            CanSerialPort::Get()->AddToTxQueue(frame_id, 8, to_send.data());

            uint8_t* sent_data = to_send.data();
            std::string hex;
            utils::ConvertHexBufferToString((const char*)sent_data, 8, hex);
            m_Result.AddToLog(std::format("SendFrame {} (FrameID: {:X}, Size: {}, Data: {})\n", frame_name, frame_id, 8, hex));
        }
        else
        {
            m_Result.AddToLog("SetFrameField or SetFrameFieldRaw wasn't callbed before SendFrame\n");
            return;
        }
    }
    else
    {
        m_Result.AddToLog(std::format("SendFrame {} failed (FrameID: {:X})\n", frame_name, frame_id));
    }
}

CanScriptReturn CanScriptHandler::WaitForFrame(std::any required_params, OperandParams& params)
{
    if(!CheckParams(params.size(), std::any_cast<int>(required_params)))
        return;

    const std::string& field_name = params[1];
    uint32_t wait_ms = 0;
    try
    {
        wait_ms = std::stoi(params[2]);
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Error, "Invalid WaitForFrame script parameter, stoi exception: {} (Input: {})", e.what(), params[2]);
        return;
    }

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

    if(!frame_id)
    {
        LOG(LogLevel::Error, "FrameID {} isn't found", frame_id);
        return;
    }

    m_Result.AddToLog(std::format("WaitForFrame START {}, timeout: {} (FrameID: {:X})\n", field_name, wait_ms, frame_id));

    m_WaitingFrame = frame_id;
    m_WaitingFrameReceived = false;
    std::unique_lock lk(cv_m);
    auto now = std::chrono::system_clock::now();
    bool ret = cv.wait_until(lk, now + std::chrono::milliseconds(wait_ms), [this]() { return m_WaitingFrameReceived || m_IsAborted.load(); });
    if(ret)
    {
        if(m_WaitingFrameReceived && !m_IsAborted)
        {
            if(raw_frame_blocks[frame_id] == m_WaitingFrameData)
                m_Result.AddToLog(std::format("WaitForFrame OK {} (FrameID: {:X})\n", field_name, frame_id));
            else
            {
                std::string recv;
                utils::ConvertHexBufferToString((const char*)m_WaitingFrameData.data(), m_WaitingFrameData.size(), recv);
                std::string expected;
                utils::ConvertHexBufferToString((const char*)raw_frame_blocks[frame_id].data(), raw_frame_blocks[frame_id].size(), expected);

                m_Result.AddToLog(std::format("WaitForFrame INVALID DATA Recv: {}, Expected: {} (FrameName: {}, FrameID: {:X})\n", recv, expected, field_name, frame_id));
            }
        }
        else
        {
            m_Result.AddToLog(std::format("WaitForFrame FAILED with ABORT {} (FrameID: {:X})\n", field_name, frame_id));
        }
    }
    else
    {
        m_Result.AddToLog(std::format("WaitForFrame FAILED with timeout {} (FrameID: {:X})\n", field_name, frame_id));
    }
    m_WaitingFrame = std::numeric_limits<uint32_t>::max();
    m_WaitingFrameReceived = false;
    raw_frame_blocks.erase(frame_id);
}

CanScriptReturn CanScriptHandler::Sleep(std::any required_params, OperandParams& params)
{
    if(!CheckParams(params.size(), std::any_cast<int>(required_params)))
        return;

    uint32_t sleep_ms = 0;
    try
    {
        sleep_ms = std::stoi(params[1]);
    }
    catch(const std::exception& e)
    {
        LOG(LogLevel::Error, "Invalid Sleep script parameter, stoi exception: {} (Input: {})", e.what(), params[1]);
        return;
    }

    m_Result.AddToLog(std::format("Wait {} ms... ", sleep_ms));

    std::unique_lock lk(cv_m);
    auto now = std::chrono::system_clock::now();
    bool ret = cv.wait_until(lk, now + std::chrono::milliseconds(sleep_ms), [this]() {return m_IsAborted.load(); });
    if(ret)  /* This is in reverse */
        m_Result.AddToLog("Aborted\n");
    else
        m_Result.AddToLog("OK\n");
}