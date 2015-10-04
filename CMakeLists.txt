cmake_minimum_required (VERSION 2.8)
project(testrwt)
set(CMAKE_C_FLAGS "-std=c99")
add_executable(testrwt test/test.c test/lodepng.c rwt.c)
