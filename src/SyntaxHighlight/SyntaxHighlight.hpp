#pragma once
#include <vector>
#include <string>
namespace SyntaxHighlight{
	struct EditorSyntax
	{
		std::vector<std::string> filematch;
		std::vector<std::string> keywords;
		std::string singlelineComment;
		std::string multilineCommentStart;
		std::string multilineCommentEnd;
	};
	void addSyntax(const std::vector<std::string>& filetypes, const std::vector<std::string>& keywords, const std::string& singlelineComment,
		const std::string& multilineCommentStart, const std::string& multilineCommentEnd);

	std::vector<EditorSyntax>& getSyntax();

	void initSyntax();

	enum class HighlightType
	{
		Normal,
		Comment,
		MultilineComment,
		KeywordType,
		KeywordLoop,
		KeywordOther,
		String,
		Number,
		Find
	};
	struct HighlightColor
	{
		int r, g, b;
	};
	static std::vector<EditorSyntax> syntax;
}