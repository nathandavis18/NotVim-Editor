#include "Input.hpp"
#include "../Console/Console.hpp"
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
char _getch();
#endif

using KeyActions::KeyAction;
#define sci(KeyAction) static_cast<int>(KeyAction)

namespace InputHandler
{
	static constexpr uint8_t specialKeyCode = 224;
	static constexpr bool functionKeyCode = 0;
	void doCommand()
	{
		unsigned char input = std::cin.get();
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

#if defined(__linux__) || defined(__APPLE__)
static termios old, current;
void initTermios()
{
	tcgetattr(STDIN_FILENO, &old);
	current = old;
	current.c_lflag &= ~ICANON;
	current.c_lflag &= ~ECHO;

	tcsetattr(STDIN_FILENO, &current);
}
unsigned char _getch()
{
	initTermios();
	unsigned char ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &old);
	return ch;
}
#endif
