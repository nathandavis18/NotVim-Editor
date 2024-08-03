cmake_minimum_required (VERSION 3.10)

project ("TextEditor")

set(SOURCES
	"src/Input/Input.cpp"
	src/File/File.cpp
	src/SyntaxHighlight/SyntaxHighlight.cpp
	src/Console/Console.cpp
	src/main.cpp
)

set (HEADERS
	src/File/File.hpp
	src/SyntaxHighlight/SyntaxHighlight.hpp
	src/Console/Console.hpp
	"src/Input/Input.hpp"
)

add_executable (TextEditor ${SOURCES} ${HEADERS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TextEditor PROPERTY CXX_STANDARD 20)
endif()

