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
#include "../KeyActions/KeyActions.hh"

#include <iostream>
#include <fstream>
#include <format>

#ifdef _WIN32
#include <Windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#endif
#define NotVimVersion "0.1a"

/// <summary>
/// Construct the window
/// </summary>
/// <param name="fileName"></param>
Console::Window::Window() : fileCursorX(0), fileCursorY(0), cols(0), rows(0), renderedCursorX(0), renderedCursorY(0),
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
/// Builds the output buffer and displays it to the user through std::cout
/// Uses ANSI escape codes for clearing screen/displaying cursor and for colors
/// </summary>
/// <param name="mode"></param>
void Console::refreshScreen()
{
	const char* emptyRowCharacter = "~";

	if (mWindow->fileRows.size() > 0 && mMode != Mode::CommandMode) fixRenderedCursorPosition(mWindow->fileRows.at(mWindow->fileCursorY));

	std::string renderBuffer = "\x1b[H"; //Move the cursor to (0, 0)
	renderBuffer.append("\x1b[?251"); //Hide the cursor
	renderBuffer.append("\x1b[J"); //Erase the screen to redraw changes

	for (size_t r = 0; r < mWindow->rows && r < mWindow->fileRows.size(); ++r)
	{
		size_t fileRow = mWindow->rowOffset + r;
		FileHandler::Row& row = mWindow->fileRows.at(fileRow);
		row.renderedLine = row.line;
		if (row.renderedLine.length() > 0)
		{
			replaceRenderedStringTabs(row.renderedLine);
		}
	}

	setHighlight(mWindow->rowOffset);

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
				renderBuffer.append("\x1b[39m");
				updateRenderedColor(row.renderedLine, fileRow, mWindow->colOffset);
				renderBuffer.append(row.renderedLine);
			}
			else
			{
				row.renderedLine.clear();
			}
		}
		renderBuffer.append("\x1b[39m"); //Set default color
		renderBuffer.append("\x1b[0K\r\n");
	}

	renderBuffer.append("\x1b[0K");
	renderBuffer.append("\x1b[7m"); //Set to inverse color mode (white background dark text) for status row

	std::string status, rStatus, modeToDisplay;
	status = std::format("{} - {} lines {}", FileHandler::fileName(), mWindow->fileRows.size(), mWindow->dirty ? "(modified)" : "");
	if (mMode == Mode::EditMode)
	{
		rStatus = std::format("row {}/{} col {}", mWindow->rowOffset + mWindow->renderedCursorY + 1, mWindow->fileRows.size(), mWindow->colOffset + mWindow->renderedCursorX + 1);
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
	renderBuffer.append("\x1b[0K");

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
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0) return;

		if (mWindow->fileCursorX == 0)
		{
			--mWindow->fileCursorY;
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		else
		{
			--mWindow->fileCursorX;
		}
		break;
	case KeyActions::KeyAction::ArrowRight:
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1)
		{
			if (mWindow->fileCursorX == mWindow->fileRows.at(mWindow->fileCursorY).line.length()) return;
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
		break;
	case KeyActions::KeyAction::ArrowUp:
		if (mWindow->fileCursorY == 0)
		{
			mWindow->fileCursorX = 0;
			return;
		}

		--mWindow->fileCursorY;

		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;
	case KeyActions::KeyAction::ArrowDown:
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1)
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
			return;
		}

		++mWindow->fileCursorY;
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
}

/// <summary>
/// Deletes a uint8_tacter behind/ahead of the cursor depending on key pressed
/// </summary>
/// <param name="key"></param>
void Console::deleteChar(const KeyActions::KeyAction key)
{
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);
	if (key == KeyActions::KeyAction::Backspace)
	{
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
	}
	else if (key == KeyActions::KeyAction::Delete)
	{
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
	}
	mWindow->dirty = true;
}

/// <summary>
/// Inserts a given uint8_tacter at the current position and moves the cursor forward
/// </summary>
/// <param name="c"></param>
void Console::insertChar(const uint8_t c)
{
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);

	row.line.insert(row.line.begin() + mWindow->fileCursorX, c);
	++mWindow->fileCursorX;
	mWindow->dirty = true;
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
/// Moves the rendered cursor to the command line
/// </summary>
void Console::enableCommandMode()
{
	mWindow->renderedCursorX = 0; mWindow->renderedCursorY = mWindow->rows + 2;
	mMode = Mode::CommandMode;
	refreshScreen();
}

