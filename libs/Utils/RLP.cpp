#include "RLP.h"

#include "Hex.h"
#include "ByteUtils.h"
#include "RLPConstants.h"

#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>
#include <algorithm>

namespace
{
struct item_properties
{
    uint32_t size = 0;
    uint32_t length_in_nibbles = 0;
};

template <typename T,
          typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
std::vector<uint8_t> split_value_to_bytes(T const& value)
{
    std::vector<uint8_t> bytes;

    for (size_t i = 0; i < sizeof(value); i++)
    {
        uint8_t byte = value >> (i * 8);
        bytes.insert(bytes.begin(), byte);
    }

    return bytes;
}

item_properties get_item_size_from_data(const std::string& data, uint32_t offset = 0)
{
    item_properties res;
    uint8_t prefix_uint8 = Utils::Hex::uint8_from_hex(data[offset], data[offset+1]);
    if (prefix_uint8 < Utils::RLP::LONG_STRING_PREFIX)
    {
        res.size = prefix_uint8 - Utils::RLP::SHORT_STRING_PREFIX;
    }
    else if (prefix_uint8 < Utils::RLP::SHORT_LIST_PREFIX)
    {
        res.length_in_nibbles = (prefix_uint8 - Utils::RLP::LONG_STRING_PREFIX) * 2;

        uint64_t size_in_bytes;
        if (res.length_in_nibbles % 2 == 0)
            size_in_bytes = res.length_in_nibbles / 2;
        else
            size_in_bytes = res.length_in_nibbles / 2 + 1;

        res.size = Utils::Byte::GetIntFromBytes(size_in_bytes, data);
    }
    else if (prefix_uint8 < Utils::RLP::LONG_LIST_PREFIX)
    {
        res.size = prefix_uint8 - Utils::RLP::SHORT_LIST_PREFIX;
    }
    else
    {
        res.length_in_nibbles = (prefix_uint8 - Utils::RLP::LONG_LIST_PREFIX) * 2;

        uint64_t size_in_bytes;
        if (res.length_in_nibbles % 2 == 0)
            size_in_bytes = res.length_in_nibbles / 2;
        else
            size_in_bytes = res.length_in_nibbles / 2 + 1;

        res.size = Utils::Byte::GetIntFromBytes(size_in_bytes, data);
    }

    return res;
}

}

namespace Utils::RLP
{

std::string generate_string_prefix(const std::string& str)
{
    std::string prefix_res;
    uint64_t bytes_length = str.size();

    /// Empty string encoding
    if (bytes_length == 0)
    {
        prefix_res = Utils::Hex::int_to_hex_str(Utils::RLP::SHORT_STRING_PREFIX);
    }
    /// Single byte encoding
    else if (bytes_length == 1 && str[0] < Utils::RLP::SINGLE_BYTE_PREFIX)
    {
        //No prefix
    }
    /// Short string encoding
    else if (bytes_length <= Utils::RLP::SHORT_STRING_MAX_WIDTH)
    {
        prefix_res = Utils::Hex::int_to_hex_str(Utils::RLP::SHORT_STRING_PREFIX + bytes_length);
    }
    /// Long string encoding
    else
    {
        uint container_bytes = Utils::Byte::BytesToFit(bytes_length);

        uint8_t prefix = Utils::RLP::LONG_STRING_PREFIX + container_bytes;
        prefix_res = Utils::Hex::int_to_hex_str(prefix);

        std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(bytes_length);
        Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

        std::string size_string;
        for (auto& i : container_size_vec)
        {
            std::string s = Utils::Hex::int_to_hex_str(i);

            if(s.size() == 1)
                size_string.insert(size_string.end(), '0');
            size_string.append(s);
        }

        prefix_res.insert(prefix_res.end(), size_string.begin(), size_string.end());
    }

    return prefix_res;
}

std::string generate_long_list_prefix(uint32_t length)
{
    std::string res;
    uint8_t container_bytes = Utils::Byte::BytesToFit(length);

    std::vector<uint8_t> container_size_vec = Utils::Byte::ToBytes(length);
    Utils::Byte::TrimLeadingZeroBytes(container_size_vec);

    res = Utils::Hex::int_to_hex_str(Utils::RLP::LONG_LIST_PREFIX + container_bytes);

    std::string size_string;
    for (auto& i : container_size_vec)
    {
        std::string s = Utils::Hex::int_to_hex_str(i);

        if(s.size() == 1)
            size_string.insert(size_string.end(), '0');
        size_string.append(s);
    }

    res.insert(res.end(), size_string.begin(), size_string.end());

    return res;
}

std::string Encode(const std::string& str)
{
    std::string result = generate_string_prefix(str);

    std::string hex_str = Utils::Hex::ascii_string_to_hex_string(str);
    if (str.size() >= 1)
        result.insert(result.end(), hex_str.begin(), hex_str.end());

    return result;
}

std::string Decode(const std::string& input)
{
    std::string data = input;
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);

