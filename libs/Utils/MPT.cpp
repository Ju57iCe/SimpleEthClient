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

void MPT::update(const std::string& key, const std::string& value)
{
    if (value.size() > 32)
        return;

    if (value.empty())
    {
        delete_node(key);
        return;
    }

    std::unique_ptr<Node> leaf_node;
    Node* node;
    Node* parent_node;
    uint64_t nibbles_matched = 0;

    std::string key_hash = hash_string(key);
    if (!m_root)
    {
        m_root = std::unique_ptr<Node>(new MPT::Node);
        node = m_root.get();
    }
    else
    {
        auto res = find_parent(key_hash, *m_root.get());
        nibbles_matched = std::get<1>(res);
        parent_node = &std::get<2>(res);

        leaf_node = std::unique_ptr<Node>(new MPT::Node);
        node = leaf_node.get();
    }

    std::string remaining_nibbles(key_hash.begin() + nibbles_matched, key_hash.end());
    node->nibbles = to_nibbles(remaining_nibbles);
    node->value = value;
    node->hash = calculate_node_hash(NodeType::LEAF_NODE, key_hash, value);
    std::cout << "Node hash - " << node->hash << std::endl;

    if (leaf_node)
    {
        uint8_t branch_point = node->nibbles[0];
        node->nibbles.erase(node->nibbles.begin());

        parent_node->branch_node.get()->branches[branch_point] = std::move(leaf_node);
    }

    recalculate_hashes(*node);
}

void MPT::add_prefix(std::string& key_hash, NodeType type) const
{
    if (type == NodeType::LEAF_NODE)
    {
        if (key_hash.size() % 2 == 0)
        {
            key_hash.insert(0, std::string("2"));
            key_hash.insert(1, std::string("0"));
        }
        else
        {
            key_hash.insert(0, std::string("3"));
            key_hash.insert(1, std::string("0"));
        }
    }
    else if (type == NodeType::EXTENSION_NODE)
    {
        if (key_hash.size() % 2 == 0)
        {
            key_hash.insert(0, std::string("0"));
            key_hash.insert(1, std::string("0"));
        }
        else
        {
            key_hash.insert(0, std::string("1"));
            key_hash.insert(1, std::string("0"));
        }
    }
}

std::string MPT::hash_string(const std::string& str) const
{
    solidity::util::h256 str_hash = solidity::util::keccak256(str);
    std::stringstream ss;
    ss << str_hash;

    return ss.str();
}

std::string MPT::hash_data(const std::vector<uint8_t>& data) const
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

std::string MPT::calculate_node_hash(NodeType type, std::string key_hash, const std::string& value) const
{
    add_prefix(key_hash, type);

    std::string key_rlp = Utils::RLP::Encode(key_hash);
    std::string value_as_hex = Utils::Hex::ASCIIStringToHexString(value);

    std::string list_rlp = Utils::RLP::Encode({ key_hash, value_as_hex});

    std::string root_hash = hash_data(std::move(Utils::Hex::string_to_hex_vector(list_rlp)));
    return root_hash;
}

std::tuple<bool, uint64_t, MPT::Node&> MPT::find_parent(const std::string& key_hash, MPT::Node& node, uint64_t total_nibbles_matched)
{
    bool node_found = false;
    uint64_t nibbles_matched = 0;
    while(!node_found)
    {
        auto key_hash_nibbles = to_nibbles(key_hash);

        for (uint64_t i = 0; i < node.nibbles.size(); ++i)
        {
            if (node.nibbles[i] == key_hash_nibbles[i])
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
            transform_leaf_node_to_extension_node(node);

            std::tuple<bool, uint64_t, MPT::Node&> res = {true, nibbles_matched, node};
            return res;
        }

        if (nibbles_matched == node.nibbles.size())
        {
            uint8_t branch_point = Utils::Hex::ASCIIHexToInt[(uint8_t)key_hash_nibbles[nibbles_matched]];
            bool branch_exists = node.branch_node.get() != nullptr && node.branch_node->branches[branch_point].get() != nullptr;
            if (branch_exists)
            {
                std::string new_key(key_hash.begin() + nibbles_matched, key_hash.end());
                return find_parent(new_key, *node.branch_node->branches[branch_point].get(), total_nibbles_matched + nibbles_matched);
            }
            else
            {
                return {true, total_nibbles_matched + nibbles_matched, node};
            }
        }
    }

    std::tuple<bool, uint64_t, MPT::Node&> res = {false, nibbles_matched, node};
    return res;
}

void MPT::transform_leaf_node_to_extension_node(MPT::Node& node)
{
    std::unique_ptr<Node> moved_node(new Node());
    moved_node->value = node.value;
    moved_node->nibbles = std::move(node.nibbles);
    moved_node->branch_node = std::move(node.branch_node);

    node.value.clear();
    node.nibbles.clear();
    node.branch_node = std::unique_ptr<BranchNode>(new BranchNode);

    uint8_t branch_point = moved_node->nibbles[0];
    moved_node->nibbles.erase(moved_node->nibbles.begin());

    node.branch_node->branches[branch_point] = std::move(moved_node);
}

void MPT::print_contents() const
{
    std::cout<< "=======================" << std::endl;
    print_contents_recursive(*m_root.get());
}

void MPT::print_contents_recursive(const MPT::Node& node, uint32_t branch_level) const
{
    std::cout << "Level " << branch_level;

    if (!node.value.empty())
        std::cout << " Node value: " << node.value;

    if (node.branch_node)
    {
        std::cout << ", branches:";
        auto& branches = node.branch_node->branches;
        for (uint8_t i = 0; i < branches.size(); ++i)
            if (branches[i] != nullptr)
                std::cout << " " << std::hex << (uint32_t)i << std::dec;
        std::cout << std::endl;
    }

    if (!node.nibbles.empty())
    {
        std::cout << "Nibbles: ";
        for (auto& n : node.nibbles)
            std::cout << (uint32_t)n << " ";
        std::cout << std::endl;
    }

    if (node.branch_node)
    {
        auto& branches = node.branch_node->branches;
        for (uint8_t i = 0; i < branches.size(); ++i)
        {
            if (branches[i] != nullptr)
            {
                std::cout << "Branch " << std::hex << (uint32_t)i << ":" << std::dec << std::endl;
                print_contents_recursive(*branches[i].get(), branch_level+1);
            }
            if (i == 15)
                std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "Node has no branches." << std::endl << std::endl;
    }
}

void MPT::recalculate_hashes(MPT::Node& changed_node)
{

}

void MPT::delete_node(std::string key)
{

}

}
