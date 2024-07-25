# CMakeList.txt : CMake project for TextEditor, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("TextEditor")

set(SOURCES
	src/TextEditor.cpp
	src/File/File.cpp
	src/SyntaxHighlight/SyntaxHighlight.cpp
	src/main.cpp
)

set (HEADERS
	src/TextEditor.hpp
	src/File/File.hpp
	src/SyntaxHighlight/SyntaxHighlight.hpp
)
#find_package(OpenGL REQUIRED)

#[[find_package(GLFW)
if(NOT GLFW_FOUND)
	include(FetchContent)
	FetchContent_Declare(
		glfw
		GIT_REPOSITORY https://github.com/glfw/glfw
		GIT_TAG 3.4
	)

	FetchContent_MakeAvailable(glfw)
endif(NOT GLFW_FOUND)
]]
# Add source to this project's executable.
add_executable (TextEditor ${SOURCES} ${HEADERS})
#target_link_libraries(TextEditor PRIVATE glfw OpenGL::GL)
#target_include_directories(TextEditor PRIVATE lib/imgui)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TextEditor PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
