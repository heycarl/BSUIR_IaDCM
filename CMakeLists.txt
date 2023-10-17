cmake_minimum_required(VERSION 3.24)
project(IaPD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")


add_executable(IaPD LAB1/main.cpp LAB1/Battery.cpp LAB1/Battery.hpp)
target_link_libraries(IaPD PRIVATE iphlpapi setupapi PowrProf)

add_executable(LAB2 LAB2/main.cpp LAB2/functions.hpp)
#target_link_libraries(LAB2 PRIVATE "C:\\Users\\Alexandr\\Documents\\CODE\\BSUIR\\_GRADE3\\_SEMESTER5\\IaPD\\LAB2\\hexiosupp.lib")
#target_link_libraries(LAB2 ${CMAKE_CURRENT_SOURCE_DIR}libs\\hexiosupp.lib)
