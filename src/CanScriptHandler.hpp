#pragma once

#include "ICanResultPanel.hpp"
#include "Logger.hpp"

#include <map>

using CanScriptReturn = void;
using OperandParams = std::vector<std::string>;

class CanScriptHandler
{
public:
    CanScriptHandler(ICanResultPanel& result_panel);
    ~CanScriptHandler();

    void RunScript(const std::string& script);

private:
    bool __forceinline CheckParams(uint8_t actual, uint8_t required)
    {
        if(actual != required)
        {
            LOG(LogLevel::Error, "Invalid param count. {} should be instead of {}", required, actual);
            return false;
        }
        return true;
    }

    template <typename T> void HandleBitWriting(uint32_t frame_id, uint8_t& pos, uint8_t offset, uint8_t size, uint8_t* byte_array, std::string& new_data);
    void ApplyEditingOnFrameId(uint32_t frame_id, const std::string field_name, std::string new_data);

    CanScriptReturn SetDataFrame(std::any required_params, OperandParams& params);
    CanScriptReturn SendFrame(std::any required_params, OperandParams& params);
    CanScriptReturn CheckFrameBlock(std::any required_params, OperandParams& params);
    CanScriptReturn Wait(std::any required_params, OperandParams& params);

    std::map<std::string, std::function<CanScriptReturn(std::any, OperandParams&)>> m_operands;
    
    std::map<std::string, uint64_t> m_FrameValues;
    std::map<uint32_t, uint64_t> m_FrameIDValues;
    ICanResultPanel& m_Result;
};