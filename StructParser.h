#pragma once

#include "utils/CSingleton.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>

const std::unordered_map < std::string, size_t> types =
{
	{"uint8_t", sizeof(uint8_t), },
	{"int8_t", sizeof(int8_t)},
	{"uint16_t", sizeof(uint16_t)},
	{"int16_t", sizeof(int16_t)},
	{"uint32_t", sizeof(uint32_t)},
	{"int32_t", sizeof(int32_t)},
	{"int", sizeof(int32_t)},
	{"uint64_t", sizeof(uint64_t)},
	{"int64_t", sizeof(int64_t)},
	{"float", sizeof(float)},
	{"double", sizeof(double)},
	{"p", 4}, /* TODO: this can change */
};


class ClassBase
{
public:
	ClassBase(std::string&& type_name_, bool is_pointer_, size_t array_size_, std::string&& name_, std::string&& desc_) :
		type_name(type_name_), is_pointer(is_pointer_), array_size(array_size_), name(name_), desc(desc_)
	{

	}
public:
	std::string type_name; /* TODO: improve this in the future, it's waste of memory */
	bool is_pointer;
	size_t array_size;  /* std::numeric_limits<size_t>::max() if isn't array */
	std::string name; /* variable name */
	std::string desc;  /* documentation */
};

class ClassElement : public ClassBase
{
public:
	ClassElement(std::string&& type_name_, bool is_pointer_, size_t array_size_, std::string&& name_, std::string&& desc_) :
		ClassBase(std::move(type_name_), is_pointer_, array_size_, std::move(name_), std::move(desc_))
	{

	}

	size_t GetSize()
	{
		size_t ret = 0;
		if(is_pointer)
			ret = types.at("p");
		else
			ret = types.at(type_name);

		if(array_size != std::numeric_limits<size_t>::max())
			ret *= array_size;
		return ret;
	}
};
#if 0
class Class  /* holds the structure elements itself */
{
public:
	size_t offset;  /* starting offset of this structure */

	elements;
};
#endif

class ClassContainer : public ClassBase /* holds a class and expands the further ones when a duplicated one found */
{
public:
	ClassContainer(std::string type_name_, bool is_pointer_ = 0, size_t array_size_ = std::numeric_limits<size_t>::max(), std::string&& name_ = "", std::string&& desc_ = "") :
		ClassBase(std::move(type_name_), is_pointer_, array_size_, std::move(name_), std::move(desc_))
	{

	}

	std::vector<std::variant<ClassElement*, ClassContainer*>> elems;
};

class StructParser : public CSingleton < StructParser >
{
    friend class CSingleton < StructParser >;
public:
    void Init();
    void ProcessIncommingData(char* recv_data, const char* from_ip);


private:
};