/// <summary>
/// Moves the cursor to the default position when edit mode is enabled -- will need work to go back to previous edit position instead of the start every time
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
/// Deletes a row when the last uint8_tacter in the row is removed
/// </summary>
/// <param name="rowNum"></param>
void Console::deleteRow(const size_t rowNum)
{
	if (rowNum > mWindow->fileRows.size()) return;
	mWindow->fileRows.erase(mWindow->fileRows.begin() + rowNum);
	mWindow->dirty = true;
}

/// <summary>
/// Fixes the rendered cursor x/y, row and column offset positions
/// </summary>
/// <param name="window"></param>
void Console::fixRenderedCursorPosition(const FileHandler::Row& row)
{
	//Fixing rendered X/Col position
	mWindow->renderedCursorX = mWindow->fileCursorX;
	mWindow->renderedCursorX += addRenderedCursorTabs(row);
	mWindow->colOffset = 0;

	if (mWindow->renderedCursorX >= mWindow->cols)
	{
		while (mWindow->renderedCursorX >= mWindow->cols)
		{
			--mWindow->renderedCursorX;
			++mWindow->colOffset;
		}
	}
	else
	{
		while (mWindow->fileCursorX + mWindow->colOffset > mWindow->renderedCursorX)
		{
			--mWindow->colOffset;
		}
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

	if (mWindow->renderedCursorY == mWindow->rows)
	{
		--mWindow->renderedCursorY;
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

		renderedLine[i] = ' ';
		uint8_t t = 7 - (i % 8);
		while (t > 0)
		{
			renderedLine.insert(renderedLine.begin() + (i), ' ');
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
/// Adjust the rendered cursor position to account for spaces in the rendered string
/// </summary>
/// <param name="row">Row to check</param>
/// <param name="window"></param>
/// <returns>The new rendered cursor X position</returns>
size_t Console::addRenderedCursorTabs(const FileHandler::Row& row)
{
	if (mWindow->fileCursorX == 0)
	{
		mWindow->colOffset = 0;
		mWindow->renderedCursorX = 0;
		return 0;
	}

	size_t spacesToAdd = 0;
	for (size_t i = 0; i < mWindow->fileCursorX; ++i)
	{
		if (i > row.line.length()) return 0;

		if (row.line[i] != static_cast<uint8_t>(KeyActions::KeyAction::Tab)) continue;

		spacesToAdd += 7 - ((i + spacesToAdd) % 8);
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
	std::string normalColorMode = "\x1b[39m";
	size_t charactersToAdjust = 0;
	for (const auto& highlight : mHighlight)
	{
		if (highlight.row < row) continue; //Cycle until on the correct row
		if (highlight.row > row) return; //To far, dont need to check any more
		if (colOffset >= highlight.col + highlight.length || highlight.col >= mWindow->cols + colOffset) continue; //If the highlighted section is off-screen

		const uint8_t color = SyntaxHighlight::color(highlight.colorType);
		std::string colorFormat = std::format("\x1b[38;5;{}m", std::to_string(color));

		if (colOffset >= highlight.col)
		{
			renderString.insert(0, colorFormat); //Set the color mode
			charactersToAdjust += colorFormat.length();
			renderString.insert(highlight.length + highlight.col + charactersToAdjust - colOffset, normalColorMode); //Return back to normal color mode
			charactersToAdjust += normalColorMode.length();
		}
		else
		{
			renderString.insert(highlight.col + charactersToAdjust - colOffset, colorFormat);
			charactersToAdjust += colorFormat.length();
			if (highlight.length + highlight.col > mWindow->cols + colOffset)
			{
				renderString.insert(mWindow->cols + charactersToAdjust, normalColorMode);
				charactersToAdjust += normalColorMode.length();
			}
			else
			{
				renderString.insert(highlight.col + charactersToAdjust + highlight.length - colOffset, normalColorMode);
				charactersToAdjust += normalColorMode.length();
			}
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
	posOffset += findPos;
	currentWord = currentWord.substr(findPos);
	size_t endPos;
	uint8_t offset = strToFind.length();
	while ((endPos = currentWord.find(strToFind, offset)) == std::string::npos)
	{
		offset = 0;
		mHighlight.emplace_back(hlType, row, posOffset, currentWord.length());
		findPos = 0;
		posOffset = 0;
		++row;
		if (row >= mWindow->fileRows.size())
		{
			currentWord.clear();
			return;
		}
		currentWord = mWindow->fileRows.at(row).renderedLine;
	}
	mHighlight.emplace_back(hlType, row, posOffset, endPos + strToFind.length());
	currentWord = currentWord.substr(endPos + strToFind.length());
	posOffset += endPos + strToFind.length();
}

/// <summary>
/// Sets the highlight points of the rendered string if a syntax is given
/// </summary>
/// <param name="startingRowNum"></param>
void Console::setHighlight(const size_t startingRowNum)
{
	if (mWindow->syntax == nullptr) return; //Can't highlight if there is no syntax

	const std::string separators = " \"',.()+-/*=~%;:[]{}<>";
	mHighlight.clear();
	for (size_t i = startingRowNum; i < mWindow->fileRows.size(); ++i)
	{
		if (i > mWindow->rowOffset + mWindow->rows) return;

		FileHandler::Row* row = &mWindow->fileRows.at(i); //The starting row

		std::string currentWord = row->renderedLine;
		size_t findPos, posOffset = 0;
		const uint8_t singlelineCommentLength = mWindow->syntax->singlelineComment.length();
		const uint8_t multilineCommentLength = mWindow->syntax->multilineCommentStart.length();

		while ((findPos = currentWord.find_first_of(separators)) != std::string::npos)
		{
			row = &mWindow->fileRows.at(i); //Makes sure the correct row is always being used

			std::string wordToCheck = currentWord.substr(0, findPos); //The word/character sequence before the separator character

			if (wordToCheck.find_first_not_of("0123456789") == std::string::npos && !wordToCheck.empty())
			{
				mHighlight.emplace_back(SyntaxHighlight::HighlightType::Number, i, posOffset, wordToCheck.length());
			}
			else if (!wordToCheck.empty())
			{
				for (const auto& type : mWindow->syntax->builtInTypeKeywords)
				{
					if (type == wordToCheck)
					{
						mHighlight.emplace_back(SyntaxHighlight::HighlightType::KeywordBuiltInType, i, posOffset, wordToCheck.length());
						break;
					}
				}
				for (const auto& control : mWindow->syntax->loopKeywords)
				{
					if (control == wordToCheck)
					{
						mHighlight.emplace_back(SyntaxHighlight::HighlightType::KeywordControl, i, posOffset, wordToCheck.length());
						break;
					}
				}
				for (const auto& other : mWindow->syntax->otherKeywords)
				{
					if (other == wordToCheck)
					{
						mHighlight.emplace_back(SyntaxHighlight::HighlightType::KeywordOther, i, posOffset, wordToCheck.length());
						break;
					}
				}
			}

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
				mHighlight.emplace_back(SyntaxHighlight::HighlightType::Comment, i, posOffset, row->renderedLine.length() - posOffset);
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
				mHighlight.emplace_back(SyntaxHighlight::HighlightType::Number, i, posOffset, currentWord.length());
				continue;
			}
			else
			{
				for (const auto& type : mWindow->syntax->builtInTypeKeywords)
				{
					if (type == currentWord)
					{
						mHighlight.emplace_back(SyntaxHighlight::HighlightType::KeywordBuiltInType, i, posOffset, currentWord.length());
						goto nextrow;
					}
				}
				for (const auto& control : mWindow->syntax->loopKeywords)
				{
					if (control == currentWord)
					{
						mHighlight.emplace_back(SyntaxHighlight::HighlightType::KeywordControl, i, posOffset, currentWord.length());
						goto nextrow;
					}
				}
				for (const auto& other : mWindow->syntax->otherKeywords)
				{
					if (other == currentWord)
					{
						mHighlight.emplace_back(SyntaxHighlight::HighlightType::KeywordOther, i, posOffset, currentWord.length());
						goto nextrow;
					}
				}
			}
		nextrow:
			continue;
		}
	}
}

bool Console::isSeparator(const uint8_t currentChar)
{
	return (currentChar == '\0' || currentChar == ' ' || (strchr(",.()+-/*=~%[];{}:<>\"'\n\t", currentChar) != nullptr));
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
	SyntaxHighlight::initSyntax(fName); //Doesn't do anything yet

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
	if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	{
		std::cout.write("\x1b[999C\x1b[999B", 12);
		std::cout.write("\x1b[6n", 4);

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
		if (sscanf(buf.data() + 2, "%d;%d", &rows, &cols) != 2)
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

	if (prevRows != mWindow->rows || prevCols != mWindow->cols) return true;

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

	return SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), rawMode);
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
	else
	{
		atexit(disableRawInput);
		return true;
	}
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
/// </summary>
void Console::clearScreen()
{
#ifdef _WIN32
	system("cls");
#elif defined(__linux__) || defined(__APPLE__)
	system("clear");
#endif
}