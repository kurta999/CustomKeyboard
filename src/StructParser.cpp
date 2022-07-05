#include "pch.hpp"

size_t ClassElement::pointer_size = 4;

size_t FindEndOfHeader(std::string& input)
{
	size_t pos_pack = input.find("#pragma pack", 0);
	size_t pos_struct = input.find("typedef struct", 0);
	size_t min_pos = std::min(pos_pack, pos_struct);
	return min_pos;
}

bool IsValidVariableType(std::string& type)
{
	auto it = types.find(type);
	bool ret = it != types.end();
	return ret;
}

bool StructParser::ParseElement(std::string& str_input, size_t& line_counter, std::shared_ptr<ClassContainer>& c)
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

	std::optional<std::shared_ptr<ClassContainer>> c_exist = IsClassAlreadyExists(var_type);
	if(!IsValidVariableType(var_type) && c_exist == std::nullopt && !is_pointer)
	{
		//LOG(LogLevel::Error, "Invalid type: {}", var_type);
		return false;
	}

	size_t array_size = std::numeric_limits<size_t>::max();
	size_t array_pos = var_name.find("[", 0);
	if(size_t array_pos2 = var_name.find("]", 0); array_pos2 != std::string::npos)
	{
		std::string sub = var_name.substr(array_pos + 1, array_pos2 - array_pos - 1);
		if(utils::is_number(sub))
			array_size = utils::stoi<size_t>(sub);
		else
		{
			auto it = definitions.find(sub);
			if(it != definitions.end())
				array_size = it->second;
			else
				throw std::runtime_error(std::format("Invalid definition {}", sub));
		}
		DBG("Array size: %zu\n", array_size);
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

	if(c_exist != std::nullopt)
	{
		c->members.insert(c->members.begin(), c_exist->get()->members.begin(), c_exist->get()->members.end());
	}
	else
	{
		c->members.push_back(std::make_shared<ClassElement>(std::move(var_type), is_pointer, array_size, std::move(var_name), std::move(var_comment)));
	}

	DBG("\n");
	return true;
}

size_t FindStructEnd(std::string str_in, size_t& input_len)
{
	bool ret = false;
	size_t struct_begin = str_in.rfind("};", input_len);
	if(struct_begin != std::string::npos)
		ret = true;
	return ret;
}

void StructParser::GenerateOffsets(std::string& output, std::shared_ptr<ClassContainer>& c, std::shared_ptr<ClassElement>& e, size_t& offset)
{
	size_t elem_size = e->GetSize();
	size_t real_size = 0;
	if(e->union_size == 0)
		real_size = elem_size + ((c->packing - (elem_size & (c->packing - 1))) & (c->packing - 1));
	else
		real_size = e->union_size;

	if(e->desc.empty())
		output += std::format("{} {} [{} - {}]\r\n", e->type_name, e->name, offset, (offset + real_size) - 1);
	else
		output += std::format("{} {} [{} - {}] /**< {} >\r\n", e->type_name, e->name, offset, (offset + real_size) - 1, e->desc);

	if(e->union_size == 0)
		offset += real_size;
}

