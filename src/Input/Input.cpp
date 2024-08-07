/**
* MIT License

Copyright (c) 2024 Nathan Davis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Input.hpp"
#include "../Console/Console.hpp"
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#elif defined(__linux__) || defined(__APPLE__)
uint8_t _getch();
#endif

using KeyActions::KeyAction;

namespace InputHandler
{
	/// <summary>
	/// Handles commands while in command/read mode
	/// i = Enter edit mode (like VIM)
	/// : = Enter command mode (like VIM)
	/// </summary>
	void doCommand()
	{
		uint8_t input = _getch();
		std::string command;
		switch (input)
		{
		case 'i':
			Console::enableRawInput();
			Console::enableEditMode();
			break;
		case ':':
			std::cout << "Here";
			Console::disableRawInput();
			Console::enableCommandMode();
			std::cout << ":";
			std::cin >> command;

			if (command == "q" && Console::isDirty()) //Quit command - requires changes to be saved
			{
				break;
			}
			else if (command == "q")
			{
				Console::mode(Mode::ExitMode);
				break;
			}
			else if (command == "q!") //Force quit. Don't bother checking anything
			{
				Console::mode(Mode::ExitMode);
				break;
			}
			else if (command == "w" || command == "s") //Save commands ([w]rite / [s]ave)
			{
				Console::save();
			}
			else if (command == "wq" || command == "sq") //Save and quit commands ([w]rite [q]uit / [s]ave [q]uit)
			{
				Console::save();
				Console::mode(Mode::ExitMode);
				break;
			}
			Console::mode(Mode::ReadMode); //Go back to read mode after executing a command
			break;
		default: //Unknown command. Just go back to read mode
			Console::mode(Mode::ReadMode);
			return;
		}
		Console::enableRawInput();
		Console::clearScreen();
	}

	/// <summary>
	/// Handles the input while in edit mode.
	/// Windows uses the _getch() function from <conio.h>.
	/// Linux uses a custom _getch() function
	/// </summary>
	void handleInput()
	{
		uint8_t input = _getch();

#ifdef _WIN32
		static constexpr uint8_t specialKeyCode = 224;
		static constexpr bool functionKeyCode = 0;
		if (input == functionKeyCode)
		{
			uint8_t _ = _getch(); //Ignore the function key specifier value
			return; //Don't do anything if a function key (F1, F2, etc.) is pressed
		}
		else if (input == specialKeyCode)
		{
			input = _getch();
		}
#endif

		if (input == static_cast<uint8_t>(KeyAction::Esc))
		{
			Console::mode(Mode::ReadMode);
		}
		else
		{
			switch (input)
			{
			case static_cast<uint8_t>(KeyAction::Delete):
			case static_cast<uint8_t>(KeyAction::Backspace):
				Console::deleteChar(input);
				break;
			case static_cast<uint8_t>(KeyAction::ArrowDown):
			case static_cast<uint8_t>(KeyAction::ArrowUp):
			case static_cast<uint8_t>(KeyAction::ArrowLeft):
			case static_cast<uint8_t>(KeyAction::ArrowRight):
				Console::moveCursor(input);
				break;
			case static_cast<uint8_t>(KeyAction::CtrlArrowDown):
			case static_cast<uint8_t>(KeyAction::CtrlArrowUp):
				Console::shiftRowOffset(input);
				break;
			case static_cast<uint8_t>(KeyAction::Enter):
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
	uint8_t c;
	while ((nread = read(STDIN_FILENO, &c, 1)) == 0);
	if (nread == -1) exit(EXIT_FAILURE);


	while (true)
	{
		switch (c)
		{
		case static_cast<uint8_t>(KeyAction::Esc): //Escape sequences for certain characters
			char seq[3];
			if (read(STDIN_FILENO, seq, 1) == 0) return static_cast<uint8_t>(KeyAction::Esc);
			if (read(STDIN_FILENO, seq + 1, 1) == 0) return static_cast<uint8_t>(KeyAction::Esc);

			if (seq[0] == '[')
			{
				if (read(STDIN_FILENO, seq + 2, 1) > 0)
				{
					std::cout << seq; exit(0);
					if (seq[2] == '~')
					{
						switch (seq[1])
						{
						case '3': return static_cast<uint8_t>(KeyAction::Delete);
						case '5': return static_cast<uint8_t>(KeyAction::PageUp);
						case '6': return static_cast<uint8_t>(KeyAction::PageDown);
						}
					}
					else if (seq[2] == ';')
					{
						switch (seq[1])
						{
						case '3': return static_cast<uint8_t>(KeyAction::CtrlDelete);
						case '5': return static_cast<uint8_t>(KeyAction::CtrlPageUp);
						case '6': return static_cast<uint8_t>(KeyAction::CtrlPageDown);
						}
					}
				}
				else
				{
					switch (seq[1])
					{
					case 'A': return static_cast<uint8_t>(KeyAction::ArrowUp);
					case 'B': return static_cast<uint8_t>(KeyAction::ArrowDown);
					case 'C': return static_cast<uint8_t>(KeyAction::ArrowRight);
					case 'D': return static_cast<uint8_t>(KeyAction::ArrowLeft);
					case 'H': return static_cast<uint8_t>(KeyAction::Home);
					case 'F': return static_cast<uint8_t>(KeyAction::End);
					}
				}
			}
			else if (seq[0] == 'O')
			{
				switch (seq[1])
				{
				case 'H': return static_cast<uint8_t>(KeyAction::Home);
				case 'F': return static_cast<uint8_t>(KeyAction::End);
				}
			}
			break;
		default:
			return c;
		}
	}
}
#endif
