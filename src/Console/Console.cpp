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

#include "Console.hpp"
#include "KeyActions/KeyActions.hh"

#include <iostream>
#include <fstream>
#include <format> //C++20 is required. MSVC/GCC-13/Clang-14/17/AppleClang-15

#ifdef _WIN32
#include <Windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#endif
#define NotVimVersion "0.3.0a"

/// <summary>
/// Construct the window
/// </summary>
/// <param name="fileName"></param>
Console::Window::Window() : fileCursorX(0), fileCursorY(0), cols(0), rows(0), renderedCursorX(0), renderedCursorY(0), colNumberToDisplay(0), savedRenderedCursorXPos(0),
rowOffset(0), colOffset(0), dirty(false), rawModeEnabled(false), fileRows(FileHandler::loadFileContents()), syntax(SyntaxHighlight::syntax())
{}

/// <summary>
/// Sets/Gets the current mode the editor is in
/// </summary>
/// <param name="m"></param>
/// <returns></returns>
Mode& Console::mode(Mode m)
{
	if (m != Mode::None)
	{
		mMode = m;
	}
	return mMode;
}

/// <summary>
/// Steps to be taken before refreshScreen() is called
/// </summary>
void Console::prepRenderedString()
{
	if (mWindow->fileRows.size() > 0 && mMode != Mode::CommandMode) fixRenderedCursorPosition(mWindow->fileRows.at(mWindow->fileCursorY));
	setRenderedString();
	setHighlight();
}

/// <summary>
/// Preps the rendered string by replacing tabs with necessary spaces
/// </summary>
void Console::setRenderedString()
{
	for (size_t r = 0; r < mWindow->rows && r < mWindow->fileRows.size(); ++r)
	{
		size_t fileRow = mWindow->rowOffset + r;
		if (fileRow >= mWindow->fileRows.size()) return;

		FileHandler::Row& row = mWindow->fileRows.at(fileRow);
		row.renderedLine = row.line;
		if (row.renderedLine.length() > 0)
		{
			replaceRenderedStringTabs(row.renderedLine);
		}
	}
}

/// <summary>
/// Builds the output buffer and displays it to the user through std::cout
/// Uses ANSI escape codes for clearing screen/displaying cursor and for colors
/// </summary>
/// <param name="mode"></param>
void Console::refreshScreen()
{
	const char* emptyRowCharacter = "~";

	std::string renderBuffer = "\x1b[H"; //Move the cursor to (0, 0)
	renderBuffer.append("\x1b[?251"); //Hide the cursor
	renderBuffer.append("\x1b[J"); //Erase the screen to redraw changes

	for (size_t y = 0; y < mWindow->rows; ++y) //For each row within the displayable range
	{
		size_t fileRow = mWindow->rowOffset + y; //The current row within the displayed rows
		if (fileRow >= mWindow->fileRows.size())
		{
			if (mWindow->fileRows.size() == 0 && y == mWindow->rows / 3) //If the file is empty and the current row is at 1/3 height (good display position)
			{
				std::string welcome = std::format("NotVim Editor -- version {}\x1b[0K\r\n", NotVimVersion);
				size_t padding = (mWindow->cols - welcome.length()) / 2;
				if (padding > 0)
				{
					renderBuffer.append(emptyRowCharacter);
					--padding;
				}
				while (padding > 0)
				{
					renderBuffer.append(" ");
					--padding;
				}
				renderBuffer.append(welcome);
			}
			else
			{
				renderBuffer.append("\x1b[0m"); //Make sure color mode is back to normal
				renderBuffer.append(emptyRowCharacter);
				renderBuffer.append("\x1b[0K\r\n"); //Clear the rest of the row
			}
			continue;
		}

		FileHandler::Row& row = mWindow->fileRows.at(fileRow);

		//Set the render string length to the lesser of the terminal width and the line length.
		const size_t renderedLength = (row.renderedLine.length() - mWindow->colOffset) > mWindow->cols ? mWindow->cols : row.renderedLine.length();
		if (renderedLength > 0)
		{
			if (mWindow->colOffset < row.renderedLine.length())
			{
				row.renderedLine = row.renderedLine.substr(mWindow->colOffset, renderedLength);
				updateRenderedColor(row.renderedLine, fileRow, mWindow->colOffset);
				renderBuffer.append(row.renderedLine);
			}
			else
			{
				row.renderedLine.clear();
			}
		}
		renderBuffer.append("\x1b[0K\r\n");
	}
	
	renderBuffer.append("\x1b[0m"); //Make sure color mode is back to normal
	renderBuffer.append("\x1b[7m"); //Set to inverse color mode (white background dark text) for status row

	std::string status, rStatus, modeToDisplay;
	status = std::format("{} - {} lines {}", FileHandler::fileName(), mWindow->fileRows.size(), mWindow->dirty ? "(modified)" : "");
	if (mMode == Mode::EditMode)
	{
		rStatus = std::format("row {}/{} col {}", mWindow->rowOffset + mWindow->renderedCursorY + 1, mWindow->fileRows.size(), mWindow->colNumberToDisplay + 1);
		modeToDisplay = "EDIT";
	}
	else if (mMode == Mode::CommandMode)
	{
		rStatus = "Enter command";
		modeToDisplay = "COMMAND";
	}
	else if (mMode == Mode::ReadMode)
	{
		rStatus = "Read mode";
		modeToDisplay = "READ ONLY";
	}
	size_t statusLength = (status.length() > mWindow->cols) ? mWindow->cols : status.length();
	renderBuffer.append(status);

	while (statusLength < (mWindow->cols / 2))
	{
		if ((mWindow->cols / 2) - statusLength == modeToDisplay.length() / 2)
		{
			renderBuffer.append(modeToDisplay);
			break;
		}
		else
		{
			renderBuffer.append(" ");
			++statusLength;
		}
	}
	statusLength += modeToDisplay.length();

	while (statusLength < mWindow->cols)
	{
		if (mWindow->cols - statusLength == rStatus.length())
		{
			renderBuffer.append(rStatus);
			break;
		}
		else
		{
			renderBuffer.append(" ");
			++statusLength;
		}
	}

	renderBuffer.append("\x1b[0m\r\n"); //Set to default mode

	std::string cursorPosition = std::format("\x1b[{};{}f", mWindow->renderedCursorY + 1, mWindow->renderedCursorX + 1); //Move the cursor to this position
	renderBuffer.append(cursorPosition);
	renderBuffer.append("\x1b[?25h"); //Show cursor
	std::cout << renderBuffer;
	std::cout.flush(); //Finally, flush the buffer so everything displays properly
}

