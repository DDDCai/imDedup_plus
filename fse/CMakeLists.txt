cmake_minimum_required(VERSION 3.10)
include_directories(./lib)
add_compile_options(-O3 -Wall -Wextra -Wcast-qual -Wcast-align -Wshadow -Wstrict-aliasing=1 -Wswitch-enum -Wstrict-prototypes -Wundef)
add_library(
    fse 
    # ./programs/fileio.c ./programs/xxhash.c ./programs/zlibh.c 
    ./lib/entropy_common.c ./lib/hist.c ./lib/fse_decompress.c ./lib/fse_compress.c ./lib/fseU16.c ./lib/huf_compress.c ./lib/huf_decompress.c
)
