#include "Input.hpp"
#include "../Console/Console.hpp"
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#elif defined(__linux__) || defined(__APPLE__)
char _getch();
#endif

using KeyActions::KeyAction;
#define sci(KeyAction) static_cast<char>(KeyAction)

namespace InputHandler
{
	static constexpr uint8_t specialKeyCode = 224;
	static constexpr bool functionKeyCode = 0;
	void doCommand()
	{
		char input = _getch();
		std::string command;
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
			Console::mode(Mode::ReadMode);
			break;
		default:
			Console::mode(Mode::ReadMode);
			return;
		}
		Console::clearScreen();
	}
	void handleInput()
	{
		uint8_t inputCount = 0;
		char input = _getch();
		std::cout << input; exit(0);
		if (input == functionKeyCode)
		{
			char _ = _getch(); //Ignore the function key specifier value
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
#if defined(__linux__) || defined(__APPLE__)
			input = _getch();
			if (input == '[')
			{
				input = _getch();
				if (input >= '0' && input <= '9')
				{
					if (read(STDIN_FILENO, &input, 1) == 0)
					{
						Console::mode(Mode::ReadMode);
						return;
					}
					if (input == '~')
					{
						//Dont have anything for this yet
						switch (seq[1])
						{
						case '3': Console::deleteChar(sci(KeyAction::Delete); return;
						case '5': return; //sci(KeyAction::PageUp);
						case '6': return; //sci(KeyAction::PageDown);
						}
					}
				}
				else
				{
					switch (input)
					{
					case 'A': Console::moveCursor(sci(KeyAction::ArrowUp)); return;
					case 'B': Console::moveCursor(sci(KeyAction::ArrowDown)); return;
					case 'C': Console::moveCursor(sci(KeyAction::ArrowRight)); return;
					case 'D': Console::moveCursor(sci(KeyAction::ArrowLeft)); return;
					case 'H': return; //sci(KeyAction::Home);
					case 'F': return; //sci(KeyAction::End);
					}
				}
			}
#else
			Console::mode(Mode::ReadMode);
#endif
		}
		else
		{
			std::cout << input;
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

#if defined(__linux__) || defined(__APPLE__)
	char _getch()
	{
		int nread;
		char c;
		while ((nread = read(STDIN_FILENO, &c, 1)) == 0);
		if (nread == -1) exit(EXIT_FAILURE);

		return c;
	}
#endif
