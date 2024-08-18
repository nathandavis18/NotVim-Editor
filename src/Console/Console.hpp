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

#pragma once
#include "../Input/Input.hpp"
#include "../SyntaxHighlight/SyntaxHighlight.hpp"
#include "../File/File.hpp"

#include <vector>
#include <string>
#include <memory>

enum class Mode
{
	CommandMode,
	EditMode,
	FindMode,
	ReadMode,
	ExitMode,
	None
};

class Console
{
public:
	static Mode& mode(Mode = Mode::None);
	static void prepRenderedString();
	static void refreshScreen();
	static void moveCursor(const KeyActions::KeyAction key);
	static void shiftRowOffset(const KeyActions::KeyAction key);
	static void addRow();
	static void deleteChar(const KeyActions::KeyAction key);
	static void insertChar(const uint8_t c);
	static bool isRawMode();
	static bool isDirty();
	static void save();
	static void enableCommandMode();
	static void enableEditMode();

	//OS Specific Functions
	static void initConsole(const std::string_view&);
	static bool setWindowSize();
	static bool enableRawInput();
	static void disableRawInput();
	static void clearScreen();

private:
	struct Window
	{
		Window();
		size_t fileCursorX, fileCursorY;
		size_t renderedCursorX, renderedCursorY;
		size_t savedRenderedCursorXPos; bool updateSavedPos = true;
		size_t colNumberToDisplay;
		size_t rowOffset, colOffset;
		size_t rows, cols;

		std::vector<FileHandler::Row> fileRows;

		bool dirty;
		bool rawModeEnabled;
		SyntaxHighlight::EditorSyntax* syntax;
	};

	struct HighlightLocations
	{
		SyntaxHighlight::HighlightType colorType;
		size_t startRow, startCol, endRow, endCol;
	};

	static void setRenderedString();
	static void deleteRow(const size_t rowNum);
	static void setCursorLinePosition();
	static void fixRenderedCursorPosition(const FileHandler::Row&);
	static void replaceRenderedStringTabs(std::string&);
	static size_t getRenderedCursorTabSpaces(const FileHandler::Row&);
	static void updateRenderedColor(std::string&, const size_t row, const size_t colOffset);
	static void findEndMarker(std::string& currentWord, size_t& row, size_t& posOffset, size_t& findPos, const std::string& strToFind, const SyntaxHighlight::HighlightType);
	static void setHighlight();

private:
	inline static std::unique_ptr<Window> mWindow;
	inline static std::vector<HighlightLocations> mHighlights;
	inline static Mode mMode = Mode::ReadMode;
	inline static const std::string separators = " \"',.()+-/*=~%;:[]{}<>";
};