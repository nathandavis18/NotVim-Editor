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
#include <array>

namespace SyntaxHighlight
{
	int8_t syntaxIndex = -1;
	std::array<uint8_t, static_cast<uint8_t>(HighlightType::EnumCount)> colors;
	std::vector<EditorSyntax> syntaxContents;

	/// <summary>
	/// Setting the color values for each type
	/// Color IDs correspond to the IDs found at this link: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797#:~:text=Where%20%7BID%7D%20should%20be%20replaced%20with%20the%20color%20index%20from%200%20to%20255%20of%20the%20following%20color%20table%3A
	/// </summary>
	void setColors()
	{
		colors[static_cast<uint8_t>(HighlightType::Normal)] = 255; //White
		colors[static_cast<uint8_t>(HighlightType::Comment)] = 40; //Light Green
		colors[static_cast<uint8_t>(HighlightType::MultilineComment)] = 28; //Dark Green
		colors[static_cast<uint8_t>(HighlightType::KeywordBuiltInType)] = 196; //Red
		colors[static_cast<uint8_t>(HighlightType::KeywordControl)] = 177; //Pinkish Purple
		colors[static_cast<uint8_t>(HighlightType::KeywordOther)] = 105; //Purple
		colors[static_cast<uint8_t>(HighlightType::String)] = 215; //Orange
		colors[static_cast<uint8_t>(HighlightType::Number)] = 6; //Blue
	}

	/// <summary>
	/// Initializes the syntax index and vector based on the current filetype
	/// </summary>
	/// <param name="fName"></param>
	void initSyntax(const std::string_view& fName)
	{
		syntaxContents.emplace_back(cppFiletypes, cppBuiltInTypes, cppControlKeywords, cppOtherKeywords, "//", "/*", "*/", '\\');

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
	/// Returns a pointer (or nullptr) to the current syntax being used.
	/// </summary>
	/// <returns> nullptr if no syntax, or a pointer to the correct syntax </returns>
	EditorSyntax* syntax()
	{
		return syntaxIndex >= 0 ? &syntaxContents[syntaxIndex] : nullptr;
	}

	/// <summary>
	/// Returns the color ID of a given highlight type
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	uint8_t color(HighlightType type)
	{
		return colors[static_cast<int>(type)];
	}
}