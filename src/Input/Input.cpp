#include "Input.hpp"
#include "../Console/Console.hpp"
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#elif defined(__linux__) || defined(__APPLE__)
uint8_t _getch();
#endif

using KeyActions::KeyAction;
#define sci(KeyAction) static_cast<uint8_t>(KeyAction)

namespace InputHandler
{
	static constexpr uint8_t specialKeyCode = 224;
	static constexpr bool functionKeyCode = 0;
	void doCommand()
	{
		char input = std::cin.get();
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
		uint8_t input = _getch();
		std::cout << std::to_string(static_cast<uint8_t>(input)); exit(0);
#ifdef _WIN32
		if (input == functionKeyCode)
		{
			uint8_t _ = _getch(); //Ignore the function key specifier value
			return; //Don't do anything if a function key (F1, F2, etc.) is pressed
		}
		if (input == specialKeyCode)
		{
			input = _getch();
		}
#endif
		if (input == sci(KeyAction::Esc))
		{
			Console::mode(Mode::ReadMode);
		}
		else
		{
			switch (input)
			{
			case sci(KeyAction::Delete):
			case sci(KeyAction::Backspace):
				Console::deleteChar(input);
				break;
			case sci(KeyAction::ArrowDown):
			case sci(KeyAction::ArrowUp):
			case sci(KeyAction::ArrowLeft):
			case sci(KeyAction::ArrowRight):
				Console::moveCursor(input);
				break;
			case sci(KeyAction::CtrlArrowDown):
			case sci(KeyAction::CtrlArrowUp):
				Console::shiftRowOffset(input);
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
uint8_t _getch()
{
	uint8_t nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) == 0);
	if (nread == -1) exit(EXIT_FAILURE);

	while (true)
	{
		switch (c)
		{
		case sci(KeyAction::Esc):
			char seq[3];
			if (read(STDIN_FILENO, seq, 1) == 0) return sci(KeyAction::Esc);
			if (read(STDIN_FILENO, seq + 1, 1) == 0) return sci(KeyAction::Esc);

			if (seq[0] == '[')
			{
				if (read(STDIN_FILENO, seq + 2, 1) == 0) return sci(KeyAction::Esc);
				if (seq[2] == '~')
				{
					switch (seq[1])
					{
					case '3': return sci(KeyAction::Delete);
					case '5': return sci(KeyAction::PageUp);
					case '6': return sci(KeyAction::PageDown);
					}
				}
				else
				{
					switch (seq[1])
					{
					case 'A': return sci(KeyAction::ArrowUp);
					case 'B': return sci(KeyAction::ArrowDown);
					case 'C': return sci(KeyAction::ArrowRight);
					case 'D': return sci(KeyAction::ArrowLeft);
					case 'H': return sci(KeyAction::Home);
					case 'F': return sci(KeyAction::End);
					}
				}
			}
			else if (seq[0] == 'O')
			{
				switch (seq[1])
				{
				case 'H': return sci(KeyAction::Home);
				case 'F': return sci(KeyAction::End);
				}
			}
			break;
		default:
			return c;
		}
	}
}
#endif
