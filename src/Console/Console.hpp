#pragma once
#include "../TextEditor.hpp"
#include "../SyntaxHighlight/SyntaxHighlight.hpp"
#include <vector>
#include <string>
#include <memory>
#include "../File/File.hpp"

#ifdef _WIN32
#include <Windows.h>
#elif __linux__ || __APPLE__
//Linux headers
#endif
namespace Console
{
	struct Window
	{
		Window();
		size_t cursorX, cursorY;
		size_t rowOffset, colOffset;
		size_t rows, cols;
		
		std::vector<FileHandler::Row>& fileRows;

		bool dirty;
		bool rawModeEnabled;
		std::string statusMessage;
		std::weak_ptr<SyntaxHighlight::EditorSyntax> syntax;
	};
	void initConsole();
	bool enableRawInput();
	void disableRawInput();
	void refreshScreen();
	void moveCursor(const int key);
	void deleteChar(const int key);
	void deleteRow(const size_t rowNum);
}