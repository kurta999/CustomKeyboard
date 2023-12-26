#pragma once

#include <filesystem>
#include <wx/wx.h>
#include <queue>

#include "IDataSender.hpp"

class DataSender
{
public:
    DataSender(IDataEntryLoader& loader);
    ~DataSender();

    // !\brief Initialize entry handler
    void Init();

    // !\brief Set log helper
    void SetLogHelper(IDataLogHelper* helper);

    // !\brief Load files (TX & RX List, Frame mapping)
    void LoadFiles();

    // !\brief Send data
    void SendData(uint8_t* data, size_t data_size, dataEntryType type);

    // !\brief Send string
    void SendString(const char* str);

    // !\brief Send string
    void SendString(const std::string& str);

    // !\brief Worker thread
    void WorkerThread(std::stop_token token);

    // !\brief Called when a can frame was sent
    void OnDataSent(size_t data_len, uint8_t* data);

    // !\brief Called when a can frame was received
    void OnDataReceived(size_t data_len, uint8_t* data);

    // !\brief Toggle automatic sending of all CAN frames which period isn't null
    // !\param toggle [in] Toggle auto send?
    void ToggleAutoSend(bool toggle);

    // !\param Get CAN auto send state
    // !\return Is auto send toggled?
    bool IsAutoSend() const { return auto_send; }

    // !\brief Toggle automatic CAN frame recording
    // !\param toggle [in] Toggle auto recording?
    void ToggleAutoRecord(bool toggle) { auto_recording = toggle; }

    // !\param Is auto recording enabled?
    // !\return Is auto recording enabled?
    bool IsAutoRecord() const { return auto_recording; }

    // !\brief Toggle recording
    // !\param toggle [in] Toggle recording?
    // !\param is_puase [in] Is pause?
    void ToggleRecording(bool toggle, bool is_puase);

    // !\brief Toggle sending
    // !\param toggle [in] Toggle recording?
    // !\param is_puase [in] Is pause?
    void ToggleSending(bool toggle) { is_sending = toggle; }

    // !\brief 10ms Timer
    void On100msTimer();

    // !\brief Load TX list from a file
    // !\param path [in] File path to load
    // !\return Is load was successfull?
    bool LoadDataList(std::filesystem::path& path);

    // !\brief Mutex for entry handler
    std::mutex m;

    // !\brief Message received over uart
    std::queue<std::string> m_RecvMsg;

    uint64_t GetTxMsgCount() { return tx_msg_cnt; }

    uint64_t GetRxMsgCount() { return rx_msg_cnt; }

    uint64_t GetErrorCount() { return m_error_count; }

    void ResetCounters();

    std::string m_lastSentData;

private:
    void HandleAutoSendFrames(std::unique_ptr<DataEntry>& entry, uint32_t& vecIndex, std::stop_token& token, std::unique_lock<std::mutex>& lock);
    void HandleTriggerSendFrames(std::unique_ptr<DataEntry>& entry, uint32_t& vecIndex, std::stop_token& token, std::unique_lock<std::mutex>& lock);

    void AddToLog(const std::string& msg);
    void IncreaseTxCounter();

    // !\brief Sending every can frame automatically at startup which period is not null? 
    bool auto_send = false;

    // !\brief Start recording automatically?
    bool auto_recording = false;

    // !\brief Is sending toggled?
    bool is_sending = true;

    // !\brief Is recording on?
    bool is_recoding = false;

    // !\brief Error count
    uint64_t m_error_count = 0;

    // !\brief TX Frame count
    uint64_t tx_msg_cnt = 0;

    // !\brief RX Frame count
    uint64_t rx_msg_cnt = 0;

    // !\brief RX Frame count
    dataEntryType m_lastSentDataType = dataEntryType::Hex;

    // !\brief Is response received?
    std::atomic<bool> m_isResponseReceived{false};

    std::string m_response;

    // !\brief Vector of CAN TX entries
    std::vector<std::unique_ptr<DataEntry>> entries;

    // !\brief Path to default TX list
    std::filesystem::path defaultDataList;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_worker;

    // !\brief Conditional variable for main thread exiting
    std::condition_variable_any m_cv;
    
    std::condition_variable_any m_RecvCv;

    // !\brief Pointer to LogPanel
    IDataLogHelper* m_helper = nullptr;

    // !\brief Starting time
    std::chrono::steady_clock::time_point start_time;

    // !\brief Reference to CAN TX entry loader
    IDataEntryLoader& m_DataEntryLoader;
};