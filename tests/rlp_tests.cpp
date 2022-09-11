#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <any>
#include <cstdint>

#include <Utils/RLP.h>
#include <Utils/RLPConstants.h>

TEST(RLP, EmptyString)
{
    std::string empty_str;
    std::string nibbles = Utils::RLP::Encode(empty_str);

    EXPECT_EQ(nibbles.size(), 2);
    EXPECT_EQ(nibbles[0], '8');
    EXPECT_EQ(nibbles[1], '0');

    std::string str = Utils::RLP::Decode(nibbles);
    ASSERT_EQ(str, "");
}

TEST(RLP, SingleLetterString)
{
    std::string nibbles = Utils::RLP::Encode(std::string("A"));

    EXPECT_EQ(nibbles.size(), 1);
    EXPECT_EQ(nibbles[0], 'A');

    std::string str = Utils::RLP::Decode(nibbles);
    ASSERT_EQ(str, "A");
}

TEST(RLP, ShortString)
{
    std::string nibbles = Utils::RLP::Encode(std::string("doge"));

    EXPECT_EQ(nibbles.size(), 6);
    EXPECT_EQ(nibbles[0], '8');
    EXPECT_EQ(nibbles[1], '4');

    EXPECT_EQ(nibbles[2], 'd');
    EXPECT_EQ(nibbles[3], 'o');

    EXPECT_EQ(nibbles[4], 'g');
    EXPECT_EQ(nibbles[5], 'e');

    std::string str = Utils::RLP::Decode(nibbles);
    ASSERT_EQ(str, "doge");
}

TEST(RLP, LongString)
{
    size_t str_size = 1024;
    std::string initial_str(str_size, 'A');
    std::string nibbles = Utils::RLP::Encode(initial_str);

    EXPECT_EQ(nibbles.size(), 6 + str_size);
    EXPECT_EQ(nibbles[0], 'b');
    EXPECT_EQ(nibbles[1], '9');

    EXPECT_EQ(nibbles[2], '0');
    EXPECT_EQ(nibbles[3], '4');
    EXPECT_EQ(nibbles[4], '0');
    EXPECT_EQ(nibbles[5], '0');

    EXPECT_EQ(nibbles[6], 'A');

    EXPECT_EQ(nibbles[nibbles.size()-2], 'A');
    EXPECT_EQ(nibbles[nibbles.size()-1], 'A');

    std::string str = Utils::RLP::Decode(nibbles);
    ASSERT_EQ(str, initial_str);
}

TEST(RLP, ReallyLongString)
{
    size_t str_size = 131126;
    std::string initial_str(str_size, 'A');
    std::string nibbles = Utils::RLP::Encode(initial_str);

    EXPECT_EQ(nibbles.size(), 8 + str_size);
    EXPECT_EQ(nibbles[0], 'b');
    EXPECT_EQ(nibbles[1], 'a');

    EXPECT_EQ(nibbles[2], '0');
    EXPECT_EQ(nibbles[3], '2');
    EXPECT_EQ(nibbles[4], '0');
    EXPECT_EQ(nibbles[5], '0');
    EXPECT_EQ(nibbles[6], '3');
    EXPECT_EQ(nibbles[7], '6');

    EXPECT_EQ(nibbles[8], 'A');
    EXPECT_EQ(nibbles[9], 'A');

    EXPECT_EQ(nibbles[nibbles.size()-2], 'A');
    EXPECT_EQ(nibbles[nibbles.size()-1], 'A');

    std::string str = Utils::RLP::Decode(nibbles);
    ASSERT_EQ(str, initial_str);
}

TEST(RLP, EmptyStringList)
{
    std::vector<std::string> list;
    std::string bytes = Utils::RLP::Encode(list);
    EXPECT_EQ(bytes[0], 'c');
    EXPECT_EQ(bytes[1], '0');

    std::vector<std::string> decoded_list = Utils::RLP::DecodeList(bytes);
    EXPECT_EQ(decoded_list.size(), 0);
}

