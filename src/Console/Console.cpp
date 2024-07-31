#include "Console.hpp"
#include <iostream>

namespace Console
{
	using namespace ConsoleDetails;

	Window::Window() : cursorX(0), cursorY(0), rowOffset(0), colOffset(0), rows(0), cols(0), dirty(false), statusMessage("Test"), editorRows(Editor::rows()) {}

	Window window{};
	DWORD defaultMode;
	void initConsole()
	{
		if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &defaultMode))
		{
			std::cerr << "Error retrieving current console mode";
			exit(EXIT_FAILURE);
		}
		if (!enableRawInput())
		{
			std::cerr << "Error enabling raw input mode";
			exit(EXIT_FAILURE);
		}
	}
	void displayConsole()
	{
		for (const auto& row : window.editorRows)
		{
			std::cout << row.line << std::endl;
		}
	}
	bool enableRawInput()
	{
		DWORD rawMode = ENABLE_EXTENDED_FLAGS | (defaultMode & ~ENABLE_LINE_INPUT & ~ENABLE_PROCESSED_INPUT & ~ENABLE_ECHO_INPUT
			& ~ENABLE_PROCESSED_OUTPUT & ~ENABLE_WRAP_AT_EOL_OUTPUT);
		return SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), rawMode);
	}
	void disableRawInput()
	{
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), defaultMode);
	}
	void getInput()
	{
		uint8_t inputs = 0;
		INPUT_RECORD inBuf;
		DWORD cNumRead;

		HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
		while (inputs < 10)
		{
			ReadConsoleInput(stdIn, &inBuf, 1, &cNumRead);
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
		char x = key.uChar.AsciiChar;
		std::cout << key.wVirtualKeyCode << std::endl;
		if (key.bKeyDown && key.wVirtualKeyCode >= 65 && key.wVirtualKeyCode <= 90)
		{
			std::cout << "Key pressed: " << x << std::endl;
			++outCount;
		}
	}
}
