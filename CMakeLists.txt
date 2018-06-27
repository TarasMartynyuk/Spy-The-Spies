cmake_minimum_required(VERSION 3.5)
project(Spy_The_Spies)

set(CMAKE_CXX_FLAGS -pg)

set (link_flags "-lprofiler")
add_definitions(${link_flags})

set(CMAKE_CXX_STANDARD 11)

add_executable(Spy_The_Spies
        spy_the_spies.cpp stl_overloads.h)

target_compile_options(Spy_The_Spies PRIVATE -lprofiler)
target_link_libraries(Spy_The_Spies profiler)
