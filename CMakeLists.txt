cmake_minimum_required (VERSION 3.12)

project ("nve")

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

add_executable (nve ${SOURCES} ${HEADERS})

 set_property(TARGET nve PROPERTY CXX_STANDARD 20)

