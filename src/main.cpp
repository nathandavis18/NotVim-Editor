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

#include "Input/Input.hpp"
#include "Console/Console.hpp"

#include <iostream>
#include <thread>

static std::atomic<bool> runThread = true; //Main thread will update this bool, while secondary thread reads from it

void updateScreen() //Makes sure the screen stays up to date with the size
{
	while (runThread)
	{
		if (Console::setWindowSize()) //Only update if the terminal screen actually got updated
		{
			Console::refreshScreen();
		}
	}
}

int main(int argc, const char** argv)
{
#ifndef NDEBUG
	argc = 2;
	argv[1] = "test.cpp";
#endif
	if (argc != 2)
	{
		std::cerr << "ERROR: Usage: nve <filename>\n";
		return EXIT_FAILURE;
	}

	Console::initConsole(argv[1]);

	std::thread t(updateScreen);
	t.detach();

	while (true)
	{
		while (Console::mode() == Mode::CommandMode || Console::mode() == Mode::ReadMode)
		{
			Console::refreshScreen();
			const KeyActions::KeyAction inputCode = InputHandler::getInput();
			if (inputCode != KeyActions::KeyAction::None)
			{
				InputHandler::doCommand(inputCode);
				Console::prepRenderedString();
			}
		}
		while (Console::mode() == Mode::EditMode)
		{
			Console::prepRenderedString();
			Console::refreshScreen();
			const KeyActions::KeyAction inputCode = InputHandler::getInput();
			if (inputCode != KeyActions::KeyAction::None)
			{
				InputHandler::handleInput(inputCode);
			}
		}
		if (Console::mode() == Mode::ExitMode)
		{
			Console::disableRawInput();
			break;
		}
	}

	runThread = false;
	if(t.joinable()) t.join();
	
	return EXIT_SUCCESS;
}