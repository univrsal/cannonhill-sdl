cmake_minimum_required(VERSION 3.26)

project(cannonhill LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(cannonhill menue.cpp panzer.cpp panzer.h)

# find sdl2
find_package(SDL2 REQUIRED)
target_link_libraries(cannonhill SDL2::SDL2)