cmake_minimum_required(VERSION 3.24)
project(IaPD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

find_package( OpenCV REQUIRED )
set(OpenCV_STATIC ON)
set(BUILD_SHARED_LIBS OFF)

# add resources file
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")

add_executable(IaPD LAB1/main.cpp LAB1/Battery.cpp LAB1/Battery.hpp)
target_link_libraries(IaPD PRIVATE iphlpapi setupapi PowrProf)

add_executable(LAB2 LAB2/main.cpp LAB2/functions.hpp)
add_executable(LAB3 LAB3/main.cpp)
add_executable(LAB4 ${APP_ICON_RESOURCE_WINDOWS} LAB4/main.cpp LAB4/camera.hpp)
target_include_directories(LAB4 PUBLIC ${OpenCV_INCLUDE_DIRS})

set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs)
target_link_libraries(LAB4 ${OpenCV_LIBS} iphlpapi setupapi)
#target_link_libraries(LAB2 PRIVATE "C:\\Users\\Alexandr\\Documents\\CODE\\BSUIR\\_GRADE3\\_SEMESTER5\\IaPD\\LAB2\\hexiosupp.lib")
#target_link_libraries(LAB2 ${CMAKE_CURRENT_SOURCE_DIR}libs\\hexiosupp.lib)
