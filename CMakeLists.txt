cmake_minimum_required(VERSION 3.22)

project(GBpp)

add_executable(gbpp
    src/main.cpp
    src/core/cpu.cpp
    src/core/mmu.cpp
    src/memory/ram.cpp
    src/memory/cartridgerom.cpp
)

