#include "pch.hpp"

TEST(StructureParserTest, TestParsing)
{
    std::string input = "\"Test % String\"";
    std::string output;
    std::string expected;

    StructParser::Get()->ParseStructure(input, output, 1, 4);
    EXPECT_TRUE(output == expected);
}