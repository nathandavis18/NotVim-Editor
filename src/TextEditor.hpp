#pragma once
#include "File/File.hpp"
#include "SyntaxHighlight/SyntaxHighlight.hpp"

#include <string>

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

	struct Row
	{
		std::string line;
		bool dirty;
	};
	void loadRows();
	void getCommand();
}