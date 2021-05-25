#include "StructParser.h"
#include "Logger.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>
#include <stack>

#include <boost/algorithm/string.hpp>

#include <fmt/format.h>

size_t ClassElement::pointer_size = 4;

#define TESTING_PARSER 1

#ifdef TESTING_PARSER
#include <iostream>
#include <fstream>
#endif


size_t StructParser::FindPragmaPack(std::string& input, size_t from, size_t& push_start)
{
	size_t packing = 1;
	size_t pos1 = input.find("#pragma pack(push, ", from);
	size_t pos2 = input.find("#pragma pack(", from);
	if(pos1 != std::string::npos && pos2 != std::string::npos)
	{
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

size_t FindEndOfHeader(std::string& input)
{
	size_t pos_pack = input.find("#pragma pack", 0);
	size_t pos_struct = input.find("typedef struct", 0);
	size_t min_pos = std::min(pos_pack, pos_struct);
	return min_pos;
}

bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

bool IsValidVariableType(std::string& type)
{
	auto it = types.find(type);
	bool ret = it != types.end();
	return ret;
}

bool StructParser::ParseElement(std::string& str_input, size_t& line_counter, ClassContainer* c)
{
	std::string str_in = str_input;// str_input.substr(line_counter, str_input.length() - line_counter);
	boost::trim_left(str_in);

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

	ClassContainer* c_exist = IsClassAlreadyExists(var_type);
	if(!IsValidVariableType(var_type) && !c_exist && !is_pointer)
	{
		LOGMSG(error, "Invalid type: {}", var_type);
		return false;
	}

	size_t array_size = std::numeric_limits<size_t>::max();
	size_t array_pos = var_name.find("[", 0);
	size_t array_pos2 = var_name.find("]", 0);
	if(array_pos != std::string::npos)
	{
		std::string sub = var_name.substr(array_pos + 1, array_pos2 - array_pos - 1);
		if(is_number(sub))
			array_size = std::stol(sub);
		else
		{
			auto it = definitions.find(sub);
			if(it != definitions.end())
				array_size = it->second;
			else
				throw fmt::format("Invalid definition {}", sub);
		}
		DBG("Array size: %d\n", array_size);
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

	if(c_exist)
	{
		c->members.insert(c->members.begin(), c_exist->members.begin(), c_exist->members.end());
	}
	else
	{
		ClassElement* e = new ClassElement(std::move(var_type), is_pointer, array_size, std::move(var_name), std::move(var_comment));
		c->members.push_back(e);
	}

	DBG("\n");
	return true;
}

namespace utils
{
	size_t find_until(std::string& str, std::string& substr, size_t offset)
	{
		size_t ret = str.find(substr, offset);
		if(ret != std::string::npos && ret > offset)
			ret = std::string::npos;
		return ret;
	}
}
/*
size_t FindStructHeader(std::string str_in, size_t offset)
{
	size_t struct_begin = 
	return ret;
}
*/
size_t FindStructEnd(std::string str_in, size_t& input_len)
{
	bool ret = false;
	size_t struct_begin = str_in.rfind("};", input_len);
	if(struct_begin != std::string::npos)
		ret = true;
	return ret;
}

void StructParser::GenerateOffsets(std::string& output, ClassContainer* c, ClassElement* e, size_t& offset)
{
	size_t elem_size = e->GetSize();
	size_t real_size = elem_size + ((c->packing - (elem_size & (c->packing - 1))) & (c->packing - 1));
	if(e->desc.empty())
		output += fmt::format("{} {} [{} - {}]\r\n", e->type_name, e->name, offset, (offset + real_size) - 1);
	else
		output += fmt::format("{} {} [{} - {}] /**< {} >\r\n", e->type_name, e->name, offset, (offset + real_size) - 1, e->desc);

	offset += real_size;
}

void StructParser::ParseStructure(std::string& input, std::string& output)
{
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	std::string str_in;
	std::string header;

	size_t header_end = FindEndOfHeader(input);
	if(header_end > 0)
	{
		header = input.substr(0, header_end);
		input.erase(0, header_end);

		boost::replace_all(header, "\t", " ");
		std::vector<std::string> defines;
		boost::split(defines, header, boost::is_any_of("\n"));
		for(auto& i : defines)
		{
			char name[32];
			int32_t val;
			int ret = sscanf(&i.c_str()[8], "%32[^ ]%d", name, &val);  /* no time for low-level solution */
			if(ret == 2)
			{
				definitions[name] = val;
			}
		}
	}
	TrimStructure(input, str_in);  /* trim structure to be able to parser it */

	size_t packing = 1;
	size_t struct_start;
	size_t push_start = 1;
	size_t pop_end = 0;
	size_t found_packing = FindPacking(str_in, 0, push_start, pop_end);
	size_t input_len = 0;


	int position = 0;
	int in_struct = 0;
	//ClassContainer* p;
	ClassContainer* old = nullptr;

	std::stack<ClassContainer*> pointer_stack;

	int try_parse_element = 0;
	int parse_ok = 0;
	while(input_len < str_in.length() - 1)
	{
#if 0
		size_t struct_start = 0;
		if(struct_start > push_start) /* struct_end < pop_end */ 
			packing = found_packing;
		else
			packing = 1;

		if(struct_start > pop_end)  /* Update packing information when whe left last pragma pop in current structure*/
			found_packing = FindPacking(input, struct_start, push_start, pop_end);
#endif
		if(str_in[input_len] == '#')
		{
			try_parse_element = 0;
			if(!str_in.compare(input_len, std::char_traits<char>::length("#pragma pack(push, "), "#pragma pack(push, "))
			{
				packing = std::stoi(&str_in[input_len + std::char_traits<char>::length("#pragma pack(push, ")]);
				size_t pos = str_in.find(')', input_len);
				input_len += pos - input_len + 1;
			}
			else if(!str_in.compare(input_len, std::char_traits<char>::length("#pragma pack()"), "#pragma pack()"))  /* end */
			{
				packing = 1;
				size_t pos = str_in.find(')', input_len);
				input_len += pos - input_len + 1;
			}
			else if(!str_in.compare(input_len, std::char_traits<char>::length("#pragma pack(pop)"), "#pragma pack(pop)"))  /* end */
			{
				packing = 1;
				size_t pos = str_in.find(')', input_len);
				input_len += pos - input_len + 1;
			}
			else if(!str_in.compare(input_len, std::char_traits<char>::length("#pragma pack("), "#pragma pack("))
			{
				packing = std::stoi(&str_in[input_len + std::char_traits<char>::length("#pragma pack(")]);
				size_t pos = str_in.find(')', input_len);
				input_len += pos - input_len + 1;
			}
			else
			{
				LOGMSG(error, "Invalid structure expression after character #");
			}
		}

		if(try_parse_element)
		{
			if(pointer_stack.empty())
			{
				try_parse_element = 0;
				continue;
			}

			ClassContainer* c = pointer_stack.top();
			parse_ok = ParseElement(str_in, input_len, c);
			if(!parse_ok)
				try_parse_element = 0;
			continue;
		}

		else if(str_in[input_len] == '}')
		{
			ClassContainer* c = pointer_stack.top();
			if(str_in[input_len + 1] != ';')  /* structure has a name */
			{
				size_t pos = str_in.find(";", input_len);

				c->type_name = str_in.substr(input_len + 2, pos - input_len - 2);
				DBG("go back 2 %s\n", c->type_name.c_str());

				input_len += (pos - input_len) + 1;
			}
			else
			{
				DBG("go back 2\n");
				input_len += 2;
			}
			try_parse_element = 1;

			pointer_stack.pop();
			
			if(pointer_stack.empty())
				classes.push_back(c);
			else
			{
				ClassContainer* prev = pointer_stack.top();
				prev->members.push_back(c);
			}
			continue;
		}

		if((struct_start = str_in.find("struct{", input_len)) != std::string::npos && !try_parse_element)
		{
			try_parse_element = 1;
			DBG("struct found %d\n", struct_start);
			
			ClassContainer* c = new ClassContainer("", packing);
			in_struct++;
			pointer_stack.push(c);

			input_len += ((struct_start - input_len)) + strlen("struct");
		}
		
		input_len++;
	}
	// std::vector<std::variant<ClassElement*, ClassContainer*>> members;

	std::stack<ClassContainer*> out_stack;
	for(auto& c : classes)
	{
		output += ":: " + c->type_name + " ::" + "\r\n\r\n";

		size_t offset = 0;
		ClassContainer* p = c;
		out_stack.push(p);
		while(p != nullptr)
		{ 
			int was_container = 0;
			auto m = p->members.begin();
			while(m != p->members.end())
			{
				if(std::holds_alternative<ClassElement*>(*m))
				{
					GenerateOffsets(output, c, std::get<ClassElement*>(*m), offset);
					//++m;
					m = p->members.erase(m);
				}
				else if(std::holds_alternative<ClassContainer*>(*m))
				{
					if(p == std::get<ClassContainer*>(*m))
						DBG("already set\n");

					ClassContainer* p_old = p;
					p = std::get<ClassContainer*>(*m);
					out_stack.push(p);
					was_container = 1;
					m = p_old->members.erase(m);
					break;
				}
			}

			if(!was_container)
			{
				ClassContainer* c = out_stack.top();
				out_stack.pop();
				if(out_stack.empty())
					p = nullptr;
				else
					p = out_stack.top();
			}

			if(out_stack.empty())
				p = nullptr;
		}

		output += fmt::format("\r\nSize: {}, Pack: {}\r\n\r\n", offset, c->packing);
	}


	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

	std::string elapsed_str = fmt::format("{} structure has been parsed in {:.6f} ms", classes.size(), (double)dif / 1000000.0);
	output += "\r\n\r\n// " + elapsed_str;
	LOGMSG(notification, elapsed_str.c_str());
	DoCleanup();
}

void StructParser::Init()
{
	static_assert(sizeof(uint8_t) == 1 && sizeof(int8_t) == 1, "Invalid uint8_t or int8_t size");
	static_assert(sizeof(uint16_t) == 2 && sizeof(int16_t) == 2, "Invalid uint16_t or int16_t size");
	static_assert(sizeof(uint32_t) == 4 && sizeof(int32_t) == 4, "Invalid uint32_t or int32_t size");
	static_assert(sizeof(uint64_t) == 8 && sizeof(int64_t) == 8 && sizeof(double) == 8, "Invalid uint64_t, int64_t or double size");

#ifdef TESTING_PARSER
	std::string in_str;
	std::string out_str;
	
	std::ifstream in("struct_in.txt", std::ios::in | std::ios::binary);
	if(in)
	{
		in.seekg(0, std::ios::end);
		in_str.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&in_str[0], in_str.size());
		in.close();
	}
	try
	{
		ParseStructure(in_str, out_str);
	}
	catch(std::exception& e)
	{
		DBG(e.what());
		DBG("\n\n");
	}
	
	std::ofstream out("struct_out.txt", std::ofstream::binary);
	out << out_str;
	out.close();
#endif
}

void StructParser::DoCleanup()
{
	
	for(auto i : classes)
	{
		delete i;
	}
	classes.clear();
	definitions.clear();
}

void StructParser::TrimStructure(std::string& str_in, std::string& str_out)
{
	int char_found = 0;
	int space_count = 0;
	boost::algorithm::replace_all(str_in, "\t", "");  /* this has one drawback: TABs can't be used between variable type and name */
	boost::algorithm::replace_all(str_in, "\r\n", "");
	for(int i = 0; i != str_in.length();)
	{
		if(str_in[i] == ' ')
		{
			size_t x = 0;
			while(x < str_in.length())
			{
				if(str_in[i + x] == ' ')
					x++;
				else
				{
					break;
				}
			}

			if(x > 1)
			{
				DBG("spaces: %d\n", x);
				i += (x - 1);
			}
			str_out.append(" ");
			i++;
		}
		else
		{
			str_out.append(1, str_in[i]);
			i++;
		}

		if(str_in.length() - i >= 2)
		{
			if(str_in[i] == ';' && str_in[i + 1] == ' ')
			{
				str_out.append(1, str_in[i]);
				i += 2;
			}
		}
		if(str_in.length() - i >= 2)
		{
			if(str_in[i] == '{' && str_in[i + 1] == ' ')
			{
				str_out.append(1, str_in[i]);
				i += 2;
			}
		}
	}
}