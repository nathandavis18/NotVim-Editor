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
			Console::clearScreen(); //Start fresh if updating
			Console::refreshScreen();
		}
	}
}

int main(int argc, const char** argv)
{
	argc = 2;
	argv[1] = "test1.txt";
	if (argc != 2)
	{
		std::cerr << "Usage: name <filename>";
		return EXIT_FAILURE;
	}

	Console::initConsole(argv[1]);

	//std::thread t(updateScreen);
	//t.detach();

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

	runThread = false;
	//t.join();

    return EXIT_SUCCESS;
}