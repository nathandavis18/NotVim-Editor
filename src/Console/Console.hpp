#pragma once
#include "../Input/Input.hpp"
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
		size_t renderedCursorX;
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
	bool isRawMode();
	bool isDirty();
	void refreshScreen(const std::string_view& mode);
	void moveCursor(const int key);
	void deleteChar(const int key);
	void deleteRow(const size_t rowNum);
	void addRow();
	void insertChar(const char c);
	void save();
	void setCursorCommand();
	void setCursorInsert();
	void shiftRowOffset(const int key);
}