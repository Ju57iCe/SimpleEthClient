#include "MPT.h"

#include "RLP.h"
#include "Hex.h"

#include "libsolutil/Keccak256.h"

#include <iostream>
#include <sstream>

namespace
{

int HexToByte(const char& nibble_one, const char& nibble_two)
{
    return nibble_one * 16 + nibble_two;
}

}
namespace Utils
{

MPT::MPT()
{
}

MPT::~MPT()
{
}

MPT::NodeType MPT::get_node_type(Node& node) const
{
    uint8_t key_size = node.nibbles.size();
    if (key_size == 0)
    {
        return NodeType::LEAF_NODE;
    }
    else if (key_size == 2)
    {

    }
    else if (key_size == 17)
    {
        return NodeType::BRANCH_NODE;
    }
}


void MPT::update(const std::string& key, const std::string& value)
{
    if (value.size() > 32)
        return;

    if (value.empty())
    {
        delete_node(key);
        return;
    }

    std::string key_hash = hash_string(key);
    if (!m_root)
    {
        m_root = std::unique_ptr<Node>(new MPT::Node);
        m_root = update_internal(*m_root.get(), key_hash, 0, value);
    }
    else
    {
        auto res = find_parent(key, *m_root.get());
        bool success = std::get<0>(res);
        if (!success)
            return;

        auto new_node = update_internal(std::get<2>(res), key_hash, std::get<1>(res), value);
    }


    // std::string hash = key;
    // if (!m_root.nibbles.empty())
    // {
    //     auto res = find_parent(hash, &m_root);
    //     bool success = std::get<0>(res);
    //     if (!success)
    //         return;

    //     uint64_t nibbles_matched = std::get<1>(res);
    //     Node* node = std::get<2>(res);

    //     std::unique_ptr<Node> branch_node(new Node());

    //     std::string shared_str(hash.begin() + nibbles_matched, hash.end());
    //     branch_node->nibbles = shared_str;
    //     branch_node->value = value;
    //     // branch_node->prefix = branch_node->nibbles.size() % 2 == 0 ? LEAF_NODE_EVEN_PREFIX :
    //     //                                                                     LEAF_NODE_ODD_PREFIX;

    //     uint8_t branch_point = ASCIIHexToInt[(uint8_t)hash[nibbles_matched]];
    //     node->branches[branch_point] = std::move(branch_node);
    //     node->has_branches = true;

    //     recalculate_hashes(branch_node.get());
    // }
    // else
    // {
    //     m_root.nibbles = hash;
    //     m_root.value = value;
    //     //m_root.prefix = hash.size() % 2 == 0 ? LEAF_NODE_EVEN_PREFIX : LEAF_NODE_ODD_PREFIX;

    //     m_root.hash = keccak256(std::string(value.begin(), value.end()));

    //     recalculate_hashes(&m_root);
    // }
}

void MPT::add_prefix(std::string& key_hash, NodeType type) const
{
    if (type == NodeType::LEAF_NODE && key_hash.size() % 2 == 0)
    {
        key_hash.insert(0, std::string("2"));
        key_hash.insert(1, std::string("0"));
    }
}

std::string MPT::hash_string(const std::string& str)
{
    solidity::util::h256 str_hash = solidity::util::keccak256(str);
    std::stringstream ss;
    ss << str_hash;

    return ss.str();
}

std::string MPT::hash_data(const std::vector<uint8_t>& data)
{
    solidity::util::h256 data_hash = solidity::util::keccak256(data);
    std::stringstream ss;
    ss << data_hash;

    return ss.str();
}

std::vector<uint8_t> MPT::to_nibbles(const std::string& key) const
{
    std::vector<uint8_t> res;
    for (char c : key)
        res.emplace_back(Utils::Hex::ASCIIHexToInt[(uint8_t)c]);

    return res;
}

std::string MPT::calculate_node_hash(NodeType type, std::string key_hash, const std::string& value)
{
    add_prefix(key_hash, type);

    std::string key_rlp = Utils::RLP::Encode(key_hash);
    std::string value_as_hex = Utils::Hex::ASCIIStringToHexString(value);

    std::string list_rlp = Utils::RLP::Encode({ key_hash, value_as_hex});

    std::string root_hash = hash_data(std::move(Utils::Hex::string_to_hex_vector(list_rlp)));
    return root_hash;
}

std::unique_ptr<MPT::Node> MPT::update_internal(Node& node, const std::string& key_hash, uint32_t nibbles_matched, const std::string& value)
{
    std::unique_ptr<Node> new_node(new Node);

    std::string nibbles_str(key_hash.begin() + nibbles_matched, key_hash.end());
    new_node->nibbles = to_nibbles(nibbles_str);

    new_node->value = value;
    new_node->hash = calculate_node_hash(NodeType::LEAF_NODE, key_hash, value);
    std::cout << new_node->hash << std::endl;

    uint8_t branch_point = Utils::Hex::ASCIIHexToInt[(uint8_t)key_hash[nibbles_matched]];
    //node.branches
    return new_node;

    // auto type = get_node_type(node);

    // if (type == NodeType::EMPTY_NODE)
    // {
    //     std::unique_ptr<Node> new_node(new Node);

    //     // std::vector<uint8_t> nibbles = to_nibbles(key_hash_str);
    //     // new_node->nibbles = nibbles;
    //     // new_node->value = std::vector<uint8_t>(value.begin(), value.end());


    //     new_node->hash = calculate_node_hash(type, key, value);
    //     std::cout << new_node->hash << std::endl;

    //     return new_node;
    // }
    // else if (type == NodeType::BRANCH_NODE)
    // {

    // }

    //return std::unique_ptr<Node>(new Node);
}

std::tuple<bool, uint64_t, MPT::Node&> MPT::find_parent(const std::string& key, MPT::Node& node, uint64_t total_nibbles_matched)
{
    bool node_found = false;
    uint64_t nibbles_matched = 0;
    while(!node_found)
    {
        for (uint64_t i = 0; i < node.nibbles.size(); ++i)
        {
            if (node.nibbles[i] == key[i])
            {
                nibbles_matched++;
                continue;
            }
            else
            {
                break;
            }
        }

        if (nibbles_matched == 0)
        {
            transform_leaf_node(node, 0);

            std::tuple<bool, uint64_t, MPT::Node&> res = {true, nibbles_matched, node};
            return res;
        }
        else
        {
            if (nibbles_matched == node.nibbles.size())
            {
                uint8_t branch_point = Utils::Hex::ASCIIHexToInt[(uint8_t)key[nibbles_matched]];
                if (node.branches[branch_point].get() != nullptr)
                {
                    std::string new_key(key.begin() + nibbles_matched, key.end());
                    return find_parent(new_key, *node.branches[branch_point].get(), total_nibbles_matched + nibbles_matched);
                }
                else
                {
                    return {true, total_nibbles_matched + nibbles_matched, node};
                }
            }
        }
    }

    std::tuple<bool, uint64_t, MPT::Node&> res = {false, nibbles_matched, node};
    return res;
}

void MPT::transform_leaf_node(MPT::Node& node, uint32_t nibbles_matched)
{
    // std::vector<uint8_t> parent_nibbles(node->nibbles.begin(),
    //                                 node->nibbles.begin() + nibbles_matched);

    //std::vector<uint8_t> moved_nibbles(node->nibbles.begin() + nibbles_matched,
    //node->nibbles.end());

    std::unique_ptr<Node> moved_node(new Node());
    moved_node->value = node.value;
   // moved_node->nibbles = moved_nibbles;
    moved_node->branches = std::move(node.branches);

    // for (uint8_t i = 0; i < moved_node->branches.size(); ++i)
    // {
    //     if (moved_node->branches[i] != nullptr)
    //     {
    //         moved_node->has_branches = true;
    //         break;
    //     }
    // }

    // moved_node->prefix = moved_node->has_branches ?
    //                         moved_node->nibbles.size() % 2 == 0 ?
    //                             EXTENSION_NODE_EVEN_PREFIX :
    //                             EXTENSION_NODE_ODD_PREFIX
    //                     :
    //                         moved_node->nibbles.size() % 2 == 0 ?
    //                             LEAF_NODE_EVEN_PREFIX :
    //                             LEAF_NODE_ODD_PREFIX;

    // node->value.clear();
    // node->nibbles = parent_nibbles;

    // node->branches = std::array<std::unique_ptr<Node>, 16>();
    // uint8_t branch_point = ASCIIHexToInt[(uint8_t)moved_node->nibbles[0]];
    // node->branches[branch_point] = std::move(moved_node);
    // node->has_branches = true;
    //node->prefix = node->nibbles.size() % 2 == 0 ? EXTENSION_NODE_EVEN_PREFIX : EXTENSION_NODE_ODD_PREFIX;
}

void MPT::print_contents()
{
    std::cout<< "=======================" << std::endl;
    print_contents_recursive(m_root.get());
}

void MPT::print_contents_recursive(MPT::Node* node, uint32_t branch_level)
{
    //std::cout << "Level " << branch_level << " Node: " << std::string(node->nibbles.begin(), node->nibbles.end());

    std::cout << " , value: '";
    for (auto& c : node->value)
        std::cout  << c;
    std::cout << "' ";

    // if (node->has_branches)
    // {
    //     std::cout << ", branches:";

    //     for (uint8_t i = 0; i < node->branches.size(); ++i)
    //         if (node->branches[i] != nullptr) std::cout << " " << std::hex << (uint32_t)i << std::dec;

    //     std::cout << std::endl;

    //     for (uint8_t i = 0; i < node->branches.size(); ++i)
    //     {
    //         if (node->branches[i] != nullptr)
    //         {
    //             print_contents_recursive(node->branches[i].get(), branch_level+1);
    //         }
    //         if (i == 15)
    //             std::cout << std::endl;
    //     }
    // }
    // else
    // {
    //     std::cout << " no branches." << std::endl;
    // }
}

void MPT::recalculate_hashes(MPT::Node* changed_node)
{

}

void MPT::delete_node(std::string key)
{

}

}
