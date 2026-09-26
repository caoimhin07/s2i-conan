project(HTTPTimeServer)
cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(server src/HTTPTimeServer.cpp)
target_link_libraries(server pthread)
