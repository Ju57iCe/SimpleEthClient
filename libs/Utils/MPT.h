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
        LEAF_NODE,
        EXTENSION_NODE
    };

    //static constexpr uint8_t NIBBLE_TERMINATOR = 16;

    struct BranchNode;
    struct Node
    {
        std::string value;
        std::vector<uint8_t> nibbles;
        std::string hash;
        std::unique_ptr<BranchNode> branch_node;
    };

    struct BranchNode
    {
        std::array<std::unique_ptr<Node>, 16> branches;
        std::string value;
    };

public:
    MPT();
    ~MPT();

    void update(const std::string& key, const std::string& value);
    void get_node(std::string key) const;
    void delete_node(std::string key);

    void generate_proof(std::string key) const;
    void verify_proof(std::string key) const;

    void print_contents() const;
private:
    std::tuple<bool, uint64_t, MPT::Node&> find_parent(const std::string& key, MPT::Node& node, uint64_t total_nibbles_matched = 0);

    void transform_leaf_node_to_extension_node(MPT::Node& node, uint8_t nibbles_matched);
    void recalculate_hashes(MPT::Node& node);
    void print_contents_recursive(const MPT::Node& parent, uint32_t branch_level = 0) const;

    void add_prefix(std::string& key_hash, NodeType type) const;
    std::vector<uint8_t> to_nibbles(const std::string& key_hash) const;

    std::string hash_string(const std::string& str) const;
    std::string hash_data(const std::vector<uint8_t>& str) const;
    std::string calculate_node_hash(NodeType type, std::string key, const std::string& value) const;
private:
    std::unique_ptr<Node> m_root;
};

}