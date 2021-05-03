#include "StructParser.h"
#include "Logger.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

size_t ClassElement::pointer_size = 4;

#define TESTING_PARSER 1

#ifdef TESTING_PARSER

#include <iostream>
#include <fstream>

/* spaces after ; have to be TRIMMED */
char test_array[] = "\
typedef struct\
{\
float x;\
float y;\
float z;\
} Vector3;\
\
#pragma pack(4)\
typedef struct\
{\
int      a;   \
uint8_t b;   \
uint16_t c;  \
uint32_t d;\
float e;\
double f;\
uint64_t items[15];  /**< 23 - 143 */\
void* g;  /**< 143 - 147 */\
Vector3 vec; /**< vector3D */\
} FirstStructure_t; \
#pragma pack()\
\
typedef struct\
{\
int a2;\
uint8_t b2;  /**< 23 - 143 */\
uint16_t c2;  /**< 24 - 144 */\
uint32_t d2;\
float e2;\
double f2;\
int8_t array_of_items[15];\
} SecondStructure_t;\
\
typedef struct\
{\
double a4;\
float z;\
FirstStructure_t a;\
SecondStructure_t b;\
uint16_t c;\
} EmbeddedStruct;\
\
#pragma pack(push, 8)\
typedef struct\
{\
float x2;\
float y2;\
float z2;\
} Pack8;\
#pragma pack(pop)\
";

/*
Vector3
FirstStructure_t
- Vector3
SecondStructure_t

EmbeddedStruct
- FirstStructure_t
+++ Vector3
- SecondStructure_t
 */
#endif

size_t StructParser::FindPragmaPack(std::string& input, size_t from, size_t& push_start)
{
	size_t packing = 1;
	size_t pos1 = input.find("#pragma pack(push, ", from);
	size_t pos2 = input.find("#pragma pack(", from);
	if(pos1 <= pos2 && input[pos1 + 1] == 'p')
	{
		packing = std::stoi(&input[pos1 + std::char_traits<char>::length("#pragma pack(push, ")]);
		push_start = pos1;
	}
	else
	{
		packing = std::stoi(&input[pos2 + std::char_traits<char>::length("#pragma pack(")]);
		push_start = pos2;
	}
	return packing;
}

size_t StructParser::FindPragmaPop(std::string& input, size_t from, size_t to)
{
	size_t packing = 1;
	size_t pos = input.find("#pragma pack()", from);
	if(pos != std::string::npos)
	{

	}
	else
	{
		pos = input.find("#pragma pack(pop, ", from);
	}
	return pos;
}

size_t StructParser::FindPacking(std::string& input, size_t from, size_t& push_start, size_t& pop_end)
{
	size_t pack = FindPragmaPack(input, from, push_start);
	pop_end = FindPragmaPop(input, from, 0);
	if(pop_end <= push_start)
	{
		LOGMSG(error, "Invalid packing format, pop is before push.");
		pack = 1;
	}
	return pack;
}

