#include "File.hpp"
#include <filesystem>
#include <fstream>

namespace FileHandler
{
	std::vector<Row> rowContents;

	std::vector<Row>& rows()
	{
		return rowContents;
	}

	std::string _fileName = "";
	std::string fileContents = "";

	std::string& fileName(const std::string_view& fName)
	{
		if (!fName.empty())
		{
			_fileName = fName;
		}
		return _fileName;
	}

	void loadFileContents()
	{
		std::filesystem::path path = std::filesystem::current_path() / _fileName;
		std::ifstream file(path);
		std::stringstream ss;
		ss << file.rdbuf();
		file.close();

		fileContents = ss.str();
	}

	void loadRows()
	{
		if (FileHandler::contents().length() > 0)
		{
			size_t lineBreak = 0;
			while ((lineBreak = FileHandler::contents().find('\n')) != std::string::npos)
			{
				rowContents.emplace_back(FileHandler::contents().substr(0, lineBreak));
				FileHandler::contents().erase(FileHandler::contents().begin(), FileHandler::contents().begin() + lineBreak + 1);
			}
			rowContents.emplace_back(FileHandler::contents().substr(0, FileHandler::contents().length()));
			FileHandler::contents().clear();
		}
	}

	std::string& contents()
	{
		return fileContents;
	}

	void saveFile(std::string_view newContents)
	{
		std::filesystem::path path = std::filesystem::current_path() / _fileName;
		std::ofstream file(path);
		file << newContents;
	}
}