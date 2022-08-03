add_executable(noodles_bench
    noodles_bench.cpp
)

target_link_libraries(noodles_bench
PUBLIC
    noodles_static
)
