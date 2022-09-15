#include "MPT.h"

#include "RLP.h"
#include "RLPConstants.h"

#include "Hex.h"

#include "libsolutil/Keccak256.h"

#include <iostream>
#include <sstream>

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

    std::string key_hash = hash_string(key);
    if (m_root)
    {
        auto res = find_parent(key_hash, *m_root.get());
        uint64_t nibbles_matched = std::get<0>(res);
        Node* parent_node = &std::get<1>(res);

        auto nibbles = to_nibbles(std::string (key_hash.begin() + nibbles_matched, key_hash.end()));
        uint8_t branch_point = nibbles[0];

        Node* leaf_node = parent_node->branch_node.get()->branches[branch_point].get();

        if (leaf_node == nullptr)
        {
            std::unique_ptr<Node> leaf_node = std::unique_ptr<Node>(new MPT::Node);
            leaf_node->nibbles = nibbles;
            leaf_node->value = value;

            leaf_node->nibbles.erase(leaf_node->nibbles.begin());
            parent_node->branch_node.get()->branches[branch_point] = std::move(leaf_node);
        }
        else
        {
            leaf_node->value = value;;
        }
    }
    else
    {
        m_root = std::unique_ptr<Node>(new MPT::Node);

        m_root->nibbles = to_nibbles(std::string(key_hash.begin(), key_hash.end()));
        m_root->value = value;
    }

    recalculate_hashes(*m_root.get());
}

