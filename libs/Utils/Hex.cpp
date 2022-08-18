#include "Hex.h"

//#include <boost/format.hpp>

#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>
#include <iomanip>

namespace Utils::Hex
{

// std::string ToHex(uint8_t value)
// {
//     return (boost::format("%x") % (unsigned)value).str();
// }

std::string ASCIIStringToHexString(std::string str)
{
    std::ostringstream result;
    result << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
    std::copy(str.begin(), str.end(), std::ostream_iterator<unsigned int>(result, ""));

    return result.str();
}

std::vector<uint8_t> string_to_hex_vector(std::string str)
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