add_executable(noodles_unittest
    noodles_unittest.cpp
)

target_link_libraries(noodles_unittest
    noodles_static
    testa
)
