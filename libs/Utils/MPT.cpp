#include "MPT.h"

#include "RLP.h"
#include "Hex.h"
#include <hash-library/keccak.h>

#include <iostream>

namespace
{

int ASCIIHexToInt[] =
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

}
namespace Utils
{

MPT::MPT()
{
    m_root = std::unique_ptr<Node>(new MPT::Node);
}

MPT::~MPT()
{

}

MPT::NodeType MPT::get_node_type(Node& node) const
{
    uint8_t key_size = node.nibbles.size();
    if (key_size == 0)
    {
        return NodeType::EMPTY_NODE;
    }
    else if (key_size == 2)
    {

    }
    else if (key_size == 17)
    {
        return NodeType::BRANCH_NODE;
    }
    //return NodeType::EXTENSION_NODE_EVEN_PREFIX;
        // EXTENSION_NODE_ODD_PREFIX,
        // LEAF_NODE_EVEN_PREFIX,
        // LEAF_NODE_ODD_PREFIX

    return NodeType::EMPTY_NODE;
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

    if (!m_root)
        m_root = std::unique_ptr<Node>(new MPT::Node);

    m_root = update_internal(*m_root.get(), key, value);


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

void MPT::add_prefix(std::vector<uint8_t>& nibbles) const
{
    if (nibbles.size() % 2 == 0)
    {
        std::vector<uint8_t> res;
        res.push_back(2);
        res.push_back(0);
        res.insert(res.end(), nibbles.begin(), nibbles.end());

        nibbles = res;
    }
}

std::vector<uint8_t> MPT::to_nibbles(const std::string& key) const
{
    std::vector<uint8_t> res;
    for (char c : key)
        res.emplace_back(ASCIIHexToInt[(uint8_t)c]);

    return res;
}

std::unique_ptr<MPT::Node> MPT::update_internal(Node& node, const std::string& key, const std::string& value)
{
    auto type = get_node_type(node);

    Keccak keccak256;
    std::string key_hash = keccak256(key);

    if (type == NodeType::EMPTY_NODE)
    {
        std::unique_ptr<Node> new_node(new Node);

        std::vector<uint8_t> nibbles = to_nibbles(key_hash);
        add_prefix(nibbles);

        new_node->nibbles = nibbles;
        new_node->value = std::vector<uint8_t>(value.begin(), value.end());

        if (key_hash.size() % 2 == 0)
        {
            key_hash.insert(0, std::string("2"));
            key_hash.insert(1, std::string("0"));
        }

        //key_hash.insert(0, std::string(std::to_string(128+key_hash.size())));
        std::string key_rlp = Utils::RLP::Encode(key_hash);

        std::string value_as_hex = Utils::Hex::ASCIIStringToHexString(value);
        std::string value_rlp = Utils::RLP::Encode(value_as_hex);


        // value_as_hex.insert(0, std::string("8"));
        // value_as_hex.insert(1, std::string("2"));
        //auto value_rlp = Utils::RLP::Encode(value_as_hex);

        //std::vector<std::string> list = { key_hash, std::string(value_rlp.begin(), value_rlp.end()) };
        //std::vector<uint8_t> node_hash = Utils::RLP::Encode(list);
        std::cout << key_hash.size() << std::endl;
        std::cout << value_as_hex.size() << std::endl;



        auto total_size = (key_hash.size() + value_as_hex.size()) / 2 + 2; // 2 for prefixes

        std::vector<uint8_t> final_rlp;
        final_rlp.insert(final_rlp.end(), 192 + total_size);
        final_rlp.insert(final_rlp.end(), key_rlp.begin(), key_rlp.end());
        final_rlp.insert(final_rlp.end(), value_rlp.begin(), value_rlp.end());

        std::string final_str(final_rlp.begin(), final_rlp.end());
        std::cout << final_str << std::endl;
        std::string final_hash = keccak256(final_str);
        std::cout << final_hash << std::endl;

        // for (auto& i : node_hash)

        // std::cout << std::endl << std::flush;

        return new_node;
    }
    else if (type == NodeType::BRANCH_NODE)
    {

    }

    return std::unique_ptr<Node>(new Node);
}

std::tuple<bool, uint64_t, MPT::Node*> MPT::find_parent(std::string key, MPT::Node* node, uint64_t total_nibbles_matched)
{
    bool node_found = false;
    uint64_t nibbles_matched = 0;
    while(!node_found)
    {
        for (uint64_t i = 0; i < node->nibbles.size(); ++i)
        {
            if (node->nibbles[i] == key[i])
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
            return {true, nibbles_matched, node};
        }
        else
        {
            if (nibbles_matched == node->nibbles.size())
            {
                uint8_t branch_point = ASCIIHexToInt[(uint8_t)key[nibbles_matched]];
                if (node->branches[branch_point].get() != nullptr)
                {
                    std::string new_key(key.begin() + nibbles_matched, key.end());
                    return find_parent(new_key, node->branches[branch_point].get(), total_nibbles_matched + nibbles_matched);
                }
                else
                {
                    return {true, total_nibbles_matched + nibbles_matched, node};
                }
            }
        }
    }

    return std::make_tuple(false, nibbles_matched, node);
}

void MPT::transform_leaf_node(MPT::Node* node, uint32_t nibbles_matched)
{
    // std::vector<uint8_t> parent_nibbles(node->nibbles.begin(),
    //                                 node->nibbles.begin() + nibbles_matched);

    //std::vector<uint8_t> moved_nibbles(node->nibbles.begin() + nibbles_matched,
    //node->nibbles.end());

    std::unique_ptr<Node> moved_node(new Node());
    moved_node->value = node->value;
   // moved_node->nibbles = moved_nibbles;
    moved_node->branches = std::move(node->branches);

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
