#include "MPT.h"

#include <iostream>

namespace Utils
{

void MPT::add_node(std::string entry, uint64_t value)
{
    //Keccak keccak256;
    std::string hash = entry; //keccak256(entry);
    if (m_root.data.empty())
    {
        m_root.data = hash;
        m_root.prefix = LEAD_NODE_ODD_PREFIX;
    }
    else
    {
        auto res = find_parent(entry, &m_root);

        add_key_to_parent(entry, std::get<1>(res));
    }
}

void MPT::print_contents()
{
    std::cout << m_root.data;
}

std::tuple<bool, MPT::Node*> MPT::find_parent(std::string key, MPT::Node* node)
{
    uint8_t nibbles_matched = 0;
    for (uint8_t i = 0; i < node->data.size(); ++i)
    {
        if (node->data[i] == key[i])
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
        return std::make_tuple(true, node);
    }
    else if (node->left_branch || node->right_branch)
    {
        if (node->left_branch)
        {
            auto res_left = find_parent(key, node->left_branch.get());
            if (std::get<0>(res_left) == true)
                return res_left;
        }
        if (node->right_branch)
        {
            auto res_right = find_parent(key, node->right_branch.get());
            if (std::get<0>(res_right) == true)
                return res_right;
        }
    }
}

void MPT::add_key_to_parent(std::string key, MPT::Node* parent)
{

}

}