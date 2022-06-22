#pragma once

#include <vector>
#include <cstdint>

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

    std::vector<uint8_t> ToBytes(uint16_t value)
    {
        std::vector<uint8_t> result;
        result.push_back(value >>  8);
        result.push_back(value      );
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
}