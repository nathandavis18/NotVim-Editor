#pragma once
#include "../TextEditor.hpp"
#include "../SyntaxHighlight/SyntaxHighlight.hpp"
#include <vector>
#include <string>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#elif __linux__ || __APPLE__
#include <termios.h>
#include <ioctl.h>
#endif
namespace Console
{
	struct Window
	{
		Window();
		int cursorX, cursorY;
		int rowOffset, colOffset;
		size_t rows, cols;
		
		std::vector<Editor::Row>& editorRows;

		bool dirty;
		bool rawModeEnabled;
		std::string statusMessage;
		std::weak_ptr<SyntaxHighlight::EditorSyntax> syntax;
	};

	void initConsole();
	bool enableRawInput();
	void disableRawInput();
	void getInput();
	void refreshScreen();

#ifdef _WIN32
	namespace ConsoleDetails
	{
		void KeyEvent(KEY_EVENT_RECORD, uint8_t& outCount);
	}
#endif //_Win32
}