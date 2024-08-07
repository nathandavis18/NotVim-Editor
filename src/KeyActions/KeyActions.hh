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