add_executable(test_connect test_connect.cpp)
target_include_directories(test_connect PRIVATE ${cpp-sqlite_SOURCE_DIR}/include/)
target_link_libraries(test_connect ${SQLite3_LIBRARIES})