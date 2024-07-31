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
		std::string statusMessage;
		std::weak_ptr<SyntaxHighlight::EditorSyntax> syntax;
	};

	void initConsole();
	void displayConsole();
	bool enableRawInput();
	void disableRawInput();
	void getInput();

	namespace ConsoleDetails
	{
		void KeyEvent(KEY_EVENT_RECORD, uint8_t& outCount);
	}
}