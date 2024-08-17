# NotVim Editor

A custom, 0 dependencies (yes, not even curses/ncurses), cross-platform VIM-like console-based text editor. Tested on Windows and Linux.
<hr>

### Demo

https://github.com/user-attachments/assets/bc71a3f9-f7f1-4bb0-9f69-fb3d8aef2c8a

<hr>

### Versions
If you would like to try the latest features, you can build the application from the master branch. However, some features may be experimental, and bugs may arise.
If you would like to use a stable release, check out the releases tab. Releases are tested to be as stable as possible. Some bugs may slip through to releases, so be sure to report those!

<hr>

### Controls/Features
Currently you can only move the cursor while in edit mode. This will soon be changed to allow moving cursor in read mode as well.

Controls are listed as follows:

	WHILE IN READ MODE (Default Mode):
	- i - Enable Edit Mode
	- : - Enable Command Mode
	
	WHILE IN COMMAND MODE:
	- q: Quit (File must be saved if changes have been made)
	- q!: Force Quit. Don't even check if file has been saved
	- w/s: [W]rite/[S]ave changes
	- wq/sq: [W]rite and [Q]uit / [S]ave and [Q]uit.

	WHILE IN EDIT MODE:
	Escape: Go back to Read Mode

	MOVEMENT FUNCTIONALITY:
	- ArrowKey Left/Right: Move 1 character left/right within the file.
	- ArrowKey Up/Down: Move one row up/down within the file
	- CtrlArrow Left/Right: Move to the start/end of the previous word/next word
	- CtrlArrow Up/Down: Shift the current view offset by one up/down
	- Home/End: Go to start of/end of current row
	- CtrlHome/End: Go to start of / end of file
	- PageUp/Down: Shift screen position by 1 page/screen rows worth
	- CtrlPage Up/Down: Move cursor to start of/end of screen without adjusting screen position

	CHARACTER EDITING:
	- Letter/Number/Symbol: Insert at current cursor position and move cursor forward
	- Enter/Return: Insert a new row, moving contents beyond the cursor onto the new row and move cursor to start of new row
	- Backspace/Delete: Delete character behind/in front of cursor. Move cursor backwards if using backspace.
	- CtrlBackspace/Delete: Delete word

More key functionality will be added as this project progresses.

I will also try to add Undo/Redo functionality, but that will be at a much later date, as this project is still in the very early stages of development.

<hr>

### Building

A CMake Build script is provided. CMake 3.12, as well as a C++20 compliant compiler is required.

To build, run

	cmake -B {buildDir} -G {buildGenerator} -S {pathToCMakeScript} -DCMAKE_BUILD_TYPE=Release

	cmake --build {buildDir} --config Release

	EXAMPLE (While in root dir):

	cmake -B ./out -G Ninja -DCMAKE_BUILD_TYPE=Release

	cmake --build ./out --config Release

<hr>

### Usage

To use, navigate to the executable (either in {buildDir} or {buildDir}/bin most commonly). Then, run

	./nve <filename.fileExtension>

	OR IF USING COMMAND PROMPT

	nve <filename.fileExtension>

	EXAMPLE:

	./nve test.cpp

This executable is a standalone executable, so you may also add this file to your system path and use it from anywhere

<hr>

### Known Bugs

 - Cursor does not save rendered position, but rather position within the file, so cursor may appear to jump around when moving up/down
 - Screen randomly flickers (not sure why, but I am working on finding the culprit) -- May be fixed, needs more testing

<hr>

### Contributing

If you would like to contribute to this project, feel free to clone this repo, make a new branch and work on any changes you see necessary.
If you come across any issues, feel free to make your own fix for it, or create an issue so I am aware of it and can fix it.

#### UNTESTED ON OSX (macOS). 
 
From my understanding, this should still work on mac due to the Unix based nature of it, but it is untested.
