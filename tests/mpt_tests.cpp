#include <gtest/gtest.h>

#include <vector>
#include <any>
#include <cstdint>

#include <Utils/MPT.h>

// TEST(MPT, GetNotExistingKey)
// {
//     ASSERT_EQ(0, 0);
// }

// TEST(MPT, GetValueFromExistingKey)
// {
//     ASSERT_EQ(0, 0);
// }

TEST(MPT, UpdateKeyValue)
{
    Utils::MPT tree;
    tree.update("32fa7b", "10");
    tree.update("32fa7c", "20");
    tree.update("32fa7b", "20");

    std::string expeted_str = "17ad474b4cfa5db24e50835a52a6f5263dbf334784462440bdfe7b4fa0b57b5f";
    std::string actual = tree.get_root_hash();
    EXPECT_TRUE(actual == expeted_str);

    ASSERT_EQ(0,0);
}

TEST(MPT, UpdateKeysAndValues)
{
    Utils::MPT tree;
    tree.update("32fa7b", "10");
    tree.update("32fa7c", "20");
    tree.update("32fa7d", "20");
    tree.update("32fa7e", "20");
    tree.update("32fa7f", "20");
    tree.update("32fa7g", "20");

    std::string expeted_str = "cd23dfb08a263216440377ea1c9f909aadfd22bba7dd0c5ca5a8718758454df5";
    std::string actual = tree.get_root_hash();
    EXPECT_TRUE(actual == expeted_str);
}

// TEST(MPT, TrieHashChanging)
// {
//     ASSERT_EQ(0, 0);
// }

// TEST(MPT, SameNodeSameTrieHash)
// {
//     ASSERT_EQ(0, 0);
// }

// TEST(MPT, GenerateKeyProof)
// {
//     ASSERT_EQ(0, 0);
// }

// TEST(MPT, VerifyKeyProof)
// {
//     ASSERT_EQ(0, 0);
// }