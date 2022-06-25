#include <gtest/gtest.h>

#include <vector>
#include <cstdint>

#include "../Utils/RLP.h"

TEST(RLP, EmptyString)
{
    std::vector<uint8_t> bytes = Utils::RLP::Encode("");

    EXPECT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 128);

    std::string str = Utils::RLP::Decode(bytes);
    ASSERT_EQ(str, "");
}

TEST(RLP, SingleLetterString)
{
    std::vector<uint8_t> bytes = Utils::RLP::Encode("A");

    EXPECT_EQ(bytes.size(), 1);
    EXPECT_EQ(bytes[0], 65);

    std::string str = Utils::RLP::Decode(bytes);
    ASSERT_EQ(str, "A");
}

TEST(RLP, ShortString)
{
    std::vector<uint8_t> bytes = Utils::RLP::Encode("doge");

    EXPECT_EQ(bytes.size(), 5);
    EXPECT_EQ(bytes[0], 132);
    EXPECT_EQ(bytes[1], 100);
    EXPECT_EQ(bytes[2], 111);
    EXPECT_EQ(bytes[3], 103);
    EXPECT_EQ(bytes[4], 101);

    std::string str = Utils::RLP::Decode(bytes);
    ASSERT_EQ(str, "doge");
}

TEST(RLP, LongString)
{
    std::string initial_str(1024, 'A');
    std::vector<uint8_t> bytes = Utils::RLP::Encode(initial_str);

    EXPECT_EQ(bytes.size(), 3 + 1024);
    EXPECT_EQ(bytes[0], 185);
    EXPECT_EQ(bytes[1], 4);
    EXPECT_EQ(bytes[2], 0);

    std::string str = Utils::RLP::Decode(bytes);
    ASSERT_EQ(str, initial_str);
}

TEST(RLP, ReallyLongString)
{
    std::string initial_str(131126, 'A');
    std::vector<uint8_t> bytes = Utils::RLP::Encode(initial_str);

    EXPECT_EQ(bytes.size(), 4 + 131126);
    EXPECT_EQ(bytes[0], 186);
    EXPECT_EQ(bytes[1], 2);
    EXPECT_EQ(bytes[2], 0);
    EXPECT_EQ(bytes[3], 54);

    std::string str = Utils::RLP::Decode(bytes);
    ASSERT_EQ(str, initial_str);
}

TEST(RLP, EmptyStringList)
{
    std::vector<std::string> list;
    std::vector<uint8_t> bytes = Utils::RLP::Encode(list);
    EXPECT_EQ(bytes[0], 192);

    std::vector<std::string> decoded_list = Utils::RLP::DecodeList(bytes);
    EXPECT_EQ(decoded_list.size(), 0);
}

TEST(RLP, ShortStringList)
{
    std::vector<std::string> list;
    list.emplace_back("cat");
    list.emplace_back("dog");
    std::vector<uint8_t> bytes = Utils::RLP::Encode(list);
    EXPECT_EQ(bytes[0], 200);
    EXPECT_EQ(bytes[1], 131);
    EXPECT_EQ(bytes[5], 131);

    std::vector<std::string> decoded_list = Utils::RLP::DecodeList(bytes);
    EXPECT_EQ(decoded_list.size(), 2);
    EXPECT_EQ(decoded_list[0], "cat");
    EXPECT_EQ(decoded_list[1], "dog");
}

TEST(RLP, LongStringList)
{
    std::vector<std::string> list;
    list.emplace_back("catcatcatcat");
    list.emplace_back("dogdogdogdog");
    list.emplace_back("catcatcatcat");
    list.emplace_back("dogdogdogdog");
    list.emplace_back("catcatcatcat");
    list.emplace_back("dogdogdogdog");
    list.emplace_back("catcatcatcat");
    list.emplace_back("dogdogdogdog");

    std::vector<uint8_t> bytes = Utils::RLP::Encode(list);
    EXPECT_EQ(bytes[0], 248);
    EXPECT_EQ(bytes[1], 104);

    std::vector<std::string> decoded_list = Utils::RLP::DecodeList(bytes);
    EXPECT_EQ(decoded_list.size(), 8);
    EXPECT_EQ(decoded_list[0], "catcatcatcat");
    EXPECT_EQ(decoded_list[1], "dogdogdogdog");
    EXPECT_EQ(decoded_list[2], "catcatcatcat");
    EXPECT_EQ(decoded_list[3], "dogdogdogdog");
    EXPECT_EQ(decoded_list[4], "catcatcatcat");
    EXPECT_EQ(decoded_list[5], "dogdogdogdog");
    EXPECT_EQ(decoded_list[6], "catcatcatcat");
    EXPECT_EQ(decoded_list[7], "dogdogdogdog");
}

TEST(RLP, MixedLongShortStringList)
{
    std::vector<std::string> list;
    list.emplace_back("cat");
    list.emplace_back("dogdogdogdog");
    list.emplace_back("cat");
    list.emplace_back("dogdogdogdog");
    list.emplace_back("cat");
    list.emplace_back("dogdogdogdog");
    list.emplace_back("cat");
    list.emplace_back("dogdogdogdog");

    std::vector<uint8_t> bytes = Utils::RLP::Encode(list);
    EXPECT_EQ(bytes[0], 248);
    EXPECT_EQ(bytes[1], 104);

    std::vector<std::string> decoded_list = Utils::RLP::DecodeList(bytes);
    EXPECT_EQ(decoded_list.size(), 8);
    EXPECT_EQ(decoded_list[0], "cat");
    EXPECT_EQ(decoded_list[1], "dogdogdogdog");
    EXPECT_EQ(decoded_list[2], "cat");
    EXPECT_EQ(decoded_list[3], "dogdogdogdog");
    EXPECT_EQ(decoded_list[4], "cat");
    EXPECT_EQ(decoded_list[5], "dogdogdogdog");
    EXPECT_EQ(decoded_list[6], "cat");
    EXPECT_EQ(decoded_list[7], "dogdogdogdog");
}

TEST(RLP, NestedLists)
{
    EXPECT_EQ(0, 1);
}