TEST(RLP, ShortStringList)
{
    std::vector<std::string> list;
    list.emplace_back("cat");
    list.emplace_back("dog");
    std::string nibbles = Utils::RLP::Encode(list);
    EXPECT_EQ(nibbles[0], 'c');
    EXPECT_EQ(nibbles[1], '8');

    EXPECT_EQ(nibbles[2], '8');
    EXPECT_EQ(nibbles[3], '3');

    EXPECT_EQ(nibbles[4], 'c');
    EXPECT_EQ(nibbles[5], 'a');
    EXPECT_EQ(nibbles[6], 't');

    EXPECT_EQ(nibbles[7], '8');
    EXPECT_EQ(nibbles[8], '3');

    EXPECT_EQ(nibbles[9], 'd');
    EXPECT_EQ(nibbles[10], 'o');
    EXPECT_EQ(nibbles[11], 'g');

    std::vector<std::string> decoded_list = Utils::RLP::DecodeList(nibbles);
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

    std::string nibbles = Utils::RLP::Encode(list);
    EXPECT_EQ(nibbles[0], 'f');
    EXPECT_EQ(nibbles[1], '8');
    EXPECT_EQ(nibbles[2], '6');
    EXPECT_EQ(nibbles[3], '8');

    std::vector<std::string> decoded_list = Utils::RLP::DecodeList(nibbles);
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

TEST(RLP, ShortLongStringList)
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

    std::string nibbles = Utils::RLP::Encode(list);
    EXPECT_EQ(nibbles[0], 'f');
    EXPECT_EQ(nibbles[1], '8');
    EXPECT_EQ(nibbles[2], '4');
    EXPECT_EQ(nibbles[3], '4');

    std::vector<std::string> decoded_list = Utils::RLP::DecodeList(nibbles);
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

TEST(RLP, MixedLongShortStringList)
{
    std::vector<std::string> list;
    list.emplace_back("cat");
    list.emplace_back("dogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdog");

    std::string nibbles = Utils::RLP::Encode(list);
    EXPECT_EQ(nibbles[0], 'f');
    EXPECT_EQ(nibbles[1], '8');
    EXPECT_EQ(nibbles[2], '5');
    EXPECT_EQ(nibbles[3], 'a');

    std::vector<std::string> decoded_list = Utils::RLP::DecodeList(nibbles);
    EXPECT_EQ(decoded_list.size(), 2);
    EXPECT_EQ(decoded_list[0], "cat");
    EXPECT_EQ(decoded_list[1], "dogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdog");
}

TEST(RLP, DecodeAnyEmptyString)
{
    std::string str("");
    std::any any_str = std::make_any<std::string>(str);

    std::string nibbles = Utils::RLP::Encode(any_str);
    EXPECT_EQ(nibbles.size(), 2);
    EXPECT_EQ(nibbles[0], '8');
    EXPECT_EQ(nibbles[1], '0');

    std::any any_res = Utils::RLP::DecodeAny(nibbles);
    std::string res_str = std::any_cast<std::string>(any_res);
    EXPECT_EQ(str.size(), res_str.size());
}

TEST(RLP, DecodeAnyEmptyList)
{
    std::vector<std::string> vec;
    std::any any_vec = std::make_any<std::vector<std::string>>(vec);

    std::string nibbles = Utils::RLP::Encode(any_vec);
    EXPECT_EQ(nibbles.size(), 2);
    EXPECT_EQ(nibbles[0], 'c');
    EXPECT_EQ(nibbles[1], '0');

    std::any any_res = Utils::RLP::DecodeAny(nibbles);
    std::vector<std::string> res_vec = std::any_cast<std::vector<std::string>>(any_res);
    EXPECT_EQ(vec.size(), res_vec.size());
}

TEST(RLP, NestedShortList)
{
    std::vector<std::string> vec_one = { "111" };
    std::vector<std::string> vec_two = { "222" };

    std::any any_vec_one = std::make_any<std::vector<std::string>>(vec_one);
    std::any any_vec_two = std::make_any<std::vector<std::string>>(vec_two);

    std::vector<std::any> nested_any_vec;
    nested_any_vec.push_back(any_vec_one);
    nested_any_vec.push_back(any_vec_two);

    std::string nibbles = Utils::RLP::Encode(nested_any_vec);

    EXPECT_EQ(nibbles[0], 'c');
    EXPECT_EQ(nibbles[1], 'a');

    EXPECT_EQ(nibbles[2], 'c');
    EXPECT_EQ(nibbles[3], '4');

    EXPECT_EQ(nibbles[4], '8');
    EXPECT_EQ(nibbles[5], '3');

    EXPECT_EQ(nibbles[9], 'c');
    EXPECT_EQ(nibbles[10], '4');

    EXPECT_EQ(nibbles[11], '8');
    EXPECT_EQ(nibbles[12], '3');

    std::any any_res = Utils::RLP::DecodeAny(nibbles);

    std::vector<std::any> any_vec_res = std::any_cast<std::vector<std::any>>(any_res);
    std::vector<std::string> str_vec_res_one = std::any_cast<std::vector<std::string>>(any_vec_res[0]);
    std::vector<std::string> str_vec_res_two = std::any_cast<std::vector<std::string>>(any_vec_res[1]);

    EXPECT_EQ(vec_one.size(), str_vec_res_one.size());
    EXPECT_EQ(vec_one[0].size(), str_vec_res_one[0].size());
    EXPECT_EQ(vec_one[0], str_vec_res_one[0]);

    EXPECT_EQ(vec_two.size(), str_vec_res_two.size());
    EXPECT_EQ(vec_two[0].size(), str_vec_res_two[0].size());
    EXPECT_EQ(vec_two[0], str_vec_res_two[0]);
}

TEST(RLP, NestedLongList)
{
    std::vector<std::string> vec_one = { "11111111111111111111111111111111111111111111111111111111" };
    std::vector<std::string> vec_two = { "22222222222222222222222222222222222222222222222222222222" };

    std::any any_vec_one = std::make_any<std::vector<std::string>>(vec_one);
    std::any any_vec_two = std::make_any<std::vector<std::string>>(vec_two);

    std::vector<std::any> nested_any_vec;
    nested_any_vec.push_back(any_vec_one);
    nested_any_vec.push_back(any_vec_two);

    std::string nibbles = Utils::RLP::Encode(nested_any_vec);

    EXPECT_EQ(nibbles[0], 'f');
    EXPECT_EQ(nibbles[1], '8');

    EXPECT_EQ(nibbles[2], '7');
    EXPECT_EQ(nibbles[3], '8');

    EXPECT_EQ(nibbles[4], 'f');
    EXPECT_EQ(nibbles[5], '8');

    EXPECT_EQ(nibbles[6], '3');
    EXPECT_EQ(nibbles[7], 'a');

    EXPECT_EQ(nibbles[8], 'b');
    EXPECT_EQ(nibbles[9], '8');

    EXPECT_EQ(nibbles[10], '3');
    EXPECT_EQ(nibbles[11], '8');

    EXPECT_EQ(nibbles[68], 'f');
    EXPECT_EQ(nibbles[69], '8');

    EXPECT_EQ(nibbles[70], '3');
    EXPECT_EQ(nibbles[71], 'a');

    EXPECT_EQ(nibbles[72], 'b');
    EXPECT_EQ(nibbles[73], '8');

    EXPECT_EQ(nibbles[74], '3');
    EXPECT_EQ(nibbles[75], '8');

    std::any any_res = Utils::RLP::DecodeAny(nibbles);

    std::vector<std::any> any_vec_res = std::any_cast<std::vector<std::any>>(any_res);
    std::vector<std::string> str_vec_res_one = std::any_cast<std::vector<std::string>>(any_vec_res[0]);
    std::vector<std::string> str_vec_res_two = std::any_cast<std::vector<std::string>>(any_vec_res[1]);

    EXPECT_EQ(vec_one.size(), str_vec_res_one.size());
    EXPECT_EQ(vec_one[0].size(), str_vec_res_one[0].size());
    EXPECT_EQ(vec_one[0], str_vec_res_one[0]);

    EXPECT_EQ(vec_two.size(), str_vec_res_two.size());
    EXPECT_EQ(vec_two[0].size(), str_vec_res_two[0].size());
    EXPECT_EQ(vec_two[0], str_vec_res_two[0]);
}