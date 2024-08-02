#include "Console.hpp"
#include "../KeyActions/KeyActions.hh"
#include <iostream>
#include <fstream>
#include <format>
#define NuttyVersion "0.1a"

#ifdef _WIN32 //Windows Console stuff	
namespace Console
{
	Window window;
	size_t fileNumRows = 0;
	Window::Window() : cursorX(0), cursorY(0), rowOffset(0), colOffset(0), dirty(false), rawModeEnabled(false), statusMessage("Test Status Message Length go BRR"), fileRows(FileHandler::rows())
	{
		CONSOLE_SCREEN_BUFFER_INFO screenInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &screenInfo);

		constexpr uint8_t statusMessageRows = 2;
		rows = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1 - statusMessageRows;
		cols = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;
	}

	DWORD defaultMode;
	void initConsole()
	{
		if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &defaultMode))
		{
			std::cerr << "Error retrieving current console mode";
			exit(EXIT_FAILURE);
		}
		if (!(window.rawModeEnabled = enableRawInput()))
		{
			std::cerr << "Error enabling raw input mode";
			exit(EXIT_FAILURE);
		}
		fileNumRows = window.fileRows.size();
		if (window.fileRows.size() < window.rows)
		{
			window.fileRows.resize(window.rows);
		}
	}
	bool enableRawInput()
	{
		DWORD rawMode = ENABLE_EXTENDED_FLAGS | (defaultMode & ~ENABLE_LINE_INPUT & ~ENABLE_PROCESSED_INPUT 
					 & ~ENABLE_ECHO_INPUT & ~ENABLE_PROCESSED_OUTPUT & ~ENABLE_WRAP_AT_EOL_OUTPUT); //Disabling input modes to enable raw mode

		atexit(Console::disableRawInput);
		return SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), rawMode);
	}
	void disableRawInput()
	{
		if (window.rawModeEnabled)
		{
			SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), defaultMode);
			window.rawModeEnabled = false;
		}
	}
	void refreshScreen()
	{
		FileHandler::Row* row;
		std::string aBuffer;

		aBuffer.append("\x1b[?251");
		aBuffer.append("\x1b[H");
		for (size_t y = 0; y < window.rows; ++y)
		{
			size_t fileRow = window.rowOffset + y;
			if (fileRow >= fileNumRows)
			{
				if (fileNumRows == 0 && y == window.rows / 3)
				{
					std::string welcome = std::format("Nutty Editor -- version {}\x1b[0K\r\n", NuttyVersion);
					size_t welcomeLength = welcome.length();
					size_t padding = (window.cols - welcomeLength) / 2;
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
			row = &window.fileRows[fileRow];

			size_t length = row->line.length() - window.colOffset;
			if (length > 0)
			{
				if (length > window.cols) length = window.cols;
				std::string s = row->line.substr(window.colOffset);
				for (size_t j = 0; j < length; ++j)
				{
					aBuffer.append("\x1b[39m");
					aBuffer += s[j];
				}
			}
			aBuffer.append("\x1b[39m");
			aBuffer.append("\x1b[0K");
			aBuffer.append("\r\n");
		}

		aBuffer.append("\x1b[0K");
		aBuffer.append("\x1b[7m");

		std::string status, rStatus;
		status = std::format("{} - {} lines {}", FileHandler::fileName(), fileNumRows, window.dirty ? "(modified)" : "");
		rStatus = std::format("{}/{}", window.rowOffset + window.cursorY + 1, window.rows);
		size_t statusLength = (status.length() > window.cols) ? window.cols : status.length();
		aBuffer.append(status);
		
		while (statusLength < window.cols)
		{
			if (window.cols - statusLength == rStatus.length())
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
		if (window.statusMessage.length() > 0)
		{
			if (window.statusMessage.length() > window.cols)
			{
				window.statusMessage.erase(window.statusMessage.begin() + window.cols, window.statusMessage.end());
			}
			aBuffer.append(window.statusMessage);
		}


		size_t cx = 1;
		size_t fileRow = window.rowOffset + window.cursorY;
		FileHandler::Row* cursorRow = (fileRow >= fileNumRows) ? nullptr : &window.fileRows[fileRow];
		if (cursorRow)
		{
			for (size_t j = window.colOffset; j < (window.cursorX + window.colOffset); ++j, ++cx)
			{
				if (j < cursorRow->renderedSize && cursorRow->line[j] == static_cast<char>(KeyActions::KeyAction::Tab))
				{
					cx += 3 - (cx % 4);
				}
			}
		}
		std::string buffer = std::format("\x1b[{};{}H", window.cursorY + 1, cx);
		aBuffer.append(buffer);
		aBuffer.append("\x1b[?25h");
		std::cout << aBuffer;
	}

	void moveCursor(const int key)
	{
		size_t fileRow = window.rowOffset + window.cursorY;
		size_t fileCol = window.colOffset + window.cursorX;
		FileHandler::Row* row = (fileRow >= window.rows) ? nullptr : &window.fileRows[fileRow];

		switch (key)
		{
		case static_cast<int>(KeyActions::KeyAction::ArrowLeft):
			if (window.cursorX == 0)
			{
				if (window.colOffset > 0)
				{
					--window.colOffset;
				}
				else
				{
					if (fileRow > 0)
					{
						--window.cursorY;
						window.cursorX = window.fileRows[fileRow - 1].line.length();
						if (window.cursorX > window.cols - 1)
						{
							window.colOffset = window.cursorX - window.cols + 1;
							window.cursorX = window.cols - 1; 
						}
					}
				}
			}
			else
			{
				--window.cursorX;
			}
			break;
		case static_cast<int>(KeyActions::KeyAction::ArrowRight):
			if (row && fileCol < row->line.length())
			{
				if (window.cursorX == window.cols - 1)
				{
					++window.colOffset;
				}
				else
				{
					++window.cursorX;
				}
			}
			else if (row && fileCol == row->line.length())
			{
				if (window.cursorY + window.rowOffset == fileNumRows - 1) break;

				window.cursorX = 0;
				window.colOffset = 0;
				if (window.cursorY == window.rows - 1)
				{
					++window.rowOffset;
				}
				else
				{
					++window.cursorY;
				}
			}
			break;
		case static_cast<int>(KeyActions::KeyAction::ArrowUp):
			if (window.cursorY == 0)
			{
				if (window.rowOffset > 0)
				{
					--window.rowOffset;
				}
			}
			else
			{
				--window.cursorY;
			}
			break;
		case static_cast<int>(KeyActions::KeyAction::ArrowDown):
			if (fileRow < window.rows)
			{
				if (window.cursorY + window.rowOffset == fileNumRows - 1) break;
				if (window.cursorY == window.rows - 1)
				{
					++window.rowOffset;
				}
				else
				{
					++window.cursorY;
				}
			}
			break;
		}

		fileRow = window.rowOffset + window.cursorY;
		fileCol = window.colOffset + window.cursorX;

		row = (fileRow >= window.rows) ? nullptr : &window.fileRows[fileRow];
		size_t rowLength = 0;
		if (row)
		{
			rowLength = row->line.length();
		}

		if (fileCol > rowLength)
		{
			window.cursorX -= fileCol - rowLength;
			if (window.cursorX < 0)
			{
				window.colOffset += window.cursorX;
				window.cursorX = 0;
			}
		}
	}

	void deleteChar(const int key)
	{
		size_t fileRow = window.cursorY + window.rowOffset;
		size_t fileCol = window.cursorX + window.colOffset;

		FileHandler::Row* row = (fileRow >= fileNumRows) ? nullptr : &window.fileRows[fileRow];

		if (!row || (fileCol == 0 && fileRow == 0 && key == static_cast<int>(KeyActions::KeyAction::Backspace))) return;
		if (fileCol == 0 && key == static_cast<int>(KeyActions::KeyAction::Backspace))
		{
			fileCol = window.fileRows[fileRow - 1].line.length();
			window.fileRows[fileRow - 1].line.append(row->line);
			deleteRow(fileRow);
			row = nullptr;

			if (window.cursorY == 0)
			{
				--window.rowOffset;
			}
			else
			{
				--window.cursorY;
			}

			window.cursorX = fileCol;
			if (window.cursorX >= window.cols)
			{
				size_t shiftAmount = window.cols - window.cursorX + 1;
				window.cursorX -= shiftAmount;
				window.colOffset += shiftAmount;
			}
		}
		else if(key == static_cast<int>(KeyActions::KeyAction::Backspace))
		{
			row->line.erase(row->line.begin() + fileCol - 1);
			if (window.cursorX == 0 && window.colOffset > 0)
			{
				--window.colOffset;
			}
			else
			{
				--window.cursorX;
			}
		}
		window.dirty = true;
	}
	void deleteRow(const size_t rowNum)
	{
		if (rowNum > fileNumRows) return;
		window.fileRows.erase(window.fileRows.begin() + rowNum);
		window.dirty = true;
		--fileNumRows;
	}
}
#endif //_WIN32
