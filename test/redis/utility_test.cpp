#include <gtest/gtest.h>
#include "redis/utility.hpp"

TEST(UtilityTest, Whitespace)
{
    std::vector<std::string> correct = {"I", "forever", "love", "akane!"};
    std::string input("  I forever    love  akane!");
    auto res = redis::utility::str_split(input);
    ASSERT_EQ(correct, res);
}

TEST(UtilityTest, Newline)
{
    std::vector<std::string> correct = {"I", "forever", "love", "hiyori!"};
    std::string input("I    \nforever\n love \n hiyori!");
    auto res = redis::utility::str_split(input);
    ASSERT_EQ(correct, res);
}