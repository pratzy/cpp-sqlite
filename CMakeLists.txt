cmake_minimum_required(VERSION 3.10.0)
project(cpp-sqlite)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "Project source directory is ${PROJECT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)

include_directories(include/**)

find_package(SQLite3 REQUIRED)
if (SQLite3_FOUND)
    message(STATUS "Include dirs for sqlite3: ${SQLite3_LIBRARIES} ${SQLite3_INCLUDE_DIRS}")
    include_directories(${SQLite3_INCLUDE_DIRS})
endif (SQLite3_FOUND)

#TODO - Make a static library
set(LIB_NAME cppsqlite)
add_library(${LIB_NAME} INTERFACE)
target_sources(${LIB_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/include/**)
target_include_directories(${LIB_NAME} INTERFACE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(${LIB_NAME} INTERFACE ${SQLite3_LIBRARIES})

add_subdirectory(src)
