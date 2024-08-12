/**
* MIT License

Copyright (c) 2024 Nathan Davis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <vector>
#include <array>
#include <string>
#include <cstdint> //uint8_t

namespace SyntaxHighlight{
	struct EditorSyntax
	{
		std::vector<std::string> filematch;
		std::vector<std::string> builtInTypeKeywords;
		std::vector<std::string> classTypeKeywords;
		std::vector<std::string> loopKeywords;
		std::vector<std::string> otherKeywords;
		std::string singlelineComment;
		std::string multilineCommentStart;
		std::string multilineCommentEnd;
	};
	void addSyntax(const std::vector<std::string>& filetypes, const std::vector<std::string>& = std::vector<std::string>(), const std::vector<std::string>&  = std::vector<std::string>(),
		const std::vector<std::string>&  = std::vector<std::string>(), const std::vector<std::string>& = std::vector<std::string>(),
		const std::string & = std::string{}, const std::string & = std::string{}, const std::string & = std::string{});

	EditorSyntax* syntax();

	void initSyntax(const std::string_view& fName);

	enum class HighlightType
	{
		Normal,
		Comment,
		MultilineComment,
		KeywordBuiltInType,
		KeywordClassType,
		KeywordControl,
		KeywordOther,
		String,
		Number,
		EnumCount
	};

	uint8_t color(HighlightType);

	static const std::vector<std::string> cppFiletypes{ ".cpp", ".cc", ".cxx", ".hpp", ".h", ".hxx", ".hh" };
	static const std::vector<std::string> cppBuiltInTypes{
		//Built-in types and main keywords
		"alignas", "alignof", "asm", "_asm", "auto", "bool", "char", "char8_t", "char16_t", "char32_t", "class",
		"compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast", "decltype", "delete", "double",
		"dynamic_cast", "enum", "explicit", "export", "extern", "false", "float", "friend", "inline", "int", "long",
		"mutable", "namespace", "new", "noexcept", "nullptr", "operator", "private", "protected", "public", "register",
		"reinterpret_cast", "requires", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
		"template", "this", "thread_local", "true", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual",
		"void", "volatile", "wchar_t"
	};
	static const std::vector<std::string> cppClassTypes{ //Nowhere near a complete list, just some that I have used and some extra ones that I know about
		//Sequence containers
		"array", "vector", "inplace_vector", "deque", "forward_list", "list",

		//Associative containers
		"set", "map", "multiset", "multimap",

		//Unordered associative containers
		"unordered_set", "unordered_map", "unordered_multiset", "unordered_multimap", 

		//Container adapters
		"stack", "queue", "priority_queue", "flat_set", "flat_multiset", "flat_multimap",

		//Views
		"span", "mdspan",

		//String stuff
		"string", "string_view", "basic_string", "basic_string_view", "u8string", "u16string", "u32string", "wstring",
		"u8string_view", "u16string_view", "u32string_view", "wstring_view"

		//File stuff
		"basic_filebuf", "filebuf", "basic_ifstream", "ifstream", "basic_ofstream", "ofstream", "basic_fstream", "fstream",
		"ios_base", "basic_ios", "ios", "wios", "fpos", "io_errc", "is_error_code_enum", "streamoff", "streamsize",
		"basic_istream", "istream", "wistream", "basic_iostream", "iostream", "wiostream", "basic_ostream", "ostream", "wostream",
		"basic_stringbuf", "basic_istringstream", "basic_ostringstream", "basic_stringstream", "stringbuf", "wstringbuf", "istringstream",
		"wistringstream", "ostringstream", "wostringstream", "stringstream", "wstringstream",
		"path", "filesystem_error", "directory_entry", "directory_iterator", "recursive_directory_iterator", "file_status", "space_info",
		"file_type", "perms", "perm_options", "copy_options", "directory_options", "file_time_type",

		//Others
		"any", "bitset", "expected", "initializer_list", "optional", "tuple", "type_index", "variant",
		"unique_ptr", "shared_ptr", "weak_ptr", "memory_resource", "pointer_traits", "pointer_safety",
		"allocator", "allocator_traits", "allocation_result", "uses_allocator",
		"auto_ptr", "atomic", "hash",
		"int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t",
		"int_fast8_t", "uint_fast8_t", "int_fast16_t", "uint_fast16_t", "int_fast32_t", "uint_fast32_t",
		"int_fast64_t", "uint_fast64_t", "int_least8_t", "uint_least8_t", "int_least16_t", "uint_least16_t",
		"int_least32_t", "uint_least32_t", "int_least64_t", "uint_least64_t", "uintmax_t", "uintptr_t", "intmax_t", "intptr_t",
		"exception", "thread"
	};
	static const std::vector<std::string> cppControlKeywords{
		//Loop/Control keywords
		"and", "and_eq", "bitand", "bitor", "break", "case", "catch", "continue", "co_await", "co_return", "co_yield", "default",
		"do", "else", "for", "goto", "if", "not", "not_eq", "or", "or_eq", "return", "switch", "throw", "try", "while", "xor", "xor_eq"
	};
	static const std::vector<std::string> cppOtherKeywords{
		//Some other keywords, such as macro definitions
		"#define", "#ifdef", "#ifndef", "#if", "defined"
	}; 
}