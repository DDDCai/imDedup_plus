cmake_minimum_required(VERSION 3.10)
include_directories(./)
add_compile_options(-Wall -W -ggdb3 -std=gnu99 -O2)
add_library(
    rabin 
    rabin.c
    gear.c
    adler32.c
)