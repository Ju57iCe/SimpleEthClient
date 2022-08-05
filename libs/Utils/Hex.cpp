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

}