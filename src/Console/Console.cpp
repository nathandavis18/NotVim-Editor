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

void setWindowSize(std::unique_ptr<Console::Window>& window);
uint16_t addRenderedCursorTabs(FileHandler::Row& row, std::unique_ptr<Console::Window>& window);
void replaceRenderTabs(std::string&);

/// <summary>
/// Fixes the rendered cursor and column offset positions
/// </summary>
/// <param name="window"></param>
void fixRenderedCursor(std::unique_ptr<Console::Window>& window)
{
	while (window->renderedCursorX >= window->cols - 1)
	{
		--window->renderedCursorX;
		++window->colOffset;
	}
	while (window->fileCursorX + window->colOffset > window->renderedCursorX)
	{
		--window->colOffset;
	}
}

namespace Console
{
	std::unique_ptr<Window> window; //The main window

	/// <summary>
	/// Construct the window and initialize all the needed dependencies
	/// </summary>
	/// <param name="fileName"></param>
	Window::Window(const std::string_view& fileName) : fileCursorX(0), fileCursorY(0), cols(0), rows(0), renderedCursorX(0), renderedCursorY(0), 
		rowOffset(0), colOffset(0), dirty(false), rawModeEnabled(false), statusMessage("Test Status Message Length go BRR")
	{
		FileHandler::fileName() = fileName;
		FileHandler::loadFileContents();
		FileHandler::loadRows();
		SyntaxHighlight::initSyntax();

		fileRows = &FileHandler::rows();
	}

#ifdef _WIN32 //Set up the window using Windows API
	DWORD defaultMode;
	void initConsole(Window&& w)
	{
		window = std::make_unique<Window>(w);
		if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &defaultMode))
		{
			std::cerr << "Error retrieving current console mode";
			exit(EXIT_FAILURE);
		}
		if (!(window->rawModeEnabled = enableRawInput()))
		{
			std::cerr << "Error enabling raw input mode";
			exit(EXIT_FAILURE);
		}
		setWindowSize(window);
	}
	bool enableRawInput()
	{
		if (window->rawModeEnabled) return true;

		DWORD rawMode = ENABLE_EXTENDED_FLAGS | (defaultMode & ~ENABLE_LINE_INPUT & ~ENABLE_PROCESSED_INPUT 
					 & ~ENABLE_ECHO_INPUT & ~ENABLE_PROCESSED_OUTPUT & ~ENABLE_WRAP_AT_EOL_OUTPUT); //Disabling certain input/output modes to enable raw mode

		atexit(Console::disableRawInput);
		window->rawModeEnabled = true;
		return SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), rawMode);
	}
	void disableRawInput()
	{
		if (window->rawModeEnabled)
		{
			SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), defaultMode);
			window->rawModeEnabled = false;
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
	void refreshScreen(const std::string_view& mode) //Temporarily a string view, will be changed later
	{
		const char* emptyRowCharacter = "~";
		window->renderedCursorX = addRenderedCursorTabs(window->fileRows->at(window->fileCursorY), window); //Get the rendered cursor position based on the file cursor
		fixRenderedCursor(window);

		std::string renderBuffer = "\x1b[H"; //Move the cursor to (0, 0)
		renderBuffer.append("\x1b[?251"); //Hide the cursor
		renderBuffer.append("\x1b[J"); //Erase the screen to redraw changes

		for (size_t y = 0; y < window->rows; ++y) //For each row within the displayable range
		{
			size_t fileRow = window->rowOffset + y; //The current row within the displayed rows
			if (fileRow >= window->fileRows->size())
			{
				if (window->fileRows->size() == 0 && y == window->rows / 3) //If the file is empty and the current row is at 1/3 height (good display position)
				{
					std::string welcome = std::format("Nutty Editor -- version {}\x1b[0K\r\n", NuttyVersion);
					size_t padding = (window->cols - welcome.length()) / 2;
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

			FileHandler::Row& row = window->fileRows->at(fileRow);
			row.renderedLine = row.line;
			if (row.line.length() > 0)
			{
				replaceRenderTabs(row.renderedLine);
			}
			uint16_t renderedLength = (row.renderedLine.length() - window->colOffset) > window->cols ? window->cols : row.renderedLine.length();
			if (renderedLength > 0)
			{
				if (window->colOffset < row.renderedLine.length())
				{
					row.renderedLine = row.renderedLine.substr(window->colOffset, renderedLength);
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

		std::string status, rStatus;
		status = std::format("{} - {} lines {}", FileHandler::fileName(), window->fileRows->size(), window->dirty ? "(modified)" : "");
		if (mode == "INSERT")
		{
			rStatus = std::format("actual row {} actual col {} row {}/{} col {}", window->fileCursorY + 1, window->fileCursorX + 1, 
				window->rowOffset + window->renderedCursorY + 1, window->fileRows->size(), window->colOffset + window->renderedCursorX + 1);
		}
		else if (mode == "COMMAND")
		{
			rStatus = "Enter command";
		}
		size_t statusLength = (status.length() > window->cols) ? window->cols : status.length();
		renderBuffer.append(status);

		while (statusLength < (window->cols / 2))
		{
			if ((window->cols / 2) - statusLength == mode.length() / 2)
			{
				renderBuffer.append(mode);
				break;
			}
			else
			{
				renderBuffer.append(" ");
				++statusLength;
			}
		}
		statusLength += mode.length();
		
		while (statusLength < window->cols)
		{
			if (window->cols - statusLength == rStatus.length())
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

		std::string cursorPosition = std::format("\x1b[{};{}H", window->renderedCursorY + 1, window->renderedCursorX + 1); //Show the rendered cursor position, offset by 1 for displaying
		renderBuffer.append(cursorPosition);
		renderBuffer.append("\x1b[?25h");
		std::cout << renderBuffer;
	}

	void moveCursor(const char key)
	{
		switch (key)
		{
		case static_cast<char>(KeyActions::KeyAction::ArrowLeft):
			if (window->fileCursorX == 0 && window->fileCursorY == 0) return;

			if (window->fileCursorX == 0)
			{
				--window->fileCursorY;
				window->fileCursorX = window->fileRows->at(window->fileCursorY).line.length();
				if (window->fileCursorX > window->cols + window->colOffset)
				{
					window->colOffset = window->fileCursorX - window->cols + 1;
					window->renderedCursorX = window->cols - 1;
				}
				else
				{
					window->renderedCursorX = window->fileCursorX - window->colOffset;
				}
				if (window->renderedCursorY == 0)
				{
					--window->rowOffset;
				}
				else
				{
					--window->renderedCursorY;
				}
			}
			else
			{
				--window->fileCursorX;
				if (window->renderedCursorX == 0)
				{
					--window->colOffset;
				}
				else
				{
					--window->renderedCursorX;
				}
			}
			break;
		case static_cast<char>(KeyActions::KeyAction::ArrowRight):
			if (window->fileCursorY == window->fileRows->size() - 1)
			{
				if (window->fileCursorX == window->fileRows->at(window->fileCursorY).line.length()) return;
			}

			if (window->fileCursorX == window->fileRows->at(window->fileCursorY).line.length())
			{
				window->fileCursorX = 0;
				window->colOffset = 0;
				window->renderedCursorX = 0;
				++window->fileCursorY;
				if (window->renderedCursorY == window->rows)
				{
					++window->rowOffset;
				}
				else
				{
					++window->renderedCursorY;
				}
			}
			else
			{
				++window->fileCursorX;
				if (window->renderedCursorX == window->cols - 1)
				{
					++window->colOffset;
				}
				else
				{
					++window->renderedCursorX;
				}
			}
			break;
		case static_cast<char>(KeyActions::KeyAction::ArrowUp):
			if (window->fileCursorY == 0)
			{
				window->colOffset = 0;
				window->fileCursorX = 0;
				window->renderedCursorX = 0;
				return;
			}

			--window->fileCursorY;
			if (window->renderedCursorY == 0)
			{
				--window->rowOffset;
			}
			else
			{
				--window->renderedCursorY;
			}

			if (window->fileCursorX > window->fileRows->at(window->fileCursorY).line.length())
			{
				window->fileCursorX = window->fileRows->at(window->fileCursorY).line.length();
				if (window->fileCursorX > window->cols + window->colOffset)
				{
					window->colOffset = window->fileCursorX - window->cols + 1;
				}
				window->renderedCursorX = window->fileCursorX;
			}
			else
			{
				window->renderedCursorX = window->fileCursorX;
			}
			break;
		case static_cast<char>(KeyActions::KeyAction::ArrowDown):
			if (window->fileCursorY == window->fileRows->size() - 1)
			{
				window->fileCursorX = window->fileRows->at(window->fileCursorY).line.length();
				if (window->fileCursorX > window->colOffset + window->cols)
				{
					window->colOffset = window->fileCursorX - window->cols + 1;
				}
				window->renderedCursorX = window->fileCursorX;
				return;
			}

			++window->fileCursorY;
			if (window->renderedCursorY == window->rows - 1)
			{
				++window->rowOffset;
			}
			else
			{
				++window->renderedCursorY;
			}
			if(window->fileCursorX > window->fileRows->at(window->fileCursorY).line.length())
			{
				window->fileCursorX = window->fileRows->at(window->fileCursorY).line.length();
				if (window->fileCursorX > window->cols + window->colOffset)
				{
					window->colOffset = window->fileCursorX - window->cols;
				}
				window->renderedCursorX = window->fileCursorX;
			}
			break;
		}
	}

	void deleteChar(const char key)
	{
		if (window->fileCursorY >= window->fileRows->size()) return;
		FileHandler::Row& row = window->fileRows->at(window->fileCursorY);
		if (key == static_cast<char>(KeyActions::KeyAction::Backspace))
		{
			if (window->fileCursorX == 0 && window->fileCursorY == 0) return;

			if (window->fileCursorX == 0)
			{
				window->fileCursorX = window->fileRows->at(window->fileCursorY - 1).line.length();
				window->fileRows->at(window->fileCursorY - 1).line.append(row.line);
				deleteRow(window->fileCursorY);
				--window->fileCursorY;
				if (window->renderedCursorY == 0)
				{
					--window->rowOffset;
				}
				else
				{
					--window->renderedCursorY;
				}

				window->renderedCursorX = window->fileCursorX;
				if (window->renderedCursorX >= window->cols)
				{
					size_t shiftAmount = window->cols - window->renderedCursorX + 1;
					window->renderedCursorX -= shiftAmount;
					window->colOffset += shiftAmount;
				}
			}
			else
			{
				row.line.erase(row.line.begin() + window->fileCursorX - 1);
				if (window->renderedCursorX == 0 && window->colOffset > 0)
				{
					--window->colOffset;
				}
				else
				{
					--window->renderedCursorX;
				}
				--window->fileCursorX;
			}
		}
		else if (key == static_cast<char>(KeyActions::KeyAction::Delete))
		{
			if (window->fileCursorY == window->fileRows->size() - 1 && window->fileCursorX == row.line.length()) return;

			if (window->fileCursorX == row.line.length())
			{
				row.line.append(window->fileRows->at(window->fileCursorY + 1).line);
				deleteRow(window->fileCursorY + 1);
			}
			else
			{
				row.line.erase(row.line.begin() + window->fileCursorX);
			}
		}
		window->dirty = true;
	}
	void deleteRow(const size_t rowNum)
	{
		if (rowNum > window->fileRows->size()) return;
		window->fileRows->erase(window->fileRows->begin() + rowNum);
		window->dirty = true;
	}

	void addRow()
	{
		if (window->fileCursorY >= window->fileRows->size()) return;
		FileHandler::Row& row = window->fileRows->at(window->fileCursorY);
		if (window->fileCursorX == row.line.length())
		{
			window->fileRows->insert(window->fileRows->begin() + window->fileCursorY + 1, FileHandler::Row());
		}
		else if (window->fileCursorX == 0)
		{
			window->fileRows->insert(window->fileRows->begin() + window->fileCursorY, FileHandler::Row());
		}
		else
		{
			FileHandler::Row newRow;
			newRow.line = row.line.substr(window->fileCursorX);
			row.line.erase(row.line.begin() + window->fileCursorX, row.line.end());
			window->fileRows->insert(window->fileRows->begin() + window->fileCursorY + 1, newRow);
		}

		window->renderedCursorX = 0; window->colOffset = 0; window->fileCursorX = 0; ++window->fileCursorY;
		if (window->renderedCursorY >= window->rows - 1)
		{
			++window->rowOffset;
		}
		else
		{
			++window->renderedCursorY;
		}
		window->dirty = true;
	}
	void insertChar(const char c)
	{
		if (window->fileCursorY >= window->fileRows->size()) return;
		FileHandler::Row& row = window->fileRows->at(window->fileCursorY);
		
		row.line.insert(row.line.begin() + window->fileCursorX, c);
		++window->fileCursorX;
		if (window->renderedCursorX >= window->cols - 1)
		{
			++window->colOffset;
		}
		else
		{
			++window->renderedCursorX;
		}
		window->dirty = true;
	}

	bool isRawMode()
	{
		return window->rawModeEnabled;
	}

	bool isDirty()
	{
		return window->dirty;
	}

	void save()
	{
		std::string output;
		for (size_t i = 0; i < window->fileRows->size(); ++i)
		{
			if (i == window->fileRows->size() - 1)
			{
				output.append(window->fileRows->at(i).line);
			}
			else
			{
				output.append(window->fileRows->at(i).line + "\n");
			}
		}
		FileHandler::saveFile(output);
		window->dirty = false;
	}

	void setCursorCommand()
	{
		window->renderedCursorX = 0; window->renderedCursorY = window->rows + 2; window->colOffset = 0; window->rowOffset = 0; window->fileCursorX = 0; window->fileCursorY = 0;
	}
	void setCursorInsert()
	{
		if (window->fileRows->size() == 0)
		{
			window->fileRows->push_back(FileHandler::Row());
		}
		window->renderedCursorX = window->renderedCursorY = window->fileCursorX = window->fileCursorY = window->colOffset = window->rowOffset = 0;
	}

	void shiftRowOffset(const char key)
	{
		if (key == static_cast<char>(KeyActions::KeyAction::CtrlArrowDown))
		{
			if (window->rowOffset == window->fileRows->size() - 1) return;

			++window->rowOffset;
			if (window->fileCursorY < window->fileRows->size() && window->renderedCursorY == 0)
			{
				++window->fileCursorY;
				if (window->fileCursorX > window->fileRows->at(window->fileCursorY).line.length())
				{
					window->fileCursorX = window->fileRows->at(window->fileCursorY).line.length();
					window->colOffset = window->fileCursorX - window->cols;
				}
			}
			else
			{
				--window->renderedCursorY;
			}
		}
		else if (key == static_cast<char>(KeyActions::KeyAction::CtrlArrowUp))
		{
			if (window->rowOffset == 0) return;
			--window->rowOffset;
			if (window->renderedCursorY == window->rows - 1)
			{
				--window->fileCursorY;
				if (window->fileCursorX > window->fileRows->at(window->fileCursorY).line.length())
				{
					window->fileCursorX = window->fileRows->at(window->fileCursorY).line.length();
					window->colOffset = window->fileCursorX - window->cols;
					return;
				}
			}
			else
			{
				++window->renderedCursorY;
			}
		}
	}
}

/// <summary>
/// This function needs work
/// </summary>
/// <param name="renderedLine"></param>
/// <param name="offset"></param>
void replaceRenderTabs(std::string& renderedLine)
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
uint16_t addRenderedCursorTabs(FileHandler::Row& row, std::unique_ptr<Console::Window>& window)
{
	if (window->fileCursorX == 0)
	{
		window->colOffset = 0;
		return 0;
	}

	size_t spacesToAdd = 0;
	for (size_t i = 0; i < window->fileCursorX; ++i)
	{
		if (i > row.line.length()) return window->renderedCursorX;

		if (row.line[i] != static_cast<char>(KeyActions::KeyAction::Tab)) continue;

		spacesToAdd += 7 - ((i + spacesToAdd) % 8);
	}
	if (spacesToAdd == 0) return window->renderedCursorX;

	size_t newRenderedCursor = window->fileCursorX + spacesToAdd;
	if (window->renderedCursorX >= window->colOffset)
	{
		if (newRenderedCursor >= window->colOffset + window->cols)
		{
			window->colOffset = newRenderedCursor - window->cols + 1;
			newRenderedCursor = window->cols - 1;
		}
		else
		{
			newRenderedCursor -= window->colOffset;
		}
	}
	else
	{
		window->colOffset -= (window->colOffset - newRenderedCursor);
		newRenderedCursor -= window->colOffset;
	}

	return newRenderedCursor;
}


/// <summary>
/// Sets the window's row/column count using the correct OS API
/// </summary>
/// <param name="window"></param>
#ifdef _WIN32
void setWindowSize(std::unique_ptr<Console::Window>& window)
{
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screenInfo);

	constexpr uint8_t statusMessageRows = 2;
	window->rows = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1;
	window->cols = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;

	window->rows -= statusMessageRows;
}
#elif __linux__ || __APPLE__
//linux/apple window size
#endif