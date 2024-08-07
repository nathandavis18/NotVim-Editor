#include "SyntaxHighlight.hpp"

namespace SyntaxHighlight
{
	std::vector<EditorSyntax> syntaxContents;
	uint8_t syntaxIndex = 0;

	void addSyntax(const std::vector<std::string>& filetypes, const std::vector<std::string>& keywords, const std::string& singlelineComment,
		const std::string& multilineCommentStart, const std::string& multilineCommentEnd)
	{
		syntaxContents.push_back(EditorSyntax(filetypes, keywords, singlelineComment, multilineCommentStart, multilineCommentEnd));
	}
	EditorSyntax& syntax()
	{
		return syntaxContents[syntaxIndex];
	}
	void initSyntax(const std::string_view& fName)
	{
		addSyntax(cppFiletypes, cppKeywords, "//", "/*", "*/");

		std::string extension;
		size_t extensionIndex;
		if ((extensionIndex = fName.find('.')) != std::string::npos)
		{
			extension = fName.substr(extensionIndex);
		}
		else
		{
			return; //There isn't a syntax, so we can't provide syntax highlighting. 
		}
		for (uint8_t i = 0; i < syntaxContents.size(); ++i)
		{
			for (uint8_t j = 0; j < syntaxContents[i].filematch.size(); ++j)
			{
				if (syntaxContents[i].filematch[j] == extension)
				{
					syntaxIndex = i;
					return;
				}
			}
		}
	}
}