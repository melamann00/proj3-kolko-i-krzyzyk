cmake_minimum_required(VERSION 3.14)
project(KolkoKrzyzyk)
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

add_executable(kolko_krzyzyk proj3.cpp)
target_include_directories(kolko_krzyzyk PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(kolko_krzyzyk ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})
