cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

project(seamcarving)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(SDL2 REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${SDL2_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

add_executable(seamcarving src/main.cpp src/SeamCarver.hpp src/SeamCarver.cpp)

target_link_libraries(seamcarving ${SDL2_LIBRARY} ${OpenCV_LIBS}) 
