cmake_minimum_required(VERSION 3.22)

project(GBpp)

find_package(OpenGL REQUIRED)

find_package(GLUT REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} )

add_executable(gbpp
    src/main.cpp
    src/core/cpu.cpp
    src/core/mmu.cpp
    src/core/iocontroller.cpp
    src/memory/ram.cpp
    src/memory/hram.cpp
    src/memory/cartridgerom.cpp
    src/video/display.cpp
    src/video/vram.cpp
    src/gui/displaygui.cpp
)

target_link_libraries(gbpp ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )

