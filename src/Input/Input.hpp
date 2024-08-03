#pragma once
#include "../SyntaxHighlight/SyntaxHighlight.hpp"
#include "../KeyActions/KeyActions.hh"
#include <string>

namespace InputHandler
{
	enum class Mode
	{
		CommandMode,
		InputMode,
		FindMode,
		ExitMode
	};
	void handleInput();
	void doCommand();
	Mode getMode();
}