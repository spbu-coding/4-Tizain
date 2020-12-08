cmake_minimum_required(VERSION 3.17)
project(converter C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(converter src/converter.c src/negative.c src/qdbmp.c)