#pragma once

#include <vector>
#include <filesystem>
#include <map>
#include <unordered_map>

class CanTxEntry;
class CanMap;

using CanMapping = std::map<uint32_t, std::map<uint8_t, std::unique_ptr<CanMap>>>;  /* [frame_id] = map[bit pos, size] */
using CanFrameNameMapping = std::map<uint32_t, std::string>;  /* TODO: this is wasteful as fuck, rewrite it */
using CanFrameSizeMapping = std::map<uint32_t, uint8_t>;  /* TODO: this is wasteful as fuck, rewrite it */
using CanFrameDirectionMapping = std::map<uint32_t, char>;  /* TODO: this is wasteful as fuck, rewrite it */

class ICanEntryLoader
{
public:
    virtual ~ICanEntryLoader() { }

    virtual bool Load(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e) = 0;
    virtual bool Save(const std::filesystem::path& path, std::vector<std::unique_ptr<CanTxEntry>>& e) const = 0;
};

class ICanRxEntryLoader
{
public:
    virtual ~ICanRxEntryLoader() { }

    virtual bool Load(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e, std::unordered_map<uint32_t, uint8_t>& loglevels) = 0;
    virtual bool Save(const std::filesystem::path& path, std::unordered_map<uint32_t, std::string>& e, std::unordered_map<uint32_t, uint8_t>& loglevels) const = 0;
};

class ICanMappingLoader
{
public:
    virtual ~ICanMappingLoader() { }

    virtual bool Load(const std::filesystem::path& path, CanMapping& mapping, CanFrameNameMapping& names, CanFrameSizeMapping& sizes, CanFrameDirectionMapping& directions) = 0;
    virtual bool Save(const std::filesystem::path& path, CanMapping& mapping, CanFrameNameMapping& names, CanFrameSizeMapping& sizes, CanFrameDirectionMapping& directions) const = 0;
};