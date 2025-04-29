cmake_minimum_required(VERSION 3.10)
project(cgrunner)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(cgrunner
    src/main.cpp
    src/CgroupManager.cpp
    src/ProcessRunner.cpp
    src/ErrorUtils.cpp
)