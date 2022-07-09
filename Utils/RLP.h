#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <any>
#include <iomanip>
#include <sstream>
#include <limits>

#include <type_traits>

#include "ByteUtils.h"
#include "RLPConstants.h"

namespace
{

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

std::vector<uint8_t> GenerateStringPrefix(std::string& str)
{
    std::vector<uint8_t> prefix_res;

    /// Empty string encoding
    if (str.size() == 0)
    {
        uint8_t prefix = Utils::RLP::SHORT_STRING_PREFIX + str.size();
        prefix_res.emplace_back(prefix);
    }
    /// Single byte encoding
    else if (str.size() == 1 && str[0] < Utils::RLP::SINGLE_BYTE_PREFIX) // ToDo research for single byte values greater that 128
    {
        prefix_res.emplace_back(str[0]);
    }
    /// Short string encoding
    else if (str.size() <= Utils::RLP::SHORT_STRING_MAX_WIDTH)
    {
        uint8_t prefix = Utils::RLP::SHORT_STRING_PREFIX + str.size();
        prefix_res.emplace_back(prefix);
    }
    /// Long string encoding
    else
    {
        uint length = str.size();
        uint container_bytes = Utils::Byte::BytesToFit(length);

        uint8_t prefix = Utils::RLP::LONG_STRING_PREFIX + container_bytes;
        prefix_res.emplace_back(prefix);

        std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
        Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

        prefix_res.insert(prefix_res.end(), container_size_vec.begin(), container_size_vec.end());
    }

    return prefix_res;
}

}

