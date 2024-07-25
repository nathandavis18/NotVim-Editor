#pragma once
#include <string>

#include "../SyntaxHighlight/SyntaxHighlight.hpp"
namespace File
{
	struct FileRow
	{
		size_t index;
		size_t size;
		int renderedSize;
		std::string contents;
		std::string render;
		uint8_t* highlight;
		int highlightOpenComment;
	};

	struct EditorConfig
	{
		EditorConfig();
		int cursorX, cursorY;
		int rowOffset, colOffset;
		int screenRows, screenCols;
		size_t numRows;
		bool rawMode;
		FileRow* row;
		bool notSaved;
		std::string fileName;
		std::string statusMessage;
		SyntaxHighlight::EditorSyntax* syntax;
	};

	static EditorConfig editor;

	void disableRawMode(int fileDescriptor);
	bool enableRawMode(int fileDescriptor);
	void updateEditorOnExit();
}