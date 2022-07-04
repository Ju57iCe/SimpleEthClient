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

namespace Utils::RLP
{

static constexpr uint8_t SINGLE_BYTE_PREFIX = 128;      // 128 dec == 0x80 hex
static constexpr uint8_t SHORT_STRING_PREFIX = 128;     // 128 dec == 0x80 hex
static constexpr uint8_t LONG_STRING_PREFIX = 183;      // 183 dec == 0xb7

static constexpr uint8_t SHORT_LIST_PREFIX = 192;       // 192 dec == 0xc0
static constexpr uint8_t LONG_LIST_PREFIX = 247;       // 247 dec == 0xf7

static constexpr uint8_t SHORT_STRING_MAX_WIDTH = 55;
static constexpr uint8_t SHORT_LIST_MAX_SIZE = 55;

std::vector<uint8_t> Encode(std::string str)
{
    std::vector<uint8_t> result;

    /// Empty string encoding
    if (str.size() == 0)
    {
        uint8_t prefix = SHORT_STRING_PREFIX + str.size();
        result.emplace_back(prefix);
    }
    /// Single byte encoding
    else if (str.size() == 1 && str[0] < SINGLE_BYTE_PREFIX) // ToDo research for single byte values greater that 128
    {
        result.emplace_back(str[0]);
    }
    /// Short string encoding
    else if (str.size() <= SHORT_STRING_MAX_WIDTH)
    {
        uint8_t prefix = SHORT_STRING_PREFIX + str.size();
        result.emplace_back(prefix);

        result.insert(result.end(), str.begin(), str.end());
    }
    /// Long string encoding
    else
    {
        uint length = str.size();
        uint container_bytes = Utils::Byte::BytesToFit(length);

        uint8_t prefix = LONG_STRING_PREFIX + container_bytes;
        result.emplace_back(prefix);

        std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
        Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

        result.insert(result.end(), container_size_vec.begin(), container_size_vec.end());
        result.insert(result.end(), str.begin(), str.end());
    }

    return result;
}

std::string Decode(std::vector<uint8_t>& data)
{
    std::string res;
    /// Empty string decoding
    if (data.size() == 0 ||
        (data.size() == 1 && data[0] == SHORT_STRING_PREFIX))
        return res;
    /// Single byte decoding
    else if (data.size() == sizeof(uint8_t))
    {
        res.append(data.begin(), data.end());
    }
    /// Short string decoding
    else if (data[0] <= LONG_STRING_PREFIX)
    {
        uint32_t str_size = data[0] - SHORT_STRING_PREFIX;
        res.reserve(str_size);
        res = std::string(data.begin() + 1, data.begin() + (1 + str_size));
    }
    /// Long string decoding
    else
    {
        uint32_t size_length = data[0] - LONG_STRING_PREFIX;
        uint64_t str_size = Utils::Byte::GetIntFromBytes(size_length, data);
        res.reserve(str_size);
        res = std::string(data.begin() + 1 + size_length, data.begin() + (1 + size_length + str_size));
    }

    return res;
}


std::vector<uint8_t> Encode(std::vector<std::string> strings)
{
    std::vector<uint8_t> result;

    if (strings.empty())
    {
        result.emplace_back(SHORT_LIST_PREFIX);
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
    if (total_size <= SHORT_LIST_MAX_SIZE)
    {
        prefix = SHORT_LIST_PREFIX + total_size;
        result.emplace_back(prefix);
    }
    else
    {
        uint length = total_size;
        uint container_bytes = Utils::Byte::BytesToFit(length);

        prefix = LONG_LIST_PREFIX + container_bytes;

        std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
        Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

        result.emplace_back(prefix);
        result.insert(result.end(), container_size_vec.begin(), container_size_vec.end());
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

    uint32_t list_contents_total_size;
    uint32_t number_of_bytes = 0;
    if (data[0] < LONG_LIST_PREFIX)
    {
        list_contents_total_size = data[0] - SHORT_LIST_PREFIX;
    }
    else
    {
        number_of_bytes = data[0] - LONG_LIST_PREFIX;
        list_contents_total_size = Utils::Byte::GetIntFromBytes(number_of_bytes, data) + 1;
    }

    uint32_t bytes_to_process = list_contents_total_size;
    uint32_t processed_bytes = list_contents_total_size - bytes_to_process + number_of_bytes;

    while(bytes_to_process != processed_bytes)
    {
        uint32_t marker = processed_bytes + 1;
        uint32_t prefix_and_bytes_count = 0;

        if (data[marker] <= LONG_STRING_PREFIX)
        {
            prefix_and_bytes_count = 1;
        }
        else
        {
            uint32_t size_length = data[marker] - LONG_STRING_PREFIX;
            prefix_and_bytes_count = 1 + size_length;
        }

        std::vector<uint8_t> string_data(data.begin() + marker, data.end()); // ToDo - inefficient!!!
        result.emplace_back(Decode(string_data));

        processed_bytes += prefix_and_bytes_count + result.back().size();
    }

    return result;
}

std::vector<uint8_t> Encode(std::vector<std::any> input)
{
    uint32_t total_size = 0;
    std::vector<std::vector<uint8_t>> input_data;
    for (uint32_t i = 0; i < input.size(); ++i)
    {
        std::vector<uint8_t> encoded_data;
        if(input[i].type() == typeid(std::vector<std::string>))
        {
            std::vector<std::string> nested_list = std::any_cast<std::vector<std::string>>(input[i]);
            encoded_data = Encode(nested_list);
            input_data.emplace_back(encoded_data);
        }
        else if (input[i].type() == typeid(std::string))
        {
            std::string str = std::any_cast<std::string>(input[i]);
            encoded_data = Encode(str);
            input_data.push_back(encoded_data);
        }
        else if (input[i].type() == typeid(std::vector<std::any>))
        {
            std::vector<std::any> any_vec = std::any_cast<std::vector<std::any>>(input[i]);
            encoded_data = Encode(any_vec);

            std::vector<uint8_t> list_data;
            if (encoded_data.size() <= SHORT_LIST_MAX_SIZE)
            {
                list_data.emplace_back(SHORT_LIST_PREFIX + encoded_data.size());
            }
            else
            {
                uint length = encoded_data.size();
                uint container_bytes = Utils::Byte::BytesToFit(length);

                std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
                Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

                list_data.emplace_back(LONG_LIST_PREFIX + container_bytes);
                list_data.insert(list_data.end(), container_size_vec.begin(), container_size_vec.end());
            }

            list_data.insert(list_data.end(), encoded_data.begin(), encoded_data.end());

            input_data.emplace_back(list_data);
        }

        total_size += encoded_data.size();
    }

    std::vector<uint8_t> result;
    uint8_t prefix = 0;
    if (total_size <= SHORT_LIST_MAX_SIZE)
    {
        prefix = SHORT_LIST_PREFIX + total_size;
        result.emplace_back(prefix);
    }
    else
    {
        uint length = total_size;
        uint container_bytes = Utils::Byte::BytesToFit(length);

        prefix = LONG_LIST_PREFIX + container_bytes;

        std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
        Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

        result.emplace_back(prefix);
        result.insert(result.end(), container_size_vec.begin(), container_size_vec.end());
    }

    for (auto& data : input_data)
        result.insert(result.end(), data.begin(), data.end());


    return result;
}

}