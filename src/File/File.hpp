#pragma once
#include <string>
#include <vector>

namespace FileHandler
{
	std::string& fileName();
	void loadFileContents();
	std::string& contents();
	void saveFile(std::string_view newContents);

	void loadRows();
	struct Row
	{
		std::string line;
		std::string renderedLine;
	};

	std::vector<Row>& rows();
}