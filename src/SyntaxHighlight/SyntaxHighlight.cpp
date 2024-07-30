#include "SyntaxHighlight.hpp"

namespace SyntaxHighlight
{
	std::vector<EditorSyntax> syntaxContents;
	void addSyntax(const std::vector<std::string>& filetypes, const std::vector<std::string>& keywords, const std::string& singlelineComment,
		const std::string& multilineCommentStart, const std::string& multilineCommentEnd)
	{
		syntaxContents.push_back(EditorSyntax(filetypes, keywords, singlelineComment, multilineCommentStart, multilineCommentEnd));
	}
	std::vector<SyntaxHighlight::EditorSyntax>& syntax()
	{
		return syntaxContents;
	}

	void initSyntax()
	{
		const std::vector<std::string> cppFiletypes{ ".cpp", ".cc", ".cxx", ".hpp", ".h", ".hxx", ".hh" };
		const std::vector<std::string> cppKeywords{
			//Types
			"int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
			"void|", "short|", "auto|", "const|", "bool|", "enum|", "nullptr|",
			"struct", "class", "constexpr", "volatile", "mutable", "union", "typedef",
			"#define", "consteval", "register", "compl", "explicit", "true", "false", "virtual",

			//Loop/Control keywords
			"for", "while", "do", "continue", "break", "if", "else", "not", "not_eq",
			"or", "or_eq", "throw", "catch", "try", "xor", "xor_eq", "goto", "return",
			"bitand", "bitor", "case",

			//Other keywords
			"decltype", "sizeof", "static_cast", "dynamic_cast", "reinterpret_cast", "template", "this",
			"operator", "private", "pubic", "protected", "inline", "typeid", "typename", "alignas", "alignof", "export"
		};

		addSyntax(cppFiletypes, cppKeywords, "//", "/*", "*/");
	}
}