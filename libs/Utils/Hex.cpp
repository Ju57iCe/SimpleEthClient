#include "Hex.h"

//#include <boost/format.hpp>

#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>
#include <iomanip>

namespace Utils::Hex
{

uint8_t uint8_from_hex(uint8_t nibble_one, uint8_t nibble_two)
{
    return ASCIIHexToInt[nibble_one] * 16 + ASCIIHexToInt[nibble_two];
}

std::string int_to_hex_str(uint8_t i)
{
    std::stringstream stream;
    stream << std::hex << (uint32_t)i;
    return stream.str();
}

std::string nibbles_to_hex_str(const std::vector<uint8_t>& nibbles)
{
    std::stringstream stream;
    for (auto& n : nibbles)
        stream << std::hex << (uint32_t)n;
    return stream.str();
}

std::string ASCIIStringToHexString(const std::string& str)
{
    std::ostringstream result;
    result << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
    std::copy(str.begin(), str.end(), std::ostream_iterator<unsigned int>(result, ""));

    return result.str();
}

std::vector<uint8_t> string_to_hex_vector(const std::string& str)
{
    std::vector<uint8_t> res;
    for (uint32_t i = 0; i < str.size(); i = i + 2)
    {
        uint8_t first_nibble = static_cast<uint8_t>(Utils::Hex::ASCIIHexToInt[str[i]]);
        uint8_t second_nibble = static_cast<uint8_t>(Utils::Hex::ASCIIHexToInt[str[i+1]]);
        res.emplace_back(first_nibble * 16 + second_nibble);
    }

    return res;
}



}