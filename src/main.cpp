#include "SyntaxHighlight/SyntaxHighlight.hpp"
#include "TextEditor.hpp"
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
		Console::refreshScreen();
		Editor::handleInput();
	}

    return EXIT_SUCCESS;
}