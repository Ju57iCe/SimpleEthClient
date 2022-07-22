#include "MPT.h"

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

std::tuple<bool, uint64_t, MPT::Node*> MPT::find_parent(std::string key, MPT::Node* node, uint64_t total_nibbles_matched)
{
    bool node_found = false;
    uint64_t nibbles_matched = 0;
    while(!node_found)
    {
        for (uint64_t i = 0; i < node->shared_nibbles.size(); ++i)
        {
            if (node->shared_nibbles[i] == key[i])
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
            if (nibbles_matched == node->shared_nibbles.size())
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

void MPT::add_node(std::string key, uint64_t value)
{
    //Keccak keccak256;
    std::string hash = key; //keccak256(entry);
    if (!m_root.shared_nibbles.empty())
    {
        auto res = find_parent(key, &m_root);
        bool success = std::get<0>(res);
        if (!success)
            return;

        uint64_t nibbles_matched = std::get<1>(res);
        Node* node = std::get<2>(res);

        std::unique_ptr<Node> branch_node(new Node());


        std::string shared_str(hash.begin() + nibbles_matched, hash.end());
        branch_node->shared_nibbles = shared_str;
        branch_node->value = value;
        branch_node->prefix = branch_node->shared_nibbles.size() % 2 == 0 ? LEAF_NODE_EVEN_PREFIX : LEAF_NODE_ODD_PREFIX;

        uint8_t branch_point = ASCIIHexToInt[(uint8_t)hash[nibbles_matched]];
        node->branches[branch_point] = std::move(branch_node);
        node->has_branches = true;
    }
    else
    {
        m_root.shared_nibbles = hash;
        m_root.value = value;
        m_root.prefix = hash.size() % 2 == 0 ? LEAF_NODE_EVEN_PREFIX : LEAF_NODE_ODD_PREFIX;
    }
}

void MPT::transform_leaf_node(MPT::Node* node, uint32_t nibbles_matched)
{
    std::string parent_shared_nibbles(node->shared_nibbles.begin(),
                                    node->shared_nibbles.begin() + nibbles_matched);

    std::string moved_nibbles(node->shared_nibbles.begin() + nibbles_matched,
    node->shared_nibbles.end());

    std::unique_ptr<Node> moved_node(new Node());
    moved_node->value = node->value;
    moved_node->shared_nibbles = moved_nibbles;
    moved_node->branches = std::move(node->branches);

    for (uint8_t i = 0; i < moved_node->branches.size(); ++i)
    {
        if (moved_node->branches[i] != nullptr)
        {
            moved_node->has_branches = true;
            break;
        }
    }

    moved_node->prefix = moved_node->has_branches ?
                            moved_node->shared_nibbles.size() % 2 == 0 ?
                                EXTENSION_NODE_EVEN_PREFIX :
                                EXTENSION_NODE_ODD_PREFIX
                        :
                            moved_node->shared_nibbles.size() % 2 == 0 ?
                                LEAF_NODE_EVEN_PREFIX :
                                LEAF_NODE_ODD_PREFIX;

    node->value = 0;
    node->shared_nibbles = parent_shared_nibbles;

    node->branches = std::array<std::unique_ptr<Node>, 16>();
    uint8_t branch_point = ASCIIHexToInt[(uint8_t)moved_node->shared_nibbles[0]];
    node->branches[branch_point] = std::move(moved_node);
    node->has_branches = true;
    node->prefix = node->shared_nibbles.size() % 2 == 0 ? EXTENSION_NODE_EVEN_PREFIX : EXTENSION_NODE_ODD_PREFIX;
}

void MPT::print_contents()
{
    std::cout<< "=======================" << std::endl;
    print_contents_recursive(&m_root);
}

void MPT::print_contents_recursive(MPT::Node* node, uint32_t branch_level)
{
    std::cout << "Level " << branch_level << " Node: " << node->shared_nibbles;

    if (node->has_branches)
    {
        std::cout << ", branches:";

        for (uint8_t i = 0; i < node->branches.size(); ++i)
            if (node->branches[i] != nullptr) std::cout << " " << std::hex << (uint32_t)i << std::dec;

        std::cout << std::endl;
    for (uint8_t i = 0; i < node->branches.size(); ++i)
    {
        if (node->branches[i] != nullptr)
        {
                print_contents_recursive(node->branches[i].get(), branch_level+1);
        }
    }
    std::cout << std::endl;
}
    else
{
        std::cout << " no branches." << std::endl;
    }
}

void MPT::add_key_to_parent(std::string key, MPT::Node* parent)
{

}

}
