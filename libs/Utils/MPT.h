#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <tuple>

namespace Utils
{
class MPT
{
private:
    struct Node
    {
        uint8_t prefix = 2;

        std::string data;
        std::vector<std::unique_ptr<Node>> nibbles;

        std::unique_ptr<Node> left_branch;
        std::unique_ptr<Node> right_branch;
    };
public:
    void add_node(std::string entry, uint64_t value);
    void print_contents();
private:
    std::tuple<bool, MPT::Node*> find_parent(std::string key, Node* node);
    void add_key_to_parent(std::string key, MPT::Node* parent);
private:
    Node m_root;

    static constexpr uint8_t EXTENSION_NODE_EVEN_PREFIX = 0;
    static constexpr uint8_t EXTENSION_NODE_ODD_PREFIX = 1;
    static constexpr uint8_t LEAF_NODE_EVEN_PREFIX = 2;
    static constexpr uint8_t LEAD_NODE_ODD_PREFIX = 3;
};

}