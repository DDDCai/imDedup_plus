cmake_minimum_required(VERSION 3.10)

project(idedup)

# include_directories(./idelta)
# include_directories(./image)
include_directories(./rabin)
include_directories(./fse)
include_directories(./jpeglib/include)
# include_directories(./2dsf)
include_directories(.)

link_directories(./jpeglib/lib/)
link_libraries(jpeg)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})
add_definitions(${GLIB2_CFLAGS_OTHER})

add_subdirectory(xdelta)
add_subdirectory(rabin)
add_subdirectory(fse)

aux_source_directory(. CODES)
# aux_source_directory(idelta CODES)
# aux_source_directory(2dsf CODES)
# aux_source_directory(image CODES)
add_compile_options(
    -O3 
    -g
)

add_executable(sid ${CODES})
target_link_libraries(sid ${GLIB2_LIBRARIES} rabin fse pthread jpeg xdelta)