cmake_minimum_required(VERSION 2.8.8)
project(parg)

include_directories(
    src/vendor
)

add_executable(
    kopf
    src/use_tiles.cpp
    src/util.cpp
)