/// <summary>
/// Moves the file cursor through the file rows depending on which key is pressed
/// </summary>
/// <param name="key">The arrow key pressed</param>
void Console::moveCursor(const KeyActions::KeyAction key)
{
	switch (key)
	{
	case KeyActions::KeyAction::ArrowLeft:
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0) return; //Can't move any farther left if we are at the beginning of the file

		if (mWindow->fileCursorX == 0)
		{
			--mWindow->fileCursorY;
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		else
		{
			--mWindow->fileCursorX;
		}
		mWindow->updateSavedPos = true;
		break;
	case KeyActions::KeyAction::ArrowRight:
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1)
		{
			if (mWindow->fileCursorX == mWindow->fileRows.at(mWindow->fileCursorY).line.length()) return; //Can't move any farther right if we are at the end of the file
		}

		if (mWindow->fileCursorX == mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = 0;
			++mWindow->fileCursorY;
		}
		else
		{
			++mWindow->fileCursorX;
		}
		mWindow->updateSavedPos = true;
		break;

	case KeyActions::KeyAction::ArrowUp:
		if (mWindow->fileCursorY == 0)
		{
			mWindow->fileCursorX = 0;
			return;
		}

		--mWindow->fileCursorY;
		setCursorLinePosition();
		break;

	case KeyActions::KeyAction::ArrowDown:
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1)
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
			return;
		}

		++mWindow->fileCursorY;
		setCursorLinePosition();
		break;

	case KeyActions::KeyAction::CtrlArrowLeft:
		//Stuff copied from ArrowLeft
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0) return; //Can't move any farther left if we are at the beginning of the file

		if (mWindow->fileCursorX == 0)
		{
			--mWindow->fileCursorY;
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		//New Stuff
		else
		{
			size_t findPos; //If there isn't a separator character before the cursor
			if ((findPos = mWindow->fileRows.at(mWindow->fileCursorY).line.substr(0, mWindow->fileCursorX).find_last_of(separators)) == std::string::npos)
			{
				mWindow->fileCursorX = 0;
			}
			else if (findPos == mWindow->fileCursorX - 1) //If the separator character is just before the cursor
			{
				--mWindow->fileCursorX;
			}
			else
			{
				mWindow->fileCursorX = findPos; //Go to the end of the previous word
			}

		}
		mWindow->updateSavedPos = true;
		break;
	case KeyActions::KeyAction::CtrlArrowRight:
		//Stuff copied from ArrowRight
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1)
		{
			if (mWindow->fileCursorX == mWindow->fileRows.at(mWindow->fileCursorY).line.length()) return; //Can't move any farther right if we are at the end of the file
		}

		if (mWindow->fileCursorX == mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = 0;
			++mWindow->fileCursorY;
		}

		//New stuff
		else
		{
			size_t findPos; //If there isn't a separator character within the remaining string
			if ((findPos = mWindow->fileRows.at(mWindow->fileCursorY).line.substr(mWindow->fileCursorX).find_first_of(separators)) == std::string::npos)
			{
				mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
			}
			else if (findPos == 0) //If the cursor is currently on the separator character
			{
				++mWindow->fileCursorX;
			}
			else
			{
				mWindow->fileCursorX += findPos + 1; //Go to the character just beyond the separator (the start of the next word)
			}
		}
		mWindow->updateSavedPos = true;
		break;

	case KeyActions::KeyAction::Home:
		mWindow->fileCursorX = 0;
		mWindow->updateSavedPos = true;
		break;
		
	case KeyActions::KeyAction::End:
		mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		mWindow->updateSavedPos = true;
		break;

	case KeyActions::KeyAction::CtrlHome:
		mWindow->fileCursorX = 0; mWindow->fileCursorY = 0;
		mWindow->updateSavedPos = true;
		break;

	case KeyActions::KeyAction::CtrlEnd:
		mWindow->fileCursorY = mWindow->fileRows.size() - 1;
		mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		mWindow->updateSavedPos = true;
		break;

	case KeyActions::KeyAction::PageUp: //Shift screen offset up by 1 page worth (mWindow->rows)
		if (mWindow->fileCursorY < mWindow->rows)
		{
			mWindow->fileCursorY = 0;
			mWindow->rowOffset = 0;
		}
		else
		{
			mWindow->fileCursorY -= mWindow->rows;
			if (mWindow->rowOffset >= mWindow->rows) mWindow->rowOffset -= mWindow->rows;
			else mWindow->rowOffset = 0;
		}
		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;

	case KeyActions::KeyAction::PageDown: //Shift screen offset down by 1 page worth (mWindow->rows)
		if (mWindow->fileCursorY + mWindow->rows > mWindow->fileRows.size() - 1)
		{
			if (mWindow->fileCursorY == mWindow->fileRows.size() - 1) return;

			mWindow->fileCursorY = mWindow->fileRows.size() - 1;
			mWindow->rowOffset += mWindow->fileCursorY % mWindow->rows;
		}
		else
		{
			mWindow->fileCursorY += mWindow->rows;
			mWindow->rowOffset += mWindow->rows;
		}
		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;

	case KeyActions::KeyAction::CtrlPageUp: //Move cursor to top of screen
		mWindow->fileCursorY -= (mWindow->fileCursorY - mWindow->rowOffset) % mWindow->rows;
		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;

	case KeyActions::KeyAction::CtrlPageDown: //Move cursor to bottom of screen
		if (mWindow->fileCursorY + mWindow->rows - ((mWindow->fileCursorY - mWindow->rowOffset) % mWindow->rows) > mWindow->fileRows.size() - 1)
		{
			mWindow->fileCursorY = mWindow->fileRows.size() - 1;
		}
		else
		{
			mWindow->fileCursorY += mWindow->rows - ((mWindow->fileCursorY - mWindow->rowOffset) % mWindow->rows);
		}

		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;
	}
}