    std::string res;
    std::string hex_string;
    /// Empty string decoding
    if (data.size() == 0 ||
        (data.size() == 2 &&
            (Utils::Hex::uint8_from_hex(data[0], data[1]) == Utils::RLP::SHORT_STRING_PREFIX)))
    {
        return res;
    }
    /// Single byte decoding
    else if (data.size() == 1 || data.size() == 2)
    {
        if (data.size() == 1)
            res.insert(res.begin(), Utils::Hex::uint8_from_hex(0, data[0]));
        else
            res.insert(res.begin(), Utils::Hex::uint8_from_hex(data[0], data[1]));
    }
    else
    {
        item_properties string_props = get_item_size_from_data(data);
        hex_string.reserve(string_props.size * 2); // size is in bytes, but we are reserving space for nibbles

        if (string_props.length_in_nibbles == 0) // Short string decoding
            hex_string = std::string(data.begin() + Utils::RLP::PREFIX_LENGTH,
                                data.begin() + Utils::RLP::PREFIX_LENGTH + string_props.size * 2);
        else // Long string decoding
            hex_string = std::string(data.begin() + Utils::RLP::PREFIX_LENGTH + string_props.length_in_nibbles,
                                data.begin() + Utils::RLP::PREFIX_LENGTH + string_props.length_in_nibbles + string_props.size * 2);
    }

    if (!hex_string.empty())
        res = Utils::Hex::hex_string_to_ascii_string(hex_string);

    return res;
}

std::string Encode(const std::vector<std::string>& strings)
{
    std::string result;

    if (strings.empty())
    {
        return Utils::Hex::int_to_hex_str(Utils::RLP::SHORT_LIST_PREFIX);
    }

    uint32_t total_size = 0;
    std::vector<std::string> strings_data;
    for (uint32_t i = 0; i < strings.size(); ++i)
    {
        strings_data.emplace_back(Encode(strings[i]));
        item_properties props = get_item_size_from_data(strings_data.back());

        if (props.length_in_nibbles == 0)
        {
            total_size += props.size + 1; // 1 byte is the prefix
        }
        else
        {
            uint64_t size = Utils::Byte::GetIntFromBytes(props.length_in_nibbles / 2, strings_data.back()) + 2; // 1 byte is the prefix
            total_size += size;
        }
    }

    std::string prefix_str;
    if (total_size <= Utils::RLP::SHORT_LIST_MAX_SIZE)
        result = Utils::Hex::int_to_hex_str(Utils::RLP::SHORT_LIST_PREFIX + total_size);
    else
        result = generate_long_list_prefix(total_size);

    for (auto& str_data : strings_data)
        result.insert(result.end(), str_data.begin(), str_data.end());

    return result;
}

std::vector<std::string> DecodeList(const std::string& data)
{
    std::vector<std::string> result;

    if (data.empty() ||
        (data.size() == 1 && data[0] == 'c' && data[1] == '0'))
        return result;

    item_properties list_props = get_item_size_from_data(data);

    uint32_t nibbles_to_process = data.size();
    uint32_t processed_nibbles = nibbles_to_process - (nibbles_to_process - 2 - list_props.length_in_nibbles);

    while(nibbles_to_process != processed_nibbles)
    {
        uint32_t prefix_and_bytes_count;

        uint8_t prefix = Utils::Hex::uint8_from_hex(data[processed_nibbles],
                                                    data[processed_nibbles+1]);
        if (prefix <= LONG_STRING_PREFIX)
            prefix_and_bytes_count = PREFIX_LENGTH;
        else
            prefix_and_bytes_count = PREFIX_LENGTH + ((prefix - LONG_STRING_PREFIX) * 2);

        item_properties item_props = get_item_size_from_data(data.substr(processed_nibbles, data.size()));

        size_t item_buffer_size = prefix_and_bytes_count + item_props.size * 2;
        std::string string_data(data.begin() + processed_nibbles,
                                data.begin() + processed_nibbles + item_buffer_size);
        result.emplace_back(Decode(string_data));

        processed_nibbles += item_buffer_size;
    }

    return result;
}

