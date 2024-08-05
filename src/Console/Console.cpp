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

void setWindowSize(Console::Window& window)
{
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screenInfo);

	constexpr uint8_t statusMessageRows = 2;
	window.rows = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1;
	window.cols = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;

	window.rows -= statusMessageRows;
}
void replaceRenderTabs(std::string&);

namespace Console
{

	#ifdef _WIN32 //Windows Console stuff	
	std::unique_ptr<Window> window;
	size_t fileNumRows = 0;

	Window::Window(const std::string_view& fileName) : actualCursorX(0), actualCursorY(0), cols(0), rows(0), renderedCursorX(0), renderedCursorY(0), 
		rowOffset(0), colOffset(0), dirty(false), rawModeEnabled(false), statusMessage("Test Status Message Length go BRR")
	{
		FileHandler::fileName() = fileName;
		FileHandler::loadFileContents();
		FileHandler::loadRows();

		fileRows = &FileHandler::rows();
		fileNumRows = fileRows->size();
	}

	DWORD defaultMode;
	void initConsole(Window& w)
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
		setWindowSize(*window);
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

	void addRenderedCursorTabs()
	{
		if (window->actualCursorX == 0)
		{
			window->renderedCursorX = 0; window->colOffset = 0;
			return;
		}

		size_t spacesToAdd = 0;
		for (size_t i = 0; i < window->actualCursorX; ++i)
		{
			if (i > window->fileRows->at(window->actualCursorY).line.length()) return;

			if (window->fileRows->at(window->actualCursorY).line[i] != static_cast<char>(KeyActions::KeyAction::Tab)) continue;
			
			spacesToAdd += 7 - ((i + spacesToAdd) % 8);
		}
		if (spacesToAdd == 0) return;

		window->renderedCursorX = window->actualCursorX + spacesToAdd;
		if (window->renderedCursorX >= window->colOffset)
		{
			if (window->renderedCursorX >= window->colOffset + window->cols)
			{
				window->colOffset = window->renderedCursorX - window->cols + 1;
				window->renderedCursorX = window->cols - 1;
			}
			else
			{
				window->renderedCursorX -= window->colOffset;
			}
		}
		else
		{
			window->colOffset -= (window->colOffset - window->renderedCursorX);
			window->renderedCursorX -= window->colOffset;
		}
	}

	void fixRenderedCursor()
	{
		if (window->renderedCursorX >= window->cols - 1)
		{
			--window->renderedCursorX;
			++window->colOffset;
		}
	}

	void refreshScreen(const std::string_view& mode)
	{
		addRenderedCursorTabs();
		fixRenderedCursor();

		std::string aBuffer = "\x1b[H"; //Move the cursor to (0, 0)
		aBuffer.append("\x1b[?251"); //Hide the cursor
		aBuffer.append("\x1b[J"); //Erase the screen to redraw changes

		for (size_t y = 0; y < window->rows; ++y)
		{
			size_t fileRow = window->rowOffset + y;
			if (fileRow >= fileNumRows)
			{
				if (fileNumRows == 0 && y == window->rows / 3)
				{
					std::string welcome = std::format("Nutty Editor -- version {}\x1b[0K\r\n", NuttyVersion);
					size_t padding = (window->cols - welcome.length()) / 2;
					if (padding > 0)
					{
						aBuffer.append("~");
						--padding;
					}
					while (padding > 0)
					{
						aBuffer.append(" ");
						--padding;
					}
					aBuffer.append(welcome);
				}
				else
				{
					aBuffer.append("~\x1b[0K\r\n"); 
				}
				continue;
			}

			FileHandler::Row& row = window->fileRows->at(fileRow);
			if (row.line.length() > 0)
			{
				row.renderedLine = row.line;
				replaceRenderTabs(row.renderedLine);
			}
			else
			{
				row.renderedLine.clear();
			}
			size_t renderedLength = row.renderedLine.length() - window->colOffset;
			if (renderedLength > 0)
			{
				if (renderedLength > window->cols) renderedLength = window->cols;
				if (window->colOffset < row.renderedLine.length())
				{
					row.renderedLine = row.renderedLine.substr(window->colOffset, renderedLength);
					aBuffer.append("\x1b[39m");
					aBuffer.append(row.renderedLine);
				}
				else
				{
					row.renderedLine.clear();
				}
			}
			aBuffer.append("\x1b[39m");
			aBuffer.append("\x1b[0K");
			aBuffer.append("\r\n");
		}

		aBuffer.append("\x1b[0K");
		aBuffer.append("\x1b[7m");

		std::string status, rStatus;
		status = std::format("{} - {} lines {}", FileHandler::fileName(), fileNumRows, window->dirty ? "(modified)" : "");
		if (mode == "INSERT")
		{
			rStatus = std::format("actual row {} actual col {} row {}/{} col {}", window->actualCursorY + 1, window->actualCursorX + 1, window->rowOffset + window->renderedCursorY + 1, fileNumRows, window->colOffset + window->renderedCursorX + 1);
		}
		else if (mode == "COMMAND")
		{
			rStatus = "Enter command";
		}
		size_t statusLength = (status.length() > window->cols) ? window->cols : status.length();
		aBuffer.append(status);

		while (statusLength < (window->cols / 2))
		{
			if ((window->cols / 2) - statusLength == mode.length() / 2)
			{
				aBuffer.append(mode);
				break;
			}
			else
			{
				aBuffer.append(" ");
				++statusLength;
			}
		}
		statusLength += mode.length();
		
		while (statusLength < window->cols)
		{
			if (window->cols - statusLength == rStatus.length())
			{
				aBuffer.append(rStatus);
				break;
			}
			else
			{
				aBuffer.append(" ");
				++statusLength;
			}
		}

		aBuffer.append("\x1b[0m\r\n");
		aBuffer.append("\x1b[0K");

		std::string cursorPosition = std::format("\x1b[{};{}H", window->renderedCursorY + 1, window->renderedCursorX + 1);
		aBuffer.append(cursorPosition);
		aBuffer.append("\x1b[?25h");
		std::cout << aBuffer;
	}

