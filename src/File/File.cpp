#include "File.hpp"
#include <filesystem>
#include <fstream>

namespace File
{
	std::string fileName = "";
	std::string fileContents = "";

	void setFileName(std::string_view name)
	{
		fileName = name;
	}
	void loadFileContents()
	{
		std::filesystem::path path = std::filesystem::current_path() / fileName;
		std::ifstream file(path);
		std::stringstream ss;
		ss << file.rdbuf();
		file.close();

		fileContents = ss.str();
	}

	std::string& contents()
	{
		return fileContents;
	}

	void saveFile(std::string_view newContents)
	{
		std::filesystem::path path = std::filesystem::current_path() / fileName;
		std::ofstream file(path);
		file << newContents;
		file.close();
	}
}