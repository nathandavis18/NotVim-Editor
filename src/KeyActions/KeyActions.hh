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

#pragma once
namespace KeyActions
{
	enum class KeyAction
	{
		CtrlC = 3,
		CtrlX = 24,
		CtrlV,
		Tab = 9,
		Enter = 13,
		Esc = 27,
#ifdef _WIN32
		Backspace = 8, CtrlBackspace = 127,
#elif defined(__linux__) || defined(__APPLE__) //For some reason, these are reverse from each other
		Backspace = 127, CtrlBackspace = 8,
#endif
		ArrowLeft = 75, CtrlArrowLeft = 115,
		ArrowRight = 77, CtrlArrowRight = 116,
		ArrowUp = 72, CtrlArrowUp = 141,
		ArrowDown = 80, CtrlArrowDown = 145,
		Home = 71, CtrlHome = 119,
		Delete = 83, CtrlDelete = 147,
		End = 79, CtrlEnd = 117,
		PageUp = 73, CtrlPageUp = 134,
		PageDown = 81, CtrlPageDown = 118
	};
}