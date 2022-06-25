#pragma once

#include <vector>
#include <cstdint>
#include <array>

namespace Utils::Byte
{

void TrimLeadingZeroBytes(std::vector<uint8_t>& bytes)
{
    uint_fast32_t elementsToRemove = 0;

    for(uint32_t i = 0; i < bytes.size(); ++i)
    {
        if (bytes[i] == 0)
            elementsToRemove++;
        else
            break;
    }

    if (elementsToRemove > 0)
        bytes.erase(bytes.begin(), bytes.begin() + elementsToRemove);
}

std::array<uint8_t, 4> ConstructUint32FromData(std::vector<uint8_t>& data,
                                                uint32_t size_length)
{
    std::array<uint8_t, 4> res;
    int32_t bytes_size = res.size();

    for (int32_t i = 0; i < bytes_size; ++i)
    {
        bool pad_with_zero = (bytes_size - (int32_t)size_length - i) > 0;
        if (pad_with_zero)
            res[i] = 0;
        else
            res[i] = data[i];
    }

    return res;
}

std::array<uint8_t, 8> ConstructUint64FromData(std::vector<uint8_t>& data,
                                                uint32_t size_length)
{
    std::array<uint8_t, 8> res;
    int32_t bytes_size = res.size();

    for (int32_t i = 0; i < bytes_size; ++i)
    {
        bool pad_with_zero = (bytes_size - (int32_t)size_length - i) > 0;
        if (pad_with_zero)
            res[i] = 0;
        else
            res[i] = data[i];
    }

    return res;
}

std::vector<uint8_t> ToBytes(uint16_t value)
{
    std::vector<uint8_t> result;
    result.push_back(value >>  8);
    result.push_back(value      );
    return result;
}

uint16_t uint16FromBytes(std::array<uint8_t, 2> bytes)
{
    uint16_t result = bytes[0] << 8 |
                        bytes[1];
    return result;
}

std::vector<uint8_t> ToBytes(uint32_t value)
{
    std::vector<unsigned char> result;
    result.push_back(value >> 24);
    result.push_back(value >> 16);
    result.push_back(value >>  8);
    result.push_back(value      );
    return result;
}

uint32_t uint32FromBytes(std::array<uint8_t, 4> bytes)
{
    uint32_t result = bytes[0] << 24 |
                        bytes[1] << 16 |
                        bytes[2] << 8 |
                        bytes[3];
    return result;
}

std::vector<uint8_t> ToBytes(uint64_t value)
{
    std::vector<unsigned char> result;
    result.push_back(value >> 56);
    result.push_back(value >> 48);
    result.push_back(value >> 40);
    result.push_back(value >> 32);
    result.push_back(value >> 24);
    result.push_back(value >> 16);
    result.push_back(value >>  8);
    result.push_back(value      );
    return result;
}

uint64_t uint64FromBytes(std::array<uint8_t, 8> bytes)
{
    uint64_t result = uint64_t(
                        ((uint64_t)(bytes[0]) << (uint64_t)56) |
                        ((uint64_t)(bytes[1]) << (uint64_t)48) |
                        ((uint64_t)(bytes[2]) << (uint64_t)40) |
                        ((uint64_t)(bytes[3]) << (uint64_t)32) |
                        ((uint64_t)(bytes[4]) << (uint64_t)24) |
                        ((uint64_t)(bytes[5]) << (uint64_t)16) |
                        ((uint64_t)(bytes[6]) << (uint64_t)8) |
                        ((uint64_t)(bytes[7]))
                        );
    return result;
}

uint64_t GetIntFromBytes(uint32_t size_length, std::vector<uint8_t>& data)
{
    uint64_t str_size = 0;
    if (size_length == 1) // ToDo - Is this valid case?
    {
        str_size = data[1];
    }
    else if (size_length == sizeof(uint16_t))
    {
        str_size = Utils::Byte::uint16FromBytes({data[1], data[2]});
    }
    else if (size_length <= sizeof(uint32_t))
    {
        str_size = Utils::Byte::uint32FromBytes(Utils::Byte::ConstructUint32FromData(data, size_length));
    }
    else if (size_length <= 8)
    {
        str_size = Utils::Byte::uint64FromBytes(Utils::Byte::ConstructUint64FromData(data, size_length));
    }
    else
    {
        assert(false); // Should not happen!
    }

    return str_size;
}

}