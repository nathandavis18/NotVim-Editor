#pragma once
#include "../Input/Input.hpp"
#include "../SyntaxHighlight/SyntaxHighlight.hpp"
#include <vector>
#include <string>
#include <memory>
#include "../File/File.hpp"

namespace Console
{
	struct Window
	{
		Window(const std::string_view& fileName);

		size_t fileCursorX, fileCursorY;
		uint16_t renderedCursorX, renderedCursorY;
		size_t rowOffset, colOffset;
		uint16_t rows, cols;
		
		std::vector<FileHandler::Row>* fileRows;

		bool dirty;
		bool rawModeEnabled;
		std::string statusMessage;
		std::weak_ptr<SyntaxHighlight::EditorSyntax> syntax;
	};
	void initConsole(Window&&);
	bool enableRawInput();
	void disableRawInput();
	bool isRawMode();
	bool isDirty();
	void refreshScreen(const std::string_view& mode);
	void moveCursor(const char key);
	void deleteChar(const char key);
	void deleteRow(const size_t rowNum);
	void addRow();
	void insertChar(const char c);
	void save();
	void setCursorCommand();
	void setCursorInsert();
	void shiftRowOffset(const char key);
}