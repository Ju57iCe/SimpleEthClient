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

void MPT::add_node(std::string key, uint64_t value)
{
    //Keccak keccak256;
    std::string hash = key; //keccak256(entry);
    if (!m_root.shared_nibbles.empty())
    {
        // if (key == node->shared_nibbles)
        //     return std::make_tuple(true, node);
        Node* node = &m_root;

        uint8_t nibbles_matched = 0;
        for (uint8_t i = 0; i < node->shared_nibbles.size(); ++i)
        {
            if (node->shared_nibbles[i] == hash[i])
            {
                nibbles_matched++;
                continue;
            }
            else
            {
                break;
            }
        }

        bool should_rebalance = nibbles_matched != node->shared_nibbles.size();

        if (should_rebalance)
        {
            transform_leaf_node(node, nibbles_matched);
        }

        std::unique_ptr<Node> branch_node(new Node());
        branch_node->shared_nibbles = std::string(hash.begin() + nibbles_matched, hash.end());
        branch_node->value = value;
        branch_node->prefix = branch_node->shared_nibbles.size() % 2 == 0 ? LEAF_NODE_EVEN_PREFIX : LEAF_NODE_ODD_PREFIX;

        uint8_t branch_point = ASCIIHexToInt[(uint8_t)hash[nibbles_matched]];
        node->branches[branch_point] = std::move(branch_node);
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
    print_contents_internal(&m_root);
}

void MPT::print_contents_internal(MPT::Node* node)
{
    std::cout << "Node: " << node->shared_nibbles << std::endl;
    for (uint8_t i = 0; i < node->branches.size(); ++i)
    {
        if (node->branches[i] != nullptr)
        {
            std::cout << "Branch - " <<  std::to_string(i) << " ";
            print_contents_internal(node->branches[i].get());
        }
    }
    std::cout << std::endl;
}

std::tuple<bool, MPT::Node*> MPT::find_parent(std::string key, MPT::Node* node)
{
    return std::make_tuple(false, node);
}

void MPT::add_key_to_parent(std::string key, MPT::Node* parent)
{

}

}