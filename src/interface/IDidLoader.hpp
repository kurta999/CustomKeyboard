#pragma once

#include <string>
#include <filesystem>
#include <map>

class DidEntry;

using Did = uint16_t;
using DidMap = std::map<uint16_t, std::unique_ptr<DidEntry>>;

class IDidLoader
{
public:
    virtual ~IDidLoader() { }

    virtual bool Load(const std::filesystem::path& path, DidMap& m) = 0;
    virtual bool Save(const std::filesystem::path& path, DidMap& m) = 0;
};