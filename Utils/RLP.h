#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <any>
#include <iomanip>
#include <sstream>

#include <type_traits>

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

std::vector<std::any> Decode(std::vector<uint8_t>& data)
{
    return std::vector<std::any>();
}

}