/// <summary>
/// When CTRL-ArrowUp / CTRL-ArrowDown is pressed, shift the viewable screen area up/down one if possible
/// </summary>
/// <param name="key"></param>
void Console::shiftRowOffset(const KeyActions::KeyAction key)
{
	if (key == KeyActions::KeyAction::CtrlArrowDown)
	{
		if (mWindow->rowOffset == mWindow->fileRows.size() - 1) return; //This is as far as the screen can be moved down

		++mWindow->rowOffset;
		if (mWindow->fileCursorY < mWindow->fileRows.size() && mWindow->renderedCursorY == 0) //Move the file cursor if the rendered cursor is at the top of the screen
		{
			moveCursor(KeyActions::KeyAction::ArrowDown);
		}
	}
	else if (key == KeyActions::KeyAction::CtrlArrowUp)
	{
		if (mWindow->rowOffset == 0) return; //A negative row offset would wrap and break the viewport so don't allow it to go negative

		--mWindow->rowOffset;
		if (mWindow->renderedCursorY == mWindow->rows - 1) //Move the file cursor if the rendered cursor is at the bottom of the screen
		{
			moveCursor(KeyActions::KeyAction::ArrowUp);
		}
	}
}

/// <summary>
/// Adds a new row when ENTER/RETURN is pressed
/// Moves data past the cursor onto the new row
/// </summary>
void Console::addRow()
{
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);

	if (mWindow->fileCursorX == row.line.length())
	{
		mWindow->fileRows.insert(mWindow->fileRows.begin() + mWindow->fileCursorY + 1, FileHandler::Row());
	}
	else if (mWindow->fileCursorX == 0)
	{
		mWindow->fileRows.insert(mWindow->fileRows.begin() + mWindow->fileCursorY, FileHandler::Row());
	}
	else
	{
		FileHandler::Row newRow;
		newRow.line = row.line.substr(mWindow->fileCursorX);
		row.line.erase(row.line.begin() + mWindow->fileCursorX, row.line.end());
		mWindow->fileRows.insert(mWindow->fileRows.begin() + mWindow->fileCursorY + 1, newRow);
	}

	mWindow->fileCursorX = 0; ++mWindow->fileCursorY;
	mWindow->dirty = true;
	mWindow->updateSavedPos = true;
}

