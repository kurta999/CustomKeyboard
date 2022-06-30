#include "pch.hpp"

TEST(EscaperTest, EscapeQuote) 
{
    std::string input = "\"Test % String\"";
    std::string output = "\\\"Test % String\\\"";
    StringEscaper escaper;
    escaper.EscapeString(input, false, false);
    EXPECT_TRUE(input == output);
}

TEST(EscaperTest, EscapeQuoteWithNewline) 
{
    std::string input = "\"Test % String\"\r\n";
    std::string output = "\\\"Test % String\\\"\r\n";
    StringEscaper escaper;
    escaper.EscapeString(input, false, false);
    EXPECT_TRUE(input == output);
}

TEST(EscaperTest, EscapeQuoteAndPercent) 
{
    std::string input = "\"Test % String\"";
    std::string output = "\\\"Test %% String\\\"";
    StringEscaper escaper;
    escaper.EscapeString(input, true, false);
    EXPECT_TRUE(input == output);
}

TEST(EscaperTest, EscapeQuoteAndPercentWithSlash) 
{
    std::string input = "\"Test % String\"\r\n";
    std::string output = "\\\"Test %% String\\\"\\\r\n";
    StringEscaper escaper;
    escaper.EscapeString(input, true, true);
    EXPECT_TRUE(input == output);
}