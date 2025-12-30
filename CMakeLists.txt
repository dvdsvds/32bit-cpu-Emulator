cmake_minimum_required(VERSION 3.20)
project(CPU LANGUAGES CXX)

add_executable(cpu
    src/decoder.cpp
    src/memory.cpp
    src/cpu.cpp
    src/dump.cpp
    src/encoder.cpp
    src/pipeline.cpp
    src/predictor.cpp
    test/test_cpu.cpp
)

target_include_directories(cpu PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(cpu PROPERTIES 
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

