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
	std::array<uint8_t, static_cast<int>(HighlightType::EnumCount)> colors;
	void setColors();

	std::vector<EditorSyntax> syntaxContents;
	uint8_t syntaxIndex = 0;

	void addSyntax(const std::vector<std::string>& filetypes, const std::vector<std::string>& builtInTypeKeywords, const std::vector<std::string>& classTypeKeywords,
		const std::vector<std::string>& loopKeywords, const std::vector<std::string>& otherKeywords, const std::string& singlelineComment,
		const std::string& multilineCommentStart, const std::string& multilineCommentEnd)
	{
		syntaxContents.push_back(EditorSyntax(filetypes, builtInTypeKeywords, classTypeKeywords, loopKeywords, otherKeywords, singlelineComment, multilineCommentStart, multilineCommentEnd));
	}
	EditorSyntax* syntax()
	{

			return syntaxIndex < syntaxContents.size() ? &syntaxContents[syntaxIndex] : nullptr;
	}
	void initSyntax(const std::string_view& fName)
	{
		addSyntax(cppFiletypes, cppBuiltInTypes, cppClassTypes, cppControlKeywords, cppOtherKeywords, "//", "/*", "*/");

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
			for (const auto& fileType : syntaxContents[i].filematch)
			{
				if (fileType == extension)
				{
					setColors();
					syntaxIndex = i;
					return;
				}
			}
		}
	}

	/// <summary>
	/// Setting the color values for each type
	/// Color IDs correspond to the IDs found at this link: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797#:~:text=Where%20%7BID%7D%20should%20be%20replaced%20with%20the%20color%20index%20from%200%20to%20255%20of%20the%20following%20color%20table%3A
	/// 
	/// </summary>
	void setColors()
	{
		colors[static_cast<int>(HighlightType::Normal)] = 255;
		colors[static_cast<int>(HighlightType::Comment)] = 40;
		colors[static_cast<int>(HighlightType::MultilineComment)] = 28;
		colors[static_cast<int>(HighlightType::KeywordBuiltInType)] = 196;
		colors[static_cast<int>(HighlightType::KeywordClassType)] = 226;
		colors[static_cast<int>(HighlightType::KeywordControl)] = 177;
		colors[static_cast<int>(HighlightType::KeywordOther)] = 105;
		colors[static_cast<int>(HighlightType::String)] = 215;
		colors[static_cast<int>(HighlightType::Number)] = 6;
	}
	uint8_t color(HighlightType type)
	{
		return colors[static_cast<int>(type)];
	}
}