/// <summary>
/// Deletes a character behind/ahead of the cursor depending on key pressed
/// </summary>
/// <param name="key"></param>
void Console::deleteChar(const KeyActions::KeyAction key)
{
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);
	switch (key)
	{
	case KeyActions::KeyAction::Backspace:
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0) return;

		if (mWindow->fileCursorX == 0)
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY - 1).line.length();
			mWindow->fileRows.at(mWindow->fileCursorY - 1).line.append(row.line);
			deleteRow(mWindow->fileCursorY);
			--mWindow->fileCursorY;
		}
		else
		{
			row.line.erase(row.line.begin() + mWindow->fileCursorX - 1);
			--mWindow->fileCursorX;
		}
		break;

	case KeyActions::KeyAction::Delete:
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1 && mWindow->fileCursorX == row.line.length()) return;

		if (mWindow->fileCursorX == row.line.length())
		{
			row.line.append(mWindow->fileRows.at(mWindow->fileCursorY + 1).line);
			deleteRow(mWindow->fileCursorY + 1);
		}
		else
		{
			row.line.erase(row.line.begin() + mWindow->fileCursorX);
		}
		break;

	case KeyActions::KeyAction::CtrlBackspace:
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0) return;

		if (mWindow->fileCursorX == 0)
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY - 1).line.length();
			mWindow->fileRows.at(mWindow->fileCursorY - 1).line.append(row.line);
			deleteRow(mWindow->fileCursorY);
			--mWindow->fileCursorY; 
		}
		else
		{
			size_t findPos;
			if ((findPos = row.line.substr(0, mWindow->fileCursorX).find_last_of(separators)) == std::string::npos) //Delete everything in the row to the beginning
			{
				row.line.erase(row.line.begin(), row.line.begin() + mWindow->fileCursorX);
				mWindow->fileCursorX = 0;
			}
			else if (findPos == mWindow->fileCursorX - 1)
			{
				deleteChar(KeyActions::KeyAction::Backspace); //Delete just the separator
			}
			else
			{
				row.line.erase(row.line.begin() + findPos + 1, row.line.begin() + mWindow->fileCursorX);
				mWindow->fileCursorX = findPos + 1;
			}
		}
		break;

	case KeyActions::KeyAction::CtrlDelete:
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1 && mWindow->fileCursorX == row.line.length()) return;

		if (mWindow->fileCursorX == row.line.length())
		{
			row.line.append(mWindow->fileRows.at(mWindow->fileCursorY + 1).line);
			deleteRow(mWindow->fileCursorY + 1);
		}
		else
		{
			size_t findPos;
			if ((findPos = row.line.substr(mWindow->fileCursorX).find_first_of(separators)) == std::string::npos) //Delete everything in the row to the beginning
			{
				row.line.erase(row.line.begin() + mWindow->fileCursorX, row.line.end());
			}
			else if (findPos == 0)
			{
				deleteChar(KeyActions::KeyAction::Delete); //Delete just the separator
			}
			else
			{
				row.line.erase(row.line.begin() + mWindow->fileCursorX, row.line.begin() + findPos + mWindow->fileCursorX);
			}
		}
		break;
	}
	mWindow->dirty = true;
	mWindow->updateSavedPos = true;
}

/// <summary>
/// Inserts a given character at the current position and moves the cursor forward
/// </summary>
/// <param name="c">The character to insert</param>
void Console::insertChar(const uint8_t c)
{
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);

	row.line.insert(row.line.begin() + mWindow->fileCursorX, c);
	++mWindow->fileCursorX;
	mWindow->dirty = true;
	mWindow->updateSavedPos = true;
}

bool Console::isRawMode()
{
	return mWindow->rawModeEnabled;
}

bool Console::isDirty()
{
	return mWindow->dirty;
}

/// <summary>
/// Saves the file and sets dirty = false
/// Appends a newline character to the end of each row, assuming it's not the last row so that when written to the file the contents are saved properly
/// </summary>
void Console::save()
{
	std::string output;
	for (size_t i = 0; i < mWindow->fileRows.size(); ++i)
	{
		if (i == mWindow->fileRows.size() - 1)
		{
			output.append(mWindow->fileRows.at(i).line);
		}
		else
		{
			output.append(mWindow->fileRows.at(i).line + "\n");
		}
	}
	FileHandler::saveFile(output);
	mWindow->dirty = false;
}