	void moveCursor(const int key)
	{
		switch (key)
		{
		case static_cast<char>(KeyActions::KeyAction::ArrowLeft):
			if (window->actualCursorX == 0 && window->actualCursorY == 0) return;

			if (window->actualCursorX == 0)
			{
				--window->actualCursorY;
				window->actualCursorX = window->fileRows->at(window->actualCursorY).line.length();
				if (window->actualCursorX > window->cols + window->colOffset)
				{
					window->colOffset = window->actualCursorX - window->cols + 1;
					window->renderedCursorX = window->cols - 1;
				}
				else
				{
					window->renderedCursorX = window->actualCursorX - window->colOffset;
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
				--window->actualCursorX;
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
			if (window->actualCursorY == fileNumRows - 1)
			{
				if (window->actualCursorX == window->fileRows->at(window->actualCursorY).line.length()) return;
			}

			if (window->actualCursorX == window->fileRows->at(window->actualCursorY).line.length())
			{
				window->actualCursorX = 0;
				window->colOffset = 0;
				window->renderedCursorX = 0;
				++window->actualCursorY;
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
				++window->actualCursorX;
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
			if (window->actualCursorY == 0)
			{
				window->colOffset = 0;
				window->actualCursorX = 0;
				window->renderedCursorX = 0;
				return;
			}

			--window->actualCursorY;
			if (window->renderedCursorY == 0)
			{
				--window->rowOffset;
			}
			else
			{
				--window->renderedCursorY;
			}

			if (window->actualCursorX > window->fileRows->at(window->actualCursorY).line.length())
			{
				window->actualCursorX = window->fileRows->at(window->actualCursorY).line.length();
				if (window->actualCursorX > window->cols + window->colOffset)
				{
					window->colOffset = window->actualCursorX - window->cols + 1;
				}
				window->renderedCursorX = window->actualCursorX - window->colOffset;
			}
			break;
		case static_cast<char>(KeyActions::KeyAction::ArrowDown):
			if (window->actualCursorY == fileNumRows - 1)
			{
				window->actualCursorX = window->fileRows->at(window->actualCursorY).line.length();
				if (window->actualCursorX > window->colOffset + window->cols)
				{
					window->colOffset = window->actualCursorX - window->cols + 1;
				}
				window->renderedCursorX = window->actualCursorX - window->colOffset;
				return;
			}

			++window->actualCursorY;
			if (window->renderedCursorY == window->rows - 1)
			{
				++window->rowOffset;
			}
			else
			{
				++window->renderedCursorY;
			}
			if(window->actualCursorX > window->fileRows->at(window->actualCursorY).line.length())
			{
				window->actualCursorX = window->fileRows->at(window->actualCursorY).line.length();
				if (window->actualCursorX > window->cols + window->colOffset)
				{
					window->colOffset = window->actualCursorX - window->cols;
				}
				window->renderedCursorX = window->actualCursorX - window->colOffset;
			}
			break;
		}
	}

	void deleteChar(const int key)
	{
		if (window->actualCursorY >= fileNumRows) return;
		FileHandler::Row& row = window->fileRows->at(window->actualCursorY);
		if (key == static_cast<char>(KeyActions::KeyAction::Backspace))
		{
			if (window->actualCursorX == 0 && window->actualCursorY == 0) return;

			if (window->actualCursorX == 0)
			{
				window->actualCursorX = window->fileRows->at(window->actualCursorY - 1).line.length();
				window->fileRows->at(window->actualCursorY - 1).line.append(row.line);
				deleteRow(window->actualCursorY);
				--window->actualCursorY;
				if (window->renderedCursorY == 0)
				{
					--window->rowOffset;
				}
				else
				{
					--window->renderedCursorY;
				}

				window->renderedCursorX = window->actualCursorX;
				if (window->renderedCursorX >= window->cols)
				{
					size_t shiftAmount = window->cols - window->renderedCursorX + 1;
					window->renderedCursorX -= shiftAmount;
					window->colOffset += shiftAmount;
				}
			}
			else
			{
				row.line.erase(row.line.begin() + window->actualCursorX - 1);
				if (window->renderedCursorX == 0 && window->colOffset > 0)
				{
					--window->colOffset;
				}
				else
				{
					--window->renderedCursorX;
				}
				--window->actualCursorX;
			}
		}
		else if (key == static_cast<char>(KeyActions::KeyAction::Delete))
		{
			if (window->actualCursorY == fileNumRows - 1 && window->actualCursorX == row.line.length()) return;

			if (window->actualCursorX == row.line.length())
			{
				row.line.append(window->fileRows->at(window->actualCursorY + 1).line);
				deleteRow(window->actualCursorY + 1);
			}
			else
			{
				row.line.erase(row.line.begin() + window->actualCursorX);
			}
		}
		window->dirty = true;
	}
	void deleteRow(const size_t rowNum)
	{
		if (rowNum > fileNumRows) return;
		window->fileRows->erase(window->fileRows->begin() + rowNum);
		window->dirty = true;
		--fileNumRows;
	}

	void addRow()
	{
		if (window->actualCursorY >= fileNumRows) return;
		FileHandler::Row& row = window->fileRows->at(window->actualCursorY);
		if (window->actualCursorX == row.line.length())
		{
			window->fileRows->insert(window->fileRows->begin() + window->actualCursorY + 1, FileHandler::Row());
		}
		else if (window->actualCursorX == 0)
		{
			window->fileRows->insert(window->fileRows->begin() + window->actualCursorY, FileHandler::Row());
		}
		else
		{
			FileHandler::Row newRow;
			newRow.line = row.line.substr(window->actualCursorX);
			row.line.erase(row.line.begin() + window->actualCursorX, row.line.end());
			window->fileRows->insert(window->fileRows->begin() + window->actualCursorY + 1, newRow);
		}

		window->renderedCursorX = 0; window->colOffset = 0; window->actualCursorX = 0; ++window->actualCursorY;
		if (window->renderedCursorY >= window->rows - 1)
		{
			++window->rowOffset;
		}
		else
		{
			++window->renderedCursorY;
		}
		++fileNumRows;
		window->dirty = true;
	}
	void insertChar(const char c)
	{
		if (window->actualCursorY >= fileNumRows) return;
		FileHandler::Row& row = window->fileRows->at(window->actualCursorY);
		
		row.line.insert(row.line.begin() + window->actualCursorX, c);
		++window->actualCursorX;
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
		for (size_t i = 0; i < fileNumRows; ++i)
		{
			if (i == fileNumRows - 1)
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
		window->renderedCursorX = 0; window->renderedCursorY = window->rows + 2; window->colOffset = 0; window->rowOffset = 0; window->actualCursorX = 0; window->actualCursorY = 0;
	}
	void setCursorInsert()
	{
		if (window->fileRows->size() == 0)
		{
			window->fileRows->push_back(FileHandler::Row());
			++fileNumRows;
		}
		window->renderedCursorX = window->renderedCursorY = window->actualCursorX = window->actualCursorY = window->colOffset = window->rowOffset = 0;
	}

	void shiftRowOffset(const int key)
	{
		if (key == static_cast<char>(KeyActions::KeyAction::CtrlArrowDown))
		{
			if (window->rowOffset == fileNumRows - 1) return;

			++window->rowOffset;
			if (window->actualCursorY < fileNumRows && window->renderedCursorY == 0)
			{
				++window->actualCursorY;
				if (window->actualCursorX > window->fileRows->at(window->actualCursorY).line.length())
				{
					window->actualCursorX = window->fileRows->at(window->actualCursorY).line.length();
					window->colOffset = window->actualCursorX - window->cols;
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
				--window->actualCursorY;
				if (window->actualCursorX > window->fileRows->at(window->actualCursorY).line.length())
				{
					window->actualCursorX = window->fileRows->at(window->actualCursorY).line.length();
					window->colOffset = window->actualCursorX - window->cols;
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