#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <tuple>
#include <array>

namespace Utils
{
class MPT
{
private:
    struct Node
    {
        uint8_t prefix = 2;
        bool has_branches = false;
        uint32_t value = 0;
        std::string shared_nibbles;
        std::string hash;
        std::array<std::unique_ptr<Node>, 16> branches;
    };
public:
    void add_node(std::string entry, uint64_t value);
    void get_node(std::string entry);
    void delete_node(std::string entry);

    void generate_proof(std::string entry);
    void verify_proof(std::string entry);

    void print_contents();
private:
    std::tuple<bool, uint64_t, MPT::Node*> find_parent(std::string key, MPT::Node* node, uint64_t total_nibbles_matched = 0);
    void add_key_to_parent(std::string key, MPT::Node* parent);
    void transform_leaf_node(MPT::Node* node, uint32_t nibbles_matched);
    void print_contents_recursive(MPT::Node* parent, uint32_t branch_level = 0);
private:
    Node m_root;

    static constexpr uint8_t EXTENSION_NODE_EVEN_PREFIX = 0;
    static constexpr uint8_t EXTENSION_NODE_ODD_PREFIX = 1;
    static constexpr uint8_t LEAF_NODE_EVEN_PREFIX = 2;
    static constexpr uint8_t LEAF_NODE_ODD_PREFIX = 3;
};

}