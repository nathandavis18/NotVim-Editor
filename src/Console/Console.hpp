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
	static void initConsole(const std::string_view&);
	static bool enableRawInput();
	static void disableRawInput();
	static bool isRawMode();
	static bool isDirty();
	static void shiftRowOffset(const char key);
	static void setCursorCommand();
	static void setCursorInsert();
	static void moveCursor(const char key);
	static void deleteChar(const char key);
	static void addRow();
	static void insertChar(const char c);
	static void refreshScreen();
	static void save();
	static Mode& mode(Mode = Mode::None);

private:
	struct Window
	{
		Window(const std::string_view& fileName);

		size_t fileCursorX, fileCursorY;
		size_t renderedCursorX, renderedCursorY;
		size_t rowOffset, colOffset;
		size_t rows, cols;

		std::vector<FileHandler::Row>& fileRows;

		bool dirty;
		bool rawModeEnabled;
		std::string statusMessage;
		SyntaxHighlight::EditorSyntax* syntax;
	};
	static void fixRenderedCursor(const FileHandler::Row&);
	static void setWindowSize();
	static size_t addRenderedCursorTabs(const FileHandler::Row&);
	static void replaceRenderedStringTabs(std::string&);
	static void deleteRow(const size_t rowNum);

private:
	inline static std::unique_ptr<Window> mWindow;
	inline static Mode mMode = Mode::ReadMode;
};