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