void StructParser::ParseStructure(std::string& input, std::string& output, uint32_t default_packing, size_t ptr_size)
{
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
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

	ClassElement::pointer_size = ptr_size;
	size_t packing = default_packing;
	size_t push_start = 1;
	size_t input_len = 0;
	std::shared_ptr<ClassContainer> old = nullptr;
	std::stack<std::shared_ptr<ClassContainer>> pointer_stack;

	bool try_parse_element = 0;
	while(input_len < str_in.length() - 1)
	{
		if(str_in[input_len] == '#')
		{
			try_parse_element = 0;
			if(!str_in.compare(input_len, std::char_traits<char>::length("#pragma pack(push, "), "#pragma pack(push, "))
			{
				packing = utils::stoi<size_t>(&str_in[input_len + std::char_traits<char>::length("#pragma pack(push, ")]);
				size_t pos = str_in.find(')', input_len);
				input_len += pos - input_len + 1;
			}
			else if(!str_in.compare(input_len, std::char_traits<char>::length("#pragma pack()"), "#pragma pack()"))  /* end */
			{
				packing = default_packing;
				size_t pos = str_in.find(')', input_len);
				input_len += pos - input_len + 1;
			}
			else if(!str_in.compare(input_len, std::char_traits<char>::length("#pragma pack(pop)"), "#pragma pack(pop)"))  /* end */
			{
				packing = default_packing;
				size_t pos = str_in.find(')', input_len);
				input_len += pos - input_len + 1;
			}
			else if(!str_in.compare(input_len, std::char_traits<char>::length("#pragma pack("), "#pragma pack("))
			{
				packing = utils::stoi<size_t>(&str_in[input_len + std::char_traits<char>::length("#pragma pack(")]);
				size_t pos = str_in.find(')', input_len);
				input_len += pos - input_len + 1;
			}
			else
			{
				LOG(LogLevel::Error, "Invalid structure expression after character #");
			}
		}

		if(try_parse_element)
		{
			if(pointer_stack.empty())
			{
				try_parse_element = 0;
				continue;
			}

			std::shared_ptr<ClassContainer> c = pointer_stack.top();
			bool parse_ok = ParseElement(str_in, input_len, c);
			if(!parse_ok)
				try_parse_element = 0;
			continue;
		}

		else if(str_in[input_len] == '}')
		{
			if(pointer_stack.empty())
			{
				DBG("Invalid structure format.");
				return;
			}

			std::shared_ptr<ClassContainer> c = pointer_stack.top();
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

			if(c->is_union)
			{
				DBG("union - %s\n", c->type_name.c_str());
				size_t offset = 0;
				std::shared_ptr<ClassContainer> p;
				size_t biggest_element = 0;					
				p = c;
				while(p != nullptr)
				{
					auto m = p->members.begin();
					while(m != p->members.end())
					{
						if(std::holds_alternative<std::shared_ptr<ClassElement>>(*m))
						{
							std::shared_ptr<ClassElement> e = std::get<std::shared_ptr<ClassElement>>(*m);
							if(e->GetSize() > biggest_element)
								biggest_element = e->GetSize();
							++m;
						}
						else if(std::holds_alternative<std::shared_ptr<ClassContainer>>(*m))
						{
							classes.clear();
							//wxMessageBox("Please remove the object from union.", "Object support in union is not implemented yet!");
							break;
						}
					}

					m = p->members.begin();
					while(m != p->members.end())
					{
						if(std::holds_alternative<std::shared_ptr<ClassElement>>(*m))
						{
							std::shared_ptr<ClassElement> e = std::get<std::shared_ptr<ClassElement>>(*m);
							e->union_size = biggest_element;
							++m;
						}
					}
					p = nullptr;
				}
			}
			try_parse_element = 1;

			pointer_stack.pop();
			
			if(pointer_stack.empty())
				classes.push_back(c);
			else
			{
				std::shared_ptr<ClassContainer> prev = pointer_stack.top();
				prev->members.push_back(c);
			}
			continue;
		}

		size_t struct_start = str_in.find("struct{", input_len);
		size_t union_start = str_in.find("union{", input_len);
		if(struct_start < union_start && struct_start != std::string::npos && !try_parse_element)
		{
			try_parse_element = 1;
			DBG("struct found %zu\n", struct_start);
			
			std::shared_ptr<ClassContainer> c = std::make_shared<ClassContainer>("", packing);
			pointer_stack.push(c);

			input_len += ((struct_start - input_len)) + std::char_traits<char>::length("struct");
		}
		else if(union_start < struct_start && union_start != std::string::npos && !try_parse_element)
		{
			try_parse_element = 1;
			DBG("union found %zu\n", union_start);
			
			std::shared_ptr<ClassContainer> c = std::make_shared<ClassContainer>("", packing);
			c->is_union = 1;
			pointer_stack.push(c);

			input_len += ((union_start - input_len)) + std::char_traits<char>::length("union");
		}
		
		input_len++;
	}

	std::stack<std::shared_ptr<ClassContainer>> out_stack;
	for(auto& c : classes)
	{
		output += ":: " + c->type_name + " ::" + "\r\n\r\n";

		size_t offset = 0;
		std::shared_ptr<ClassContainer> p = c;
		out_stack.push(p);
		while(p != nullptr)
		{ 
			int was_container = 0;
			auto m = p->members.begin();
			while(m != p->members.end())
			{
				if(std::holds_alternative<std::shared_ptr<ClassElement>>(*m))
				{
					std::shared_ptr<ClassElement> e = std::get<std::shared_ptr<ClassElement>>(*m);
					GenerateOffsets(output, c, e, offset);
					m = p->members.erase(m);
					if(p->members.size() == 0)
					{
						if(e->union_size != 0)
							offset += e->union_size;
					}
				}
				else if(std::holds_alternative<std::shared_ptr<ClassContainer>>(*m))
				{
					if(p == std::get<std::shared_ptr<ClassContainer>>(*m))
					{
						DBG("already set\n");
					}

					std::shared_ptr<ClassContainer> p_old = p;
					p = std::get<std::shared_ptr<ClassContainer>>(*m);
					out_stack.push(p);
					was_container = 1;
					m = p_old->members.erase(m);
					break;
				}
			}

			if(!was_container)
			{
				std::shared_ptr<ClassContainer> c = out_stack.top();
				out_stack.pop();
				if(out_stack.empty())
					p = nullptr;
				else
					p = out_stack.top();
			}

			if(out_stack.empty())
				p = nullptr;
		}

		output += std::format("\r\nSize: {}, Pack: {}\r\n\r\n", offset, c->packing);
	}

	std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
	int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

	std::string elapsed_str = std::format("{} structure has been parsed in {:.6f} ms", classes.size(), (double)dif / 1000000.0);
	output += "\r\n\r\n// " + elapsed_str;
	LOG(LogLevel::Notification, elapsed_str.c_str());
	DoCleanup();
}

void StructParser::Init()
{
	static_assert(sizeof(uint8_t) == 1 && sizeof(int8_t) == 1, "Invalid uint8_t or int8_t size");
	static_assert(sizeof(uint16_t) == 2 && sizeof(int16_t) == 2, "Invalid uint16_t or int16_t size");
	static_assert(sizeof(uint32_t) == 4 && sizeof(int32_t) == 4, "Invalid uint32_t or int32_t size");
	static_assert(sizeof(uint64_t) == 8 && sizeof(int64_t) == 8 && sizeof(double) == 8, "Invalid uint64_t, int64_t or double size");
}

void StructParser::DoCleanup()
{
	classes.clear();
	definitions.clear();
}

void StructParser::TrimStructure(std::string& str_in, std::string& str_out)
{
	int char_found = 0;
	int space_count = 0;
	boost::algorithm::replace_all(str_in, "\t", "");  /* this has one drawback: TABs can't be used between variable type and name */
	boost::algorithm::replace_all(str_in, "\r\n", "");
	for(size_t i = 0; i != str_in.length();)
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
				DBG("spaces: %zu\n", x);
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