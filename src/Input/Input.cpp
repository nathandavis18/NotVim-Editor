#include "Input.hpp"
#include "../Console/Console.hpp"
#include <iostream>
#include <conio.h>

using KeyActions::KeyAction;
#define sci(KeyAction) static_cast<int>(KeyAction)

namespace InputHandler
{
	static constexpr uint8_t specialKeyCode = 224;
	static constexpr bool functionKeyCode = 0;
	void doCommand()
	{
		char input = std::cin.get();
		switch (input)
		{
		case 'i':
			if (!Console::isRawMode())
			{
				Console::enableRawInput();
			}
			Console::enableEditMode();
			break;
		case ':':
			if (Console::isRawMode())
			{
				Console::disableRawInput();
			}
			Console::enableCommandMode();
			std::string command; 
			std::cout << ":";
			std::cin >> command;

			if (command == "q" && Console::isDirty())
			{
				break; //don't quit if file isn't saved
			}
			else if (command == "q")
			{
				Console::mode(Mode::ExitMode);
				break;
			}
			else if (command == "q!")
			{
				Console::mode(Mode::ExitMode); //Force quit, don't check if file isn't saved
				break;
			}
			else if (command == "w" || command == "s")
			{
				Console::save();
			}
			else if (command == "wq" || command == "sq")
			{
				Console::save();
				Console::mode(Mode::ExitMode);
				break;
			}
			else
			{
				break;
			}
		}
		Console::clearScreen();
	}
	void handleInput()
	{
		uint8_t inputCount = 0;
		int input = _getch();

		if (input == functionKeyCode)
		{
			int _ = _getch(); //Ignore the function key specifier value
			return; //Don't do anything if a function key (F1, F2, etc.) is pressed
		}
		if (input == specialKeyCode)
		{
			input = _getch();
			switch (input)
			{
			case sci(KeyAction::ArrowLeft):
			case sci(KeyAction::ArrowRight):
			case sci(KeyAction::ArrowUp):
			case sci(KeyAction::ArrowDown):
				Console::moveCursor(input);
				break;
			case sci(KeyAction::Delete):
				Console::deleteChar(input);
				break;
			case sci(KeyAction::CtrlArrowDown):
			case sci(KeyAction::CtrlArrowUp):
				Console::shiftRowOffset(input);
				break;
			default:
				std::cout << "Why are we here";
				break;
			}
		}
		else if (input == sci(KeyAction::Esc))
		{
			Console::mode(Mode::ReadMode);
		}
		else
		{
			switch (input)
			{
			case sci(KeyAction::Backspace):
				Console::deleteChar(input);
				break;
			case sci(KeyAction::Enter):
				Console::addRow();
				break;
			default:
				Console::insertChar(input);
				break;
			}
		}
	}
}