void MPT::add_prefix(std::string& key_hash, NodeType type) const
{
    if (type == NodeType::LEAF_NODE)
    {
        if (key_hash.size() % 2 == 0)
            key_hash.insert(0, std::string("20"));
        else
            key_hash.insert(0, std::string("3"));
    }
    else if (type == NodeType::EXTENSION_NODE)
    {
        if (key_hash.size() % 2 == 0)
            key_hash.insert(0, std::string("00"));
        else
            key_hash.insert(0, std::string("1"));
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

std::string MPT::calculate_node_hash(NodeType type, std::variant<const Node*, const BranchNode*> node) const
{
    std::string node_hash;

    if (type == NodeType::LEAF_NODE  || type == NodeType::EXTENSION_NODE)
    {
        const Node* leaf = std::get<const Node*>(node);

        std::string key_hash;
        for(auto& n : leaf->nibbles)
            key_hash.push_back(Utils::Hex::HexIntToASCII[n]);

        add_prefix(key_hash, type);

        std::string key_hash_rlp =  Utils::RLP::Encode(key_hash);

        std::string value_as_hex = Utils::Hex::ascii_string_to_hex_string(leaf->value);
        std::string value_rlp = Utils::RLP::Encode(value_as_hex);

        uint32_t final_size = (key_hash_rlp.size() + value_rlp.size()) / 2;
        std::string final_rlp = Utils::Hex::int_to_hex_str(Utils::RLP::SHORT_LIST_PREFIX + final_size);
        final_rlp.insert(final_rlp.end(), key_hash_rlp.begin(), key_hash_rlp.end());
        final_rlp.insert(final_rlp.end(), value_rlp.begin(), value_rlp.end());

        node_hash = hash_data(std::move(Utils::Hex::string_to_hex_vector(final_rlp)));
    }
    else if (type == NodeType::BRANCH_NODE)
    {
        const BranchNode* branch_node = std::get<const BranchNode*>(node);

        std::string keys_to_rlp;
        for (auto& branch : branch_node->branches)
        {
            if (branch == nullptr)
                keys_to_rlp.append(Utils::RLP::Encode(std::string("")));
            else
                keys_to_rlp.append(Utils::RLP::Encode(branch->hash));
        }

        std::string value_as_hex = branch_node->value.empty() ? "80" : Utils::Hex::ascii_string_to_hex_string(branch_node->value);
        keys_to_rlp.insert(keys_to_rlp.end(), value_as_hex.begin(), value_as_hex.end());

        std::string prefix = Utils::RLP::generate_long_list_prefix(keys_to_rlp.size()/2);

        std::string list_rlp;
        list_rlp.insert(list_rlp.begin(), prefix.begin(), prefix.end());
        list_rlp.insert(list_rlp.end(), keys_to_rlp.begin(), keys_to_rlp.end());

        node_hash = hash_data(std::move(Utils::Hex::string_to_hex_vector(list_rlp)));
    }

    return node_hash;
}

std::tuple<uint64_t, MPT::Node&> MPT::find_parent(const std::string& key_hash, MPT::Node& node, uint64_t total_nibbles_matched)
{
    bool node_found = false;
    uint64_t nibbles_matched = 0;
    while(!node_found)
    {
        auto key_hash_nibbles = to_nibbles(key_hash);
        for (uint64_t i = 0; i < node.nibbles.size(); ++i)
        {
            if (node.nibbles[i] == key_hash_nibbles[i])
                nibbles_matched++;
            else
                break;
        }

        if (nibbles_matched != node.nibbles.size())
        {
            transform_leaf_node_to_extension_node(node, nibbles_matched);
            return {nibbles_matched, node};
        }
        else
        {
            uint8_t branch_point = key_hash_nibbles[nibbles_matched];
            bool branch_exists = node.branch_node.get() != nullptr
                && node.branch_node->branches[branch_point].get() != nullptr;

            if (branch_exists)
            {
                std::string node_nibbles = Utils::Hex::nibbles_to_hex_str(node.branch_node->branches[branch_point]->nibbles);
                node_nibbles.insert(node_nibbles.begin(), Utils::Hex::HexIntToASCII[branch_point]);

                if(node_nibbles == key_hash)
                {
                    return {total_nibbles_matched + nibbles_matched, node};
                }
                else
                {
                    std::string new_key(key_hash.begin() + nibbles_matched, key_hash.end());
                    return find_parent(new_key, *node.branch_node->branches[branch_point].get(), total_nibbles_matched + nibbles_matched);
                }
            }
            else
            {
                return {total_nibbles_matched + nibbles_matched, node};
            }
        }
    }

    return {nibbles_matched, node};
}

void MPT::transform_leaf_node_to_extension_node(MPT::Node& node, uint8_t nibbles_matched)
{
    std::unique_ptr<Node> moved_node(new Node());
    moved_node->value = node.value;
    moved_node->nibbles = std::vector(node.nibbles.begin() + nibbles_matched, node.nibbles.end());
    moved_node->branch_node = std::move(node.branch_node);

    uint8_t branch_point = moved_node->nibbles[0];

    node.value.clear();
    node.nibbles = std::vector(node.nibbles.begin(), node.nibbles.begin() + nibbles_matched);
    node.branch_node = std::unique_ptr<BranchNode>(new BranchNode);

    moved_node->nibbles.erase(moved_node->nibbles.begin());

    node.branch_node->branches[branch_point] = std::move(moved_node);
}

void MPT::print_contents() const
{
    if (m_root)
    {
        std::cout<< "=======================" << std::endl;
        print_contents_recursive(*m_root.get());
    }
}

void MPT::print_contents_recursive(const MPT::Node& node, uint32_t branch_level) const
{
    std::cout << std::endl << "Level " << branch_level << ": ";

    NodeType type; // ToDo move this as a member of the node and determine it when the node is added/changed
    if (node.branch_node)
    {
        if (node.nibbles.empty())
        {
            type = NodeType::BRANCH_NODE;
            std::cout << "Branch node" << std::endl;
        }
        else
        {
            type = NodeType::EXTENSION_NODE;
            std::cout << "Extension node" << std::endl;
        }
    }
    else
    {
        type = type = NodeType::LEAF_NODE;
        std::cout << "Leaf node" << std::endl;
    }

    std::cout << "\t Hash: " << node.hash << std::endl;

    if (type == NodeType::EXTENSION_NODE || type == NodeType::LEAF_NODE)
    {
        std::cout << "\t Nibbles: ";
        for (auto& n : node.nibbles)
            std::cout << (uint32_t)n << " ";
        std::cout << std::endl;
    }

    if (type == NodeType::EXTENSION_NODE || type == NodeType::BRANCH_NODE)
    {
        std::cout << "\t Branches:" << std::endl;

        const auto& branches = node.branch_node->branches;
        for (uint8_t i = 0; i < branches.size(); ++i)
        {
            std::cout << "\t\t branch " << std::hex << (uint32_t)i << std::dec << " ";
            std::cout << (branches[i] != nullptr ? branches[i]->hash : "") << std::endl;
        }
    }

    if (!node.value.empty())
        std::cout << "\t Value: " << node.value << std::endl;

    if (node.branch_node)
    {
        std::cout << std::endl;
        auto& branches = node.branch_node->branches;
        for (uint8_t i = 0; i < branches.size(); ++i)
        {
            if (branches[i] != nullptr)
            {
                print_contents_recursive(*branches[i].get(), branch_level+1);
            }
            if (i == 15)
                std::cout << std::endl;
        }
    }
}

void MPT::recalculate_hashes(MPT::Node& node)
{
    if (node.branch_node.get() != nullptr)
    {
        for (auto& branched_node : node.branch_node->branches)
        {
            if (branched_node.get() != nullptr)
                recalculate_hashes(*branched_node.get());
        }
        node.hash = calculate_node_hash(NodeType::BRANCH_NODE,std::variant<const Node*, const BranchNode*>(node.branch_node.get()));

    }
    else
    {
        node.hash = calculate_node_hash(NodeType::LEAF_NODE,std::variant<const Node*, const BranchNode*>(&node));
    }
}

void MPT::delete_node(std::string key)
{

}

std::string MPT::get_root_hash() const
{
    return m_root ? m_root->hash : "";
}

}

