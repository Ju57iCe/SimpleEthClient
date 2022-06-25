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

std::vector<uint8_t> Encode(std::vector<std::any> values)
{
    // ToDo - handle 0x7f, 0x80, 0xbf, 0xc0 variants
    std::vector<uint8_t> result;

    if (values.size() == 1 &&
        values[0].type() == typeid(uint8_t) &&
        std::any_cast<uint8_t>(values[0]) <= 127)
    {
        result.emplace_back(std::any_cast<uint8_t>(values[0]));
        return result;
    }
    else
    {
        for(uint32_t i = 0; i < values.size(); ++i)
        {
            if(values[i].has_value())
            {
                try
                {
                    if (values[i].type() == typeid(uint8_t) ||
                        values[i].type() == typeid(uint16_t) ||
                        values[i].type() == typeid(uint32_t) ||
                        values[i].type() == typeid(uint64_t)
                        // values[i].type() == typeid(double) ||  // ToDo Research
                        // values[i].type() == typeid(float) ||
                        )
                    {
                        std::vector<uint8_t> bytes;

                        if (values[i].type() == typeid(uint8_t))
                        {
                            bytes = splitValueToBytes(std::any_cast<uint8_t>(values[i]));
                        }
                        else if (values[i].type() == typeid(uint16_t))
                        {
                            bytes = splitValueToBytes(std::any_cast<uint16_t>(values[i]));
                        }
                        else if (values[i].type() == typeid(uint32_t))
                        {
                            bytes = splitValueToBytes(std::any_cast<uint32_t>(values[i]));
                        }
                        else if(values[i].type() == typeid(uint64_t))
                        {
                            bytes = splitValueToBytes(std::any_cast<uint64_t>(values[i]));
                        }

                        result.insert(result.end(), bytes.begin(), bytes.end());
                    }
                    else if (values[i].type() == typeid(std::string))
                    {
                        //std::cout << "string: " << std::any_cast<std::string>(values[i]) << "\n";
                    }
                    else if (values[i].type() == typeid(std::vector<uint64_t>)) // ToDo specify desired vector instantiations
                    {
                        //std::cout << "vector: " << std::any_cast<std::vector>(values[i]) << "\n";
                    }
                }
                catch(const std::bad_any_cast& e)
                {
                    std::cout << e.what() << '\n';
                    assert(false);
                    return std::vector<uint8_t>();
                }
            }
        }
    }

    return result;
}

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

        uint container_size = 1;
        while(container_size < length)
            container_size*=2;

        uint container_bytes = 0;
        uint test_length = length;
        while (test_length != 0)
        {
            test_length >>= 8;
            container_bytes++;
        }

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

        uint container_size = 1;
        while(container_size < length)
            container_size*=2;

        uint container_bytes = 0;
        uint test_length = length;
        while (test_length != 0)
        {
            test_length >>= 8;
            container_bytes++;
        }

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
    else if (data[0] < LONG_LIST_PREFIX)
    {
        uint32_t list_contents_total_size = data[0] - SHORT_LIST_PREFIX;
        uint32_t bytes_to_process = list_contents_total_size;
        uint32_t processed_bytes = list_contents_total_size - bytes_to_process;

        while(bytes_to_process != processed_bytes)
        {
            uint32_t marker = processed_bytes + 1;
            uint32_t string_size = data[marker] - SHORT_STRING_PREFIX;

            result.emplace_back(std::string(data.begin() + marker + 1,
                                            data.begin() + marker + 1 + string_size));

            processed_bytes += 1 + string_size;
        }
    }
    else
    {
        uint32_t number_of_bytes = data[0] - LONG_LIST_PREFIX;
        uint64_t list_size = Utils::Byte::GetIntFromBytes(number_of_bytes, data);

        uint container_bytes = 0;
        uint test_length = list_size;
        while (test_length != 0)
        {
            test_length >>= 8;
            container_bytes++;
        }

        uint32_t bytes_to_process = data.size() - 1 - container_bytes;
        uint32_t processed_bytes = list_size - bytes_to_process;
        while(bytes_to_process != processed_bytes)
        {
            uint32_t marker = processed_bytes + 1 + 1;
            uint32_t string_size = data[marker] - SHORT_STRING_PREFIX;

            result.emplace_back(std::string(data.begin() + marker + 1,
                                            data.begin() + marker + 1 + string_size));

            processed_bytes += 1 + string_size;
        }
    }

    return result;
}

}