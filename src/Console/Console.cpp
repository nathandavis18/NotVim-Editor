#include "Console.hpp"
#include "../KeyActions/KeyActions.hh"

#include <iostream>
#include <fstream>
#include <format>

#ifdef _WIN32
#include <Windows.h>
#elif __linux__ || __APPLE__
//Linux headers
#endif
#define NuttyVersion "0.1a"

/// <summary>
/// Construct the window and initialize all the needed dependencies
/// </summary>
/// <param name="fileName"></param>
Console::Window::Window(const std::string_view& fileName) : fileCursorX(0), fileCursorY(0), cols(0), rows(0), renderedCursorX(0), renderedCursorY(0), 
	rowOffset(0), colOffset(0), dirty(false), rawModeEnabled(false), statusMessage("Test Status Message Length go BRR"), fileRows(FileHandler::rows()), syntax(nullptr)
{}

#ifdef _WIN32 //Set up the window using Windows API
DWORD defaultMode;
void Console::initConsole(const std::string_view& fileName)
{
	FileHandler::fileName() = fileName;
	FileHandler::loadFileContents();
	FileHandler::loadRows();
	SyntaxHighlight::initSyntax();

	mWindow = std::make_unique<Window>(Window(fileName));
	if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &defaultMode))
	{
		std::cerr << "Error retrieving current console mode";
		exit(EXIT_FAILURE);
	}
	if (!(mWindow->rawModeEnabled = enableRawInput()))
	{
		std::cerr << "Error enabling raw input mode";
		exit(EXIT_FAILURE);
	}
	setWindowSize();
}
bool Console::enableRawInput()
{
	if (mWindow->rawModeEnabled) return true;

	DWORD rawMode = ENABLE_EXTENDED_FLAGS | (defaultMode & ~ENABLE_LINE_INPUT & ~ENABLE_PROCESSED_INPUT 
					& ~ENABLE_ECHO_INPUT & ~ENABLE_PROCESSED_OUTPUT & ~ENABLE_WRAP_AT_EOL_OUTPUT); //Disabling certain input/output modes to enable raw mode

	atexit(Console::disableRawInput);
	mWindow->rawModeEnabled = true;
	return SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), rawMode);
}
void Console::disableRawInput()
{
	if (mWindow->rawModeEnabled)
	{
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), defaultMode);
		mWindow->rawModeEnabled = false;
	}
}
#elif __linux__ || __APPLE__
//Apple/Linux raw mode console stuff
#endif //OS Terminal Raw Mode

