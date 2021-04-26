#include "StructParser.h"
#include "Logger.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>

#include <boost/algorithm/string.hpp>

#if 0

/**< 15 - 23 */
#endif

/* spaces after ; have to be TRIMMED */
char test_array[] = "\
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
} FirstStructure_t; \
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
";

std::unordered_map<std::string, size_t> custom_types;

bool IsDefaultType(std::string& type)
{
	auto it = types.find(type);
	return it != types.end();
}

size_t GetType(std::string& type)
{
	if(IsDefaultType(type))
		return types.at(type);
	return custom_types.at(type);
}

void CreateType(std::string& type, size_t value)
{
	if(IsDefaultType(type))
		return;
	auto it = custom_types.find(type);
	if(it == types.end())
		custom_types[type] = value;
}

#if 0
+ input, 150	0x00b07670 "typedef struct{int       a;  /**< 0 */uint8_t b; /**< 4 */uint16_t c;  /**< 5 */uint32_t d;float e;  /**< 11 */double f;  /**< 15 - 23 */} FirstS"	char[150]
#endif

std::vector<ClassContainer*> classes;  /* all classes in a container */

ClassContainer* IsClassAlreadyExists(std::string& class_name)
{
	for(auto& i : classes)
	{
		if(i->type_name == class_name)
			return i;
	}
	return NULL;
}

void ParserStructure(char* input)
{
	std::string str_in(input);

	size_t input_len = 0;
	while(input_len < str_in.length() - 1)
	{
		size_t struct_begin = str_in.find("{", input_len);
		size_t struct_end = str_in.find("}", input_len + 1);
		size_t struct_name_pos = str_in.find(";", struct_end);

		if(struct_begin == std::string::npos || struct_end == std::string::npos || struct_name_pos == std::string::npos)
		{
			LOGMSG(notification, "End of structure has been reached.");
			break;
		}

		std::string struct_name = str_in.substr(struct_end + 1, struct_name_pos - struct_end - 1);
		boost::trim(struct_name);

		ClassContainer* c = new ClassContainer(struct_name);
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

#if 0

				if(sscanf(var_name.c_str(), "%*[^[][%d]", &array_size) == 1)  /* Good old sscanf... */
				{

				}
#endif
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
					c->elems.push_back(c_exist);
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
		LOGMSG(error, "{}\n\n", c->type_name);
		size_t offset = 0;
		for(auto& e : c->elems)
		{
			if(std::holds_alternative<ClassElement*>(e))
			{
				LOGMSG(error, "{} {} [{} - {}]\n", std::get<ClassElement*>(e)->type_name, std::get<ClassElement*>(e)->name, 
					offset, offset + std::get<ClassElement*>(e)->GetSize());
				offset += std::get<ClassElement*>(e)->GetSize();
			}
			else if(std::holds_alternative<ClassContainer*>(e))
			{
				for(auto& cx : std::get<ClassContainer*>(e)->elems)
				{
					LOGMSG(error, "{} {} [{} - {}]\n", std::get<ClassElement*>(cx)->type_name, std::get<ClassElement*>(cx)->name,
						offset, offset + std::get<ClassElement*>(cx)->GetSize());
					offset += std::get<ClassElement*>(cx)->GetSize();
				}
			}
		}
		LOGMSG(error, "\n\n", c->type_name);
	}
}

void StructParser::Init()
{
	ParserStructure(test_array);
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