#include "TextEditor.hpp"
#include <iostream>
namespace Editor
{
	std::vector<Row> rowContents;

	std::vector<Row>& rows()
	{
		return rowContents;
	}

	void loadRows()
	{
		size_t lineBreak = 0;
		while ((lineBreak = File::contents().find('\n')) != std::string::npos)
		{
			rowContents.emplace_back(File::contents().substr(0, lineBreak), File::contents().substr(0, lineBreak), lineBreak);
			File::contents().erase(File::contents().begin(), File::contents().begin() + lineBreak + 1);
		}
		rowContents.emplace_back(File::contents().substr(0, File::contents().length()), File::contents().substr(0, lineBreak), File::contents().length());
		File::contents().clear();

		for (const auto& row : rowContents)
		{
			std::cout << row.line << std::endl;
		}
	}
	void getCommand()
	{
		std::string newContents;
		while (std::cin)
		{
			std::string str;
			std::getline(std::cin, str);
			if (str == "q")
			{
				break;
			}
			else if (str == "s")
			{
				File::saveFile(newContents);
			}
			else if (str == "sq")
			{
				File::saveFile(newContents);
				break;
			}
			else if (str == "i")
			{
				std::getline(std::cin, newContents);
			}
			else if (str == "p")
			{
				std::cout << newContents << std::endl;
			}
			else
			{
				std::cout << str << std::endl;
			}
		}
	}
}
