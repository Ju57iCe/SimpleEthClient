#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <bitset>
#include <any>

namespace Utils
{
class MPT
{
private:
    enum class NodeType
    {
        EMPTY_NODE,
        LEAF_NODE,
        EXTENSION_NODE,
        BRANCH_NODE
    };

    static constexpr uint8_t NIBBLE_TERMINATOR = 16;

    struct Node
    {
        bool has_branches = false;
        std::vector<uint8_t> value;
        std::vector<std::bitset<4>> shared_nibbles;
        std::string hash;
        std::array<std::unique_ptr<Node>, 16> branches;
    };
public:
    MPT();
    ~MPT();

    void update(std::string key, std::vector<uint8_t> value);
    void get_node(std::string key);
    void delete_node(std::string key);

    void generate_proof(std::string key);
    void verify_proof(std::string key);

    void print_contents();
private:
    std::tuple<bool, uint64_t, MPT::Node*> find_parent(std::string key, MPT::Node* node, uint64_t total_nibbles_matched = 0);
    void transform_leaf_node(MPT::Node* node, uint32_t nibbles_matched);
    void recalculate_hashes(MPT::Node* node);
    void print_contents_recursive(MPT::Node* parent, uint32_t branch_level = 0);
    NodeType get_node_type(Node& node) const;
    std::unique_ptr<MPT::Node> update_internal(Node& node, std::string key, std::vector<uint8_t> value);

    void add_terminator(std::string& key_nibbles) const;
    std::vector<std::bitset<4>> to_nibbles(std::string key) const;

private:
    std::unique_ptr<Node> m_root;
};

}