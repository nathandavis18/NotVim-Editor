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
	static void refreshScreen();
	static bool enableRawInput();
	static void disableRawInput();
	static bool isRawMode();
	static bool isDirty();
	static void shiftRowOffset(const char key);
	static void enableCommandMode();
	static void enableEditMode();
	static void moveCursor(const char key);
	static void deleteChar(const char key);
	static void addRow();
	static void insertChar(const char c);
	static void save();
	static void initConsole(const std::string_view&);
	static void clearScreen();
	static bool setWindowSize();

private:
	struct Window
	{
		Window();
		size_t fileCursorX, fileCursorY;
		size_t renderedCursorX, renderedCursorY;
		size_t rowOffset, colOffset;
		uint16_t rows, cols;

		std::vector<FileHandler::Row>& fileRows;

		bool dirty;
		bool rawModeEnabled;
		std::string statusMessage;
		SyntaxHighlight::EditorSyntax* syntax;
	};
	static void fixRenderedCursorPosition(const FileHandler::Row&);
	static size_t addRenderedCursorTabs(const FileHandler::Row&);
	static void replaceRenderedStringTabs(std::string&);
	static void deleteRow(const size_t rowNum);

private:
	inline static std::unique_ptr<Window> mWindow;
	inline static Mode mMode = Mode::ReadMode;
};