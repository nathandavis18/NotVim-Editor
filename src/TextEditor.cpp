#include "TextEditor.hpp"
#include <string>
namespace Editor
{
	int editorReadKey(int fileDescriptor)
	{
		while (std::cin)
		{
			return 0;
		}
	}

	bool getCursorPosition(int* rows, int* cols)
	{
		return true;
	}

	bool isSeparator(char c)
	{
		return c == '\0' || std::isspace(c) || (std::strchr(",.()+-/*=~[];", c) != 0);
	}

	bool rowHasOpenComment(File::FileRow* row)
	{
		using HT = SyntaxHighlight::HighlightType;
		if (row->highlight && row->renderedSize && row->highlight[row->renderedSize - 1] == static_cast<int>(HT::MultilineComment) &&
			(row->renderedSize < 2 || (row->render[row->renderedSize - 2] != '*' || row->render[row->renderedSize - 1] != '/'))) 
			return true;
		return false;
	}
}
