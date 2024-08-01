#include "SyntaxHighlight/SyntaxHighlight.hpp"
#include "TextEditor.hpp"
#include "File/File.hpp"
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
	SyntaxHighlight::initSyntax();
	File::setFileName(argv[1]);

	File::loadFileContents();
	Editor::loadRows();
	Console::initConsole();
	//Console::displayConsole();
	Console::refreshScreen();
	//Editor::getCommand();

    return EXIT_SUCCESS;
}