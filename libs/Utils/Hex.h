#pragma once

#include <string>
#include <vector>

namespace Utils::Hex
{

static const int ASCIIHexToInt[] =
{
    // ASCII
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

static const int HexIntToASCII[] =
{
    // ASCII
    '0', '1', '2', '3', '4', '5' ,'6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

uint8_t uint8_from_hex(uint8_t nibble_one, uint8_t nibble_two);
std::string int_to_hex_str(uint8_t i);
std::string nibbles_to_hex_str(const std::vector<uint8_t>& nibbles);
std::string ASCIIStringToHexString(const std::string& str);
std::vector<uint8_t> string_to_hex_vector(const std::string& str);

}