/// <summary>
/// Builds the output buffer and displays it to the user through std::cout
/// Uses ANSI escape codes for clearing screen/displaying cursor and for colors
/// </summary>
/// <param name="mode"></param>
void Console::refreshScreen() //Temporarily a string view, will be changed later
{
	const char* emptyRowCharacter = "~";

	if (mWindow->fileRows.size() > 0 && mMode != Mode::CommandMode) fixRenderedCursor(mWindow->fileRows.at(mWindow->fileCursorY));

	std::string renderBuffer = "\x1b[H"; //Move the cursor to (0, 0)
	renderBuffer.append("\x1b[?251"); //Hide the cursor
	renderBuffer.append("\x1b[J"); //Erase the screen to redraw changes

	for (size_t y = 0; y < mWindow->rows; ++y) //For each row within the displayable range
	{
		size_t fileRow = mWindow->rowOffset + y; //The current row within the displayed rows
		if (fileRow >= mWindow->fileRows.size())
		{
			if (mWindow->fileRows.size() == 0 && y == mWindow->rows / 3) //If the file is empty and the current row is at 1/3 height (good display position)
			{
				std::string welcome = std::format("Nutty Editor -- version {}\x1b[0K\r\n", NuttyVersion);
				size_t padding = (mWindow->cols - welcome.length()) / 2;
				if (padding > 0)
				{
					renderBuffer.append(emptyRowCharacter);
					--padding;
				}
				while (padding > 0)
				{
					renderBuffer.append(" ");
					--padding;
				}
				renderBuffer.append(welcome);
			}
			else
			{
				renderBuffer.append(emptyRowCharacter);
				renderBuffer.append("\x1b[0K\r\n"); //Clear the rest of the row, perform carriage return, and move to next line
			}
			continue;
		}

		FileHandler::Row& row = mWindow->fileRows.at(fileRow);
		row.renderedLine = row.line;
		if (row.line.length() > 0)
		{
			replaceRenderedStringTabs(row.renderedLine);
		}
		uint16_t renderedLength = (row.renderedLine.length() - mWindow->colOffset) > mWindow->cols ? mWindow->cols : row.renderedLine.length();
		if (renderedLength > 0)
		{
			if (mWindow->colOffset < row.renderedLine.length())
			{
				row.renderedLine = row.renderedLine.substr(mWindow->colOffset, renderedLength);
				renderBuffer.append("\x1b[39m");
				renderBuffer.append(row.renderedLine);
			}
			else
			{
				row.renderedLine.clear();
			}
		}
		renderBuffer.append("\x1b[39m");
		renderBuffer.append("\x1b[0K\r\n");
	}

	renderBuffer.append("\x1b[0K");
	renderBuffer.append("\x1b[7m");

	std::string status, rStatus, modeToDisplay;
	status = std::format("{} - {} lines {}", FileHandler::fileName(), mWindow->fileRows.size(), mWindow->dirty ? "(modified)" : "");
	if (mMode == Mode::EditMode)
	{
		rStatus = std::format("actual row {} actual col {} row {}/{} col {}", mWindow->fileCursorY + 1, mWindow->fileCursorX + 1, 
			mWindow->rowOffset + mWindow->renderedCursorY + 1, mWindow->fileRows.size(), mWindow->colOffset + mWindow->renderedCursorX + 1);
		modeToDisplay = "EDIT";
	}
	else if (mMode == Mode::CommandMode)
	{
		rStatus = "Enter command";
		modeToDisplay = "COMMAND";
	}
	else if (mMode == Mode::ReadMode)
	{
		rStatus = "Read mode";
		modeToDisplay = "READ ONLY";
	}
	size_t statusLength = (status.length() > mWindow->cols) ? mWindow->cols : status.length();
	renderBuffer.append(status);

	while (statusLength < (mWindow->cols / 2))
	{
		if ((mWindow->cols / 2) - statusLength == modeToDisplay.length() / 2)
		{
			renderBuffer.append(modeToDisplay);
			break;
		}
		else
		{
			renderBuffer.append(" ");
			++statusLength;
		}
	}
	statusLength += modeToDisplay.length();
		
	while (statusLength < mWindow->cols)
	{
		if (mWindow->cols - statusLength == rStatus.length())
		{
			renderBuffer.append(rStatus);
			break;
		}
		else
		{
			renderBuffer.append(" ");
			++statusLength;
		}
	}

	renderBuffer.append("\x1b[0m\r\n");
	renderBuffer.append("\x1b[0K");

	std::string cursorPosition = std::format("\x1b[{};{}H", mWindow->renderedCursorY + 1, mWindow->renderedCursorX + 1); //Show the rendered cursor position, offset by 1 for displaying
	renderBuffer.append(cursorPosition);
	renderBuffer.append("\x1b[?25h");
	std::cout << renderBuffer;
}

void Console::moveCursor(const char key)
{
	switch (key)
	{
	case static_cast<char>(KeyActions::KeyAction::ArrowLeft):
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0) return;

		if (mWindow->fileCursorX == 0)
		{
			--mWindow->fileCursorY;
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		else
		{
			--mWindow->fileCursorX;
		}
		break;
	case static_cast<char>(KeyActions::KeyAction::ArrowRight):
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1)
		{
			if (mWindow->fileCursorX == mWindow->fileRows.at(mWindow->fileCursorY).line.length()) return;
		}

		if (mWindow->fileCursorX == mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = 0;
			++mWindow->fileCursorY;
		}
		else
		{
			++mWindow->fileCursorX;
		}
		break;
	case static_cast<char>(KeyActions::KeyAction::ArrowUp):
		if (mWindow->fileCursorY == 0)
		{
			mWindow->fileCursorX = 0;
			return;
		}

		--mWindow->fileCursorY;

		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;
	case static_cast<char>(KeyActions::KeyAction::ArrowDown):
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1)
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
			return;
		}

		++mWindow->fileCursorY;
		if(mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;
	}
}

