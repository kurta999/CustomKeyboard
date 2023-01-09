#pragma once

#include "utils/CSingleton.hpp"
#include <string.h>

enum class MapConverterFlags : uint8_t
{
    NONE = 0,
    ONLY_CREATE_VEHICLE = 1,
    VEHICLE_COMPONENTS_INLINE = 2,
    SAVE_NUMBER_PLATE = 4,
    HIDE_WHEN_ALPHA_NOT_255 = 8,
    CONVERT_ELEMENT_NAME = 16
};

class Vec3D
{
public:
    Vec3D() :
        x(0.0f), y(0.0f), z(0.0f) { }
    Vec3D(float x_, float y_, float z_) :
        x(x_), y(y_), z(z_) { }  

    union
    {
        float vec[3];
        struct
        {
            float x;
            float y;
            float z;
        };
    };
};

template<typename T> using Underlying = typename std::underlying_type<T>::type;
template<typename T> constexpr Underlying<T>
underlying(T t) { return Underlying<T>(t); }

template<typename T> struct TruthValue {
    T t;
    constexpr TruthValue(T t) : t(t) { }
    constexpr operator T() const { return t; }
    constexpr explicit operator bool() const { return underlying(t); }
};

constexpr TruthValue<MapConverterFlags> operator&(MapConverterFlags l, MapConverterFlags r) { return MapConverterFlags(underlying(l) & underlying(r)); }
constexpr TruthValue<MapConverterFlags> operator|=(MapConverterFlags& l, MapConverterFlags r) { return l = MapConverterFlags(underlying(l) | underlying(r)); }

class MapConverter : public CSingleton < MapConverter >
{
    friend class CSingleton < MapConverter >;

public:
    MapConverter() = default;
    ~MapConverter() = default;

    // !\brief Convert map from MTA:DM or MTA:Race to SA-MP format
    // !\param str [in] Input map file
    // !\param flags [in] Conversion flags
    // !\param offset [in] XYZ offset
    // !\return SA-MP map file format
    std::string MtaToSamp(const std::string& input, MapConverterFlags flags, Vec3D offset);

private:
    enum class ItemType : uint8_t
    {
        OBJECT = 0,
        REMOVE_OBJECT = 1,
        VEHICLE,
        MARKER,
        PICKUP,
        PED,
    };
};