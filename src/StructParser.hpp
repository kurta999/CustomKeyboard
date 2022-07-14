#pragma once

#include "utils/CSingleton.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>
#include <boost/core/noncopyable.hpp>
#include <memory>
#include <optional>

class ClassBase : private boost::noncopyable
{
public:
	ClassBase()
	{

	}
	ClassBase(std::string&& type_name_, bool is_pointer_, size_t array_size_, std::string&& name_, std::string&& desc_) :
		type_name(type_name_), is_pointer(is_pointer_), array_size(array_size_), name(name_), desc(desc_)
	{

	}
	~ClassBase()
	{

	}

public:
	std::string type_name;
	bool is_pointer = false;
	size_t array_size = std::numeric_limits<size_t>::max();  /* std::numeric_limits<size_t>::max() if isn't array */
	std::string name; /* variable name */
	std::string desc;  /* documentation */
	size_t union_size = 0; /* if not 0, this is a member of union */
};

class ClassElement : public ClassBase
{
public:
	ClassElement(std::string&& type_name_, bool is_pointer_, size_t array_size_, std::string&& name_, std::string&& desc_) :
		ClassBase(std::move(type_name_), is_pointer_, array_size_, std::move(name_), std::move(desc_))
	{

	}
	~ClassElement()
	{

	}

	size_t GetSize();

	static size_t pointer_size;
	static const std::unordered_map<std::string, size_t> types;
	static bool IsValidVariableType(std::string& type);
private:

};

class ClassContainer : public ClassBase /* holds a class and expands the further ones when a duplicated one found */
{
public:
	ClassContainer()
	{

	}
	ClassContainer(std::string type_name_, size_t packing_, bool is_pointer_ = 0, size_t array_size_ = std::numeric_limits<size_t>::max(), std::string&& name_ = "", std::string&& desc_ = "") :
		ClassBase(std::move(type_name_), is_pointer_, array_size_, std::move(name_), std::move(desc_)), packing(packing_)
	{

	}
	~ClassContainer()
	{

	}

	std::vector<std::variant<std::shared_ptr<ClassElement>, std::shared_ptr<ClassContainer>>> members;
	size_t packing = 0;
	std::vector<std::shared_ptr<ClassContainer>> classes;
	bool is_union = false;
};

class StructParser : public CSingleton < StructParser >
{
    friend class CSingleton < StructParser >;
public:
    void Init();
	void ParseStructure(std::string& input, std::string& output, uint32_t default_packing = 1, size_t ptr_size = 4);

private:
	std::optional<std::shared_ptr<ClassContainer>> IsClassAlreadyExists(std::string& class_name);
	size_t FindEndOfHeader(std::string& input);
	bool ParseElement(std::string& str_input, size_t& line_counter, std::shared_ptr<ClassContainer>& c);
	void GenerateOffsets(std::string& output, std::shared_ptr<ClassContainer>& c, std::shared_ptr<ClassElement>& e, size_t& offset);
	void PreParseStructure(std::string& input, std::string& output);
	void ConstructStuctureInMemory(std::string& input, uint32_t default_packing, size_t ptr_size);
	void GenerateOutput(std::string& output);

	void DoCleanup();
	void TrimStructure(std::string& str_in, std::string& str_out);
	
	std::unordered_map<std::string, int32_t> definitions;
	std::vector<std::shared_ptr<ClassContainer>> classes;  /* all classes in a container */

};