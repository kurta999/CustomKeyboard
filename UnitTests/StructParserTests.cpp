#include "pch.hpp"

TEST(StructParserTest, BasicStructureWithMoreTypes)
{
    std::string input = "#define MAX_ITEMS		10\r\n\
typedef struct\r\n\
{\r\n\
    float x;\r\n\
    float y;\r\n\
    float z;\r\n\
    int a[MAX_ITEMS];\r\n\
    uint8_t b;\r\n\
    int8_t b_;\r\n\
    uint16_t c;\r\n\
    int16_t c_;\r\n\
    uint32_t d;\r\n\
    int32_t d_;\r\n\
    uint64_t d;\r\n\
    int64_t d_;\r\n\
    double e;\r\n\
    char* ptr;\r\n\
} Vector3;\r\n";
    std::string output;
    StructParser::Get()->ParseStructure(input, output, 1, 8);
    EXPECT_TRUE(output.find("float x [0 - 3]") != std::string::npos);
    EXPECT_TRUE(output.find("float y [4 - 7]") != std::string::npos);
    EXPECT_TRUE(output.find("float z [8 - 11]") != std::string::npos);
    EXPECT_TRUE(output.find("int a[MAX_ITEMS] [12 - 51]") != std::string::npos);
    EXPECT_TRUE(output.find("uint8_t b [52 - 52]") != std::string::npos);
    EXPECT_TRUE(output.find("int8_t b_ [53 - 53]") != std::string::npos);
    EXPECT_TRUE(output.find("uint16_t c [54 - 55]") != std::string::npos);
    EXPECT_TRUE(output.find("int16_t c_ [56 - 57]") != std::string::npos);
    EXPECT_TRUE(output.find("uint32_t d [58 - 61]") != std::string::npos);
    EXPECT_TRUE(output.find("int32_t d_ [62 - 65]") != std::string::npos);
    EXPECT_TRUE(output.find("uint64_t d [66 - 73]") != std::string::npos);
    EXPECT_TRUE(output.find("int64_t d_ [74 - 81]") != std::string::npos);
    EXPECT_TRUE(output.find("double e [82 - 89]") != std::string::npos);
    EXPECT_TRUE(output.find("char* ptr [90 - 97]") != std::string::npos);
    EXPECT_TRUE(output.find("Size: 98, Pack: 1") != std::string::npos);
}

TEST(StructParserTest, EmbeddedStructure)
{
    std::string input = "typedef struct\r\n\
{\r\n\
	float a;\r\n\
	float b;\r\n\
	float c;\r\n\
	struct\r\n\
	{\r\n\
		int d;\r\n\
		uint8_t e;\r\n\
		uint64_t f;\r\n\
		uint16_t modbus_reg[5];\r\n\
		struct\r\n\
		{\r\n\
			float triple1;\r\n\
			float triple2;\r\n\
			float triple3;\r\n\
			struct\r\n\
			{\r\n\
				double quad1;\r\n\
				double quad2;\r\n\
				double quad3;\r\n\
			};\r\n\
			float triple4;\r\n\
		};\r\n\
\r\n\
		struct\r\n\
		{\r\n\
			int x;\r\n\
			int y;\r\n\
			int z;\r\n\
		} double_embedded;\r\n\
	};\r\n\
\r\n\
	int test_int;\r\n\
	struct\r\n\
	{\r\n\
		char password[32];\r\n\
	} wifi;\r\n\
} a;\r\n\r\n";
    std::string output;
    StructParser::Get()->ParseStructure(input, output, 1, 8);
    EXPECT_TRUE(output.find("float a [0 - 3]") != std::string::npos);
    EXPECT_TRUE(output.find("float b [4 - 7]") != std::string::npos);
    EXPECT_TRUE(output.find("float c [8 - 11]") != std::string::npos);
    EXPECT_TRUE(output.find("int d [12 - 15]") != std::string::npos);
    EXPECT_TRUE(output.find("uint8_t e [16 - 16]") != std::string::npos);
    EXPECT_TRUE(output.find("uint64_t f [17 - 24]") != std::string::npos);
    EXPECT_TRUE(output.find("uint16_t modbus_reg[5] [25 - 34]") != std::string::npos);
    EXPECT_TRUE(output.find("int test_int [87 - 90]") != std::string::npos);
    EXPECT_TRUE(output.find("char password[32] [91 - 122]") != std::string::npos);
    EXPECT_TRUE(output.find("Size: 123, Pack: 1") != std::string::npos);
}

TEST(StructParserTest, TestVoidPointer)
{
    std::string input = "typedef struct\r\n\
{\r\n\
    void* p;\r\n\
    float x;\r\n\
} Vector3;\r\n";
    std::string output;
    StructParser::Get()->ParseStructure(input, output, 1, 8);
    EXPECT_TRUE(output.find("void* p [0 - 7]") != std::string::npos);
    EXPECT_TRUE(output.find("float x [8 - 11]") != std::string::npos);
    EXPECT_TRUE(output.find("Size: 12, Pack: 1") != std::string::npos);
}

/* float a [0 - 3]
float b [4 - 7]
float c [8 - 11]
int d [12 - 15]
uint8_t e [16 - 16]
uint64_t f [17 - 24]
uint16_t modbus_reg[5] [25 - 34]
float triple1 [35 - 38]
float triple2 [39 - 42]
float triple3 [43 - 46]
double quad1 [47 - 54]
double quad2 [55 - 62]
double quad3 [63 - 70]
float triple4 [71 - 74]
int x [75 - 78]
int y [79 - 82]
int z [83 - 86]
int test_int [87 - 90]
char password[32] [91 - 122]*/