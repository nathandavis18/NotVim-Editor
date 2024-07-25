#pragma once
#include <iostream>
#include "File/File.hpp"
#include "SyntaxHighlight/SyntaxHighlight.hpp"

namespace Editor
{
	enum class KeyAction
	{
		KeyNull,
		CtrlC,
		CtrlF,
		CtrlX,
		CtrlV,
		Tab,
		Enter,
		CtrlS,
		Esc,
		Backspace,
		Delete,

		ArrowLeft,
		ArrowRight,
		ArrowUp,
		ArrowDown,
		Home,
		End,
		PageUp,
		PageDown
	};

	int editorReadKey(int fileDescriptor);
	bool getCursorPosition(int* rows, int* cols);
	bool isSeparator(char c);
	bool rowHasOpenComment(File::FileRow* row);
}