void Console::deleteChar(const char key)
{
	if (mWindow->fileCursorY >= mWindow->fileRows.size()) return;
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);
	if (key == static_cast<char>(KeyActions::KeyAction::Backspace))
	{
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0) return;

		if (mWindow->fileCursorX == 0)
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY - 1).line.length();
			mWindow->fileRows.at(mWindow->fileCursorY - 1).line.append(row.line);
			deleteRow(mWindow->fileCursorY);
			--mWindow->fileCursorY;
		}
		else
		{
			row.line.erase(row.line.begin() + mWindow->fileCursorX - 1);
			--mWindow->fileCursorX;
		}
	}
	else if (key == static_cast<char>(KeyActions::KeyAction::Delete))
	{
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1 && mWindow->fileCursorX == row.line.length()) return;

		if (mWindow->fileCursorX == row.line.length())
		{
			row.line.append(mWindow->fileRows.at(mWindow->fileCursorY + 1).line);
			deleteRow(mWindow->fileCursorY + 1);
		}
		else
		{
			row.line.erase(row.line.begin() + mWindow->fileCursorX);
		}
	}
	mWindow->dirty = true;
}
void Console::deleteRow(const size_t rowNum)
{
	if (rowNum > mWindow->fileRows.size()) return;
	mWindow->fileRows.erase(mWindow->fileRows.begin() + rowNum);
	mWindow->dirty = true;
}

void Console::addRow()
{
	if (mWindow->fileCursorY >= mWindow->fileRows.size()) return;
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);
	if (mWindow->fileCursorX == row.line.length())
	{
		mWindow->fileRows.insert(mWindow->fileRows.begin() + mWindow->fileCursorY + 1, FileHandler::Row());
	}
	else if (mWindow->fileCursorX == 0)
	{
		mWindow->fileRows.insert(mWindow->fileRows.begin() + mWindow->fileCursorY, FileHandler::Row());
	}
	else
	{
		FileHandler::Row newRow;
		newRow.line = row.line.substr(mWindow->fileCursorX);
		row.line.erase(row.line.begin() + mWindow->fileCursorX, row.line.end());
		mWindow->fileRows.insert(mWindow->fileRows.begin() + mWindow->fileCursorY + 1, newRow);
	}

	mWindow->fileCursorX = 0; ++mWindow->fileCursorY;
	mWindow->dirty = true;
}
void Console::insertChar(const char c)
{
	if (mWindow->fileCursorY >= mWindow->fileRows.size()) return;
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);
		
	row.line.insert(row.line.begin() + mWindow->fileCursorX, c);
	++mWindow->fileCursorX;
	mWindow->dirty = true;
}

bool Console::isRawMode()
{
	return mWindow->rawModeEnabled;
}

bool Console::isDirty()
{
	return mWindow->dirty;
}

void Console::save()
{
	std::string output;
	for (size_t i = 0; i < mWindow->fileRows.size(); ++i)
	{
		if (i == mWindow->fileRows.size() - 1)
		{
			output.append(mWindow->fileRows.at(i).line);
		}
		else
		{
			output.append(mWindow->fileRows.at(i).line + "\n");
		}
	}
	FileHandler::saveFile(output);
	mWindow->dirty = false;
}

void Console::setCursorCommand()
{
	mWindow->renderedCursorX = 0; mWindow->renderedCursorY = mWindow->rows + 2; mWindow->colOffset = 0; mWindow->rowOffset = 0; mWindow->fileCursorX = 0; mWindow->fileCursorY = 0;
}
void Console::setCursorInsert()
{
	if (mWindow->fileRows.size() == 0)
	{
		mWindow->fileRows.push_back(FileHandler::Row());
	}
	mWindow->renderedCursorX = mWindow->renderedCursorY = mWindow->fileCursorX = mWindow->fileCursorY = mWindow->colOffset = mWindow->rowOffset = 0;
}

