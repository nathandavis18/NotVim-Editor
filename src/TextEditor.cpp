#include "TextEditor.hpp"
#include "Console/Console.hpp"
#include <iostream>
#include <conio.h>

using KeyActions::KeyAction;
#define sci(KeyAction) static_cast<int>(KeyAction)

namespace Editor
{
	static constexpr uint8_t arrowKeyCode = 224;
	void handleInput()
	{
		uint8_t inputCount = 0;
		int x = _getch();

		if (x == arrowKeyCode)
		{
			x = _getch();
			switch (x)
			{
			case sci(KeyAction::ArrowLeft):
			case sci(KeyAction::ArrowRight):
			case sci(KeyAction::ArrowUp):
			case sci(KeyAction::ArrowDown):
				Console::moveCursor(x);
				break;
			default:
				std::cout << "Why are we here";
				break;
			}
		}
		else
		{
			switch (x)
			{
			case sci(KeyAction::Delete):
			case sci(KeyAction::Backspace):
				Console::deleteChar(x);
				break;
			}
		}
	}
}