/// <summary>
/// Moves the rendered cursor to the command mode area and enable command mode
/// </summary>
void Console::enableCommandMode()
{
	mWindow->renderedCursorX = 0; mWindow->renderedCursorY = mWindow->rows + 2;
	mMode = Mode::CommandMode;
	refreshScreen();
}

/// <summary>
/// Enables edit mode. If the file is empty, add an empty row to start the file
/// </summary>
void Console::enableEditMode()
{
	if (mWindow->fileRows.size() == 0)
	{
		mWindow->fileRows.push_back(FileHandler::Row());
	}
	mMode = Mode::EditMode;
}

/// <summary>
/// Deletes a row when the last character in the row is removed
/// </summary>
/// <param name="rowNum"></param>
void Console::deleteRow(const size_t rowNum)
{
	if (rowNum > mWindow->fileRows.size()) return;
	mWindow->fileRows.erase(mWindow->fileRows.begin() + rowNum);
	mWindow->dirty = true;
}

/// <summary>
/// Allows for smooth movement of the cursor when moving up/down
/// Compares the last value since the cursor was moved left/right (either by inserting/deleting character or moving left/right manually)
/// Finds the closest position to the last saved value without going past it
/// </summary>
void Console::setCursorLinePosition()
{
	if (mWindow->renderedCursorX > mWindow->fileRows.at(mWindow->fileCursorY).renderedLine.length())
	{
		mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		return;
	}
	mWindow->fileCursorX = 0;
	size_t spaces = getRenderedCursorTabSpaces(mWindow->fileRows.at(mWindow->fileCursorY));
	while (mWindow->fileCursorX + spaces < mWindow->savedRenderedCursorXPos)
	{
		++mWindow->fileCursorX;
		spaces = getRenderedCursorTabSpaces(mWindow->fileRows.at(mWindow->fileCursorY));
	}
	if (mWindow->fileCursorX + spaces > mWindow->savedRenderedCursorXPos)
	{
		--mWindow->fileCursorX;
	}
	if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
	{
		mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
	}
}

/// <summary>
/// Fixes the rendered cursor x/y and row/column offset positions
/// </summary>
/// <param name="row">The row we are currently on</param>
void Console::fixRenderedCursorPosition(const FileHandler::Row& row)
{
	//Fixing rendered X/Col position
	mWindow->renderedCursorX = mWindow->fileCursorX;
	mWindow->renderedCursorX += getRenderedCursorTabSpaces(row);
	mWindow->colNumberToDisplay = mWindow->renderedCursorX;

	while (mWindow->renderedCursorX - mWindow->colOffset >= mWindow->cols && mWindow->renderedCursorX >= mWindow->colOffset)
	{
		++mWindow->colOffset;
	}
	while (mWindow->renderedCursorX < mWindow->colOffset)
	{
		--mWindow->colOffset;
	}
	mWindow->renderedCursorX = mWindow->renderedCursorX - mWindow->colOffset;
	if (mWindow->renderedCursorX == mWindow->cols)
	{
		--mWindow->renderedCursorX;
	}
	//Fixing rendered Y/Row position
	while (mWindow->fileCursorY - mWindow->rowOffset >= mWindow->rows && mWindow->fileCursorY >= mWindow->rowOffset)
	{
		++mWindow->rowOffset;
	}
	while (mWindow->fileCursorY < mWindow->rowOffset)
	{
		--mWindow->rowOffset;
	}
	mWindow->renderedCursorY = mWindow->fileCursorY - mWindow->rowOffset;

	if (mWindow->renderedCursorY == mWindow->rows) //renderedCursorY might be 1 too many rows down, so just bring it back one row if it is
	{
		--mWindow->renderedCursorY;
	}

	if (mWindow->updateSavedPos)
	{
		mWindow->savedRenderedCursorXPos = mWindow->renderedCursorX;
		mWindow->updateSavedPos = false;
	}
}


/// <summary>
/// This function needs work
/// </summary>
/// <param name="renderedLine"></param>
/// <param name="offset"></param>
void Console::replaceRenderedStringTabs(std::string& renderedLine)
{
	size_t lineLength = renderedLine.length();
	for (size_t i = 0; i < lineLength; ++i)
	{
		if (i >= renderedLine.length()) return;
		if (renderedLine[i] != static_cast<uint8_t>(KeyActions::KeyAction::Tab)) continue;

		renderedLine[i] = ' '; //Replace the tab character with a space
		uint8_t t = 7 - (i % 8);
		while (t > 0)
		{
			renderedLine.insert(renderedLine.begin() + (i), ' '); //Add spaces until we reach a multiple of 8
			--t;
			++i;
		}
		if (renderedLine.length() > lineLength)
		{
			lineLength = renderedLine.length();
		}
	}
}

