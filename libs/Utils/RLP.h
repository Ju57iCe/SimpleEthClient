#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <any>
#include <vector>

namespace Utils::RLP
{

std::string generate_string_prefix(const std::string& str);
std::string generate_long_list_prefix(uint32_t length);

std::string Encode(const std::string& str);
std::string Decode(const std::string& data);

std::string Encode(const std::vector<std::string>& strings);
std::vector<std::string> DecodeList(const std::string& data);

std::string Encode(const std::any& input);
std::any DecodeAny(const std::string& data);

}