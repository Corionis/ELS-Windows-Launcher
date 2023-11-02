cmake_minimum_required(VERSION 3.26)
project(ELS_Win_Launch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-mwindows -Wl,--subsystem,windows -static")

add_executable(ELS-Navigator main.cpp icon.rc
        version.rc)
