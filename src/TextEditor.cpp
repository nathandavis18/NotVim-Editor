#include "TextEditor.hpp"
#include <iostream>
namespace Editor
{
	std::vector<Row> rows;
	void loadRows()
	{
		size_t lineBreak = 0;
		while ((lineBreak = File::contents().find('\n')) != std::string::npos)
		{
			rows.emplace_back(File::contents().substr(0, lineBreak), false);
			File::contents().erase(File::contents().begin(), File::contents().begin() + lineBreak + 1);
		}
		rows.emplace_back(File::contents().substr(0, File::contents().length()), false);
		File::contents().clear();

		for (const auto& row : rows)
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
