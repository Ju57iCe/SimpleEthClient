#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <any>
#include <iomanip>
#include <sstream>
#include <limits>

#include <type_traits>

#include "ByteUtils.h"
#include "RLPConstants.h"

namespace
{

struct ItemProperties
{
    uint32_t size = 0;
    uint32_t lengthInBytes = 0;
};

template <typename T,
          typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
std::vector<uint8_t> splitValueToBytes(T const& value)
{
    std::vector<uint8_t> bytes;

    for (size_t i = 0; i < sizeof(value); i++)
    {
        uint8_t byte = value >> (i * 8);
        bytes.insert(bytes.begin(), byte);
    }

    return bytes;
}

std::vector<uint8_t> GenerateStringPrefix(std::string& str)
{
    std::vector<uint8_t> prefix_res;

    /// Empty string encoding
    if (str.size() == 0)
    {
        uint8_t prefix = Utils::RLP::SHORT_STRING_PREFIX + str.size();
        prefix_res.emplace_back(prefix);
    }
    /// Single byte encoding
    else if (str.size() == 1 && str[0] < Utils::RLP::SINGLE_BYTE_PREFIX) // ToDo research for single byte values greater that 128
    {
        prefix_res.emplace_back(str[0]);
    }
    /// Short string encoding
    else if (str.size() <= Utils::RLP::SHORT_STRING_MAX_WIDTH)
    {
        uint8_t prefix = Utils::RLP::SHORT_STRING_PREFIX + str.size();
        prefix_res.emplace_back(prefix);
    }
    /// Long string encoding
    else
    {
        uint length = str.size();
        uint container_bytes = Utils::Byte::BytesToFit(length);

        uint8_t prefix = Utils::RLP::LONG_STRING_PREFIX + container_bytes;
        prefix_res.emplace_back(prefix);

        std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
        Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

        prefix_res.insert(prefix_res.end(), container_size_vec.begin(), container_size_vec.end());
    }

    return prefix_res;
}

std::vector<uint8_t> GenerateLongListPrefix(uint32_t length)
{
    std::vector<uint8_t> res;
    uint8_t container_bytes = Utils::Byte::BytesToFit(length);

    std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
    Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

    res.emplace_back(Utils::RLP::LONG_LIST_PREFIX + container_bytes);
    res.insert(res.end(), container_size_vec.begin(), container_size_vec.end());

    return res;
}

ItemProperties GetItemSizeFromData(std::vector<uint8_t>& data, uint32_t offset = 0)
{
    ItemProperties res;

    if (data[offset] < Utils::RLP::LONG_STRING_PREFIX)
    {
        res.size = data[offset] - Utils::RLP::SHORT_STRING_PREFIX;
    }
    else if (data[offset] < Utils::RLP::SHORT_LIST_PREFIX)
    {
        res.lengthInBytes = data[offset] - Utils::RLP::LONG_STRING_PREFIX;
        res.size = Utils::Byte::GetIntFromBytes(res.lengthInBytes, data);
    }
    else if (data[offset] < Utils::RLP::LONG_LIST_PREFIX)
    {
        res.size = data[offset] - Utils::RLP::SHORT_LIST_PREFIX;
    }
    else
    {
        res.lengthInBytes = data[offset] - Utils::RLP::LONG_LIST_PREFIX;
        res.size = Utils::Byte::GetIntFromBytes(res.lengthInBytes, data);
    }

    return res;
}

}

