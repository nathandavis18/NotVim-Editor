#include "Input/Input.hpp"
#include "Console/Console.hpp"

#include <iostream>

int main(int argc, const char** argv)
{
	argc = 2;
	argv[1] = "test1.txt";
	if (argc < 2)
	{
		std::cerr << "Usage: name <filename>";
		return EXIT_FAILURE;
	}

	Console::initConsole(argv[1]);

	while (true)
	{
		while (Console::mode() == Mode::CommandMode || Console::mode() == Mode::ReadMode)
		{
			if (!Console::isRawMode())
			{
				Console::enableRawInput();
			}
			Console::refreshScreen();
			InputHandler::doCommand();
		}
		while (Console::mode() == Mode::EditMode)
		{
			Console::refreshScreen();
			InputHandler::handleInput();
		}
		if (Console::mode() == Mode::ExitMode)
		{
			if (Console::isRawMode())
			{
				Console::disableRawInput();
			}
			break;
		}
	}

    return EXIT_SUCCESS;
}