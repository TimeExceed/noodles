cmake_minimum_required(VERSION 3.0)

add_library(noodles_static STATIC
    noodles.cpp
    semaphore.cpp
)

target_link_libraries(noodles_static
PUBLIC
    fassert
    prettyprint
)

target_include_directories(noodles_static
PUBLIC
    include/
)

