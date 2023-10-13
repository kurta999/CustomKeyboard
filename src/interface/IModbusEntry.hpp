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

class ModbusItem 
{
public:
    ModbusItem(const std::string& name, ModbusBitfieldType type, uint64_t value, std::optional<uint32_t> color_, std::optional<uint32_t> bg_color_, std::optional<bool> is_bold_,
        std::optional<float> scale = {}, std::optional<std::string> font_face = {}) :
        m_Name(name), m_Type(type), m_Value(value), m_color(color_), m_bg_color(bg_color_), m_is_bold(is_bold_)
    {
        if(scale.has_value())
            m_scale = *scale;
        if(font_face.has_value())
            m_font_face = *font_face;
    }

    std::string m_Name;
    ModbusBitfieldType m_Type;
    uint64_t m_Value;

    // !\brief Text color
    std::optional<uint32_t> m_color;

    // !\brief Text background color
    std::optional<uint32_t> m_bg_color;

    // !\brief Is text bold?
    bool m_is_bold{ false };

    // !\brief Text scale
    float m_scale{ 1.0f };

    // !\brief Font face
    std::string m_font_face;

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