/// <summary>
/// Gets the amount of spaces the rendered cursor needs to be adjusted to account for tabs
/// </summary>
/// <param name="row">Row to check</param>
/// <returns>The amount of spaces to add</returns>
size_t Console::getRenderedCursorTabSpaces(const FileHandler::Row& row)
{
	size_t spacesToAdd = 0;
	for (size_t i = 0; i < mWindow->fileCursorX; ++i)
	{
		if (i > row.line.length()) return 0;

		if (row.line[i] != static_cast<uint8_t>(KeyActions::KeyAction::Tab)) continue;

		spacesToAdd += 7 - ((i + spacesToAdd) % 8); //Tabs are replaced with up to 8 spaces, depending on how close to a multiple of 8 the tab is
	}
	return spacesToAdd;
}

/// <summary>
/// Sets the rendered color of the current row based on what setHighlight() does
/// </summary>
/// <param name="renderString">The string being prepped for render</param>
/// <param name="row">The current row number</param>
/// <param name="colOffset">The current colOffset to know if a certain highlight is off-screen or needs to be rendered</param>
void Console::updateRenderedColor(std::string& renderString, const size_t row, const size_t colOffset)
{
	std::string normalColorMode = "\x1b[0m";
	size_t charactersToAdjust = 0; //The amount of characters to adjust for in the string position based on how many color code escape sequences have been added
	for (const auto& highlight : mHighlights)
	{
		if (row != highlight.startRow && row != highlight.endRow) continue; //No point in doing anything if the row we are currently on doesn't have a highlight position

		const uint8_t color = SyntaxHighlight::color(highlight.colorType);
		std::string colorFormat = std::format("\x1b[38;5;{}m", std::to_string(color));

		if (row == highlight.startRow)
		{
			size_t insertPos = highlight.startCol;

			//Need to make sure the insert position is in within the rendered string
			if (insertPos < colOffset) insertPos = 0;
			else if (insertPos >= colOffset) insertPos -= colOffset;
			if (insertPos >= mWindow->cols) insertPos = mWindow->cols;

			renderString.insert(insertPos + charactersToAdjust, colorFormat);
			charactersToAdjust += colorFormat.length();
		}
		if (row == highlight.endRow)
		{
			size_t insertPos = highlight.endCol;

			//Need to make sure the insert position is in within the rendered string
			if (insertPos < colOffset) insertPos = 0;
			else if (insertPos >= colOffset) insertPos -= colOffset;
			if (insertPos >= mWindow->cols) insertPos = mWindow->cols;

			renderString.insert(insertPos + charactersToAdjust, normalColorMode);
			charactersToAdjust += normalColorMode.length();
		}
	}
}

/// <summary>
/// Tries to find the end marker, denoted as strToFind
/// Either runs until the max row count is reached or until it is found
/// </summary>
/// <param name="currentWord"></param>
/// <param name="row"></param>
/// <param name="posOffset"></param>
/// <param name="findPos"></param>
/// <param name="strToFind"></param>
/// <param name="hlType"></param>
void Console::findEndMarker(std::string& currentWord, size_t& row, size_t& posOffset, size_t& findPos, const std::string& strToFind, const SyntaxHighlight::HighlightType hlType)
{
	size_t startRow = row;
	posOffset += findPos;
	size_t startCol = posOffset;
	currentWord = currentWord.substr(findPos);
	size_t endPos;
	uint8_t offset = strToFind.length(); //Offsets by the opening marker length while on the same row as the opening marker. 
	while ((endPos = currentWord.find(strToFind, offset)) == std::string::npos)
	{
		offset = 0; //If on a new row, no need to offset the check position
		findPos = 0;
		posOffset = 0;
		++row;
		if (row >= mWindow->fileRows.size())
		{
			mHighlights.emplace_back(hlType, startRow, startCol, row - 1, mWindow->fileRows.at(row - 1).renderedLine.length());
			currentWord.clear();
			return;
		}
		currentWord = mWindow->fileRows.at(row).renderedLine;
	}
	mHighlights.emplace_back(hlType, startRow, startCol, row, posOffset + endPos + strToFind.length());
	currentWord = currentWord.substr(endPos + strToFind.length());
	posOffset += endPos + strToFind.length();
}

