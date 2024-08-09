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
	void addSyntax(const std::vector<std::string>& filetypes, const std::vector<std::string>& builtInTypeKeywords, const std::vector<std::string>& classTypeKeywords, 
		const std::vector<std::string>& loopKeywords, const std::vector<std::string>& otherKeywords, 
		const std::string& singlelineComment, const std::string& multilineCommentStart, const std::string& multilineCommentEnd);

	EditorSyntax& syntax();

	void initSyntax(const std::string_view& fName);

	enum class HighlightType
	{
		Normal,
		Comment,
		MultilineComment,
		KeywordBuiltInType,
		KeywordClassType,
		KeywordLoop,
		KeywordOther,
		String,
		Number,
		EnumCount
	};

	uint8_t color(HighlightType);

	static const std::vector<std::string> cppFiletypes{ ".cpp", ".cc", ".cxx", ".hpp", ".h", ".hxx", ".hh" };
	static const std::vector<std::string> cppBuiltInTypes{
		//Types
		"int", "long", "double", "float", "char", "unsigned", "signed",
		"void", "short", "auto", "const", "bool", "enum", "nullptr",
		"struct", "class", "constexpr", "volatile", "mutable", "union", "typedef",
		"consteval", "register", "explicit", "true", "false", "virtual",

		//Other keywords
		"decltype", "sizeof", "static_cast", "dynamic_cast", "reinterpret_cast", "template", "this",
		"operator", "private", "public", "protected", "inline", "typeid", "typename", "alignas", "alignof", "export"
	};
	static const std::vector<std::string> cppClassTypes{
		"string", "vector", "array", "string_view", "atomic", "thread",
		"uint8_t", "uint16_t", "uint32_t", "uint64_t", "size_t", "int8_t", "int16_t", "int32_t", "int64_t"
	};
	static const std::vector<std::string> cppLoopKeywords{
		//Loop/Control keywords
		"for", "while", "do", "continue", "break", "if", "else", "not", "not_eq",
		"or", "or_eq", "throw", "catch", "try", "xor", "xor_eq", "goto", "return",
		"bitand", "bitor", "case",
	};
	static const std::vector<std::string> cppOtherKeywords{
		"#define", "#ifdef", "#ifndef", "#if", "defined"
	};
}