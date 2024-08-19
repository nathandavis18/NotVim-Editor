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
		None = 0,
		CtrlC = 3,
		CtrlX = 24,
		CtrlY = 25,
		CtrlZ = 26,
		Tab = 9,
		Enter = 13,
		Esc = 27,
#ifdef _WIN32
		Backspace = 8, CtrlBackspace = 127,
#elif defined(__linux__) || defined(__APPLE__) //For some reason, these are reverse from each other
		Backspace = 127, CtrlBackspace = 8,
#endif
		ArrowLeft = 1000,	CtrlArrowLeft,
		ArrowRight,			CtrlArrowRight,
		ArrowUp,			CtrlArrowUp,
		ArrowDown,			CtrlArrowDown,
		Home,				CtrlHome,
		Delete,				CtrlDelete,
		End,				CtrlEnd,
		PageUp,				CtrlPageUp,
		PageDown,			CtrlPageDown
	};
}