namespace Utils::RLP
{

std::vector<uint8_t> Encode(std::string str)
{
    std::vector<uint8_t> result, prefix;

    prefix = GenerateStringPrefix(str);
    result.insert(result.end(), prefix.begin(), prefix.end());

    if (str.size() > 1)
    {
        result.insert(result.end(), str.begin(), str.end());
    }

    return result;
}

std::string Decode(std::vector<uint8_t>& data)
{
    std::string res;
    /// Empty string decoding
    if (data.size() == 0 ||
        (data.size() == 1 && data[0] == Utils::RLP::SHORT_STRING_PREFIX))
    {
        return res;
    }
    /// Single byte decoding
    else if (data.size() == sizeof(uint8_t))
    {
        res.append(data.begin(), data.end());
    }
    else
    {
        ItemProperties stringProps = GetItemSizeFromData(data);
        res.reserve(stringProps.size);

        if (stringProps.lengthInBytes == 0) // Short string decoding
            res = std::string(data.begin() + 1, data.begin() + (1 + stringProps.size));
        else                         // Long string decoding
            res = std::string(data.begin() + 1 + stringProps.lengthInBytes, data.begin() + (1 + stringProps.lengthInBytes + stringProps.size));
    }

    return res;
}

std::vector<uint8_t> Encode(std::vector<std::string> strings)
{
    std::vector<uint8_t> result;

    if (strings.empty())
    {
        result.emplace_back(Utils::RLP::SHORT_LIST_PREFIX);
        return result;
    }

    uint32_t total_size = 0;
    std::vector<std::vector<uint8_t>> strings_data;
    for (uint32_t i = 0; i < strings.size(); ++i)
    {
        strings_data.emplace_back(Encode(strings[i]));
        total_size += strings_data[i].size();
    }

    uint8_t prefix = 0;
    if (total_size <= Utils::RLP::SHORT_LIST_MAX_SIZE)
    {
        prefix = Utils::RLP::SHORT_LIST_PREFIX + total_size;
        result.emplace_back(prefix);
    }
    else
    {
        std::vector<uint8_t> prefix = GenerateLongListPrefix(total_size);
        result.insert(result.end(), prefix.begin(), prefix.end());
    }

    for (auto& str_data : strings_data)
        result.insert(result.end(), str_data.begin(), str_data.end());

    return result;
}

std::vector<std::string> DecodeList(std::vector<uint8_t>& data)
{
    std::vector<std::string> result;

    if (data.empty() ||
        (data.size() == 1 && data[0] == SHORT_LIST_PREFIX))
    {
        return result;
    }

    ItemProperties listProps = GetItemSizeFromData(data);

    uint32_t bytes_to_process = data.size();
    uint32_t processed_bytes = bytes_to_process - (bytes_to_process - 1 - listProps.lengthInBytes);

    while(bytes_to_process != processed_bytes)
    {
        uint32_t prefix_and_bytes_count = data[processed_bytes] <= LONG_STRING_PREFIX ? 1 :
                                            1 + data[processed_bytes] - LONG_STRING_PREFIX;

        std::vector<uint8_t> string_data(data.begin() + processed_bytes, data.end()); // ToDo - inefficient!!!
        result.emplace_back(Decode(string_data));

        processed_bytes += prefix_and_bytes_count + result.back().size();
    }

    return result;
}

std::vector<uint8_t> Encode(std::any input)
{
    std::vector<uint8_t> result;

    if(input.type() == typeid(std::vector<std::string>))
    {
        std::vector<std::string> nested_list = std::any_cast<std::vector<std::string>>(input);
        result = Encode(nested_list);
    }
    else if (input.type() == typeid(std::string))
    {
        std::string str = std::any_cast<std::string>(input);
        result = Encode(str);
    }
    else if (input.type() == typeid(std::vector<std::any>))
    {
        std::vector<std::any> any_vec = std::any_cast<std::vector<std::any>>(input);

        uint32_t list_total_size = 0;
        std::vector<std::vector<uint8_t>> list_data;
        for (uint32_t i = 0; i < any_vec.size(); ++i)
        {
            std::vector<uint8_t> list_item_data = Encode(any_vec[i]);
            list_data.push_back(list_item_data);

            list_total_size += list_item_data.size();
        }

        if (list_total_size == 0)
        {
            result.emplace_back(Utils::RLP::SHORT_LIST_PREFIX);
        }
        else if (list_total_size <= Utils::RLP::SHORT_LIST_MAX_SIZE)
        {
            result.emplace_back(Utils::RLP::SHORT_LIST_PREFIX + list_total_size);
        }
        else
        {
            std::vector<uint8_t> prefix = GenerateLongListPrefix(list_total_size);
            result.insert(result.end(), prefix.begin(), prefix.end());
        }

        for (auto& list_item_data : list_data)
            result.insert(result.end(), list_item_data.begin(), list_item_data.end());
    }

    return result;
}

std::any DecodeAny(std::vector<uint8_t>& data)
{
    std::any result;
    std::vector<std::any> list_results;
    std::vector<std::string> string_results;

    if (data.empty())
    {
        return result;
    }
    else if (data.size() == 1 && data[0] == Utils::RLP::SHORT_STRING_PREFIX)
    {
        result = std::string();
        return result;
    }
    else if (data.size() == 1 && data[0] == Utils::RLP::SHORT_LIST_PREFIX)
    {
        result = std::vector<std::string>();
        return result;
    }

    ItemProperties listProps = GetItemSizeFromData(data);

    uint32_t bytes_to_process = 0;
    uint32_t processed_bytes = 0;
    if (data[0] < Utils::RLP::SHORT_LIST_PREFIX)
    {
        bytes_to_process = 1 + listProps.lengthInBytes + listProps.size;
        processed_bytes = 0;
    }
    else
    {
        bytes_to_process = data.size();
        processed_bytes = bytes_to_process - (bytes_to_process - 1 - listProps.lengthInBytes);
    }


    while(bytes_to_process != processed_bytes)
    {
        uint32_t next_item_start_offset = processed_bytes;
        uint32_t prefix_and_bytes_count = 0;
        uint32_t item_size = 0;

        ItemProperties itemProps;

        if (data[next_item_start_offset] < Utils::RLP::SHORT_LIST_PREFIX)
        {
            ItemProperties stringProps = GetItemSizeFromData(data, next_item_start_offset);
            prefix_and_bytes_count = 1 + stringProps.lengthInBytes;

            std::vector<uint8_t> string_data(data.begin() + next_item_start_offset,
                                            data.begin() + next_item_start_offset + 1 + stringProps.lengthInBytes + stringProps.size); // ToDo - inefficient!!!

            std::string str_res = Decode(string_data);
            item_size = str_res.size();
            result = str_res;
            string_results.emplace_back(str_res);
        }
        else
        {
            std::any decoded_any;
            std::vector<uint8_t> list_data;
            if (data[next_item_start_offset] < Utils::RLP::LONG_LIST_PREFIX)
            {
                uint32_t list_len = data[next_item_start_offset] - Utils::RLP::SHORT_LIST_PREFIX;

                list_data = std::vector<uint8_t>(data.begin() + next_item_start_offset,
                                                data.begin() + next_item_start_offset + 1 + list_len); // ToDo - inefficient!!!
                item_size = list_len;
                prefix_and_bytes_count = 1;
            }
            else
            {
                uint32_t list_len = data[next_item_start_offset] - Utils::RLP::LONG_LIST_PREFIX;
                std::vector<uint8_t> size_buff = std::vector<uint8_t>(data.begin() + next_item_start_offset,
                                                                        data.begin() + next_item_start_offset + 1 + list_len);

                uint64_t list_size = Utils::Byte::GetIntFromBytes(list_len, size_buff);

                list_data = std::vector<uint8_t>(data.begin() + next_item_start_offset + list_len + 1,
                                                data.begin() + next_item_start_offset + list_len + 1 + list_size); // ToDo - inefficient!!!
                item_size = list_size;
                prefix_and_bytes_count = size_buff.size();
            }

            decoded_any = Utils::RLP::DecodeAny(list_data);
            list_results.emplace_back(decoded_any);
        }

        processed_bytes += prefix_and_bytes_count + item_size;
    }

    if (list_results.empty() && string_results.empty())
    {
        return result;
    }
    else
    {
        std::any res;
        if (!list_results.empty())
            res = list_results;
        else
            res = string_results;
        return res;
    }
}

}