/// <summary>
/// Sets the highlight points of the rendered string if a syntax is given
/// The stored format is (HighlightType, startRow, startCol, endRow, endCol)
/// </summary>
/// <param name="startingRowNum"></param>
void Console::setHighlight()
{
	if (mWindow->syntax == nullptr) return; //Can't highlight if there is no syntax

	mHighlights.clear(); //Could be optimized to only clear the current row, but I haven't seen a lack of performance, so this is fine for now

	for (size_t i = 0; i < mWindow->fileRows.size(); ++i)
	{
		if (i > mWindow->rowOffset + mWindow->rows) return;

		FileHandler::Row* row = &mWindow->fileRows.at(i); //The starting row

		std::string currentWord = row->renderedLine;
		size_t findPos, posOffset = 0; //posOffset keeps track of how far into the string we are, since findPos depends on currentWord, which progressively gets smaller
		const uint8_t singlelineCommentLength = mWindow->syntax->singlelineComment.length();
		const uint8_t multilineCommentLength = mWindow->syntax->multilineCommentStart.length();

		while ((findPos = currentWord.find_first_of(separators)) != std::string::npos)
		{
			row = &mWindow->fileRows.at(i); //Makes sure the correct row is always being used

			std::string wordToCheck = currentWord.substr(0, findPos); //The word/character sequence before the separator character

			if (!wordToCheck.empty() && wordToCheck.find_first_not_of("0123456789") == std::string::npos)
			{
				mHighlights.emplace_back(SyntaxHighlight::HighlightType::Number, i, posOffset, i, posOffset + wordToCheck.length());
			}
			else if (!wordToCheck.empty())
			{
				for (const auto& type : mWindow->syntax->builtInTypeKeywords)
				{
					if (type == wordToCheck)
					{
						mHighlights.emplace_back(SyntaxHighlight::HighlightType::KeywordBuiltInType, i, posOffset, i, posOffset + wordToCheck.length());
						goto commentcheck;
					}
				}
				for (const auto& control : mWindow->syntax->loopKeywords)
				{
					if (control == wordToCheck)
					{
						mHighlights.emplace_back(SyntaxHighlight::HighlightType::KeywordControl, i, posOffset, i, posOffset + wordToCheck.length());
						goto commentcheck;
					}
				}
				for (const auto& other : mWindow->syntax->otherKeywords)
				{
					if (other == wordToCheck)
					{
						mHighlights.emplace_back(SyntaxHighlight::HighlightType::KeywordOther, i, posOffset, i, posOffset + wordToCheck.length());
						goto commentcheck;
					}
				}
			}

		commentcheck: //Skip the remaining for-loop checks
			if (currentWord[findPos] == '"' || currentWord[findPos] == '\'') //String highlights are open until the next string marker of the same type is found
			{
				findEndMarker(currentWord, i, posOffset, findPos, std::string() + currentWord[findPos], SyntaxHighlight::HighlightType::String);
			}
			else if (findPos + multilineCommentLength - 1 < currentWord.length() //Multiline comments stay open until the closing marker is found
				&& currentWord.substr(findPos, multilineCommentLength) == mWindow->syntax->multilineCommentStart)
			{
				findEndMarker(currentWord, i, posOffset, findPos, mWindow->syntax->multilineCommentEnd, SyntaxHighlight::HighlightType::MultilineComment);
			}
			else if (findPos + singlelineCommentLength - 1 < currentWord.length() //Singleline comments take the rest of the row
				&& currentWord.substr(findPos, singlelineCommentLength) == mWindow->syntax->singlelineComment)
			{
				mHighlights.emplace_back(SyntaxHighlight::HighlightType::Comment, i, findPos + posOffset, i, row->renderedLine.length());
				goto nextrow;
			}
			else
			{
				posOffset += findPos + 1;
				currentWord = currentWord.substr(findPos + 1);
				continue;
			}
		}
		if (!currentWord.empty()) //If the last character in the string isn't a separator character/comment/string
		{
			if (currentWord.find_first_not_of("0123456789") == std::string::npos)
			{
				mHighlights.emplace_back(SyntaxHighlight::HighlightType::Number, i, posOffset, i, posOffset + currentWord.length());
				continue;
			}
			else
			{
				for (const auto& type : mWindow->syntax->builtInTypeKeywords)
				{
					if (type == currentWord)
					{
						mHighlights.emplace_back(SyntaxHighlight::HighlightType::KeywordBuiltInType, i, posOffset, i, posOffset + currentWord.length());
						goto nextrow;
					}
				}
				for (const auto& control : mWindow->syntax->loopKeywords)
				{
					if (control == currentWord)
					{
						mHighlights.emplace_back(SyntaxHighlight::HighlightType::KeywordControl, i, posOffset, i, posOffset + currentWord.length());
						goto nextrow;
					}
				}
				for (const auto& other : mWindow->syntax->otherKeywords)
				{
					if (other == currentWord)
					{
						mHighlights.emplace_back(SyntaxHighlight::HighlightType::KeywordOther, i, posOffset, i, posOffset + currentWord.length());
						goto nextrow;
					}
				}
			}
		nextrow: //Nothing left to do on this row
			continue;
		}
	}
}

//=================================================================== OS-SPECIFIC FUNCTIONS =============================================================================\\

#ifdef _WIN32
DWORD defaultMode;
#elif defined(__linux__) || defined(__APPLE__)
static termios defaultMode;
#endif

