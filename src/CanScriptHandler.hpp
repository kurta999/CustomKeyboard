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

    // !\brief Execute script
    // !\param script Script to execute
    void RunScript(std::string script);

    // !\brief Is script running
    // !\return true if script is running, otherwise false
    bool IsScriptRunning() const;

    // !\brief Abort running script
    void AbortRunningScript();

private:
    // !\brief Check if params count is correct
    // !\param actual Actual params count
    // !\param required Required params count
    bool __forceinline CheckParams(uint8_t actual, uint8_t required)
    {
        if(actual != required)
        {
            LOG(LogLevel::Error, "Invalid param count. {} should be instead of {}", required, actual);
            return false;
        }
        return true;
    }

    // !\brief Execute script
    // !\param script Script to execute
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

    // !\brief Bound operands
    std::map<std::string, std::function<CanScriptReturn(std::any, OperandParams&)>> m_operands;

    // !\brief Frame values set by SetFrameField
    std::map<uint32_t, uint64_t> m_FrameIDValues;

    // !\brief Result panel
    ICanResultPanel& m_Result;

    // !\brief Is running script aborted?
    std::atomic<bool> m_IsAborted{};

    // !\brief Conditon variable for waiting
    std::condition_variable cv;

    // !\brief Mutex for condition variable
    std::mutex cv_m;

    // !\brief Waiting Frame ID
    uint32_t m_WaitingFrame = std::numeric_limits<uint32_t>::max();

    // !\brief Is the frame received which we are waiting for?
    bool m_WaitingFrameReceived = false;

    // !\brief Data received for the waiting frame
    std::vector<uint8_t> m_WaitingFrameData;

    // !\brief Future for executing CAN script
    std::future<void> m_FutureHandle;

    // !\brief RAW frame values set by SetFrameFieldRaw
    std::map<uint16_t, std::vector<uint8_t>> raw_frame_blocks;

};