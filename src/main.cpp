#include "SyntaxHighlight/SyntaxHighlight.hpp"
#include "Input/Input.hpp"
#include "File/File.hpp"
#include "Console/Console.hpp"

#include <iostream>

int main(int argc, const char** argv)
{
	argc = 2;
	argv[1] = "test.txt";
	if (argc < 2)
	{
		std::cerr << "Usage: name <filename>";
		return EXIT_FAILURE;
	}
	SyntaxHighlight::initSyntax();
	FileHandler::fileName() = argv[1];

	FileHandler::loadFileContents();
	FileHandler::loadRows();
	Console::initConsole();

	while (true)
	{
		while (InputHandler::getMode() == InputHandler::Mode::CommandMode)
		{
			if (!Console::isRawMode())
			{
				Console::enableRawInput();
			}
			Console::refreshScreen("COMMAND");
			InputHandler::doCommand();
		}
		while (InputHandler::getMode() == InputHandler::Mode::InputMode)
		{
			Console::refreshScreen("INSERT");
			InputHandler::handleInput();
		}
		if (InputHandler::getMode() == InputHandler::Mode::ExitMode)
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