cmake_minimum_required(VERSION 3.10.0)
project(cpp-sqlite)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)

include_directories(include/**)

find_package (SQLite3 REQUIRED)
if (SQLite3_FOUND)
    message(STATUS "Include dirs for sqlite3: ${SQLite3_LIBRARIES} ${SQLite3_INCLUDE_DIRS}")
    include_directories(${SQLite3_INCLUDE_DIRS})
endif (SQLite3_FOUND)
add_subdirectory(src)
