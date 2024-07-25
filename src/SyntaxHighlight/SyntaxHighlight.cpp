#include "SyntaxHighlight.hpp"

namespace SyntaxHighlight
{
	void addSyntax(const std::vector<std::string>& filetypes, const std::vector<std::string>& keywords, const std::string& singlelineComment,
		const std::string& multilineCommentStart, const std::string& multilineCommentEnd)
	{
		syntax.push_back(EditorSyntax(filetypes, keywords, singlelineComment, multilineCommentStart, multilineCommentEnd));
	}
	std::vector<SyntaxHighlight::EditorSyntax>& getSyntax()
	{
		return syntax;
	}
}