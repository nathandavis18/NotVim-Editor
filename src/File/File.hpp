#pragma once
#include <string>

namespace File
{
	void setFileName(std::string_view name);
	void loadFileContents();
	std::string& contents();
	void saveFile(std::string_view newContents);
}