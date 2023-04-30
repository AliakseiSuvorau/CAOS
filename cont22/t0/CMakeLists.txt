cmake_minimum_required(VERSION 3.25)
project(t0 C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE REQUIRED fuse3)

link_libraries(${FUSE_LIBRARIES})         # -lfuse3 -lpthread
include_directories(${FUSE_INCLUDE_DIRS}) # -I/usr/include/fuse3

add_executable(t0 main.c)
