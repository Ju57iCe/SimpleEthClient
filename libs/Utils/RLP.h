#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <any>
#include <vector>

namespace Utils::RLP
{

std::string Encode(std::string str);
std::string Decode(std::vector<uint8_t>& data);

std::vector<uint8_t> Encode(std::vector<std::string> strings);
std::vector<std::string> DecodeList(std::vector<uint8_t>& data);

std::vector<uint8_t> Encode(std::any input);
std::any DecodeAny(std::vector<uint8_t>& data);

}