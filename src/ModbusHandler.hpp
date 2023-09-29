#pragma once

#include "IModbusEntry.hpp"

class XmlModbusEnteryLoader : public IModbusEntryLoader
{
public:
    virtual ~XmlModbusEnteryLoader() = default;

    bool Load(const std::filesystem::path& path, uint8_t& slave_id, ModbusCoils& coils, ModbusInputStatus& input_status, 
        ModbusHoldingRegisters& holding, ModbusHoldingRegisters& input, NumModbusEntries& num_entries) override;
    bool Save(const std::filesystem::path& path, uint8_t& slave_id, ModbusCoils& coils, ModbusInputStatus& input_status, 
        ModbusHoldingRegisters& holding, ModbusHoldingRegisters& input, NumModbusEntries& num_entries) const override;
};

class ModbusEntryHandler
{
public:
    ModbusEntryHandler(XmlModbusEnteryLoader& loader);
    ~ModbusEntryHandler();

    // !\brief Initialize entry handler
    void Init();

    void Save();

    uint8_t m_slaveId;
    ModbusCoils m_coils;
    ModbusInputStatus m_inputStatus;
    ModbusHoldingRegisters m_Holding;
    ModbusInputRegisters m_Input;
    NumModbusEntries m_numEntries;

    void SetPollingRate(uint16_t rate_ms) { m_pollingRate = rate_ms; }
    uint16_t GetPollingRate() const { return m_pollingRate; }

    void SetDefaultConfigName(const std::string& default_config) { m_DefaultConfigName = default_config; }
    const std::string& GetDefaultConfigName() const { return m_DefaultConfigName; }

    void EditCoil(size_t id, bool value) { m_pendingCoilWrites.push_back({ id, value }); }
    void EditHolding(size_t id, uint64_t value) { m_pendingHoldingWrites.push_back({ id, value }); }

private:
    void ModbusWorker(std::stop_token token);
    
    void HandleBoolReading(std::vector<uint8_t>& reg, ModbusCoils& coils, size_t num_coils, CoilStatusPanel* panel);
    void HandleRegisterReading(std::vector<uint16_t>& reg, ModbusHoldingRegisters& holding, size_t num_coils, ModbusRegisterPanel* panel);

    IModbusEntryLoader& m_ModbusEntryLoader;

    uint16_t m_pollingRate = 500;

    std::string m_DefaultConfigName = "Modbus.xml";

    std::vector<std::pair<int, bool>> m_pendingCoilWrites;
    std::vector<std::pair<int, uint64_t>> m_pendingHoldingWrites;

    // !\brief Worker thread
    std::unique_ptr<std::jthread> m_workerModbus;
};