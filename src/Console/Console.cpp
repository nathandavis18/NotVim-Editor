#include "Console.hpp"
#include "../KeyActions/KeyActions.hh"
#include <iostream>
#define NuttyVersion "0.1a"
namespace Console
{
	const char* testStatusMessage = "Test Status Message Length go BRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR\0";
	Window::Window() : cursorX(0), cursorY(0), rowOffset(0), colOffset(0), dirty(false), rawModeEnabled(false), statusMessage(testStatusMessage), editorRows(Editor::rows()) 
	{
		CONSOLE_SCREEN_BUFFER_INFO screenInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screenInfo);

		constexpr uint8_t statusMessageRows = 2;
		rows = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1 - statusMessageRows;
		cols = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;
	}
	Window window{};

#ifdef _WIN32 //Windows Console stuff
	using namespace ConsoleDetails;
	
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
	void getInput()
	{
		uint8_t inputs = 0;
		INPUT_RECORD inBuf;
		DWORD cNumRead;

		while (inputs < 1)
		{
			ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &inBuf, 1, &cNumRead);
			switch (inBuf.EventType)
			{
			case KEY_EVENT:
				KeyEvent(inBuf.Event.KeyEvent, inputs);
				break;
			default:
				break;
			}
		}
	}
	void ConsoleDetails::KeyEvent(KEY_EVENT_RECORD key, uint8_t& outCount)
	{
		constexpr uint8_t aKeyCode = 65, zKeyCode = 90;
		if (key.bKeyDown && key.wVirtualKeyCode >= aKeyCode && key.wVirtualKeyCode <= zKeyCode)
		{
			char x = key.uChar.AsciiChar;
			std::cout << "Key pressed: " << x << std::endl;
			++outCount;
		}
	}
	void refreshScreen()
	{
		Editor::Row* row;
		std::string buffer; buffer.reserve(32);
		std::string aBuffer;

		aBuffer.append("\x1b[?251");
		aBuffer.append("\x1b[H");
		for (uint8_t y = 0; y < window.rows; ++y)
		{
			size_t fileRow = window.rowOffset + y;
			if (fileRow >= window.editorRows.size())
			{
				if (window.editorRows.size() == 0 && y == window.rows / 3)
				{
					std::string welcome; welcome.resize(80);
					int welcomeLength = std::snprintf(welcome.data(), welcome.size(), "Nutty Editor -- version %s\x1b[0K\r\n", NuttyVersion);
					int padding = (window.cols - welcomeLength) / 2;
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
			
			row = &window.editorRows[fileRow];

			uint8_t length = row->renderedSize - window.colOffset;
			if (length > 0)
			{
				if (length > window.cols) length = window.cols;
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

		int cx = 1;
		size_t fileRow = window.rowOffset + window.cursorY;
		Editor::Row* cursorRow = (fileRow >= window.editorRows.size()) ? nullptr : &window.editorRows[fileRow];
		if (cursorRow)
		{
			for (uint8_t j = window.colOffset; j < (window.cursorX + window.colOffset); ++j, ++cx)
			{
				if (j < cursorRow->renderedSize && cursorRow->line[j] == static_cast<char>(KeyActions::KeyAction::Tab))
				{
					cx += 7 - (cx % 8);
				}
			}
		}
		snprintf(buffer.data(), buffer.length(), "\x1b[%d;%dH", window.cursorY + 1, cx);
		aBuffer.append(buffer);
		aBuffer.append("\x1b[?25h");
		std::cout << aBuffer;
	}
#endif //_WIN32
}
