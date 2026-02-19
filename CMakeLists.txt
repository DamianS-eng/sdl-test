cmake_minimum_required(VERSION 3.20)
project(demo C)

find_package(SDL3 REQUIRED)

add_executable(demo demo.c)
target_link_libraries(demo PRIVATE SDL3::SDL3)
