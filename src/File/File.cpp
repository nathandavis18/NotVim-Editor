#include "File.hpp"
namespace File
{
	EditorConfig::EditorConfig() : cursorX(0), cursorY(0), rowOffset(0), colOffset(0), screenRows(0), screenCols(0),
		numRows(0), rawMode(false), notSaved(true), row(nullptr), syntax(nullptr)
	{}
	void disableRawMode(int fileDescriptor)
	{
		if (editor.rawMode)
		{
			editor.rawMode = false;
		}
	}
	bool enableRawMode(int fileDescriptor)
	{
		if (editor.rawMode) return true;
		if (fileDescriptor != _fileno(stdin)) return false;
		atexit(updateEditorOnExit);

	}
	void updateEditorOnExit()
	{
		disableRawMode(_fileno(stdin));
	}
}