#include "pch.hpp"
#include "Utils.hpp"

constexpr int DATA_SIZE = 1024;

bool XmlDataEntryLoader::Load(const std::filesystem::path& path, std::vector<std::unique_ptr<DataEntry>>& e)
{
    bool ret = true;
    boost::property_tree::ptree pt;
    try
    {
        read_xml(path.generic_string(), pt);
        for (const boost::property_tree::ptree::value_type& v : pt.get_child("DataSenderXml")) /* loop over each Frame */
        {
            char bytes[DATA_SIZE] = { 0 };
            std::string hex_str = v.second.get_child("Data").get_value<std::string>();

            char bytes_response[DATA_SIZE] = { 0 };
            std::string response_hex_str = v.second.get_child("ExpectedResponse").get_value<std::string>();

            dataEntryType type;
            std::string data_type_str = v.second.get_child("Type").get_value<std::string>();

            size_t data_len = 0;
            size_t response_len = 0;
            if (data_type_str == "hex")
            {
                type = dataEntryType::Hex;

                boost::algorithm::erase_all(hex_str, " ");
                utils::ConvertHexStringToBuffer(hex_str, std::span{ bytes });

                boost::algorithm::erase_all(response_hex_str, " ");
                utils::ConvertHexStringToBuffer(response_hex_str, std::span{ bytes_response });

                data_len = (hex_str.length() / 2);
                response_len = (response_hex_str.length() / 2);
            }
            else if (data_type_str == "string")
            {
                type = dataEntryType::String;

                boost::algorithm::replace_all(hex_str, "\\r", "\r");  /* Fix for newlines */
                boost::algorithm::replace_all(hex_str, "\\n", "\n");  /* Fix for newlines */
                strncat(bytes, hex_str.c_str(), std::min(sizeof(bytes), hex_str.length()));
                strncat(bytes_response, response_hex_str.c_str(), std::min(sizeof(bytes_response), response_hex_str.length()));

                data_len = hex_str.length();
                response_len = response_hex_str.length();
            }
            else
            {
                LOG(LogLevel::Error, "Invalid data type: {}", data_type_str);
                continue;
            }

            dataEntrySendType send_type;
            std::string data_send_type_str = v.second.get_child("SendType").get_value<std::string>();
            if(data_send_type_str == "auto")
                send_type = dataEntrySendType::Auto;
			else if(data_send_type_str == "trigger")
                send_type = dataEntrySendType::Trigger;
            else
            {
				LOG(LogLevel::Error, "Invalid data send type: {}", data_send_type_str);
				continue;
			}

            std::string comment = v.second.get_child("Comment").get_value<std::string>();
            size_t step = v.second.get_child("Step").get_value<size_t>();
            uint32_t period = v.second.get_child("Period").get_value<uint32_t>();
            uint32_t response_timeout = v.second.get_child("ResponseTimeout").get_value<uint32_t>();

            std::unique_ptr<DataEntry> local_entry = std::make_unique<DataEntry>((uint8_t*)bytes, data_len, (uint8_t*)bytes_response, response_len, type, send_type,
                step, period, response_timeout, comment);

            e.push_back(std::move(local_entry));
        }
    }
    catch (const boost::property_tree::xml_parser_error& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}, {}", e.filename(), e.what());
        ret = false;
    }
    catch (const std::exception& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}", e.what());
        ret = false;
    }
    return ret;
}

bool XmlDataEntryLoader::Save(const std::filesystem::path& path, std::vector<std::unique_ptr<DataEntry>>& e) const
{
    bool ret = true;
    return ret;
}
