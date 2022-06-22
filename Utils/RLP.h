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

static constexpr uint8_t SINGLE_BYTE_PREFIX = 128;  // 128 dec == 0x80 hex
static constexpr uint8_t SHORT_STRING_PREFIX = 128;  // 128 dec == 0x80 hex
static constexpr uint8_t LONG_STRING_PREFIX = 183;   // 183 dec == 0xb7

static constexpr uint8_t SHORT_STRING_MAX_RANGE = 55;

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
        result.emplace_back(0);
    }
    /// Single byte encoding
    else if (str.size() == 1 && str[0] < SINGLE_BYTE_PREFIX) // ToDo research for single byte values greater that 128
    {
        result.emplace_back(str[0]);
    }
    /// Short string encoding
    else if (str.size() <= SHORT_STRING_MAX_RANGE)
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

        std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(container_size);
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
    if (data.size() == 0)
        return res;
    /// Single byte decoding
    else if (data.size() == 1)
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

        //uint32_t length_bytes_to_read = size_length / 255;

        // String length is 1 byte
        if (size_length == 1) // ToDo - Is this valid case?
        {
            uint8_t str_size = data[1];
            res = std::string(data.begin() + 2, data.begin() + (2 + str_size));
        }
        // String length is 2 bytes
        else if (size_length == 2)
        {
            uint16_t str_size = Utils::Byte::uint16FromBytes({data[1], data[2]});
            res = std::string(data.begin() + 3, data.begin() + (3 + str_size));
        }
        // String length is 4 bytes
        else if (size_length == 4)
        {
            uint32_t str_size = Utils::Byte::uint32FromBytes({data[1], data[2], data[3], data[4]});
            res = std::string(data.begin() + 5, data.begin() + (5 + str_size));
        }
        // String length is 8 bytes
        else if (size_length == 8)
        {

        }
        else
        {
            assert(false); // Should not happen!
        }
    }

    return res;
}

}