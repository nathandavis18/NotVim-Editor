#pragma once
#include "../TextEditor.hpp"
#include "../SyntaxHighlight/SyntaxHighlight.hpp"
#include <vector>
#include <string>
#include <memory>
namespace Console
{
	struct Window
	{
		int cursorX, cursorY;
		int rowOffset, colOffset;
		size_t rows, cols;
		
		std::vector<Editor::Row> rows = Editor::rows();

		bool dirty;
		std::string statusMessage;
		std::weak_ptr<SyntaxHighlight::EditorSyntax> syntax;
	};
}