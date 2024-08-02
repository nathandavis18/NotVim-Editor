#include "TextEditor.hpp"
#include "Console/Console.hpp"
#include <iostream>
#include <conio.h>

using KeyActions::KeyAction;
#define sci(KeyAction) static_cast<int>(KeyAction)

namespace Editor
{
	static constexpr uint8_t arrowKeyCode = 224;
	static constexpr bool functionKeyCode = 0;
	void handleInput()
	{
		uint8_t inputCount = 0;
		int x = _getch();

		if (x == functionKeyCode)
		{
			int _ = _getch(); //Ignore the function key specifier value
			return; //Don't do anything if a function key (F1, F2, etc.) is pressed
		}
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
		else if (x == sci(KeyAction::Esc))
		{
			return; //just for now
		}
		else
		{
			switch (x)
			{
			case sci(KeyAction::Delete):
			case sci(KeyAction::Backspace):
				Console::deleteChar(x);
				break;
			case sci(KeyAction::Enter):
				Console::addRow();
				break;
			default:
				Console::insertChar(x);
				break;
			}
		}
	}
}
