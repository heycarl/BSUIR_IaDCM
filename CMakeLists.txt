cmake_minimum_required(VERSION 3.24)
project(IaPD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")


add_executable(IaPD LAB1/main.cpp LAB1/Battery.cpp LAB1/Battery.hpp LAB1/libraries.hpp)
target_link_libraries(IaPD PRIVATE iphlpapi setupapi PowrProf)