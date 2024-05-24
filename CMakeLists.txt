cmake_minimum_required(VERSION 3.10)

project(file-synchronizer)

set(CMAKE_C_STANDARD 23)

include_directories(include)

file(GLOB SRC_FILES "src/*.c")

add_executable(file-synchronizer ${SRC_FILES})

target_link_libraries(file-synchronizer pthread)