void Console::shiftRowOffset(const char key)
{
	if (key == static_cast<char>(KeyActions::KeyAction::CtrlArrowDown))
	{
		if (mWindow->rowOffset == mWindow->fileRows.size() - 1) return;

		++mWindow->rowOffset;
		if (mWindow->fileCursorY < mWindow->fileRows.size() && mWindow->renderedCursorY == 0)
		{
			++mWindow->fileCursorY;
			if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
			{
				mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
			}
		}
	}
	else if (key == static_cast<char>(KeyActions::KeyAction::CtrlArrowUp))
	{
		if (mWindow->rowOffset == 0) return;
		--mWindow->rowOffset;
		if (mWindow->renderedCursorY == mWindow->rows - 1)
		{
			--mWindow->fileCursorY;
			if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
			{
				mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
				return;
			}
		}
	}
}

/// <summary>
/// This function needs work
/// </summary>
/// <param name="renderedLine"></param>
/// <param name="offset"></param>
void Console::replaceRenderedStringTabs(std::string& renderedLine)
{
	size_t lineLength = renderedLine.length();
	for (size_t i = 0; i < lineLength; ++i)
	{
		if (i >= renderedLine.length()) return;
		if (renderedLine[i] != static_cast<char>(KeyActions::KeyAction::Tab)) continue;

		renderedLine[i] = ' ';
		uint8_t t = 7 - (i % 8);
		while (t > 0)
		{
			renderedLine.insert(renderedLine.begin() + (i), ' ');
			--t;
			++i;
		}
		if (renderedLine.length() > lineLength)
		{
			lineLength = renderedLine.length();
		}
	}
}

/// <summary>
/// Adjust the rendered cursor position to account for spaces in the rendered string
/// </summary>
/// <param name="row">Row to check</param>
/// <param name="window"></param>
/// <returns>The new rendered cursor X position</returns>
size_t Console::addRenderedCursorTabs(const FileHandler::Row& row)
{
	if (mWindow->fileCursorX == 0)
	{
		mWindow->colOffset = 0;
		mWindow->renderedCursorX = 0;
		return 0;
	}

	size_t spacesToAdd = 0;
	for (size_t i = 0; i < mWindow->fileCursorX; ++i)
	{
		if (i > row.line.length()) return 0;

		if (row.line[i] != static_cast<char>(KeyActions::KeyAction::Tab)) continue;

		spacesToAdd += 7 - ((i + spacesToAdd) % 8);
	}
	return spacesToAdd;
}


/// <summary>
/// Sets the window's row/column count using the correct OS API
/// </summary>
/// <param name="window"></param>
#ifdef _WIN32
void Console::setWindowSize()
{
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screenInfo);

	constexpr uint8_t statusMessageRows = 2;
	mWindow->rows = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1;
	mWindow->cols = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;

	mWindow->rows -= statusMessageRows;
}
#elif __linux__ || __APPLE__
//linux/apple window size
#endif

/// <summary>
/// Fixes the rendered cursor and column offset positions
/// </summary>
/// <param name="window"></param>
void Console::fixRenderedCursor(const FileHandler::Row& row)
{
	mWindow->renderedCursorX = mWindow->fileCursorX - mWindow->colOffset;
	mWindow->renderedCursorX += addRenderedCursorTabs(row);
	if (mWindow->renderedCursorX >= mWindow->cols)
	{
		while (mWindow->renderedCursorX >= mWindow->cols)
		{
			--mWindow->renderedCursorX;
			++mWindow->colOffset;
		}
	}
	else
	{
		while (mWindow->fileCursorX + mWindow->colOffset > mWindow->renderedCursorX)
		{
			--mWindow->colOffset;
		}
	}
	mWindow->renderedCursorY = mWindow->fileCursorY - mWindow->rowOffset;
	if (mWindow->fileCursorY - mWindow->rowOffset >= mWindow->rows)
	{
		mWindow->rowOffset = mWindow->fileCursorY - mWindow->rows;
	}
}

Mode& Console::mode(Mode m)
{
	if (m != Mode::None)
	{
		mMode = m;
	}
	return mMode;
}