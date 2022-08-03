cmake_minimum_required(VERSION 3.2)
project(noodles VERSION 0.0)

add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:--std=c++17> $<$<COMPILE_LANGUAGE:C>:--std=c17>
    -pthread -g -fwrapv -Wall)
add_link_options(-pthread)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    message(STATUS "release mode")
    add_compile_options(-Werror)
else()
    message(STATUS "debug mode")
    add_compile_options(-O0 -fsanitize=address -fvar-tracking-assignments)
    add_link_options(-fsanitize=address)
endif()

add_subdirectory(3rd-parties)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(bench)
