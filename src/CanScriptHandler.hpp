#pragma once

#include "ICanResultPanel.hpp"
#include "ICanObserver.hpp"
#include "Logger.hpp"

#include <map>

using CanScriptReturn = void;
using OperandParams = std::vector<std::string>;

class CanScriptHandler : public ICanObserver
{
public:
    CanScriptHandler(ICanResultPanel& result_panel);
    ~CanScriptHandler();

    void RunScript(std::string script);
    bool IsScriptRunning();
    void AbortRunningScript();

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

    void ExecuteScript(std::string script);

    template <typename T> void HandleBitWriting(uint32_t frame_id, uint8_t& pos, uint8_t offset, uint8_t size, uint8_t* byte_array, std::string& new_data);
    void ApplyEditingOnFrameId(uint32_t frame_id, const std::string field_name, std::string new_data);

    CanScriptReturn SetFrameField(std::any required_params, OperandParams& params);
    CanScriptReturn SetFrameFieldRaw(std::any required_params, OperandParams& params);
    CanScriptReturn SendFrame(std::any required_params, OperandParams& params);
    CanScriptReturn WaitForFrame(std::any required_params, OperandParams& params);
    CanScriptReturn Sleep(std::any required_params, OperandParams& params);

    void OnFrameOnBus(uint32_t frame_id, uint8_t* data, uint16_t size) override;
    void OnIsoTpDataReceived(uint32_t frame_id, uint8_t* data, uint16_t size) override;

    std::map<std::string, std::function<CanScriptReturn(std::any, OperandParams&)>> m_operands;
    
    std::map<std::string, uint64_t> m_FrameValues;
    std::map<uint32_t, uint64_t> m_FrameIDValues;
    ICanResultPanel& m_Result;

    std::atomic<bool> m_IsAborted{};

    std::condition_variable cv;
    std::mutex cv_m;

    uint32_t m_WaitingFrame = std::numeric_limits<uint32_t>::max();

    bool m_WaitingFrameReceived = false;

    std::vector<uint8_t> m_WaitingFrameData;

    // !\brief Future for executing async crypto price reading
    std::future<void> m_FutureHandle;

    std::map<uint16_t, std::vector<uint8_t>> raw_frame_blocks;

};