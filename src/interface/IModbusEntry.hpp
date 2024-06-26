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
    size_t coils = 0xFFFF;
    size_t inputStatus = 0xFFFF;
    size_t Input = 0xFFFF;
    size_t holdingRegisters = 0xFFFF;

    uint16_t coilsOffset;
    uint16_t inputStatusOffset;
    uint16_t inputOffset;
    uint16_t holdingOffset;
};

enum ModbusBitfieldType : uint8_t
{
    MBT_BOOL, MBT_UI8, MBT_I8, MBT_UI16, MBT_I16, MBT_UI32, MBT_I32, MBT_UI64, MBT_I64, MBT_FLOAT, MBT_DOUBLE, MBT_STRING, MBT_INVALID
};

enum ModbusValueFormat : uint8_t
{
    MVF_DEC, MVF_HEX, MVF_BIN
};

class ModbusMap : public BasicGuiTextCustomization
{
public:
    ModbusMap(const std::string& name, ModbusBitfieldType type, uint8_t size, size_t min_val, size_t max_val, const std::string& description,
        uint32_t color, uint32_t bg_color, bool is_bold, float scale) :
        m_Name(name), m_Type(type), m_Size(size), m_MinVal(min_val), m_MaxVal(max_val), m_Description(description),
        BasicGuiTextCustomization(color, bg_color, is_bold, scale)
    {

    }

    //std::map<uint8_t, std::variant<bool, uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float, double, std::string>> m_Type;

    // !\brief Mapping type
    ModbusBitfieldType m_Type;

    // !\brief Mapping name
    std::string m_Name;

    // !\brief Bit length (starting from it's offset)
    uint8_t m_Size;

    // !\brief Minimum value
    size_t m_MinVal;

    // !\brief Maximum value
    size_t m_MaxVal;

    // !\brief Description (or whatever, more info about bitfields)
    std::string m_Description;
};

using ModbusMapping = std::map<uint8_t, std::unique_ptr<ModbusMap>>;

class ModbusItem 
{
public:
    ModbusItem(const std::string& name, uint8_t fav_level, size_t offset, ModbusBitfieldType type, ModbusValueFormat value_format, const std::string& desc, 
        ModbusMapping& map, int64_t min_val, int64_t max_val, uint64_t value, std::optional<uint32_t> color_ = {}, std::optional<uint32_t> bg_color_ = {}, std::optional<bool> is_bold_ = false,
        std::optional<float> scale = {}, std::optional<std::string> font_face = {}) :
        m_Name(name), m_FavLevel(fav_level), m_Offset(offset), m_Type(type), m_Format(value_format), m_Desc(desc), m_Mapping(std::move(map)), m_Min(min_val), m_Max(max_val), m_Value(value),
        m_color(color_), m_bg_color(bg_color_), m_is_bold(is_bold_)
    {
        if(scale.has_value())
            m_scale = *scale;
        if(font_face.has_value())
            m_font_face = *font_face;
        if(is_bold_.has_value())
            m_is_bold = *is_bold_;
    }

    uint8_t GetSize()
    {
        uint8_t size = 1;
        if (m_Type == ModbusBitfieldType::MBT_UI32 || m_Type == ModbusBitfieldType::MBT_FLOAT)
            size = 2;
        return size;
    }

    std::string m_Name;
    uint8_t m_FavLevel = 0;

    ModbusBitfieldType m_Type;
    ModbusValueFormat m_Format;

    size_t m_Offset;

    uint64_t m_Value;
    float m_fValue = 0.0f;

    ModbusMapping m_Mapping;


    std::string m_Desc;

    int64_t m_Min;
    
    int64_t m_Max;

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

    virtual void AppendLog(std::chrono::steady_clock::time_point& t1, uint8_t direction, uint8_t fcode, uint8_t error, const std::vector<uint8_t>& data) = 0;
    virtual void OnMaxEntriesReached() = 0;
};