/// <summary>
/// Initializes the window and all other dependencies
/// </summary>
/// <param name="fileName">The filename grabbed from argv[1]</param>
void Console::initConsole(const std::string_view& fName)
{
	FileHandler::fileName(fName);
	SyntaxHighlight::initSyntax(fName);

	mWindow = std::make_unique<Window>(Window());
	setWindowSize();

#ifdef _WIN32
	if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &defaultMode)) //Try to get the default terminal settings
	{
		std::cerr << "Error retrieving current console mode";
		exit(EXIT_FAILURE);
	}
#elif defined(__linux__) || defined(__APPLE__)
	if (tcgetattr(STDOUT_FILENO, &defaultMode) == -1)
	{
		std::cerr << "Error retrieving current console mode";
		exit(EXIT_FAILURE);
	}
	signal(SIGWINCH, nullptr);
#endif

	if (!(mWindow->rawModeEnabled = enableRawInput())) //Try to enable raw mode
	{
		std::cerr << "Error enabling raw input mode";
		exit(EXIT_FAILURE);
	}

	prepRenderedString();
}

/// <summary>
/// Sets the window's row/column count
/// </summary>
/// <param name="window"></param>
/// <returns>True if size has changed, false otherwise</returns>
bool Console::setWindowSize()
{
	const size_t prevRows = mWindow->rows;
	const size_t prevCols = mWindow->cols;
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screenInfo))
	{
		std::cerr << "Error getting console screen buffer info";
		exit(EXIT_FAILURE);
	}
	mWindow->rows = static_cast<size_t>(screenInfo.srWindow.Bottom - screenInfo.srWindow.Top) + 1;
	mWindow->cols = static_cast<size_t>(screenInfo.srWindow.Right - screenInfo.srWindow.Left) + 1;

#elif defined(__linux__) || defined(__APPLE__)
	winsize ws;
	int rows, cols;
	if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) //If getting the window size from ioctl fails
	{
		std::cout.write("\x1b[999C\x1b[999B", 12); //Move to the bottom-right corner of the screen
		std::cout.write("\x1b[6n", 4); //Report cursor position - Gets reported as ESC[rows;colsR

		std::string buf; buf.resize(32);
		uint8_t i = 0;
		while (i < buf.size())
		{
			std::cin.read(buf.data(), 1);
			if (buf[i] == 'R') break;
			++i;
		}

		if (buf[0] != static_cast<uint8_t>(KeyActions::KeyAction::Esc) || buf[1] != '[')
		{
			std::cerr << "Error getting window size";
			exit(EXIT_FAILURE);
		}
		if (sscanf(buf.data() + 2, "%d;%d", &rows, &cols) != 2) //If the rows/cols data wasn't reported
		{
			std::cerr << "Error getting window size";
			exit(EXIT_FAILURE);
		}
		mWindow->rows = rows; mWindow->cols = cols;
	}
	else
	{
		mWindow->cols = ws.ws_col;
		mWindow->rows = ws.ws_row;
	}
#endif
	constexpr uint8_t statusMessageRows = 2;
	mWindow->rows -= statusMessageRows;

	if (prevRows != mWindow->rows || prevCols != mWindow->cols) return true; //Checks if the window size has changed

	return false;
}

/// <summary>
/// Enables raw input mode by disabling specific flags
/// </summary>
/// <returns></returns>
bool Console::enableRawInput()
{
	if (mWindow->rawModeEnabled) return true;
#ifdef _WIN32
	DWORD rawMode = ENABLE_EXTENDED_FLAGS | (defaultMode & ~ENABLE_LINE_INPUT & ~ENABLE_PROCESSED_INPUT
		& ~ENABLE_ECHO_INPUT & ~ENABLE_PROCESSED_OUTPUT & ~ENABLE_WRAP_AT_EOL_OUTPUT); //Disabling certain input/output modes to enable raw mode

	if (SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), rawMode))
	{
		atexit(disableRawInput); //Make sure raw input mode gets disabled if the program exits due to an error
		return true;
	}
	return false;
#elif defined(__linux__) || defined(__APPLE__)
	termios raw;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &raw) < 0)
	{
		return false;
	}
	atexit(disableRawInput); //Make sure raw input mode gets disabled if the program exits due to an error
	return true;
#endif
}

/// <summary>
/// Disabled raw input mode by setting the terminal back to the default mode
/// </summary>
void Console::disableRawInput()
{
#ifdef _WIN32
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), defaultMode);
#elif defined(__linux__) || defined(__APPLE__)
	tcsetattr(STDOUT_FILENO, TCSAFLUSH, &defaultMode);
#endif
	mWindow->rawModeEnabled = false;
}

/// <summary>
/// Clears the screen after a command is used
/// Return value of the system call should be ignored since it should never fail
/// </summary>
void Console::clearScreen()
{
#ifdef _WIN32
	int _ = system("cls");
#elif defined(__linux__) || defined(__APPLE__)
	int _ = system("clear");
#endif
}