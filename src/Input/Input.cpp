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
uint16_t _getch();
#endif

using KeyActions::KeyAction;

namespace InputHandler
{
	/// <summary>
	/// Filters through some sequences to determine which key was pressed
	/// </summary>
	/// <returns></returns>
	uint16_t getInput()
	{
		uint8_t input = _getch();
#ifdef _WIN32
		static constexpr uint8_t specialKeyCode = 224;
		static constexpr bool functionKeyCode = 0;
		if (input == functionKeyCode)
		{
			uint8_t _ = _getch(); //Ignore the function key specifier value
			return 0; //Don't do anything if a function key (F1, F2, etc.) is pressed
		}
		else if (input == specialKeyCode) //This is the code to determine arrow key presses, delete, end, home, pageup/down
		{
			input = _getch(); //Get the special key identifier
			switch (input)
			{
				//When not holding control
			case 'K': return static_cast<uint16_t>(KeyAction::ArrowLeft);
			case 'M': return static_cast<uint16_t>(KeyAction::ArrowRight);
			case 'P': return static_cast<uint16_t>(KeyAction::ArrowDown);
			case 'H': return static_cast<uint16_t>(KeyAction::ArrowUp);
			case 'S': return static_cast<uint16_t>(KeyAction::Delete);
			case 'O': return static_cast<uint16_t>(KeyAction::End);
			case 'G': return static_cast<uint16_t>(KeyAction::Home);
			case 'Q': return static_cast<uint16_t>(KeyAction::PageDown);
			case 'I': return static_cast<uint16_t>(KeyAction::PageUp);

				//When holding control
			case 's': return static_cast<uint16_t>(KeyAction::CtrlArrowLeft);
			case 't': return static_cast<uint16_t>(KeyAction::CtrlArrowRight);
			case 145: return static_cast<uint16_t>(KeyAction::CtrlArrowDown); //Non-letter ASCII Code
			case 141: return static_cast<uint16_t>(KeyAction::CtrlArrowUp); //Unused ASCII Code
			case '"': return static_cast<uint16_t>(KeyAction::CtrlDelete);
			case 'u': return static_cast<uint16_t>(KeyAction::CtrlEnd);
			case 'w': return static_cast<uint16_t>(KeyAction::CtrlHome);
			case 'v': return static_cast<uint16_t>(KeyAction::CtrlPageDown);
			case 134: return static_cast<uint16_t>(KeyAction::CtrlPageUp); //Non-letter ASCII Code
			}
		}
#endif
		return input;
	}
	/// <summary>
	/// Handles commands while in command/read mode
	/// i = Enter edit mode (like VIM)
	/// : = Enter command mode (like VIM)
	/// </summary>
	void doCommand(const uint16_t input)
	{
		std::string command;
		switch (input)
		{
		case 'i':
			Console::enableRawInput();
			Console::enableEditMode();
			break;
		case ':':
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
	void handleInput(const uint16_t input)
	{
		KeyAction key = static_cast<KeyAction>(input); //If there is an associated KeyAction
		switch (key)
		{
		case KeyAction::Esc:
			Console::mode(Mode::ReadMode);
			break;
		case KeyAction::Delete:
		case KeyAction::Backspace:
			Console::deleteChar(key);
			break;
		case KeyAction::ArrowDown:
		case KeyAction::ArrowUp:
		case KeyAction::ArrowLeft:
		case KeyAction::ArrowRight:
			Console::moveCursor(key);
			break;
		case KeyAction::CtrlArrowDown:
		case KeyAction::CtrlArrowUp:
			Console::shiftRowOffset(key);
			break;
		case KeyAction::Enter:
			Console::addRow();
			break;
		default:
			Console::insertChar(input);
			break;
		}
	}
}

#if defined(__linux__) || defined(__APPLE__)
/// <summary>
/// A custom implementation of the _getch function
/// </summary>
/// <returns></returns>
uint16_t _getch()
{
	uint8_t nread;
	uint8_t c;
	while ((nread = read(STDIN_FILENO, &c, 1)) == 0);
	if (nread == -1) exit(EXIT_FAILURE);


	while (true)
	{
		switch (c)
		{
		case static_cast<uint16_t>(KeyAction::Esc): //Escape sequences for certain characters
			char seq[3];
			if (read(STDIN_FILENO, seq, 1) == 0) return static_cast<uint16_t>(KeyAction::Esc);
			if (read(STDIN_FILENO, seq + 1, 1) == 0) return static_cast<uint16_t>(KeyAction::Esc);

			if (seq[0] == '[')
			{
				if (read(STDIN_FILENO, seq + 2, 1) != 0)
				{
					if (seq[2] == '~')
					{
						switch (seq[1])
						{
						case '3': return static_cast<uint16_t>(KeyAction::Delete);
						case '5': return static_cast<uint16_t>(KeyAction::PageUp);
						case '6': return static_cast<uint16_t>(KeyAction::PageDown);
						}
					}
					else if (seq[2] == ';')
					{
						switch (seq[1])
						{
						case '1':
							if (read(STDIN_FILENO, seq, 1) == 0) return static_cast<uint16_t>(KeyAction::Esc);
							if (read(STDIN_FILENO, seq + 1, 1) == 0) return static_cast<uint16_t>(KeyAction::Esc);
							if (seq[0] == '5')
							{
								switch (seq[1])
								{
								case 'A': return static_cast<uint16_t>(KeyAction::CtrlArrowUp);
								case 'B': return static_cast<uint16_t>(KeyAction::CtrlArrowDown);
								case 'C': return static_cast<uint16_t>(KeyAction::CtrlArrowRight);
								case 'D': return static_cast<uint16_t>(KeyAction::CtrlArrowLeft);
								}
							}

						case '3': return static_cast<uint16_t>(KeyAction::CtrlDelete);
						case '5': return static_cast<uint16_t>(KeyAction::CtrlPageUp);
						case '6': return static_cast<uint16_t>(KeyAction::CtrlPageDown);
						}
					}
				}
				else
				{
					switch (seq[1])
					{
					case 'A': return static_cast<uint16_t>(KeyAction::ArrowUp);
					case 'B': return static_cast<uint16_t>(KeyAction::ArrowDown);
					case 'C': return static_cast<uint16_t>(KeyAction::ArrowRight);
					case 'D': return static_cast<uint16_t>(KeyAction::ArrowLeft);
					case 'H': return static_cast<uint16_t>(KeyAction::Home);
					case 'F': return static_cast<uint16_t>(KeyAction::End);
					}
				}
			}
			else if (seq[0] == 'O')
			{
				switch (seq[1])
				{
				case 'H': return static_cast<uint16_t>(KeyAction::Home);
				case 'F': return static_cast<uint16_t>(KeyAction::End);
				}
			}
			break;
		default:
			return c;
		}
	}
}
#endif
