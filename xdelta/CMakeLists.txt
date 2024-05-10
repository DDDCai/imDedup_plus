cmake_minimum_required(VERSION 3.10)
project(xdelta3 C)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_compile_options(-O3)
link_libraries(lzma)
add_library(xdelta xdelta3.c)
# add_executable(main xdelta-demo.c xdelta3.c)
