cmake_minimum_required(VERSION 3.15)

project(reflstruct LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)


find_package(doctest REQUIRED)

add_executable(test 
    test_main.cpp
    envconfig_test.cpp
    reflstruct_test.cpp
)

target_link_libraries(test PRIVATE doctest::doctest)
