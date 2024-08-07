# NotVim Editor

A custom, 0 dependencies, cross-platform VIM-like console-based text editor. Tested on Windows and Linux.
<hr>

### Demo

![NVE demo](https://github.com/user-attachments/assets/87a79a56-9151-4e79-af1d-8ef54c9c7c4a)

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

 - Arrow keys cannot be held to quickly navigate through the screen on Linux.
 - Cursor does not save rendered position, but rather position within the file, so cursor may appear to jump around when moving up/down
 - No syntax highlighting (WIP)


<hr>

#### UNTESTED ON OSX (macOS). 
 
From my understanding, this should still work on mac due to the Unix based nature of it, but it is untested.