std::string Encode(const std::any& input)
{
    std::string result;

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
        std::vector<std::string> list_data;
        for (uint32_t i = 0; i < any_vec.size(); ++i)
        {
            list_data.emplace_back(Encode(any_vec[i]));
            item_properties props = get_item_size_from_data(list_data.back());

            if (props.length_in_nibbles == 0)
            {
                list_total_size += props.size + 1; // 1 byte is the prefix
            }
            else
            {
                uint64_t size = Utils::Byte::GetIntFromBytes(props.length_in_nibbles / 2, list_data.back()) + 2; // 1 byte is the prefix
                list_total_size += size;
            }
        }

        if (list_total_size == 0)
        {
            result = Utils::Hex::int_to_hex_str(Utils::RLP::SHORT_LIST_PREFIX);
        }
        else if (list_total_size <= Utils::RLP::SHORT_LIST_MAX_SIZE)
        {
            result = Utils::Hex::int_to_hex_str(Utils::RLP::SHORT_LIST_PREFIX + list_total_size);
        }
        else
        {
            result = generate_long_list_prefix(list_total_size);
        }

        for (auto& list_item_data : list_data)
            result.insert(result.end(), list_item_data.begin(), list_item_data.end());
    }

    return result;
}

std::any DecodeAny(const std::string& data)
{
    std::any result;
    std::vector<std::any> list_results;
    std::vector<std::string> string_results;

    uint8_t prefix = Utils::Hex::uint8_from_hex(data[0],
                                            data[1]);

    if (data.empty())
    {
        return result;
    }
    else if (data.size() == 2 && prefix == Utils::RLP::SHORT_STRING_PREFIX)
    {
        result = std::string();
        return result;
    }
    else if (data.size() == 2 && prefix == Utils::RLP::SHORT_LIST_PREFIX)
    {
        result = std::vector<std::string>();
        return result;
    }

    item_properties list_props = get_item_size_from_data(data);

    uint32_t nibbles_to_process = 0;
    uint32_t processed_bytes = 0;
    if (prefix < Utils::RLP::SHORT_LIST_PREFIX)
    {
        nibbles_to_process = PREFIX_LENGTH + list_props.length_in_nibbles + list_props.size;
        processed_bytes = 0;
    }
    else
    {
        nibbles_to_process = data.size();
        processed_bytes = nibbles_to_process - (nibbles_to_process - PREFIX_LENGTH - list_props.length_in_nibbles);
    }


    while(nibbles_to_process != processed_bytes)
    {
        uint32_t next_item_start_offset = processed_bytes;
        uint32_t prefix_and_bytes_count = 0;
        uint32_t item_size = 0;

        uint8_t prefix = Utils::Hex::uint8_from_hex(data[next_item_start_offset],
                                                    data[next_item_start_offset+1]);
        if (prefix < Utils::RLP::SHORT_LIST_PREFIX)
        {
            item_properties string_props = get_item_size_from_data(data, next_item_start_offset);
            prefix_and_bytes_count = PREFIX_LENGTH + string_props.length_in_nibbles;

            std::string string_data(data.begin() + next_item_start_offset,
                                            data.begin() + next_item_start_offset + PREFIX_LENGTH + string_props.length_in_nibbles + string_props.size); // ToDo - inefficient!!!

            std::string str_res = Decode(string_data);
            item_size = str_res.size();
            result = str_res;
            string_results.emplace_back(str_res);
        }
        else
        {
            std::any decoded_any;
            std::string list_data;
            if (prefix < Utils::RLP::LONG_LIST_PREFIX)
            {
                uint32_t list_len = Utils::Hex::uint8_from_hex(data[next_item_start_offset], data[next_item_start_offset+1]) - Utils::RLP::SHORT_LIST_PREFIX;
                list_len *= 2;

                list_data = std::string(data.begin() + next_item_start_offset,
                                                data.begin() + next_item_start_offset + PREFIX_LENGTH + list_len);
                item_size = list_len;
                prefix_and_bytes_count = 2;
            }
            else
            {
                uint32_t list_len = Utils::Hex::uint8_from_hex(data[next_item_start_offset], data[next_item_start_offset+1]) - Utils::RLP::LONG_LIST_PREFIX;
                std::string size_buff = std::string(data.begin() + next_item_start_offset,
                                                                        data.begin() + next_item_start_offset + PREFIX_LENGTH + list_len * 2);

                uint64_t list_size = Utils::Byte::GetIntFromBytes(list_len, size_buff);

                list_data = std::string(data.begin() + next_item_start_offset + list_len + 1,
                                                data.begin() + next_item_start_offset + list_len + 1 + list_size); // ToDo - inefficient!!!
                item_size = list_size;
                prefix_and_bytes_count = size_buff.size();
            }

            decoded_any = Utils::RLP::DecodeAny(list_data);
            list_results.emplace_back(decoded_any);
        }

        processed_bytes += prefix_and_bytes_count + item_size;
    }

    if (list_results.empty() && string_results.empty())
    {
        return result;
    }
    else
    {
        std::any res;
        if (!list_results.empty())
            res = list_results;
        else
            res = string_results;
        return res;
    }
}

}