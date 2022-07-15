#pragma once

#include <string>
#include <boost/format.hpp>

namespace Utils::Hex
{

std::string ToHex(uint8_t value)
{
    return (boost::format("%x") % (unsigned)value).str();
}

}