void StructParser::ParseStructure(std::string& input, std::string& output)
{
	std::string& str_in = input;

	size_t packing = 1;
	size_t push_start = 1;
	size_t pop_end = 0;
	size_t found_packing = FindPacking(input, 0, push_start, pop_end);
	size_t input_len = 0;
	while(input_len < str_in.length() - 1)
	{
		size_t struct_begin = str_in.find("{", input_len);
		size_t struct_end = str_in.find("}", input_len + 1);
		size_t struct_name_pos = str_in.find(";", struct_end);

		if(struct_begin > push_start && struct_end < pop_end)
			packing = found_packing;
		else
			packing = 1;

		if(struct_begin > pop_end)  /* Update packing information when whe left last pragma pop in current structure*/
			found_packing = FindPacking(input, struct_begin, push_start, pop_end);

		if(struct_begin == std::string::npos || struct_end == std::string::npos || struct_name_pos == std::string::npos)
		{
			LOGMSG(notification, "End of structure has been reached.");
			break;
		}

		std::string struct_name = str_in.substr(struct_end + 1, struct_name_pos - struct_end - 1);
		boost::trim(struct_name);

		ClassContainer* c = new ClassContainer(struct_name, packing);
		classes.push_back(c);

		size_t line_counter = struct_begin;
		while(line_counter < struct_end - 1)
		{
			if(struct_begin != std::string::npos && struct_end != std::string::npos)
			{
				size_t trim_space_pos = str_in.find_first_not_of(" ", line_counter);
				size_t semicolon_pos = str_in.find(";", trim_space_pos);
				size_t first_space_pos = str_in.find_first_of(" ", trim_space_pos);
				size_t last_space_pos = str_in.find_last_of(" ", semicolon_pos);

				std::string var_type;
				if((str_in[trim_space_pos] == '{'))
					var_type = str_in.substr(trim_space_pos + 1, first_space_pos - trim_space_pos - 1);
				else
					var_type = str_in.substr(trim_space_pos, first_space_pos - trim_space_pos);

				std::string var_name = str_in.substr(last_space_pos + 1, semicolon_pos - last_space_pos - 1);

				bool is_pointer = var_type.find("*", 0) != std::string::npos || var_name.find("*", 0) != std::string::npos;
				if(is_pointer)
					DBG("Pointer!!!");

				size_t array_size = std::numeric_limits<size_t>::max();
				size_t array_pos = var_name.find("[", 0);
				size_t array_pos2 = var_name.find("]", 0);
				if(array_pos != std::string::npos)
				{
					//std::string sub = var_name.substr(, array_pos2 - array_pos);
					array_size = std::stoi(&var_name[array_pos + 1]);
					DBG("Array size: %d", array_size);
				}


				size_t doxygen_style_comment_start = str_in.find_first_of("/**<", semicolon_pos);
				size_t doxygen_style_comment_end = str_in.find("*/", doxygen_style_comment_start);
				std::string var_comment;

				DBG("type: %s, var: %s", var_type.c_str(), var_name.c_str());
				if((doxygen_style_comment_start != std::string::npos && doxygen_style_comment_end != std::string::npos) &&
					(doxygen_style_comment_start - semicolon_pos < 5))
				{
					line_counter = doxygen_style_comment_end + 2;
					var_comment = str_in.substr(doxygen_style_comment_start + 5, doxygen_style_comment_end - doxygen_style_comment_start - 1 - 5);
					DBG(", comment: %s", var_comment.c_str());
				}
				else
				{
					line_counter = semicolon_pos + 1;
				}

				ClassContainer* c_exist = IsClassAlreadyExists(var_type);
				if(c_exist)
				{
					c->elems.insert(c->elems.begin(), c_exist->elems.begin(), c_exist->elems.end());
				}
				else
				{
					ClassElement* e = new ClassElement(std::move(var_type), is_pointer, array_size, std::move(var_name), std::move(var_comment));
					c->elems.push_back(e);
				}

				DBG("\n");
			}
		}
		input_len = struct_end;
		DBG("complete\n");
	}

	for(auto& c : classes)
	{
		output += ":: " + c->type_name + " ::" + "\r\n\r\n";

		size_t offset = 0;
		for(auto& e : c->elems)
		{
			size_t real_size = c->packing > 1 ? (e->GetSize() + (c->packing - e->GetSize())) : e->GetSize();
			if(e->desc.empty())
				output += fmt::format("{} {} [{} - {}]\r\n", e->type_name, e->name, offset, (offset + real_size) - 1);
			else
				output += fmt::format("{} {} [{} - {}] /**< {} >\r\n", e->type_name, e->name, offset, (offset + real_size) - 1, e->desc);

			offset += real_size;
		}
		output += fmt::format("\r\nSize: {}, Pack: {}\r\n\r\n", offset, c->packing);
	}
}

void StructParser::Init()
{
#ifdef TESTING_PARSER
	std::string in_str(test_array);
	std::string out_str;
	ParseStructure(in_str, out_str);

	std::ofstream out("output.txt", std::ofstream::binary);
	out << out_str;
	out.close();
#endif
}



#if 0
typedef struct
{
	int a;  /**< 0 */
	uint8_t b; /**< 4 */
	uint16_t c;  /**< 5 */
	uint32_t d;  /**< 7 */
	float e;  /**< 11 */
	double f;  /**< 15 - 23 */
} FirstStructure_t;

typedef struct
{
	int a2;
	uint8_t b2;
	uint16_t c2;
	uint32_t d2;
	float e2;
	double f2;
	int8_t array_of_items[15];
} SecondStructure_t;

typedef struct
{
	double a4;
	float b4;
	FirstStructure_t a;
	SecondStructure_t b;
	uint16_t c;
} EmbeddedStruct;
#endif