namespace Utils::RLP
{

std::vector<uint8_t> Encode(std::string str)
{
    std::vector<uint8_t> result, prefix;

    prefix = GenerateStringPrefix(str);
    result.insert(result.end(), prefix.begin(), prefix.end());

    if (str.size() > 1)
    {
        result.insert(result.end(), str.begin(), str.end());
    }

    return result;
}

std::string Decode(std::vector<uint8_t>& data)
{
    std::string res;
    /// Empty string decoding
    if (data.size() == 0 ||
        (data.size() == 1 && data[0] == Utils::RLP::SHORT_STRING_PREFIX))
        return res;
    /// Single byte decoding
    else if (data.size() == sizeof(uint8_t))
    {
        res.append(data.begin(), data.end());
    }
    /// Short string decoding
    else if (data[0] <= Utils::RLP::LONG_STRING_PREFIX)
    {
        uint32_t str_size = data[0] - Utils::RLP::SHORT_STRING_PREFIX;
        res.reserve(str_size);
        res = std::string(data.begin() + 1, data.begin() + (1 + str_size));
    }
    /// Long string decoding
    else
    {
        uint32_t size_length = data[0] - Utils::RLP::LONG_STRING_PREFIX;
        uint64_t str_size = Utils::Byte::GetIntFromBytes(size_length, data);
        res.reserve(str_size);
        res = std::string(data.begin() + 1 + size_length, data.begin() + (1 + size_length + str_size));
    }

    return res;
}

std::vector<uint8_t> Encode(std::vector<std::string> strings)
{
    std::vector<uint8_t> result;

    if (strings.empty())
    {
        result.emplace_back(Utils::RLP::SHORT_LIST_PREFIX);
        return result;
    }

    uint32_t total_size = 0;
    std::vector<std::vector<uint8_t>> strings_data;
    for (uint32_t i = 0; i < strings.size(); ++i)
    {
        strings_data.emplace_back(Encode(strings[i]));
        total_size += strings_data[i].size();
    }

    uint8_t prefix = 0;
    if (total_size <= Utils::RLP::SHORT_LIST_MAX_SIZE)
    {
        prefix = Utils::RLP::SHORT_LIST_PREFIX + total_size;
        result.emplace_back(prefix);
    }
    else
    {
        uint length = total_size;
        uint container_bytes = Utils::Byte::BytesToFit(length);

        prefix = Utils::RLP::LONG_LIST_PREFIX + container_bytes;

        std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
        Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

        result.emplace_back(prefix);
        result.insert(result.end(), container_size_vec.begin(), container_size_vec.end());
    }

    for (auto& str_data : strings_data)
        result.insert(result.end(), str_data.begin(), str_data.end());

    return result;
}

std::vector<std::string> DecodeList(std::vector<uint8_t>& data)
{
    std::vector<std::string> result;

    if (data.empty() ||
        (data.size() == 1 && data[0] == SHORT_LIST_PREFIX))
    {
        return result;
    }

    uint32_t list_contents_total_size;
    uint32_t number_of_bytes = 0;
    if (data[0] < LONG_LIST_PREFIX)
    {
        list_contents_total_size = data[0] - SHORT_LIST_PREFIX;
    }
    else
    {
        number_of_bytes = data[0] - LONG_LIST_PREFIX;
        list_contents_total_size = Utils::Byte::GetIntFromBytes(number_of_bytes, data) + 1;
    }

    uint32_t bytes_to_process = list_contents_total_size;
    uint32_t processed_bytes = list_contents_total_size - bytes_to_process + number_of_bytes;

    while(bytes_to_process != processed_bytes)
    {
        uint32_t marker = processed_bytes + 1;
        uint32_t prefix_and_bytes_count = 0;

        if (data[marker] <= LONG_STRING_PREFIX)
        {
            prefix_and_bytes_count = 1;
        }
        else
        {
            uint32_t size_length = data[marker] - LONG_STRING_PREFIX;
            prefix_and_bytes_count = 1 + size_length;
        }

        std::vector<uint8_t> string_data(data.begin() + marker, data.end()); // ToDo - inefficient!!!
        result.emplace_back(Decode(string_data));

        processed_bytes += prefix_and_bytes_count + result.back().size();
    }

    return result;
}

std::vector<uint8_t> Encode(std::any input)
{
    std::vector<uint8_t> result;

    if(input.type() == typeid(std::vector<std::string>))
    {
        std::vector<std::string> nested_list = std::any_cast<std::vector<std::string>>(input);
        result = Encode(nested_list);
    }
    else if (input.type() == typeid(std::string))
    {
        std::string str = std::any_cast<std::string>(input);
        result = Encode(str);
    }
    else if (input.type() == typeid(std::vector<std::any>))
    {
        std::vector<std::any> any_vec = std::any_cast<std::vector<std::any>>(input);

        uint32_t list_total_size = 0;
        std::vector<std::vector<uint8_t>> list_data;
        for (uint32_t i = 0; i < any_vec.size(); ++i)
        {
            std::vector<uint8_t> list_item_data = Encode(any_vec[i]);
            list_data.push_back(list_item_data);

            list_total_size += list_item_data.size();
        }

        if (list_total_size == 0)
        {
            result.emplace_back(Utils::RLP::SHORT_LIST_PREFIX);
        }
        else if (list_total_size <= Utils::RLP::SHORT_LIST_MAX_SIZE)
        {
            result.emplace_back(Utils::RLP::SHORT_LIST_PREFIX + list_total_size);
        }
        else
        {
            uint length = list_total_size;
            uint container_bytes = Utils::Byte::BytesToFit(length);

            std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
            Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

            result.emplace_back(Utils::RLP::LONG_LIST_PREFIX + container_bytes);
            result.insert(result.end(), container_size_vec.begin(), container_size_vec.end());
        }

        for (auto& list_item_data : list_data)
            result.insert(result.end(), list_item_data.begin(), list_item_data.end());
    }

    return result;
}

std::any DecodeAny(std::vector<uint8_t>& data)
{
    static int i = 0;
    std::cout << "Entering Decode any " << i++ << std::endl;
    std::any result;
    std::vector<std::any> list_results;
    std::vector<std::string> string_results;

    if (data.empty())
    {
        return result;
    }
    else if (data.size() == 1 && data[0] == Utils::RLP::SHORT_STRING_PREFIX)
    {
        result = std::string();
        return result;
    }
    else if (data.size() == 1 && data[0] == Utils::RLP::SHORT_LIST_PREFIX)
    {
        result = std::vector<std::string>();
        return result;
    }

    uint32_t list_contents_total_size;
    uint32_t number_of_bytes = 0;
    if (data[0] < Utils::RLP::LONG_LIST_PREFIX)
    {
        list_contents_total_size = data[0] - Utils::RLP::SHORT_LIST_PREFIX;
    }
    else
    {
        number_of_bytes = data[0] - LONG_LIST_PREFIX;
        list_contents_total_size = Utils::Byte::GetIntFromBytes(number_of_bytes, data) + 1;
    }

    uint32_t bytes_to_process = list_contents_total_size;
    uint32_t processed_bytes = list_contents_total_size - bytes_to_process + number_of_bytes;

    while(bytes_to_process != processed_bytes)
    {
        uint32_t marker = processed_bytes + 1;
        uint32_t prefix_and_bytes_count = 0;
        uint32_t item_size = 0;

        if (data[marker] <= Utils::RLP::LONG_STRING_PREFIX) // Short string
        {
            std::cout << "Decoding short string" << std::endl;
            prefix_and_bytes_count = 1;
            uint32_t str_len = data[marker] - Utils::RLP::SHORT_STRING_PREFIX;
            std::vector<uint8_t> string_data(data.begin() + marker, data.begin() + marker + 1 + str_len); // ToDo - inefficient!!!
            std::string str_res = Decode(string_data);
            item_size = str_res.size();
            result = str_res;
            string_results.emplace_back(str_res);
        }
        else if (data[marker] <= Utils::RLP::SHORT_LIST_PREFIX) // Long string
        {
            std::cout << "Decoding long string" << std::endl;
            uint32_t size_length = data[marker] - Utils::RLP::LONG_STRING_PREFIX;

            std::vector<uint8_t> size_buff(data.begin() + marker, data.begin() + marker + 1 + size_length);

            uint64_t str_size = Utils::Byte::GetIntFromBytes(size_length, size_buff);
            prefix_and_bytes_count = 1 + size_buff.size() - 1;

            std::vector<uint8_t> string_data(data.begin() + marker, data.begin() + marker + 1 + str_size); // ToDo - inefficient!!!
            std::string str_res = Decode(string_data);
            item_size = str_res.size();
            result = str_res;
            string_results.emplace_back(str_res);
        }
        else if (data[marker] <= Utils::RLP::LONG_LIST_PREFIX) // Short list
        {
            std::cout << "Decoding short list" << std::endl;
            prefix_and_bytes_count = 1;
            uint32_t list_len = data[marker] - Utils::RLP::SHORT_LIST_PREFIX;

            std::vector<uint8_t> list_data(data.begin() + marker, data.begin() + marker + 1 + list_len); // ToDo - inefficient!!!
            std::any decoded_any = Utils::RLP::DecodeAny(list_data);
            //std::cout << "decoded any " << decoded_any.type().name() << std::endl;
            item_size = list_len;
            list_results.emplace_back(decoded_any);
        }
        else // Long list
        {
            std::cout << "Decoding long string" << std::endl;
            uint32_t size_length = data[marker] - Utils::RLP::LONG_LIST_PREFIX;

            std::vector<uint8_t> size_buff(data.begin() + marker, data.begin() + marker + 1 + size_length);

            uint64_t list_size = Utils::Byte::GetIntFromBytes(size_length, size_buff);
            prefix_and_bytes_count = 1 + size_buff.size();

            std::vector<uint8_t> list_data(data.begin() + marker + size_length + 1, data.begin() + marker + size_length  + 1 + list_size); // ToDo - inefficient!!!
            std::any decoded_any = Utils::RLP::DecodeAny(list_data);
            // std::cout << "decoded any " << decoded_any.type().name() << std::endl;
            item_size = list_size;
            list_results.emplace_back(decoded_any);
        }

        processed_bytes += prefix_and_bytes_count + item_size;
    }


    if (list_results.empty() && string_results.empty())
    {
        std::cout << "Returning single any - " << result.type().name() << std::endl;
        return result;
    }
    else
    {

        std::any res;

        if (!list_results.empty())
            res = list_results;
        else
            res = string_results;

        //std::cout << "Returning any list - " << res.type().name() << std::endl;
        return res;
    }

}

}