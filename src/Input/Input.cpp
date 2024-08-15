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
KeyAction _getch();
#endif

using KeyActions::KeyAction;

namespace InputHandler
{
	/// <summary>
	/// Filters through some sequences to determine which key was pressed
	/// </summary>
	/// <returns></returns>
	const KeyAction getInput()
	{
#ifdef _WIN32
		uint8_t input;
#elif defined(__linux__) || defined(__APPLE__)
		KeyAction input;
#endif

		input = _getch();

#ifdef _WIN32
		static constexpr uint8_t specialKeyCode = 224;
		static constexpr bool functionKeyCode = 0;
		if (input == functionKeyCode)
		{
			uint8_t _ = _getch(); //Ignore the function key specifier value
			return KeyAction::None; //Don't do anything if a function key (F1, F2, etc.) is pressed
		}
		else if (input == specialKeyCode) //This is the code to determine arrow key presses, delete, end, home, pageup/down
		{
			input = _getch(); //Get the special key identifier
			switch (input)
			{
				//When not holding control
			case 'K': return KeyAction::ArrowLeft;
			case 'M': return KeyAction::ArrowRight;
			case 'P': return KeyAction::ArrowDown;
			case 'H': return KeyAction::ArrowUp;
			case 'S': return KeyAction::Delete;
			case 'O': return KeyAction::End;
			case 'G': return KeyAction::Home;
			case 'Q': return KeyAction::PageDown;
			case 'I': return KeyAction::PageUp;

				//When holding control
			case 's': return KeyAction::CtrlArrowLeft;
			case 't': return KeyAction::CtrlArrowRight;
			case 145: return KeyAction::CtrlArrowDown; //Non-letter ASCII Code
			case 141: return KeyAction::CtrlArrowUp; //Unused ASCII Code
			case '"': return KeyAction::CtrlDelete;
			case 'u': return KeyAction::CtrlEnd;
			case 'w': return KeyAction::CtrlHome;
			case 'v': return KeyAction::CtrlPageDown;
			case 134: return KeyAction::CtrlPageUp; //Non-letter ASCII Code
			}
		}
#endif

		return static_cast<KeyAction>(input);
	}
	/// <summary>
	/// Handles commands while in command/read mode
	/// i = Enter edit mode (like VIM)
	/// : = Enter command mode (like VIM)
	/// </summary>
	void doCommand(const uint8_t input)
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
	void handleInput(KeyAction key)
	{
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
			Console::insertChar(static_cast<uint8_t>(key));
			break;
		}
	}
}

#if defined(__linux__) || defined(__APPLE__)
/// <summary>
/// A custom implementation of the _getch function
/// </summary>
/// <returns></returns>
KeyAction _getch()
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
			if (read(STDIN_FILENO, seq, 1) == 0) return KeyAction::Esc;
			if (read(STDIN_FILENO, seq + 1, 1) == 0) return KeyAction::Esc;

			if (seq[0] == '[')
			{
				if (read(STDIN_FILENO, seq + 2, 1) != 0)
				{
					if (seq[2] == '~')
					{
						switch (seq[1])
						{
						case '3': return KeyAction::Delete;
						case '5': return KeyAction::PageUp;
						case '6': return KeyAction::PageDown;
						}
					}
					else if (seq[2] == ';')
					{
						switch (seq[1])
						{
						case '1':
							if (read(STDIN_FILENO, seq, 1) == 0) return KeyAction::Esc;
							if (read(STDIN_FILENO, seq + 1, 1) == 0) return KeyAction::Esc;
							if (seq[0] == '5')
							{
								switch (seq[1])
								{
								case 'A': return KeyAction::CtrlArrowUp;
								case 'B': return KeyAction::CtrlArrowDown;
								case 'C': return KeyAction::CtrlArrowRight;
								case 'D': return KeyAction::CtrlArrowLeft;
								}
							}

						case '3': return KeyAction::CtrlDelete;
						case '5': return KeyAction::CtrlPageUp;
						case '6': return KeyAction::CtrlPageDown;
						}
					}
				}
				else
				{
					switch (seq[1])
					{
					case 'A': return KeyAction::ArrowUp;
					case 'B': return KeyAction::ArrowDown;
					case 'C': return KeyAction::ArrowRight;
					case 'D': return KeyAction::ArrowLeft;
					case 'H': return KeyAction::Home;
					case 'F': return KeyAction::End;
					}
				}
			}
			else if (seq[0] == 'O')
			{
				switch (seq[1])
				{
				case 'H': return KeyAction::Home;
				case 'F': return KeyAction::End;
				}
			}
			break;
		default:
			return static_cast<KeyAction>(c);
		}
	}
}
#endif
