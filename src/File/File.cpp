/**
* MIT License

Copyright (c) 2024 Nathan Davis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "File.hpp"
#include <filesystem>
#include <fstream>

namespace FileHandler
{
	std::string _fileName = "";

	std::string& fileName(const std::string_view& fName)
	{
		if (!fName.empty())
		{
			_fileName = fName;
		}
		return _fileName;
	}

	std::vector<Row> loadFileContents()
	{
		std::filesystem::path path = std::filesystem::current_path() / _fileName;
		std::ifstream file(path);
		std::stringstream ss;
		ss << file.rdbuf();
		file.close();

		return loadRows(std::move(ss.str()));
	}

	/// <summary>
	/// Loads the
	/// </summary>
	/// <param name="str"></param>
	std::vector<Row> loadRows(std::string&& str)
	{
		std::vector<Row> contents;
		if (str.length() > 0)
		{
			size_t lineBreak = 0;
			while ((lineBreak = str.find('\n')) != std::string::npos)
			{
				contents.emplace_back(str.substr(0, lineBreak));
				str.erase(str.begin(), str.begin() + lineBreak + 1);
			}
			contents.emplace_back(str.substr(0, str.length()));
			str.clear();
		}
		return contents;
	}

	/// <summary>
	/// Writes the current contents to the file
	/// </summary>
	/// <param name="newContents"></param>
	void saveFile(const std::string_view& newContents)
	{
		std::filesystem::path path = std::filesystem::current_path() / _fileName;
		std::ofstream file(path);
		file << newContents;
	}
}