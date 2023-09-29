#pragma once

#include <filesystem>
#include <bitset>

#include "IBasicGuiCustomization.hpp"

enum class ModbusByteOrder
{
    LittleEndian,
    BigEndian
};

class NumModbusEntries
{
public:
    size_t coils;
    size_t inputStatus;
    size_t inputRegisters;
    size_t holdingRegisters;
};

class ModbusItem : public BasicGuiTextCustomization
{
public:
    ModbusItem(const std::string& name, uint64_t value, uint32_t color, uint32_t bg_color, bool is_bold, float scale) :
        m_Name(name), m_Value(value), BasicGuiTextCustomization(color, bg_color, is_bold, scale)
    {

    }

    std::string m_Name;

    enum CanBitfieldType : uint8_t
    {
        CBT_BOOL, CBT_UI8, CBT_I8, CBT_UI16, CBT_I16, CBT_UI32, CBT_I32, CBT_UI64, CBT_I64, CBT_FLOAT, CBT_DOUBLE, CBT_INVALID
    };

    uint64_t m_Value;
};


using ModbusCoils = std::vector<std::pair<std::string, bool>>;
using ModbusInputStatus = std::vector<std::pair<std::string, bool>>;
using ModbusHoldingRegisters = std::vector<std::unique_ptr<ModbusItem>>;
using ModbusInputRegisters = std::vector<std::unique_ptr<ModbusItem>>;

class IModbusEntryLoader
{
public:
    virtual ~IModbusEntryLoader() = default;

    virtual bool Load(const std::filesystem::path& path, uint8_t& slave_id, ModbusCoils& coils, ModbusInputStatus& input_status, 
        ModbusHoldingRegisters& holding, ModbusHoldingRegisters& input, NumModbusEntries& num_entries) = 0;
    virtual bool Save(const std::filesystem::path& path, uint8_t& slave_id, ModbusCoils& coils, ModbusInputStatus& input_status, 
        ModbusHoldingRegisters& holding, ModbusHoldingRegisters& input, NumModbusEntries& num_entries) const = 0;
};
