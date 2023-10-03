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

enum ModbusBitfieldType : uint8_t
{
    MBT_BOOL, MBT_UI8, MBT_I8, MBT_UI16, MBT_I16, MBT_UI32, MBT_I32, MBT_UI64, MBT_I64, MBT_FLOAT, MBT_DOUBLE, MBT_INVALID
};

class ModbusItem : public BasicGuiTextCustomization
{
public:
    ModbusItem(const std::string& name, ModbusBitfieldType type, uint64_t value, uint32_t color, uint32_t bg_color, bool is_bold, float scale) :
        m_Name(name), m_Type(type), m_Value(value), BasicGuiTextCustomization(color, bg_color, is_bold, scale)
    {

    }

    std::string m_Name;
    ModbusBitfieldType m_Type;
    uint64_t m_Value;
};


using ModbusItemType = std::vector<std::unique_ptr<ModbusItem>>;

class IModbusEntryLoader
{
public:
    virtual ~IModbusEntryLoader() = default;

    virtual bool Load(const std::filesystem::path& path, uint8_t& slave_id, ModbusItemType& coils, ModbusItemType& input_status,
        ModbusItemType& holding, ModbusItemType& input, NumModbusEntries& num_entries) = 0;
    virtual bool Save(const std::filesystem::path& path, uint8_t& slave_id, ModbusItemType& coils, ModbusItemType& input_status,
        ModbusItemType& holding, ModbusItemType& input, NumModbusEntries& num_entries) const = 0;
};

class IModbusHelper
{
public:
    virtual ~IModbusHelper() = default;

    virtual void AppendLog(std::chrono::steady_clock::time_point& t1, uint8_t direction, uint8_t fcode, const std::vector<uint8_t>& data) = 0;
};