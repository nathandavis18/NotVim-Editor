cmake_minimum_required (VERSION 3.10)

project ("TextEditor")

set(SOURCES
	src/TextEditor.cpp
	src/File/File.cpp
	src/SyntaxHighlight/SyntaxHighlight.cpp
	src/Console/Console.cpp
	src/main.cpp
)

set (HEADERS
	src/TextEditor.hpp
	src/File/File.hpp
	src/SyntaxHighlight/SyntaxHighlight.hpp
	src/Console/Console.hpp
)

add_executable (TextEditor ${SOURCES} ${HEADERS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TextEditor PROPERTY CXX_STANDARD 20)
endif()

